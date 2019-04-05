// Copyright 2018 The Amber Authors.
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

#ifndef SRC_EXECUTOR_H_
#define SRC_EXECUTOR_H_

#include "amber/result.h"
#include "src/engine.h"
#include "src/script.h"
#include "src/verifier.h"

namespace amber {

enum class ExecutionType { kExecute = 0, kPipelineCreateOnly };

/// The executor is responsible for running the given script against an engine.
class Executor {
 public:
  /// Create a new executor.
  Executor();
  ~Executor();

  /// Executes |script| against |engine|. For each shader described in |script|
  /// if the shader name exists in |map| the value for that map'd key will be
  /// used as the shader binary.
  Result Execute(Engine* engine,
                 const Script* script,
                 const ShaderMap& map,
                 ExecutionType executionType);

 private:
  Result CompileShaders(const Script* script, const ShaderMap& shader_map);
  Result ExecuteCommand(Engine* engine, Command* cmd);

  Verifier verifier_;
};

}  // namespace amber

#endif  // SRC_EXECUTOR_H_
