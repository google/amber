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
#include "spirv-tools/optimizer.hpp"
#endif  // AMBER_ENABLE_SPIRV_TOOLS

#if AMBER_ENABLE_SHADERC
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wshadow-uncaptured-local"
#pragma clang diagnostic ignored "-Wweak-vtables"
#include "shaderc/shaderc.hpp"
#pragma clang diagnostic pop
#endif  // AMBER_ENABLE_SHADERC

#if AMBER_ENABLE_DXC
#include "src/dxc_helper.h"
#endif  // AMBER_ENABLE_DXC

#if AMBER_ENABLE_CLSPV
#include "src/clspv_helper.h"
#endif  // AMBER_ENABLE_CLSPV

namespace amber {

ShaderCompiler::ShaderCompiler() = default;

ShaderCompiler::ShaderCompiler(const std::string& env,
                               bool disable_spirv_validation,
                               VirtualFileStore* virtual_files)
    : spv_env_(env),
      disable_spirv_validation_(disable_spirv_validation),
      virtual_files_(virtual_files) {
  // Do not warn about virtual_files_ not being used.
  // This is conditionally used based on preprocessor defines.
  (void)virtual_files_;
}

ShaderCompiler::~ShaderCompiler() = default;

std::pair<Result, std::vector<uint32_t>> ShaderCompiler::Compile(
    Pipeline* pipeline,
    Pipeline::ShaderInfo* shader_info,
    const ShaderMap& shader_map) const {
  const auto shader = shader_info->GetShader();
  std::string key = shader->GetName();
  const std::string pipeline_name = pipeline->GetName();
  if (pipeline_name != "") {
    key = pipeline_name + "-" + key;
  }
  auto it = shader_map.find(key);
  if (it != shader_map.end()) {
#if AMBER_ENABLE_CLSPV
    if (shader->GetFormat() == kShaderFormatOpenCLC) {
      return {Result("OPENCL-C shaders do not support pre-compiled shaders"),
              {}};
    }
#endif  // AMBER_ENABLE_CLSPV
    return {{}, it->second};
  }

#if AMBER_ENABLE_SPIRV_TOOLS
  std::string spv_errors;

  spv_target_env target_env = SPV_ENV_UNIVERSAL_1_0;
  if (!spv_env_.empty()) {
    if (!spvParseTargetEnv(spv_env_.c_str(), &target_env))
      return {Result("Unable to parse SPIR-V target environment"), {}};
  }

  auto msg_consumer = [&spv_errors](spv_message_level_t level, const char*,
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
  };

  spvtools::SpirvTools tools(target_env);
  tools.SetMessageConsumer(msg_consumer);
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

#if AMBER_ENABLE_DXC
  } else if (shader->GetFormat() == kShaderFormatHlsl) {
    Result r = CompileHlsl(shader, &results);
    if (!r.IsSuccess())
      return {r, {}};
#endif  // AMBER_ENABLE_DXC

#if AMBER_ENABLE_SPIRV_TOOLS
  } else if (shader->GetFormat() == kShaderFormatSpirvAsm) {
    if (!tools.Assemble(shader->GetData(), &results,
                        spvtools::SpirvTools::kDefaultAssembleOption)) {
      return {Result("Shader assembly failed: " + spv_errors), {}};
    }
#endif  // AMBER_ENABLE_SPIRV_TOOLS

#if AMBER_ENABLE_CLSPV
  } else if (shader->GetFormat() == kShaderFormatOpenCLC) {
    Result r = CompileOpenCLC(shader_info, pipeline, target_env, &results);
    if (!r.IsSuccess())
      return {r, {}};
#endif  // AMBER_ENABLE_CLSPV

  } else {
    return {Result("Invalid shader format"), results};
  }

