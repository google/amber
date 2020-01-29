// Copyright 2020 The Amber Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "src/vulkan/engine_vulkan.h"

#if AMBER_ENABLE_VK_DEBUGGING

#include <chrono>              // NOLINT(build/c++11)
#include <condition_variable>  // NOLINT(build/c++11)
#include <fstream>
#include <mutex>  // NOLINT(build/c++11)
#include <sstream>
#include <thread>  // NOLINT(build/c++11)
#include <unordered_map>

#include "dap/network.h"
#include "dap/protocol.h"
#include "dap/session.h"

// Set to 1 to enable verbose debugger logging
#define ENABLE_DEBUGGER_LOG 0

#if ENABLE_DEBUGGER_LOG
#define DEBUGGER_LOG(...) \
  do {                    \
    printf(__VA_ARGS__);  \
    printf("\n");         \
  } while (false)
#else
#define DEBUGGER_LOG(...)
#endif

namespace amber {
namespace vulkan {

namespace {

static constexpr auto kThreadTimeout = std::chrono::minutes(1);

// Event provides a basic wait-and-signal synchronization primitive.
class Event {
 public:
  // Wait blocks until the event is fired.
  void Wait() {
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [&] { return signalled_; });
  }

  // Wait blocks until the event is fired, or the timeout is reached.
  // If the Event was signalled, then Wait returns true, otherwise false.
  template <typename Rep, typename Period>
  bool Wait(const std::chrono::duration<Rep, Period>& duration) {
    std::unique_lock<std::mutex> lock(mutex_);
    return cv_.wait_for(lock, duration, [&] { return signalled_; });
  }

  // Signal signals the Event, unblocking any calls to Wait.
  void Signal() {
    std::unique_lock<std::mutex> lock(mutex_);
    signalled_ = true;
    cv_.notify_all();
  }

 private:
  std::condition_variable cv_;
  std::mutex mutex_;
  bool signalled_ = false;
};

// Split slices str into all substrings separated by sep and returns a vector of
// the substrings between those separators.
std::vector<std::string> Split(const std::string& str, const std::string& sep) {
  std::vector<std::string> out;
  std::size_t cur = 0;
  std::size_t prev = 0;
  while ((cur = str.find(sep, prev)) != std::string::npos) {
    out.push_back(str.substr(prev, cur - prev));
    prev = cur + 1;
  }
  out.push_back(str.substr(prev));
  return out;
}

// Forward declaration.
struct Variable;

// Variables is a list of Variable (), with helper methods.
class Variables : public std::vector<Variable> {
 public:
  inline const Variable* Find(const std::string& name) const;
  inline std::string AllNames() const;
};

// Variable holds a debugger returned named value (local, global, etc).
// Variables can hold child variables (for structs, arrays, etc).
struct Variable {
  std::string name;
  std::string value;
  Variables children;

  // Get parses the Variable value for the requested type, assigning the result
  // to |out|. Returns true on success, otherwise false.
  bool Get(int* out) const {
    *out = std::atoi(value.c_str());
    return true;  // TODO(bclayton): Verify the value parsed correctly.
  }

  bool Get(uint32_t* out) const {
    *out = static_cast<uint32_t>(std::atoi(value.c_str()));
    return true;  // TODO(bclayton): Verify the value parsed correctly.
  }

  bool Get(int64_t* out) const {
    *out = static_cast<int64_t>(std::atoi(value.c_str()));
    return true;  // TODO(bclayton): Verify the value parsed correctly.
  }

  bool Get(float* out) const {
    *out = std::atof(value.c_str());
    return true;  // TODO(bclayton): Verify the value parsed correctly.
  }

  bool Get(double* out) const {
    *out = std::atof(value.c_str());
    return true;  // TODO(bclayton): Verify the value parsed correctly.
  }

  bool Get(std::string* out) const {
    *out = value;
    return true;
  }

