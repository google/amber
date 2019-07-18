// Copyright 2019 The Amber Authors.
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

#ifndef SRC_CLSPV_HELPER_H_
#define SRC_CLSPV_HELPER_H_

#include <string>
#include <vector>

#include "amber/result.h"
#include "src/pipeline.h"

namespace amber {
namespace clspvhelper {

// Passes the OpenCL C source code to Clspv.
// Returns the generated SPIR-V binary via |generated_binary| argument.
Result Compile(Pipeline::ShaderInfo* shader_info,
               std::vector<uint32_t>* generated_binary);

}  // namespace clspvhelper
}  // namespace amber

#endif  // SRC_CLSPV_HELPER_H_
