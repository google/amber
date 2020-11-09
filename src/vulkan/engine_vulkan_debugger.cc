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

#include <stdlib.h>

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
#include "src/virtual_file_store.h"

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

// kThreadTimeout is the maximum time to wait for a thread to complete.
static constexpr const auto kThreadTimeout = std::chrono::minutes(1);

// kStepEventTimeout is the maximum time to wait for a thread to complete a step
// request.
static constexpr const auto kStepEventTimeout = std::chrono::seconds(1);

// Event provides a basic wait-and-signal synchronization primitive.
class Event {
 public:
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

// WindowSpacePosition holds a two-element unsigned integer index, used to
// identifiy a single fragment invocation.
struct WindowSpacePosition {
  size_t hash() const { return x << 10 | y; }
  bool operator==(const WindowSpacePosition& other) const {
    return x == other.x && y == other.y;
  }

  uint32_t x;
  uint32_t y;
};

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
  bool Get(uint32_t* out) const {
    *out = static_cast<uint32_t>(std::atoi(value.c_str()));
    return true;  // TODO(bclayton): Verify the value parsed correctly.
  }

  bool Get(int64_t* out) const {
    *out = static_cast<int64_t>(std::atoi(value.c_str()));
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

  bool Get(GlobalInvocationId* out) const {
    auto x = children.Find("x");
    auto y = children.Find("y");
    auto z = children.Find("z");
    return (x != nullptr && y != nullptr && z != nullptr && x->Get(&out->x) &&
            y->Get(&out->y) && z->Get(&out->z));
  }

  bool Get(WindowSpacePosition* out) const {
    auto x = children.Find("x");
    auto y = children.Find("y");
    return (x != nullptr && y != nullptr && x->Get(&out->x) && y->Get(&out->y));
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
  bool TopStackFrame(dap::integer thread_id, dap::StackFrame* frame) {
    std::vector<dap::StackFrame> stack;
    if (!Callstack(thread_id, &stack)) {
      return false;
    }
    *frame = stack.front();
    return true;
  }

  // Callstack retrieves the thread's full call stack.
  // Returns true on success, false on error.
  bool Callstack(dap::integer thread_id, std::vector<dap::StackFrame>* stack) {
    dap::StackTraceRequest request;
    request.threadId = thread_id;
    auto response = session_->send(request).get();
    if (response.error) {
      onerror_(response.error.message);
      return false;
    }
    if (response.response.stackFrames.size() == 0) {
      onerror_("Stack frame is empty");
      return false;
    }
    *stack = response.response.stackFrames;
    return true;
  }

  // FrameLocation retrieves the current frame source location, and optional
  // source line text.
  // Returns true on success, false on error.
  bool FrameLocation(VirtualFileStore* virtual_files,
                     const dap::StackFrame& frame,
                     debug::Location* location,
                     std::string* line = nullptr) {
    location->line = static_cast<uint32_t>(frame.line);

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
      if (!SourceContent(virtual_files, frame.source.value(), &lines)) {
        return false;
      }
      if (location->line > lines.size()) {
        onerror_("Line " + std::to_string(location->line) +
                 " is greater than the number of lines in the source file (" +
                 std::to_string(lines.size()) + ")");
        return false;
      }
      if (location->line < 1) {
        onerror_("Line number reported was " + std::to_string(location->line));
        return false;
      }
      *line = lines[location->line - 1];
    }

    return true;
  }

  // SourceContext retrieves the the SourceLines for the given source.
  // Returns true on success, false on error.
  bool SourceContent(VirtualFileStore* virtual_files,
                     const dap::Source& source,
                     SourceLines* out) {
    auto path = source.path.value("");

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

    if (path != "") {
      auto it = sourceCache_.by_path.find(path);
      if (it != sourceCache_.by_path.end()) {
        *out = it->second;
        return true;
      }

      // First search the virtual file store.
      std::string content;
      if (virtual_files->Get(path, &content).IsSuccess()) {
        auto lines = Split(content, "\n");
        sourceCache_.by_path.emplace(path, lines);
        *out = lines;
        return true;
      }

      // TODO(bclayton) - We shouldn't be doing direct file IO here. We should
      // bubble the IO request to the amber 'embedder'.
      // See: https://github.com/google/amber/issues/777
      if (auto file = std::ifstream(path)) {
        SourceLines lines;
        std::string line;
        while (std::getline(file, line)) {
          lines.emplace_back(line);
        }

        sourceCache_.by_path.emplace(path, lines);
        *out = lines;
        return true;
      }
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
    std::unordered_map<int64_t, SourceLines> by_ref;
    std::unordered_map<std::string, SourceLines> by_path;
  };

  std::shared_ptr<dap::Session> session_;
  ErrorHandler onerror_;
  SourceCache sourceCache_;
};

// InvocationKey is a tagged-union structure that identifies a single shader
// invocation.
struct InvocationKey {
  // Hash is a custom hasher that can enable InvocationKeys to be used as keys
  // in std containers.
  struct Hash {
    size_t operator()(const InvocationKey& key) const;
  };

  enum class Type { kGlobalInvocationId, kVertexIndex, kWindowSpacePosition };
  union Data {
    GlobalInvocationId global_invocation_id;
    uint32_t vertex_id;
    WindowSpacePosition window_space_position;
  };

  explicit InvocationKey(const GlobalInvocationId&);
  explicit InvocationKey(const WindowSpacePosition&);
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
      hash += key.data.global_invocation_id.hash();
      break;
    case Type::kVertexIndex:
      hash += key.data.vertex_id;
      break;
    case Type::kWindowSpacePosition:
      hash += key.data.window_space_position.hash();
      break;
  }
  return hash;
}

InvocationKey::InvocationKey(const GlobalInvocationId& id)
    : type(Type::kGlobalInvocationId) {
  data.global_invocation_id = id;
}

InvocationKey::InvocationKey(const WindowSpacePosition& pos)
    : type(Type::kWindowSpacePosition) {
  data.window_space_position = pos;
}

InvocationKey::InvocationKey(Type type_, const Data& data_)
    : type(type_), data(data_) {}

std::string InvocationKey::String() const {
  std::stringstream ss;
  switch (type) {
    case Type::kGlobalInvocationId:
      ss << "GlobalInvocation(" << data.global_invocation_id.x << ", "
         << data.global_invocation_id.y << ", " << data.global_invocation_id.z
         << ")";
      break;
    case Type::kVertexIndex:
      ss << "VertexIndex(" << data.vertex_id << ")";
      break;
    case Type::kWindowSpacePosition:
      ss << "WindowSpacePosition(" << data.window_space_position.x << ", "
         << data.window_space_position.y << ")";
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
      return data.global_invocation_id == other.data.global_invocation_id;
    case Type::kVertexIndex:
      return data.vertex_id == other.data.vertex_id;
    case Type::kWindowSpacePosition:
      return data.window_space_position == other.data.window_space_position;
  }
  return false;
}

// Thread controls and verifies a single debugger thread of execution.
class Thread : public debug::Thread {
 public:
  Thread(VirtualFileStore* virtual_files,
         std::shared_ptr<dap::Session> session,
         dap::integer threadId,
         int lane,
         std::shared_ptr<const debug::ThreadScript> script)
      : virtual_files_(virtual_files),
        thread_id_(threadId),
        lane_(lane),
        client_(session, [this](const std::string& err) { OnError(err); }) {
    // The thread script runs concurrently with other debugger thread scripts.
    // Run on a separate amber thread.
    thread_ = std::thread([this, script] {
      script->Run(this);  // Begin running the thread script.
      done_.Signal();     // Signal when done.
    });
  }