  // Validate the shader, but have an option to disable that.
  // Always use the data member, to avoid an unused-variable warning
  // when not using SPIRV-Tools support.
  if (!disable_spirv_validation_) {
#if AMBER_ENABLE_SPIRV_TOOLS
    spvtools::ValidatorOptions options;
    if (!tools.Validate(results.data(), results.size(), options))
      return {Result("Invalid shader: " + spv_errors), {}};
#endif  // AMBER_ENABLE_SPIRV_TOOLS
  }

#if AMBER_ENABLE_SPIRV_TOOLS
  // Optimize the shader if any optimizations were specified.
  if (!shader_info->GetShaderOptimizations().empty()) {
    spvtools::Optimizer optimizer(target_env);
    optimizer.SetMessageConsumer(msg_consumer);
    if (!optimizer.RegisterPassesFromFlags(
            shader_info->GetShaderOptimizations())) {
      return {Result("Invalid optimizations: " + spv_errors), {}};
    }
    if (!optimizer.Run(results.data(), results.size(), &results))
      return {Result("Optimizations failed: " + spv_errors), {}};
  }
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

  uint32_t env = 0u;
  uint32_t env_version = 0u;
  uint32_t spirv_version = 0u;
  auto r = ParseSpvEnv(spv_env_, &env, &env_version, &spirv_version);
  if (!r.IsSuccess())
    return r;