  template <typename T>
  bool Get(std::tuple<T, T, T>* out) const {
    auto x = children.Find("x");
    auto y = children.Find("y");
    auto z = children.Find("z");
    if (x != nullptr && y != nullptr && z != nullptr) {
      T elX;
      T elY;
      T elZ;
      if (x->Get(&elX) && y->Get(&elY) && z->Get(&elZ)) {
        *out = std::tuple<T, T, T>{elX, elY, elZ};
        return true;
      }
    }
    return false;
  }
};

const Variable* Variables::Find(const std::string& name) const {
  for (auto& child : *this) {
    if (child.name == name) {
      return &child;
    }
  }
  return nullptr;
}

std::string Variables::AllNames() const {
  std::string out;
  for (auto& var : *this) {
    if (out.size() > 0) {
      out += ", ";
    }
    out += "'" + var.name + "'";
  }
  return out;
}

// Client wraps a dap::Session and a error handler, and provides a more
// convenient interface for talking to the debugger. Client also provides basic
// immutable data caching to help performance.
class Client {
  static constexpr const char* kLocals = "locals";
  static constexpr const char* kLane = "Lane";

 public:
  using ErrorHandler = std::function<void(const std::string&)>;
  using SourceLines = std::vector<std::string>;

  Client(const std::shared_ptr<dap::Session>& session,
         const ErrorHandler& onerror)
      : session_(session), onerror_(onerror) {}

  // TopStackFrame retrieves the frame at the top of the thread's call stack.
  // Returns true on success, false on error.
  bool TopStackFrame(dap::integer threadId, dap::StackFrame* frame) {
    dap::StackTraceRequest request;
    request.threadId = threadId;
    auto response = session_->send(request).get();
    if (response.error) {
      onerror_(response.error.message);
      return false;
    }
    if (response.response.stackFrames.size() == 0) {
      onerror_("Stack frame is empty");
      return false;
    }
    *frame = response.response.stackFrames.front();
    return true;
  }

  // FrameLocation retrieves the current frame source location, and optional
  // source line text.
  // Returns true on success, false on error.
  bool FrameLocation(const dap::StackFrame& frame,
                     debug::Location* location,
                     std::string* line = nullptr) {
    location->line = frame.line;

    if (!frame.source.has_value()) {
      onerror_("Stack frame with name '" + frame.name + "' has no source");
      return false;
    } else if (frame.source->path.has_value()) {
      location->file = frame.source.value().path.value();
    } else if (frame.source->name.has_value()) {
      location->file = frame.source.value().name.value();
    } else {
      onerror_("Frame source had no path or name");
      return false;
    }

    if (location->line < 1) {
      onerror_("Line location is " + std::to_string(location->line));
      return false;
    }

    if (line != nullptr) {
      SourceLines lines;
      if (!SourceContent(frame.source.value(), &lines)) {
        return false;
      }
      if (location->line > lines.size()) {
        onerror_("Line " + std::to_string(location->line) +
                 " is greater than the number of lines in the source file (" +
                 std::to_string(lines.size()) + ")");
      }
      *line = lines[location->line - 1];
    }

    return true;
  }

  // SourceContext retrieves the the SourceLines for the given source.
  // Returns true on success, false on error.
  bool SourceContent(const dap::Source& source, SourceLines* out) {
    auto path = source.path.value("");
    if (path != "") {
      auto it = sourceCache_.by_path.find(path);
      if (it != sourceCache_.by_path.end()) {
        *out = it->second;
        return true;
      }

      // TODO(bclayton) - We shouldn't be doing direct file IO here. We should
      // bubble the IO request to the amber 'embedder'.
      // See: https://github.com/google/amber/issues/777
      std::ifstream file(path);
      if (!file) {
        onerror_("Could not open source file '" + path + '"');
        return false;
      }

      SourceLines lines;
      std::string line;
      while (std::getline(file, line)) {
        lines.emplace_back(line);
      }

      sourceCache_.by_path.emplace(path, lines);
      *out = lines;
      return true;
    }

    if (source.sourceReference.has_value()) {
      auto ref = source.sourceReference.value();
      auto it = sourceCache_.by_ref.find(ref);
      if (it != sourceCache_.by_ref.end()) {
        *out = it->second;
        return true;
      }

      dap::SourceRequest request;
      dap::SourceResponse response;
      request.sourceReference = ref;
      if (!Send(request, &response)) {
        return false;
      }
      auto lines = Split(response.content, "\n");
      sourceCache_.by_ref.emplace(ref, lines);
      *out = lines;
      return true;
    }

    onerror_("Could not get source content");
    return false;
  }

