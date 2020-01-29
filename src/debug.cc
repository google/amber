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

#include "src/debug.h"

#include <memory>
#include <string>

#include "src/make_unique.h"

namespace amber {
namespace debug {

namespace {

// ThreadScript is an implementation of amber::debug::Thread that records all
// calls made on it, which can be later replayed using ThreadScript::Run().
class ThreadScript : public Thread {
 public:
  void Run(Thread* thread) {
    for (auto f : sequence_) {
      f(thread);
    }
  }
  // Thread compliance
  void StepOver() override {
    sequence_.emplace_back([](Thread* t) { t->StepOver(); });
  }

  void StepIn() override {
    sequence_.emplace_back([](Thread* t) { t->StepIn(); });
  }

  void StepOut() override {
    sequence_.emplace_back([](Thread* t) { t->StepOut(); });
  }

  void Continue() override {
    sequence_.emplace_back([](Thread* t) { t->Continue(); });
  }

  void ExpectLocation(const Location& location,
                      const std::string& line) override {
    sequence_.emplace_back(
        [=](Thread* t) { t->ExpectLocation(location, line); });
  }

  void ExpectLocal(const std::string& name, int64_t value) override {
    sequence_.emplace_back([=](Thread* t) { t->ExpectLocal(name, value); });
  }

  void ExpectLocal(const std::string& name, double value) override {
    sequence_.emplace_back([=](Thread* t) { t->ExpectLocal(name, value); });
  }

  void ExpectLocal(const std::string& name, const std::string& value) override {
    sequence_.emplace_back([=](Thread* t) { t->ExpectLocal(name, value); });
  }

 private:
  using Event = std::function<void(Thread*)>;
  std::vector<Event> sequence_;
};

}  // namespace

Thread::~Thread() = default;
Events::~Events() = default;

void Script::Run(Events* e) {
  for (auto f : sequence_) {
    f(e);
  }
}

void Script::BreakOnComputeGlobalInvocation(uint32_t x,
                                            uint32_t y,
                                            uint32_t z,
                                            const OnThread& callback) {
  auto script = std::make_shared<ThreadScript>();
  callback(script.get());  // Record

  sequence_.emplace_back([=](Events* events) {
    events->BreakOnComputeGlobalInvocation(x, y, z, [=](Thread* thread) {
      script->Run(thread);  // Replay
    });
  });
}

void Script::BreakOnVertexIndex(uint32_t index, const OnThread& callback) {
  // std::make_shared is used here instead of MakeUnique as std::function is
  // copyable, and cannot capture move-only values.
  auto script = std::make_shared<ThreadScript>();
  callback(script.get());  // Record

  sequence_.emplace_back([=](Events* events) {
    events->BreakOnVertexIndex(index, [=](Thread* thread) {
      script->Run(thread);  // Replay
    });
  });
}

}  // namespace debug
}  // namespace amber