  ~Thread() override { Flush(); }

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

  // OnStep is called when the debugger sends a 'stopped' event with reason
  // 'step'.
  void OnStep() { on_stepped_.Signal(); }

  // WaitForSteppedEvent halts execution of the thread until OnStep() is
  // called, or kStepEventTimeout is reached.
  void WaitForStepEvent(const std::string& request) {
    if (!on_stepped_.Wait(kStepEventTimeout)) {
      OnError("Timeout waiting for " + request + " request to complete");
    }
  }

  // debug::Thread compliance
  void StepOver() override {
    DEBUGGER_LOG("StepOver()");
    dap::NextRequest request;
    request.threadId = thread_id_;
    client_.Send(request);
    WaitForStepEvent("step-over");
  }

  void StepIn() override {
    DEBUGGER_LOG("StepIn()");
    dap::StepInRequest request;
    request.threadId = thread_id_;
    client_.Send(request);
    WaitForStepEvent("step-in");
  }

  void StepOut() override {
    DEBUGGER_LOG("StepOut()");
    dap::StepOutRequest request;
    request.threadId = thread_id_;
    client_.Send(request);
    WaitForStepEvent("step-out");
  }

  void Continue() override {
    DEBUGGER_LOG("Continue()");
    dap::ContinueRequest request;
    request.threadId = thread_id_;
    client_.Send(request);
  }