  // Send sends the request to the debugger, waits for the request to complete,
  // and then assigns the response to |res|.
  // Returns true on success, false on error.
  template <typename REQUEST, typename RESPONSE>
  bool Send(const REQUEST& request, RESPONSE* res) {
    auto r = session_->send(request).get();
    if (r.error) {
      onerror_(r.error.message);
      return false;
    }
    *res = r.response;
    return true;
  }

  // Send sends the request to the debugger, and waits for the request to
  // complete.
  // Returns true on success, false on error.
  template <typename REQUEST>
  bool Send(const REQUEST& request) {
    using RESPONSE = typename REQUEST::Response;
    RESPONSE response;
    return Send(request, &response);
  }

  // GetVariables fetches the fully traversed set of Variables from the debugger
  // for the given reference identifier.
  // Returns true on success, false on error.
  bool GetVariables(dap::integer variablesRef, Variables* out) {
    dap::VariablesRequest request;
    dap::VariablesResponse response;
    request.variablesReference = variablesRef;
    if (!Send(request, &response)) {
      return false;
    }
    for (auto var : response.variables) {
      Variable v;
      v.name = var.name;
      v.value = var.value;
      if (var.variablesReference > 0) {
        if (!GetVariables(var.variablesReference, &v.children)) {
          return false;
        }
      }
      out->emplace_back(v);
    }
    return true;
  }

  // GetLocals fetches the fully traversed set of local Variables from the
  // debugger for the given stack frame.
  // Returns true on success, false on error.
  bool GetLocals(const dap::StackFrame& frame, Variables* out) {
    dap::ScopesRequest scopeReq;
    dap::ScopesResponse scopeRes;
    scopeReq.frameId = frame.id;
    if (!Send(scopeReq, &scopeRes)) {
      return false;
    }

    for (auto scope : scopeRes.scopes) {
      if (scope.presentationHint.value("") == kLocals) {
        return GetVariables(scope.variablesReference, out);
      }
    }

    onerror_("Locals scope not found");
    return false;
  }

  // GetLane returns a pointer to the Variables representing the thread's SIMD
  // lane with the given index, or nullptr if the lane was not found.
  const Variables* GetLane(const Variables& lanes, int lane) {
    auto out = lanes.Find(std::string(kLane) + " " + std::to_string(lane));
    if (out == nullptr) {
      return nullptr;
    }
    return &out->children;
  }

 private:
  struct SourceCache {
    std::unordered_map<int, SourceLines> by_ref;
    std::unordered_map<std::string, SourceLines> by_path;
  };

  std::shared_ptr<dap::Session> session_;
  ErrorHandler onerror_;
  SourceCache sourceCache_;
};

// GlobalInvocationId holds a three-element unsigned integer index, used to
// identifiy a single compute invocation.
struct GlobalInvocationId {
  size_t hash() const { return x << 20 | y << 10 | z; }
  bool operator==(const GlobalInvocationId& other) const {
    return x == other.x && y == other.y && z == other.z;
  }

  uint32_t x;
  uint32_t y;
  uint32_t z;
};

// InvocationKey is a tagged-union structure that identifies a single shader
// invocation.
struct InvocationKey {
  // Hash is a custom hasher that can enable InvocationKeys to be used as keys
  // in std containers.
  struct Hash {
    size_t operator()(const InvocationKey& key) const;
  };

  enum class Type { kGlobalInvocationId, kVertexIndex };
  union Data {
    GlobalInvocationId globalInvocationId;
    uint32_t vertexId;
  };

  explicit InvocationKey(const GlobalInvocationId& id);
  InvocationKey(Type, const Data&);

  bool operator==(const InvocationKey& other) const;

  // String returns a human-readable description of the key.
  std::string String() const;

