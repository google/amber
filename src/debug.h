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

#ifndef SRC_DEBUG_H_
#define SRC_DEBUG_H_

#include <stdint.h>

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "amber/result.h"

/// amber::debug holds the types used for testing a graphics debugger.
namespace amber {
namespace debug {

// Forward declaration.
class ThreadScript;

/// Location holds a file path and a 1-based line number.
struct Location {
  std::string file;   // empty represents unspecified.
  uint32_t line = 0;  // 0 represents unspecified.
};

// StackFrame holds name and location of a stack frame.
struct StackFrame {
  std::string name;
  Location location;
};

/// Thread is the interface used to control a single debugger thread of
/// execution.
class Thread {
 public:
  virtual ~Thread();

  /// StepOver instructs the debugger to perform a single line step on the given
  /// thread of execution, stepping over any function call instructions.
  virtual void StepOver() = 0;

  /// StepIn instructs the debugger to perform a single line step on the given
  /// thread of execution, stepping into any function call instructions.
  virtual void StepIn() = 0;

  /// StepOut instructs the debugger to resume execution of the given thread of
  /// execution. If the current function is not the top most of the call stack,
  /// then the debugger will pause at the next line after the call to the
  /// current function.
  virtual void StepOut() = 0;

  /// Continue instructs the debugger to resume execution of the given thread of
  /// execution.
  virtual void Continue() = 0;

  /// ExpectLocation verifies that the debugger is currently suspended for the
  /// given thread of execution at the specified source location. If |line| is
  /// non-empty, then the line's textual source will also be verified.
  virtual void ExpectLocation(const Location& location,
                              const std::string& line) = 0;

  /// ExpectCallstack verifies that the debugger is currently suspended for the
  /// given thread of execution with the specified callstack.
  /// callstack is ordered with the 0th element representing the most nested
  /// call.
  virtual void ExpectCallstack(const std::vector<StackFrame>& callstack) = 0;

  /// ExpectLocal verifies that the local variable with the given name has the
  /// expected value. |name| may contain `.` delimiters to index structure or
  /// array types.
  virtual void ExpectLocal(const std::string& name, int64_t value) = 0;
  virtual void ExpectLocal(const std::string& name, double value) = 0;
  virtual void ExpectLocal(const std::string& name,
                           const std::string& value) = 0;
};

/// Events is the interface used to control the debugger.
class Events {
 public:
  virtual ~Events();

  /// BreakOnComputeGlobalInvocation instructs the debugger to set a breakpoint
  /// at the start of the compute shader program for the invocation with the
  /// global invocation identifier [|x|, |y|, |z|], and run the |ThreadScript|
  /// once the breakpoint is hit.
  virtual void BreakOnComputeGlobalInvocation(
      uint32_t x,
      uint32_t y,
      uint32_t z,
      const std::shared_ptr<const ThreadScript>&) = 0;

  /// BreakOnVertexIndex instructs the debugger to set a breakpoint at the start
  /// of the vertex shader program for the invocation with the vertex index
  /// |index|, and run the |ThreadScript| once the breakpoint is hit.
  virtual void BreakOnVertexIndex(
      uint32_t index,
      const std::shared_ptr<const ThreadScript>&) = 0;

  /// BreakOnFragmentWindowSpacePosition instructs the debugger to set a
  /// breakpoint at the start of the fragment shader program for the invocation
  /// with the window space coordinate [x, y], and run the |ThreadScript| once
  /// the breakpoint is hit.
  virtual void BreakOnFragmentWindowSpacePosition(
      uint32_t x,
      uint32_t y,
      const std::shared_ptr<const ThreadScript>&) = 0;
};

/// ThreadScript is a specialization of the |amber::debug::Thread| interface,
/// and is used to record all the calls made on it, which can be later replayed
/// with |ThreadScript::Run|.
class ThreadScript : public Thread {
 public:
  ~ThreadScript() override;

  /// Run replays all the calls made to the |ThreadScript| on the given |Thread|
  /// parameter.
  virtual void Run(Thread*) const = 0;

  // Create constructs and returns a new ThreadScript.
  static std::shared_ptr<ThreadScript> Create();
};

/// Script is an specialization of the |amber::debug::Events| interface, and is
/// used to record all the calls made on it, which can be later replayed with
/// |Script::Run|.
class Script : public Events {
 public:
  ~Script() override;

  /// Run replays all the calls made to the |Script| on the given |Events|
  /// parameter.
  virtual void Run(Events*) const = 0;

  // Create constructs and returns a new Script.
  static std::unique_ptr<Script> Create();
};

}  // namespace debug
}  // namespace amber

#endif  // SRC_DEBUG_H_
