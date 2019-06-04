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

#ifndef SRC_DXC_HELPER_H_
#define SRC_DXC_HELPER_H_

#include <string>
#include <vector>

#include "amber/result.h"

namespace amber {
namespace dxchelper {

// Passes the HLSL source code to the DXC compiler with SPIR-V CodeGen.
// Returns the generated SPIR-V binary via |generated_binary| argument.
Result Compile(const std::string& src_str,
               const std::string& entry_str,
               const std::string& profile_str,
               const std::string& spv_env,
               std::vector<uint32_t>* generated_binary);

}  // namespace dxchelper
}  // namespace amber

#endif  // SRC_DXC_HELPER_H_