  Type type;
  Data data;
};

size_t InvocationKey::Hash::operator()(const InvocationKey& key) const {
  size_t hash = 31 * static_cast<size_t>(key.type);
  switch (key.type) {
    case Type::kGlobalInvocationId:
      hash += key.data.globalInvocationId.hash();
      break;
    case Type::kVertexIndex:
      hash += key.data.vertexId;
      break;
  }
  return hash;
}

InvocationKey::InvocationKey(const GlobalInvocationId& id)
    : type(Type::kGlobalInvocationId) {
  data.globalInvocationId = id;
}

InvocationKey::InvocationKey(Type type, const Data& data)
    : type(type), data(data) {}

std::string InvocationKey::String() const {
  std::stringstream ss;
  switch (type) {
    case Type::kGlobalInvocationId: {
      auto& id = data.globalInvocationId;
      ss << "GlobalInvocation(" << id.x << ", " << id.y << ", " << id.z << ")";
      break;
    }
    case Type::kVertexIndex:
      ss << "VertexIndex(" << data.vertexId << ")";
      break;
  }
  return ss.str();
}

bool InvocationKey::operator==(const InvocationKey& other) const {
  if (type != other.type) {
    return false;
  }
  switch (type) {
    case Type::kGlobalInvocationId:
      return data.globalInvocationId == other.data.globalInvocationId;
    case Type::kVertexIndex:
      return data.vertexId == other.data.vertexId;
  }
  return false;
}

// Thread controls and verifies a single debugger thread of execution.
class Thread : public debug::Thread {
  using Result = Result;

 public:
  Thread(std::shared_ptr<dap::Session> session,
         int threadId,
         int lane,
         const debug::Events::OnThread& callback)
      : threadId_(threadId),
        lane_(lane),
        client_(session, [this](const std::string& err) { OnError(err); }) {
    // The thread script runs concurrently with other debugger thread scripts.
    // Run on a separate amber thread.
    thread_ = std::thread([this, callback] {
      callback(this);  // Begin running the thread script.
      done_.Signal();  // Signal when done.
    });
  }

  ~Thread() { Flush(); }

  // Flush waits for the debugger thread script to complete, and returns any
  // errors encountered.
  Result Flush() {
    if (done_.Wait(kThreadTimeout)) {
      if (thread_.joinable()) {
        thread_.join();
      }
    } else {
      error_ += "Timed out performing actions";
    }
    return error_;
  }

  // debug::Thread compliance
  void StepOver() override {
    DEBUGGER_LOG("StepOver()");
    if (error_.IsSuccess()) {
      dap::NextRequest request;
      request.threadId = threadId_;
      client_.Send(request);
    }
  }

  void StepIn() override {
    DEBUGGER_LOG("StepIn()");
    if (error_.IsSuccess()) {
      dap::StepInRequest request;
      request.threadId = threadId_;
      client_.Send(request);
    }
  }

  void StepOut() override {
    DEBUGGER_LOG("StepOut()");
    if (error_.IsSuccess()) {
      dap::StepOutRequest request;
      request.threadId = threadId_;
      client_.Send(request);
    }
  }

  void Continue() override {
    DEBUGGER_LOG("Continue()");
    if (error_.IsSuccess()) {
      dap::ContinueRequest request;
      request.threadId = threadId_;
      client_.Send(request);
    }
  }

  void ExpectLocation(const debug::Location& location,
                      const std::string& line) override {
    DEBUGGER_LOG("ExpectLocation('%s', %d)", location.file.c_str(),
                 location.line);

    dap::StackFrame frame;
    if (!client_.TopStackFrame(threadId_, &frame)) {
      return;
    }

    debug::Location got_location;
    std::string got_source_line;
    if (!client_.FrameLocation(frame, &got_location, &got_source_line)) {
      return;
    }

    if (got_location.file != location.file) {
      OnError("Expected file to be '" + location.file + "' but file was " +
              got_location.file);
    } else if (got_location.line != location.line) {
      OnError("Expected line number to be " + std::to_string(location.line) +
              " but line number was " + std::to_string(got_location.line));
    } else if (line != "" && got_source_line != line) {
      OnError("Expected source line to be:\n  " + line + "\nbut line was:\n  " +
              got_source_line);
    }
  }

  void ExpectLocal(const std::string& name, int64_t value) override {
    DEBUGGER_LOG("ExpectLocal('%s', %d)", name.c_str(), (int)value);
    ExpectLocalT(name, value);
  }

  void ExpectLocal(const std::string& name, double value) override {
    DEBUGGER_LOG("ExpectLocal('%s', %f)", name.c_str(), value);
    ExpectLocalT(name, value);
  }

