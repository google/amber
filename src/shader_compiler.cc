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

#include "src/shader_compiler.h"

#include <algorithm>
#include <cstdlib>
#include <iterator>
#include <string>
#include <utility>

#if AMBER_ENABLE_SPIRV_TOOLS
#include "spirv-tools/libspirv.hpp"
#include "spirv-tools/linker.hpp"
#endif  // AMBER_ENABLE_SPIRV_TOOLS

#if AMBER_ENABLE_SHADERC
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wshadow-uncaptured-local"
#pragma clang diagnostic ignored "-Wweak-vtables"
#include "third_party/shaderc/libshaderc/include/shaderc/shaderc.hpp"
#pragma clang diagnostic pop
#endif  // AMBER_ENABLE_SHADERC

namespace amber {

ShaderCompiler::ShaderCompiler() = default;

ShaderCompiler::ShaderCompiler(const std::string& env) : spv_env_(env) {}

ShaderCompiler::~ShaderCompiler() = default;

std::pair<Result, std::vector<uint32_t>> ShaderCompiler::Compile(
    const Shader* shader,
    const ShaderMap& shader_map) const {
  auto it = shader_map.find(shader->GetName());
  if (it != shader_map.end())
    return {{}, it->second};

#if AMBER_ENABLE_SPIRV_TOOLS
  std::string spv_errors;

  spv_target_env target_env = SPV_ENV_UNIVERSAL_1_0;
  if (!spv_env_.empty()) {
    if (!spvParseTargetEnv(spv_env_.c_str(), &target_env))
      return {Result("Unable to parse SPIR-V target environment"), {}};
  }

  spvtools::SpirvTools tools(target_env);
  tools.SetMessageConsumer([&spv_errors](spv_message_level_t level, const char*,
                                         const spv_position_t& position,
                                         const char* message) {
    switch (level) {
      case SPV_MSG_FATAL:
      case SPV_MSG_INTERNAL_ERROR:
      case SPV_MSG_ERROR:
        spv_errors += "error: line " + std::to_string(position.index) + ": " +
                      message + "\n";
        break;
      case SPV_MSG_WARNING:
        spv_errors += "warning: line " + std::to_string(position.index) + ": " +
                      message + "\n";
        break;
      case SPV_MSG_INFO:
        spv_errors += "info: line " + std::to_string(position.index) + ": " +
                      message + "\n";
        break;
      case SPV_MSG_DEBUG:
        break;
    }
  });
#endif  // AMBER_ENABLE_SPIRV_TOOLS

  std::vector<uint32_t> results;

  if (shader->GetFormat() == kShaderFormatSpirvHex) {
    Result r = ParseHex(shader->GetData(), &results);
    if (!r.IsSuccess())
      return {Result("Unable to parse shader hex."), {}};

#if AMBER_ENABLE_SHADERC
  } else if (shader->GetFormat() == kShaderFormatGlsl) {
    Result r = CompileGlsl(shader, &results);
    if (!r.IsSuccess())
      return {r, {}};
#endif  // AMBER_ENABLE_SHADERC

#if AMBER_ENABLE_SPIRV_TOOLS
  } else if (shader->GetFormat() == kShaderFormatSpirvAsm) {
    if (!tools.Assemble(shader->GetData(), &results,
                        spvtools::SpirvTools::kDefaultAssembleOption)) {
      return {Result("Shader assembly failed: " + spv_errors), {}};
    }
#endif  // AMBER_ENABLE_SPIRV_TOOLS

  } else {
    return {Result("Invalid shader format"), results};
  }

#if AMBER_ENABLE_SPIRV_TOOLS
  spvtools::ValidatorOptions options;
  if (!tools.Validate(results.data(), results.size(), options))
    return {Result("Invalid shader: " + spv_errors), {}};
#endif  // AMBER_ENABLE_SPIRV_TOOLS

  return {{}, results};
}

Result ShaderCompiler::ParseHex(const std::string& data,
                                std::vector<uint32_t>* result) const {
  size_t used = 0;
  const char* str = data.c_str();
  uint8_t converted = 0;
  uint32_t tmp = 0;
  while (used < data.length()) {
    char* new_pos = nullptr;
    uint64_t v = static_cast<uint64_t>(std::strtol(str, &new_pos, 16));

    ++converted;

    // TODO(dsinclair): Is this actually right?
    tmp = tmp | (static_cast<uint32_t>(v) << (8 * (converted - 1)));
    if (converted == 4) {
      result->push_back(tmp);
      tmp = 0;
      converted = 0;
    }

    used += static_cast<size_t>(new_pos - str);
    str = new_pos;
  }
  return {};
}

#if AMBER_ENABLE_SHADERC
Result ShaderCompiler::CompileGlsl(const Shader* shader,
                                   std::vector<uint32_t>* result) const {
  shaderc::Compiler compiler;
  shaderc::CompileOptions options;

  shaderc_shader_kind kind;
  if (shader->GetType() == kShaderTypeCompute)
    kind = shaderc_compute_shader;
  else if (shader->GetType() == kShaderTypeFragment)
    kind = shaderc_fragment_shader;
  else if (shader->GetType() == kShaderTypeGeometry)
    kind = shaderc_geometry_shader;
  else if (shader->GetType() == kShaderTypeVertex)
    kind = shaderc_vertex_shader;
  else if (shader->GetType() == kShaderTypeTessellationControl)
    kind = shaderc_tess_control_shader;
  else if (shader->GetType() == kShaderTypeTessellationEvaluation)
    kind = shaderc_tess_evaluation_shader;
  else
    return Result("Unknown shader type");

  shaderc::SpvCompilationResult module =
      compiler.CompileGlslToSpv(shader->GetData(), kind, "-", options);

  if (module.GetCompilationStatus() != shaderc_compilation_status_success)
    return Result(module.GetErrorMessage());

  std::copy(module.cbegin(), module.cend(), std::back_inserter(*result));
  return {};
}
#else
Result ShaderCompiler::CompileGlsl(const Shader*,
                                   std::vector<uint32_t>*) const {
  return {};
}
#endif  // AMBER_ENABLE_SHADERC

}  // namespace amber
