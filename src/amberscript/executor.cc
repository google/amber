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

#include "src/amberscript/executor.h"

namespace amber {
namespace amberscript {

Executor::Executor() : amber::Executor() {}

Executor::~Executor() = default;

Result Executor::Execute(Engine*,
                         const amber::Script* src_script,
                         const ShaderMap&) {
  if (!src_script->IsAmberScript())
    return Result("AmberScript executor called with non-amber script source");

  // const amberscript::Script* script = ToAmberScript(src_script);

  return {};
}

}  // namespace amberscript
}  // namespace amber