  void ExpectLocal(const std::string& name, const std::string& value) override {
    DEBUGGER_LOG("ExpectLocal('%s', '%s')", name.c_str(), value.c_str());
    ExpectLocalT(name, value);
  }

  template <typename T>
  void ExpectLocalT(const std::string& name, const T& expect) {
    dap::StackFrame frame;
    if (!client_.TopStackFrame(threadId_, &frame)) {
      return;
    }

    Variables locals;
    if (!client_.GetLocals(frame, &locals)) {
      return;
    }

    if (auto lane = client_.GetLane(locals, lane_)) {
      auto owner = lane;
      const Variable* var = nullptr;
      std::string path;
      for (auto part : Split(name, ".")) {
        var = owner->Find(part);
        if (!var) {
          if (owner == lane) {
            OnError("Local '" + name + "' not found\nAll Locals: " +
                    lane->AllNames() + ".\nLanes: " + locals.AllNames() + ".");
          } else {
            OnError("Local '" + path + "' does not contain '" + part +
                    "'\nChildren: " + owner->AllNames());
          }
          return;
        }
        owner = &var->children;
        path += (path.size() > 0) ? "." + part : part;
      }

      T got = {};
      if (!var->Get(&got)) {
        OnError("Local '" + name + "' was not of expected type");
        return;
      }

      if (got != expect) {
        std::stringstream ss;
        ss << "Local '" << name << "' did not have expected value. Value is '"
           << got << "', expected '" << expect << "'";
        OnError(ss.str());
        return;
      }
    }
  }

 private:
  void OnError(const std::string& err) {
    DEBUGGER_LOG("ERROR: %s", err.c_str());
    error_ += err;
  }

  const dap::integer threadId_;
  const int lane_;
  Client client_;
  std::thread thread_;
  Event done_;
  Result error_;
};

}  // namespace

// EngineVulkan::VkDebugger is a private implementation of the Engine::Debugger
// interface.
class EngineVulkan::VkDebugger : public Engine::Debugger {
  static constexpr const char* kComputeShaderFunctionName = "ComputeShader";
  static constexpr const char* kVertexShaderFunctionName = "VertexShader";
  static constexpr const char* kGlobalInvocationId = "globalInvocationId";
  static constexpr const char* kVertexIndex = "vertexIndex";

 public:
  /// Connect establishes the connection to the shader debugger. Must be
  /// called before any of the |debug::Events| methods.
  Result Connect() {
    constexpr int kMaxAttempts = 10;
    // The socket might take a while to open - retry connecting.
    for (int attempt = 0; attempt < kMaxAttempts; attempt++) {
      auto connection = dap::net::connect("localhost", 19020);
      if (!connection) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        continue;
      }

      // Socket opened. Create the debugger session and bind.
      session_ = dap::Session::create();
      session_->bind(connection);

      // Register the thread stopped event.
      // This is fired when breakpoints are hit (amongst other reasons).
      // See:
      // https://microsoft.github.io/debug-adapter-protocol/specification#Events_Stopped
      session_->registerHandler([&](const dap::StoppedEvent& event) {
        DEBUGGER_LOG("THREAD STOPPED. Reason: %s", event.reason.c_str());
        if (event.reason == "function breakpoint") {
          OnBreakpointHit(event.threadId.value(0));
        }
      });

      // Start the debugger initialization sequence.
      // See: https://microsoft.github.io/debug-adapter-protocol/overview for
      // details.

      dap::InitializeRequest init_req = {};
      auto init_res = session_->send(init_req).get();
      if (init_res.error) {
        DEBUGGER_LOG("InitializeRequest failed: %s",
                     init_res.error.message.c_str());
        return Result(init_res.error.message);
      }

      // Set breakpoints on the various shader types, we do this even if we
      // don't actually care about these threads. Once the breakpoint is hit,
      // the pendingThreads_ map is probed, if nothing matches the thread is
      // resumed.
      // TODO(bclayton): Once we have conditional breakpoint support, we can
      // reduce the number of breakpoints / scope of breakpoints.
      dap::SetFunctionBreakpointsRequest fbp_req = {};
      dap::FunctionBreakpoint fbp = {};
      fbp.name = kComputeShaderFunctionName;
      fbp_req.breakpoints.emplace_back(fbp);
      fbp.name = kVertexShaderFunctionName;
      fbp_req.breakpoints.emplace_back(fbp);
      auto fbp_res = session_->send(fbp_req).get();
      if (fbp_res.error) {
        DEBUGGER_LOG("SetFunctionBreakpointsRequest failed: %s",
                     fbp_res.error.message.c_str());
        return Result(fbp_res.error.message);
      }

      // ConfigurationDone signals the initialization has completed.
      dap::ConfigurationDoneRequest cfg_req = {};
      auto cfg_res = session_->send(cfg_req).get();
      if (cfg_res.error) {
        DEBUGGER_LOG("ConfigurationDoneRequest failed: %s",
                     cfg_res.error.message.c_str());
        return Result(cfg_res.error.message);
      }

      return Result();
    }
    return Result("Unable to connect to debugger");
  }

