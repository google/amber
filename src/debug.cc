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

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "src/make_unique.h"

namespace amber {
namespace debug {

namespace {

class ScriptImpl : public Script {
 public:
  void Run(Events* e) const override {
    for (auto f : sequence_) {
      f(e);
    }
  }

  void BreakOnComputeGlobalInvocation(
      uint32_t x,
      uint32_t y,
      uint32_t z,
      const std::shared_ptr<const ThreadScript>& thread) override {
    sequence_.emplace_back([=](Events* events) {
      events->BreakOnComputeGlobalInvocation(x, y, z, thread);
    });
  }

  void BreakOnVertexIndex(
      uint32_t index,
      const std::shared_ptr<const ThreadScript>& thread) override {
    sequence_.emplace_back(
        [=](Events* events) { events->BreakOnVertexIndex(index, thread); });
  }

  void BreakOnFragmentWindowSpacePosition(
      uint32_t x,
      uint32_t y,
      const std::shared_ptr<const ThreadScript>& thread) override {
    sequence_.emplace_back([=](Events* events) {
      events->BreakOnFragmentWindowSpacePosition(x, y, thread);
    });
  }

 private:
  using Event = std::function<void(Events*)>;
  std::vector<Event> sequence_;
};

class ThreadScriptImpl : public ThreadScript {
 public:
  void Run(Thread* thread) const override {
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

  void ExpectCallstack(const std::vector<StackFrame>& callstack) override {
    sequence_.emplace_back([=](Thread* t) { t->ExpectCallstack(callstack); });
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
ThreadScript::~ThreadScript() = default;
Script::~Script() = default;

std::shared_ptr<ThreadScript> ThreadScript::Create() {
  return std::make_shared<ThreadScriptImpl>();
}

std::unique_ptr<Script> Script::Create() {
  return MakeUnique<ScriptImpl>();
}

}  // namespace debug
}  // namespace amber
