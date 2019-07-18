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
#include <vector>

namespace amber {

enum ShaderFormat {
  kShaderFormatDefault = 0,
  kShaderFormatText,
  kShaderFormatGlsl,
  kShaderFormatHlsl,
  kShaderFormatSpirvAsm,
  kShaderFormatSpirvHex,
  kShaderFormatOpenCLC,
};

enum ShaderType {
  kShaderTypeCompute = 0,
  kShaderTypeGeometry,
  kShaderTypeFragment,
  kShaderTypeVertex,
  kShaderTypeTessellationControl,
  kShaderTypeTessellationEvaluation,
  kShaderTypeMulti,
};

/// Stores information for a shader.
struct ShaderInfo {
  /// The format of the shader.
  ShaderFormat format;
  /// The type of shader.
  ShaderType type;
  /// This is a unique name for this shader. The name is produced from the
  /// input script, possibly with extra prefix contents. This name, if used
  /// in the ShaderMap will map to this specific shader.
  std::string shader_name;
  /// This is the shader source, the source is in the |format| given above.
  std::string shader_source;
  /// A list of SPIR-V optimization passes to execute on the shader.
  std::vector<std::string> optimizations;
};

}  // namespace amber

#endif  // AMBER_SHADER_INFO_H_