  // Flush checks that all breakpoints were hit, waits for all threads to
  // complete, and returns the globbed together results for all threads.
  Result Flush() override {
    Result result;
    {
      std::unique_lock<std::mutex> lock(error_mutex_);
      result += error_;
    }
    {
      std::unique_lock<std::mutex> lock(threads_mutex_);
      for (auto& pending : pendingThreads_) {
        result += "Thread did not run: " + pending.first.String();
      }
      for (auto& thread : runningThreads_) {
        result += thread->Flush();
      }
      runningThreads_.clear();
    }
    return result;
  }

  // debug::Events compliance
  void BreakOnComputeGlobalInvocation(
      uint32_t x,
      uint32_t y,
      uint32_t z,
      const debug::Events::OnThread& callback) override {
    std::unique_lock<std::mutex> lock(threads_mutex_);
    pendingThreads_.emplace(GlobalInvocationId{x, y, z}, callback);
  };

  void BreakOnVertexIndex(uint32_t index, const OnThread& callback) override {
    InvocationKey::Data data;
    data.vertexId = index;
    auto key = InvocationKey{InvocationKey::Type::kVertexIndex, data};
    std::unique_lock<std::mutex> lock(threads_mutex_);
    pendingThreads_.emplace(key, callback);
  }

 private:
  // OnBreakpointHit is called when a debugger breakpoint is hit (breakpoints
  // are set at shader entry points). pendingThreads_ is checked to see if this
  // thread needs testing, and if so, creates a new ::Thread.
  // If there's no pendingThread_ entry for the given thread, it is resumed to
  // allow the shader to continue executing.
  void OnBreakpointHit(dap::integer threadId) {
    DEBUGGER_LOG("Breakpoint hit: thread %d", (int)threadId);
    Client client(session_, [this](const std::string& err) { OnError(err); });

    std::unique_lock<std::mutex> lock(threads_mutex_);
    for (auto it = pendingThreads_.begin(); it != pendingThreads_.end(); it++) {
      auto& key = it->first;
      auto& callback = it->second;
      switch (key.type) {
        case InvocationKey::Type::kGlobalInvocationId: {
          auto invocation_id = key.data.globalInvocationId;
          int lane;
          if (FindGlobalInvocationId(threadId, invocation_id, &lane)) {
            DEBUGGER_LOG("Breakpoint hit: GetGlobalInvocationId: [%d, %d, %d]",
                         invocation_id.x, invocation_id.y, invocation_id.z);
            auto thread =
                MakeUnique<Thread>(session_, threadId, lane, callback);
            runningThreads_.emplace_back(std::move(thread));
            pendingThreads_.erase(it);
            return;
          }
          break;
        }
        case InvocationKey::Type::kVertexIndex: {
          auto vertex_id = key.data.vertexId;
          int lane;
          if (FindVertexIndex(threadId, vertex_id, &lane)) {
            DEBUGGER_LOG("Breakpoint hit: VertexId: %d", vertex_id);
            auto thread =
                MakeUnique<Thread>(session_, threadId, lane, callback);
            runningThreads_.emplace_back(std::move(thread));
            pendingThreads_.erase(it);
            return;
          }
          break;
        }
      }
    }

    // No pending tests for this thread. Let it carry on...
    dap::ContinueRequest request;
    request.threadId = threadId;
    client.Send(request);
  }

