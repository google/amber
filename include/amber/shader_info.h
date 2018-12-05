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

#ifndef AMBER_SHADER_INFO_H_
#define AMBER_SHADER_INFO_H_

#include <string>

namespace amber {

enum class ShaderFormat : uint8_t {
  kDefault = 0,
  kText,
  kGlsl,
  kSpirvAsm,
  kSpirvHex,
};

enum class ShaderType : uint8_t {
  kCompute = 0,
  kGeometry,
  kFragment,
  kVertex,
  kTessellationControl,
  kTessellationEvaluation,
};

struct ShaderInfo {
  ShaderFormat format;
  ShaderType type;
  std::string shader_name;
  std::string shader_data;
};

}  // namespace amber

#endif  // AMBER_SHADER_INFO_H_