  options.SetTargetEnvironment(static_cast<shaderc_target_env>(env),
                               env_version);
  options.SetTargetSpirv(static_cast<shaderc_spirv_version>(spirv_version));

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
  else if (shader->GetType() == kShaderTypeRayGeneration)
    kind = shaderc_raygen_shader;
  else if (shader->GetType() == kShaderTypeAnyHit)
    kind = shaderc_anyhit_shader;
  else if (shader->GetType() == kShaderTypeClosestHit)
    kind = shaderc_closesthit_shader;
  else if (shader->GetType() == kShaderTypeMiss)
    kind = shaderc_miss_shader;
  else if (shader->GetType() == kShaderTypeIntersection)
    kind = shaderc_intersection_shader;
  else if (shader->GetType() == kShaderTypeCall)
    kind = shaderc_callable_shader;
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

#if AMBER_ENABLE_DXC
Result ShaderCompiler::CompileHlsl(const Shader* shader,
                                   std::vector<uint32_t>* result) const {
  std::string target;
  if (shader->GetType() == kShaderTypeCompute)
    target = "cs_6_2";
  else if (shader->GetType() == kShaderTypeFragment)
    target = "ps_6_2";
  else if (shader->GetType() == kShaderTypeGeometry)
    target = "gs_6_2";
  else if (shader->GetType() == kShaderTypeVertex)
    target = "vs_6_2";
  else
    return Result("Unknown shader type");

  return dxchelper::Compile(shader->GetData(), "main", target, spv_env_,
                            shader->GetFilePath(), virtual_files_, result);
}
#else
Result ShaderCompiler::CompileHlsl(const Shader*,
                                   std::vector<uint32_t>*) const {
  return {};
}
#endif  // AMBER_ENABLE_DXC

#if AMBER_ENABLE_CLSPV
Result ShaderCompiler::CompileOpenCLC(Pipeline::ShaderInfo* shader_info,
                                      Pipeline* pipeline,
                                      spv_target_env env,
                                      std::vector<uint32_t>* result) const {
  return clspvhelper::Compile(shader_info, pipeline, env, result);
}
#endif  // AMBER_ENABLE_CLSPV

namespace {

// Value for the Vulkan API, used in the Shaderc API
const uint32_t kVulkan = 0;
// Values for versions of the Vulkan API, used in the Shaderc API
const uint32_t kVulkan_1_0 = (uint32_t(1) << 22);
const uint32_t kVulkan_1_1 = (uint32_t(1) << 22) | (1 << 12);
const uint32_t kVulkan_1_2 = (uint32_t(1) << 22) | (2 << 12);
// Values for SPIR-V versions, used in the Shaderc API
const uint32_t kSpv_1_0 = uint32_t(0x10000);
const uint32_t kSpv_1_1 = uint32_t(0x10100);
const uint32_t kSpv_1_2 = uint32_t(0x10200);
const uint32_t kSpv_1_3 = uint32_t(0x10300);
const uint32_t kSpv_1_4 = uint32_t(0x10400);
const uint32_t kSpv_1_5 = uint32_t(0x10500);

#if AMBER_ENABLE_SHADERC
// Check that we have the right values, from the original definitions
// in the Shaderc API.
static_assert(kVulkan == shaderc_target_env_vulkan,
              "enum vulkan* value mismatch");
static_assert(kVulkan_1_0 == shaderc_env_version_vulkan_1_0,
              "enum vulkan1.0 value mismatch");
static_assert(kVulkan_1_1 == shaderc_env_version_vulkan_1_1,
              "enum vulkan1.1 value mismatch");
static_assert(kVulkan_1_2 == shaderc_env_version_vulkan_1_2,
              "enum vulkan1.2 value mismatch");
static_assert(kSpv_1_0 == shaderc_spirv_version_1_0,
              "enum spv1.0 value mismatch");
static_assert(kSpv_1_1 == shaderc_spirv_version_1_1,
              "enum spv1.1 value mismatch");
static_assert(kSpv_1_2 == shaderc_spirv_version_1_2,
              "enum spv1.2 value mismatch");
static_assert(kSpv_1_3 == shaderc_spirv_version_1_3,
              "enum spv1.3 value mismatch");
static_assert(kSpv_1_4 == shaderc_spirv_version_1_4,
              "enum spv1.4 value mismatch");
static_assert(kSpv_1_5 == shaderc_spirv_version_1_5,
              "enum spv1.5 value mismatch");
#endif

}  // namespace

Result ParseSpvEnv(const std::string& spv_env,
                   uint32_t* target_env,
                   uint32_t* target_env_version,
                   uint32_t* spirv_version) {
  if (!target_env || !target_env_version || !spirv_version)
    return Result("ParseSpvEnv: null pointer parameter");

  // Use the same values as in Shaderc's shaderc/env.h
  struct Values {
    uint32_t env;
    uint32_t env_version;
    uint32_t spirv_version;
  };
  Values values{kVulkan, kVulkan_1_0, kSpv_1_0};

  if (spv_env == "" || spv_env == "spv1.0") {
    values = {kVulkan, kVulkan_1_0, kSpv_1_0};
  } else if (spv_env == "spv1.1") {
    values = {kVulkan, kVulkan_1_1, kSpv_1_1};
  } else if (spv_env == "spv1.2") {
    values = {kVulkan, kVulkan_1_1, kSpv_1_2};
  } else if (spv_env == "spv1.3") {
    values = {kVulkan, kVulkan_1_1, kSpv_1_3};
  } else if (spv_env == "spv1.4") {
    // Vulkan 1.2 requires support for SPIR-V 1.4,
    // but Vulkan 1.1 permits it with an extension.
    // So Vulkan 1.2 is the right answer here.
    values = {kVulkan, kVulkan_1_2, kSpv_1_4};
  } else if (spv_env == "spv1.5") {
    values = {kVulkan, kVulkan_1_2, kSpv_1_5};
  } else if (spv_env == "vulkan1.0") {
    values = {kVulkan, kVulkan_1_0, kSpv_1_0};
  } else if (spv_env == "vulkan1.1") {
    // Vulkan 1.1 requires support for SPIR-V 1.3.
    values = {kVulkan, kVulkan_1_1, kSpv_1_3};
  } else if (spv_env == "vulkan1.1spv1.4") {
    values = {kVulkan, kVulkan_1_1, kSpv_1_4};
  } else if (spv_env == "vulkan1.2") {
    values = {kVulkan, kVulkan_1_2, kSpv_1_5};
  } else {
    return Result(std::string("Unrecognized environment ") + spv_env);
  }

  *target_env = values.env;
  *target_env_version = values.env_version;
  *spirv_version = values.spirv_version;
  return {};
}

}  // namespace amber
