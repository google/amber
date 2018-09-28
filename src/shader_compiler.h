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

#ifndef SRC_SHADER_COMPILER_H_
#define SRC_SHADER_COMPILER_H_

#include <tuple>
#include <vector>

#include "amber/result.h"
#include "src/shader_data.h"

namespace amber {

class ShaderCompiler {
 public:
  ShaderCompiler();
  ~ShaderCompiler();

  std::pair<Result, std::vector<uint32_t>>
  Compile(ShaderType type, ShaderFormat fmt, const std::string& data) const;

 private:
  Result ParseHex(const std::string& data, std::vector<uint32_t>* result) const;
  Result CompileGlsl(ShaderType shader_type,
                     const std::string& data,
                     std::vector<uint32_t>* result) const;
};

}  // namespace amber

#endif  // SRC_SHADER_COMPILER_H_