  void ExpectLocation(const debug::Location& location,
                      const std::string& line) override {
    DEBUGGER_LOG("ExpectLocation('%s', %d)", location.file.c_str(),
                 location.line);

    dap::StackFrame frame;
    if (!client_.TopStackFrame(thread_id_, &frame)) {
      return;
    }

    debug::Location got_location;
    std::string got_source_line;
    if (!client_.FrameLocation(virtual_files_, frame, &got_location,
                               &got_source_line)) {
      return;
    }

    if (got_location.file != location.file) {
      OnError("Expected file to be '" + location.file + "' but file was '" +
              got_location.file + "'");
    } else if (got_location.line != location.line) {
      std::stringstream ss;
      ss << "Expected line " << std::to_string(location.line);
      if (line != "") {
        ss << " `" << line << "`";
      }
      ss << " but line was " << std::to_string(got_location.line) << " `"
         << got_source_line << "`";
      OnError(ss.str());
    } else if (line != "" && got_source_line != line) {
      OnError("Expected source for line " + std::to_string(location.line) +
              " to be: `" + line + "` but line was: `" + got_source_line + "`");
    }
  }

  void ExpectCallstack(
      const std::vector<debug::StackFrame>& callstack) override {
    DEBUGGER_LOG("ExpectCallstack()");

    std::vector<dap::StackFrame> got_stack;
    if (!client_.Callstack(thread_id_, &got_stack)) {
      return;
    }

    std::stringstream ss;

    size_t count = std::min(callstack.size(), got_stack.size());
    for (size_t i = 0; i < count; i++) {
      auto const& got_frame = got_stack[i];
      auto const& want_frame = callstack[i];
      bool ok = got_frame.name == want_frame.name;
      if (ok && want_frame.location.file != "") {
        ok = got_frame.source.has_value() &&
             got_frame.source->name.value("") == want_frame.location.file;
      }
      if (ok && want_frame.location.line != 0) {
        ok = got_frame.line == static_cast<int>(want_frame.location.line);
      }
      if (!ok) {
        ss << "Unexpected stackframe at frame " << i
           << "\nGot:      " << FrameString(got_frame)
           << "\nExpected: " << FrameString(want_frame) << "\n";
      }
    }

    if (got_stack.size() > callstack.size()) {
      ss << "Callstack has an additional "
         << (got_stack.size() - callstack.size()) << " unexpected frames\n";
    } else if (callstack.size() > got_stack.size()) {
      ss << "Callstack is missing " << (callstack.size() - got_stack.size())
         << " frames\n";
    }

    if (ss.str().size() > 0) {
      ss << "Full callstack:\n";
      for (auto& frame : got_stack) {
        ss << "  " << FrameString(frame) << "\n";
      }
      OnError(ss.str());
    }
  }

  void ExpectLocal(const std::string& name, int64_t value) override {
    DEBUGGER_LOG("ExpectLocal('%s', %d)", name.c_str(), (int)value);
    ExpectLocalT(name, value, [](int64_t a, int64_t b) { return a == b; });
  }

  void ExpectLocal(const std::string& name, double value) override {
    DEBUGGER_LOG("ExpectLocal('%s', %f)", name.c_str(), value);
    ExpectLocalT(name, value,
                 [](double a, double b) { return std::abs(a - b) < 0.00001; });
  }

