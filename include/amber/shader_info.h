// Copyright 2018 The Amber Authors.
// Copyright (C) 2024 Advanced Micro Devices, Inc. All rights reserved.
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

#include <cstdint>
#include <string>
#include <vector>

namespace amber {

enum class ShaderFormat : uint8_t {
  kDefault = 0,
  kText,
  kGlsl,
  kHlsl,
  kSpirvAsm,
  kSpirvHex,
  kSpirvBin,
  kOpenCLC,
};

enum class ShaderType : uint8_t {
  kCompute = 0,
  kGeometry,
  kFragment,
  kVertex,
  kTessellationControl,
  kTessellationEvaluation,
  kRayGeneration,
  kAnyHit,
  kClosestHit,
  kMiss,
  kIntersection,
  kCall,
  kMulti,
};

inline bool isRayTracingShaderType(ShaderType type) {
  return type == ShaderType::kRayGeneration || type == ShaderType::kAnyHit ||
         type == ShaderType::kClosestHit || type == ShaderType::kMiss ||
         type == ShaderType::kIntersection || type == ShaderType::kCall;
}

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
  /// Target environment for the shader compilation.
  std::string target_env;
  /// The shader SPIR-V if it was compiled by Amber
  std::vector<uint32_t> shader_data;
};

}  // namespace amber

#endif  // AMBER_SHADER_INFO_H_