  // FindGlobalInvocationId looks for the compute shader's global invocation id
  // in the stack frames' locals, returning true if found, and assigns the index
  // of the SIMD lane it was found in to |lane|.
  // TODO(bclayton): This value should probably be in the globals, not locals!
  bool FindGlobalInvocationId(dap::integer threadId,
                              const GlobalInvocationId& id,
                              int* lane) {
    Client client(session_, [this](const std::string& err) { OnError(err); });

    dap::StackFrame frame;
    if (!client.TopStackFrame(threadId, &frame)) {
      return false;
    }

    dap::ScopesRequest scopeReq;
    dap::ScopesResponse scopeRes;
    scopeReq.frameId = frame.id;
    if (!client.Send(scopeReq, &scopeRes)) {
      return false;
    }

    Variables locals;
    if (!client.GetLocals(frame, &locals)) {
      return false;
    }

    for (int i = 0;; i++) {
      auto lane_var = client.GetLane(locals, i);
      if (!lane_var) {
        break;
      }
      if (auto var = lane_var->Find(kGlobalInvocationId)) {
        std::tuple<uint32_t, uint32_t, uint32_t> got;
        if (var->Get(&got)) {
          uint32_t x;
          uint32_t y;
          uint32_t z;
          std::tie(x, y, z) = got;
          if (x == id.x && y == id.y && z == id.z) {
            *lane = i;
            return true;
          }
        }
      }
    }

    return false;
  }

  // FindVertexIndex looks for the requested vertex shader's vertex index in the
  // stack frames' locals, returning true if found, and assigns the index of the
  // SIMD lane it was found in to |lane|.
  // TODO(bclayton): This value should probably be in the globals, not locals!
  bool FindVertexIndex(dap::integer threadId, uint32_t index, int* lane) {
    Client client(session_, [this](const std::string& err) { OnError(err); });

    dap::StackFrame frame;
    if (!client.TopStackFrame(threadId, &frame)) {
      return false;
    }

    dap::ScopesRequest scopeReq;
    dap::ScopesResponse scopeRes;
    scopeReq.frameId = frame.id;
    if (!client.Send(scopeReq, &scopeRes)) {
      return false;
    }

    Variables locals;
    if (!client.GetLocals(frame, &locals)) {
      return false;
    }

    for (int i = 0;; i++) {
      auto lane_var = client.GetLane(locals, i);
      if (!lane_var) {
        break;
      }
      if (auto var = lane_var->Find(kVertexIndex)) {
        uint32_t got;
        if (var->Get(&got)) {
          if (got == index) {
            *lane = i;
            return true;
          }
        }
      }
    }

    return false;
  }

  void OnError(const std::string& error) {
    DEBUGGER_LOG("ERROR: %s", error.c_str());
    error_ += error;
  }

  using PendingThreadsMap = std::unordered_map<InvocationKey,
                                               debug::Script::OnThread,
                                               InvocationKey::Hash>;
  using ThreadVector = std::vector<std::unique_ptr<Thread>>;
  std::shared_ptr<dap::Session> session_;
  std::mutex threads_mutex_;
  PendingThreadsMap pendingThreads_;  // guarded by threads_mutex_
  ThreadVector runningThreads_;       // guarded by threads_mutex_
  std::mutex error_mutex_;
  Result error_;  // guarded by error_mutex_
};

std::pair<Engine::Debugger*, Result> EngineVulkan::GetDebugger() {
  if (!debugger_) {
    auto debugger = new VkDebugger();
    debugger_.reset(debugger);
    auto res = debugger->Connect();
    if (!res.IsSuccess()) {
      return {nullptr, res};
    }
  }
  return {debugger_.get(), Result()};
}

}  // namespace vulkan
}  // namespace amber

#else  // AMBER_ENABLE_VK_DEBUGGING

namespace amber {
namespace vulkan {

std::pair<Engine::Debugger*, Result> EngineVulkan::GetDebugger() {
  return {nullptr,
          Result("Amber was not built with AMBER_ENABLE_VK_DEBUGGING enabled")};
}

}  // namespace vulkan
}  // namespace amber

#endif  // AMBER_ENABLE_VK_DEBUGGING