  void ExpectLocal(const std::string& name, const std::string& value) override {
    DEBUGGER_LOG("ExpectLocal('%s', '%s')", name.c_str(), value.c_str());
    ExpectLocalT(name, value, [](const std::string& a, const std::string& b) {
      return a == b;
    });
  }

  // ExpectLocalT verifies that the local variable with the given name has the
  // expected value. |name| may contain `.` delimiters to index structure or
  // array types. ExpectLocalT uses the function equal to compare the expected
  // and actual values, returning true if considered equal, otherwise false.
  // EQUAL_FUNC is a function-like type with the signature:
  //   bool(T a, T b)
  template <typename T, typename EQUAL_FUNC>
  void ExpectLocalT(const std::string& name,
                    const T& expect,
                    EQUAL_FUNC&& equal) {
    dap::StackFrame frame;
    if (!client_.TopStackFrame(thread_id_, &frame)) {
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
            OnError("Local '" + name + "' not found.\nLocals: " +
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

      if (!equal(got, expect)) {
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

  std::string FrameString(const dap::StackFrame& frame) {
    std::stringstream ss;
    ss << frame.name;
    if (frame.source.has_value() && frame.source->name.has_value()) {
      ss << " " << frame.source->name.value() << ":" << frame.line;
    }
    return ss.str();
  }

  std::string FrameString(const debug::StackFrame& frame) {
    std::stringstream ss;
    ss << frame.name;
    if (frame.location.file != "") {
      ss << " " << frame.location.file;
      if (frame.location.line != 0) {
        ss << ":" << frame.location.line;
      }
    }
    return ss.str();
  }

  VirtualFileStore* const virtual_files_;
  const dap::integer thread_id_;
  const int lane_;
  Client client_;
  std::thread thread_;
  Event done_;
  Event on_stepped_;
  Result error_;
};

// VkDebugger is a private implementation of the Engine::Debugger interface.
class VkDebugger : public Engine::Debugger {
  static constexpr const char* kComputeShaderFunctionName = "ComputeShader";
  static constexpr const char* kVertexShaderFunctionName = "VertexShader";
  static constexpr const char* kFragmentShaderFunctionName = "FragmentShader";
  static constexpr const char* kGlobalInvocationId = "globalInvocationId";
  static constexpr const char* kWindowSpacePosition = "windowSpacePosition";
  static constexpr const char* kVertexIndex = "vertexIndex";

 public:
  explicit VkDebugger(VirtualFileStore* virtual_files)
      : virtual_files_(virtual_files) {}

  /// Connect establishes the connection to the shader debugger. Must be
  /// called before any of the |debug::Events| methods.
  Result Connect() {
    auto port_str = getenv("VK_DEBUGGER_PORT");
    if (!port_str) {
      return Result("The environment variable VK_DEBUGGER_PORT was not set\n");
    }
    int port = atoi(port_str);
    if (port == 0) {
      return Result("Could not parse port number from VK_DEBUGGER_PORT ('" +
                    std::string(port_str) + "')");
    }

    constexpr int kMaxAttempts = 10;
    // The socket might take a while to open - retry connecting.
    for (int attempt = 0; attempt < kMaxAttempts; attempt++) {
      auto connection = dap::net::connect("localhost", port);
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
        if (event.reason == "function breakpoint" ||
            event.reason == "breakpoint") {
          OnBreakpointHit(event.threadId.value(0));
        } else if (event.reason == "step") {
          OnStep(event.threadId.value(0));
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
      fbp.name = kFragmentShaderFunctionName;
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
    return Result("Unable to connect to debugger on port " +
                  std::to_string(port));
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
      for (auto& it : runningThreads_) {
        result += it.second->Flush();
      }
      runningThreads_.clear();
      completedThreads_.clear();
    }
    return result;
  }

  // debug::Events compliance
  void BreakOnComputeGlobalInvocation(
      uint32_t x,
      uint32_t y,
      uint32_t z,
      const std::shared_ptr<const debug::ThreadScript>& script) override {
    std::unique_lock<std::mutex> lock(threads_mutex_);
    pendingThreads_.emplace(GlobalInvocationId{x, y, z}, script);
  }

  void BreakOnVertexIndex(
      uint32_t index,
      const std::shared_ptr<const debug::ThreadScript>& script) override {
    InvocationKey::Data data;
    data.vertex_id = index;
    auto key = InvocationKey{InvocationKey::Type::kVertexIndex, data};
    std::unique_lock<std::mutex> lock(threads_mutex_);
    pendingThreads_.emplace(key, script);
  }

  void BreakOnFragmentWindowSpacePosition(
      uint32_t x,
      uint32_t y,
      const std::shared_ptr<const debug::ThreadScript>& script) override {
    std::unique_lock<std::mutex> lock(threads_mutex_);
    pendingThreads_.emplace(WindowSpacePosition{x, y}, script);
  }

 private:
  void StartThread(
      dap::integer thread_id,
      int lane,
      const std::shared_ptr<const amber::debug::ThreadScript>& script) {
    auto thread =
        MakeUnique<Thread>(virtual_files_, session_, thread_id, lane, script);
    auto it = runningThreads_.find(thread_id);
    if (it != runningThreads_.end()) {
      // The debugger has now started working on something else for the given
      // thread id.
      // Flush the Thread to ensure that all tasks have actually finished, and
      // move the Thread to the completedThreads_ list.
      auto completed_thread = std::move(it->second);
      error_ += completed_thread->Flush();
      completedThreads_.emplace_back(std::move(completed_thread));
    }
    runningThreads_[thread_id] = std::move(thread);
  }

  // OnBreakpointHit is called when a debugger breakpoint is hit (breakpoints
  // are set at shader entry points). pendingThreads_ is checked to see if this
  // thread needs testing, and if so, creates a new ::Thread.
  // If there's no pendingThread_ entry for the given thread, it is resumed to
  // allow the shader to continue executing.
  void OnBreakpointHit(dap::integer thread_id) {
    DEBUGGER_LOG("Breakpoint hit: thread %d", (int)thread_id);
    Client client(session_, [this](const std::string& err) { OnError(err); });

    std::unique_lock<std::mutex> lock(threads_mutex_);
    for (auto it = pendingThreads_.begin(); it != pendingThreads_.end(); it++) {
      auto& key = it->first;
      auto& script = it->second;
      switch (key.type) {
        case InvocationKey::Type::kGlobalInvocationId: {
          auto invocation_id = key.data.global_invocation_id;
          int lane;
          if (FindGlobalInvocationId(thread_id, invocation_id, &lane)) {
            DEBUGGER_LOG("Breakpoint hit: GetGlobalInvocationId: [%d, %d, %d]",
                         invocation_id.x, invocation_id.y, invocation_id.z);
            StartThread(thread_id, lane, script);
            pendingThreads_.erase(it);
            return;
          }
          break;
        }
        case InvocationKey::Type::kVertexIndex: {
          auto vertex_id = key.data.vertex_id;
          int lane;
          if (FindVertexIndex(thread_id, vertex_id, &lane)) {
            DEBUGGER_LOG("Breakpoint hit: VertexId: %d", vertex_id);
            StartThread(thread_id, lane, script);
            pendingThreads_.erase(it);
            return;
          }
          break;
        }
        case InvocationKey::Type::kWindowSpacePosition: {
          auto position = key.data.window_space_position;
          int lane;
          if (FindWindowSpacePosition(thread_id, position, &lane)) {
            DEBUGGER_LOG("Breakpoint hit: VertexId: [%d, %d]", position.x,
                         position.y);
            StartThread(thread_id, lane, script);
            pendingThreads_.erase(it);
            return;
          }
          break;
        }
      }
    }

    // No pending tests for this thread. Let it carry on...
    dap::ContinueRequest request;
    request.threadId = thread_id;
    client.Send(request);
  }

  // OnStep is called when a debugger finishes a step request.
  void OnStep(dap::integer thread_id) {
    DEBUGGER_LOG("Step event: thread %d", (int)thread_id);
    Client client(session_, [this](const std::string& err) { OnError(err); });

    std::unique_lock<std::mutex> lock(threads_mutex_);
    auto it = runningThreads_.find(thread_id);
    if (it == runningThreads_.end()) {
      OnError("Step event reported for non-running thread " +
              std::to_string(thread_id.operator int64_t()));
    }

    it->second->OnStep();
  }

  // FindLocal looks for the shader's local variable with the given name and
  // value in the stack frames' locals, returning true if found, and assigns the
  // index of the SIMD lane it was found in to |lane|.
  template <typename T>
  bool FindLocal(dap::integer thread_id,
                 const char* name,
                 const T& value,
                 int* lane) {
    Client client(session_, [this](const std::string& err) { OnError(err); });

    dap::StackFrame frame;
    if (!client.TopStackFrame(thread_id, &frame)) {
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
      if (auto var = lane_var->Find(name)) {
        T got;
        if (var->Get(&got)) {
          if (got == value) {
            *lane = i;
            return true;
          }
        }
      }
    }

    return false;
  }

  // FindGlobalInvocationId looks for the compute shader's global invocation id
  // in the stack frames' locals, returning true if found, and assigns the index
  // of the SIMD lane it was found in to |lane|.
  // TODO(bclayton): This value should probably be in the globals, not locals!
  bool FindGlobalInvocationId(dap::integer thread_id,
                              const GlobalInvocationId& id,
                              int* lane) {
    return FindLocal(thread_id, kGlobalInvocationId, id, lane);
  }

  // FindVertexIndex looks for the requested vertex shader's vertex index in the
  // stack frames' locals, returning true if found, and assigns the index of the
  // SIMD lane it was found in to |lane|.
  // TODO(bclayton): This value should probably be in the globals, not locals!
  bool FindVertexIndex(dap::integer thread_id, uint32_t index, int* lane) {
    return FindLocal(thread_id, kVertexIndex, index, lane);
  }

  // FindWindowSpacePosition looks for the fragment shader's window space
  // position in the stack frames' locals, returning true if found, and assigns
  // the index of the SIMD lane it was found in to |lane|.
  // TODO(bclayton): This value should probably be in the globals, not locals!
  bool FindWindowSpacePosition(dap::integer thread_id,
                               const WindowSpacePosition& pos,
                               int* lane) {
    return FindLocal(thread_id, kWindowSpacePosition, pos, lane);
  }

  void OnError(const std::string& error) {
    DEBUGGER_LOG("ERROR: %s", error.c_str());
    error_ += error;
  }

  using PendingThreadsMap =
      std::unordered_map<InvocationKey,
                         std::shared_ptr<const debug::ThreadScript>,
                         InvocationKey::Hash>;
  using ThreadVector = std::vector<std::unique_ptr<Thread>>;
  using ThreadMap = std::unordered_map<int64_t, std::unique_ptr<Thread>>;
  VirtualFileStore* const virtual_files_;
  std::shared_ptr<dap::Session> session_;
  std::mutex threads_mutex_;

  // The threads that are waiting to be started.
  PendingThreadsMap pendingThreads_;  // guarded by threads_mutex_

  // All threads that have been started.
  ThreadMap runningThreads_;  // guarded by threads_mutex_

  // Threads that have finished.
  ThreadVector completedThreads_;  // guarded by threads_mutex_
  std::mutex error_mutex_;
  Result error_;  // guarded by error_mutex_
};
}  // namespace

std::pair<Engine::Debugger*, Result> EngineVulkan::GetDebugger(
    VirtualFileStore* virtual_files) {
  if (!debugger_) {
    auto debugger = new VkDebugger(virtual_files);
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

std::pair<Engine::Debugger*, Result> EngineVulkan::GetDebugger(
    VirtualFileStore*) {
  return {nullptr,
          Result("Amber was not built with AMBER_ENABLE_VK_DEBUGGING enabled")};
}

}  // namespace vulkan
}  // namespace amber

#endif  // AMBER_ENABLE_VK_DEBUGGING
