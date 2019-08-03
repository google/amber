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
#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "src/shader_data.h"
#if AMBER_ENABLE_SHADERC
#include "shaderc/env.h"
#endif

namespace amber {
namespace {

const char kHexShader[] =
    R"(0x03 0x02 0x23 0x07 0x00 0x00 0x01 0x00 0x07 0x00 0x08 0x00
   0x15 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x11 0x00 0x02 0x00
   0x01 0x00 0x00 0x00 0x0b 0x00 0x06 0x00 0x01 0x00 0x00 0x00
   0x47 0x4c 0x53 0x4c 0x2e 0x73 0x74 0x64 0x2e 0x34 0x35 0x30
   0x00 0x00 0x00 0x00 0x0e 0x00 0x03 0x00 0x00 0x00 0x00 0x00
   0x01 0x00 0x00 0x00 0x0f 0x00 0x07 0x00 0x00 0x00 0x00 0x00
   0x04 0x00 0x00 0x00 0x6d 0x61 0x69 0x6e 0x00 0x00 0x00 0x00
   0x0d 0x00 0x00 0x00 0x11 0x00 0x00 0x00 0x03 0x00 0x03 0x00
   0x02 0x00 0x00 0x00 0xae 0x01 0x00 0x00 0x05 0x00 0x04 0x00
   0x04 0x00 0x00 0x00 0x6d 0x61 0x69 0x6e 0x00 0x00 0x00 0x00
   0x05 0x00 0x06 0x00 0x0b 0x00 0x00 0x00 0x67 0x6c 0x5f 0x50
   0x65 0x72 0x56 0x65 0x72 0x74 0x65 0x78 0x00 0x00 0x00 0x00
   0x06 0x00 0x06 0x00 0x0b 0x00 0x00 0x00 0x00 0x00 0x00 0x00
   0x67 0x6c 0x5f 0x50 0x6f 0x73 0x69 0x74 0x69 0x6f 0x6e 0x00
   0x06 0x00 0x07 0x00 0x0b 0x00 0x00 0x00 0x01 0x00 0x00 0x00
   0x67 0x6c 0x5f 0x50 0x6f 0x69 0x6e 0x74 0x53 0x69 0x7a 0x65
   0x00 0x00 0x00 0x00 0x06 0x00 0x07 0x00 0x0b 0x00 0x00 0x00
   0x02 0x00 0x00 0x00 0x67 0x6c 0x5f 0x43 0x6c 0x69 0x70 0x44
   0x69 0x73 0x74 0x61 0x6e 0x63 0x65 0x00 0x05 0x00 0x03 0x00
   0x0d 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x05 0x00 0x05 0x00
   0x11 0x00 0x00 0x00 0x70 0x6f 0x73 0x69 0x74 0x69 0x6f 0x6e
   0x00 0x00 0x00 0x00 0x48 0x00 0x05 0x00 0x0b 0x00 0x00 0x00
   0x00 0x00 0x00 0x00 0x0b 0x00 0x00 0x00 0x00 0x00 0x00 0x00
   0x48 0x00 0x05 0x00 0x0b 0x00 0x00 0x00 0x01 0x00 0x00 0x00
   0x0b 0x00 0x00 0x00 0x01 0x00 0x00 0x00 0x48 0x00 0x05 0x00
   0x0b 0x00 0x00 0x00 0x02 0x00 0x00 0x00 0x0b 0x00 0x00 0x00
   0x03 0x00 0x00 0x00 0x47 0x00 0x03 0x00 0x0b 0x00 0x00 0x00
   0x02 0x00 0x00 0x00 0x47 0x00 0x04 0x00 0x11 0x00 0x00 0x00
   0x1e 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x13 0x00 0x02 0x00
   0x02 0x00 0x00 0x00 0x21 0x00 0x03 0x00 0x03 0x00 0x00 0x00
   0x02 0x00 0x00 0x00 0x16 0x00 0x03 0x00 0x06 0x00 0x00 0x00
   0x20 0x00 0x00 0x00 0x17 0x00 0x04 0x00 0x07 0x00 0x00 0x00
   0x06 0x00 0x00 0x00 0x04 0x00 0x00 0x00 0x15 0x00 0x04 0x00
   0x08 0x00 0x00 0x00 0x20 0x00 0x00 0x00 0x00 0x00 0x00 0x00
   0x2b 0x00 0x04 0x00 0x08 0x00 0x00 0x00 0x09 0x00 0x00 0x00
   0x01 0x00 0x00 0x00 0x1c 0x00 0x04 0x00 0x0a 0x00 0x00 0x00
   0x06 0x00 0x00 0x00 0x09 0x00 0x00 0x00 0x1e 0x00 0x05 0x00
   0x0b 0x00 0x00 0x00 0x07 0x00 0x00 0x00 0x06 0x00 0x00 0x00
   0x0a 0x00 0x00 0x00 0x20 0x00 0x04 0x00 0x0c 0x00 0x00 0x00
   0x03 0x00 0x00 0x00 0x0b 0x00 0x00 0x00 0x3b 0x00 0x04 0x00
   0x0c 0x00 0x00 0x00 0x0d 0x00 0x00 0x00 0x03 0x00 0x00 0x00
   0x15 0x00 0x04 0x00 0x0e 0x00 0x00 0x00 0x20 0x00 0x00 0x00
   0x01 0x00 0x00 0x00 0x2b 0x00 0x04 0x00 0x0e 0x00 0x00 0x00
   0x0f 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x20 0x00 0x04 0x00
   0x10 0x00 0x00 0x00 0x01 0x00 0x00 0x00 0x07 0x00 0x00 0x00
   0x3b 0x00 0x04 0x00 0x10 0x00 0x00 0x00 0x11 0x00 0x00 0x00
   0x01 0x00 0x00 0x00 0x20 0x00 0x04 0x00 0x13 0x00 0x00 0x00
   0x03 0x00 0x00 0x00 0x07 0x00 0x00 0x00 0x36 0x00 0x05 0x00
   0x02 0x00 0x00 0x00 0x04 0x00 0x00 0x00 0x00 0x00 0x00 0x00
   0x03 0x00 0x00 0x00 0xf8 0x00 0x02 0x00 0x05 0x00 0x00 0x00
   0x3d 0x00 0x04 0x00 0x07 0x00 0x00 0x00 0x12 0x00 0x00 0x00
   0x11 0x00 0x00 0x00 0x41 0x00 0x05 0x00 0x13 0x00 0x00 0x00
   0x14 0x00 0x00 0x00 0x0d 0x00 0x00 0x00 0x0f 0x00 0x00 0x00
   0x3e 0x00 0x03 0x00 0x14 0x00 0x00 0x00 0x12 0x00 0x00 0x00
   0xfd 0x00 0x01 0x00 0x38 0x00 0x01 0x00)";

}  // namespace

using ShaderCompilerTest = testing::Test;

#if AMBER_ENABLE_SHADERC
TEST_F(ShaderCompilerTest, CompilesGlsl) {
  std::string contents = R"(
#version 420
layout(location = 0) in vec4 position;

void main() {
  gl_Position = position;
})";

  Shader shader(kShaderTypeVertex);
  shader.SetName("TestShader");
  shader.SetFormat(kShaderFormatGlsl);
  shader.SetData(contents);

  ShaderCompiler sc;
  Result r;
  std::vector<uint32_t> binary;
  Pipeline::ShaderInfo shader_info(&shader, kShaderTypeCompute);
  std::tie(r, binary) = sc.Compile(&shader_info, ShaderMap());
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  EXPECT_FALSE(binary.empty());
  EXPECT_EQ(0x07230203, binary[0]);  // Verify SPIR-V header present.
}
#endif  // AMBER_ENABLE_SHADERC

#if AMBER_ENABLE_SPIRV_TOOLS
TEST_F(ShaderCompilerTest, CompilesSpirvAsm) {
  Shader shader(kShaderTypeVertex);
  shader.SetName("TestShader");
  shader.SetFormat(kShaderFormatSpirvAsm);
  shader.SetData(kPassThroughShader);

  ShaderCompiler sc;
  Result r;
  std::vector<uint32_t> binary;
  Pipeline::ShaderInfo shader_info(&shader, kShaderTypeCompute);
  std::tie(r, binary) = sc.Compile(&shader_info, ShaderMap());
  ASSERT_TRUE(r.IsSuccess());
  EXPECT_FALSE(binary.empty());
  EXPECT_EQ(0x07230203, binary[0]);  // Verify SPIR-V header present.
}

TEST_F(ShaderCompilerTest, InvalidSpirvHex) {
  std::string contents = kHexShader;
  contents[3] = '0';

  Shader shader(kShaderTypeVertex);
  shader.SetName("BadTestShader");
  shader.SetFormat(kShaderFormatSpirvHex);
  shader.SetData(contents);

  ShaderCompiler sc;
  Result r;
  std::vector<uint32_t> binary;
  Pipeline::ShaderInfo shader_info(&shader, kShaderTypeCompute);
  std::tie(r, binary) = sc.Compile(&shader_info, ShaderMap());
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("Invalid shader: error: line 0: Invalid SPIR-V magic number.\n",
            r.Error());
}

TEST_F(ShaderCompilerTest, InvalidHex) {
  Shader shader(kShaderTypeVertex);
  shader.SetName("BadTestShader");
  shader.SetFormat(kShaderFormatSpirvHex);
  shader.SetData("aaaaaaaaaa");

  ShaderCompiler sc;
  Result r;
  std::vector<uint32_t> binary;
  Pipeline::ShaderInfo shader_info(&shader, kShaderTypeCompute);
  std::tie(r, binary) = sc.Compile(&shader_info, ShaderMap());
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("Invalid shader: error: line 0: Invalid SPIR-V magic number.\n",
            r.Error());
}

TEST_F(ShaderCompilerTest, OptimizeShader) {
  const std::string spirv = R"(
OpCapability Shader
OpExtension "SPV_KHR_storage_buffer_storage_class"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpDecorate %block Block
OpMemberDecorate %block 0 Offset 0
OpDecorate %in DescriptorSet 0
OpDecorate %in Binding 0
OpDecorate %out DescriptorSet 0
OpDecorate %out Binding 1
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%block = OpTypeStruct %int
%ptr_ssbo_block = OpTypePointer StorageBuffer %block
%ptr_ssbo_int = OpTypePointer StorageBuffer %int
%in = OpVariable %ptr_ssbo_block StorageBuffer
%out = OpVariable %ptr_ssbo_block StorageBuffer
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%in_gep = OpAccessChain %ptr_ssbo_int %in %int_0
%ld = OpLoad %int %in_gep
%dead = OpIAdd %int %ld %int_0
%out_gep = OpAccessChain %ptr_ssbo_int %out %int_0
OpStore %out_gep %ld
OpReturn
OpFunctionEnd
)";

  Shader shader(kShaderTypeCompute);
  shader.SetName("TestShader");
  shader.SetFormat(kShaderFormatSpirvAsm);
  shader.SetData(spirv);

  Pipeline::ShaderInfo unoptimized(&shader, kShaderTypeCompute);
  Pipeline::ShaderInfo optimized(&shader, kShaderTypeCompute);
  optimized.SetShaderOptimizations({"--eliminate-dead-code-aggressive"});

  ShaderCompiler sc;
  Result r;
  std::vector<uint32_t> unopt_binary;
  std::tie(r, unopt_binary) = sc.Compile(&unoptimized, ShaderMap());
  ASSERT_TRUE(r.IsSuccess());

  std::vector<uint32_t> opt_binary;
  std::tie(r, opt_binary) = sc.Compile(&optimized, ShaderMap());
  ASSERT_TRUE(r.IsSuccess());
  EXPECT_NE(opt_binary.size(), unopt_binary.size());
}
#endif  // AMBER_ENABLE_SPIRV_TOOLS

TEST_F(ShaderCompilerTest, CompilesSpirvHex) {
  Shader shader(kShaderTypeVertex);
  shader.SetName("TestShader");
  shader.SetFormat(kShaderFormatSpirvHex);
  shader.SetData(kHexShader);

  ShaderCompiler sc;
  Result r;
  std::vector<uint32_t> binary;
  Pipeline::ShaderInfo shader_info(&shader, kShaderTypeCompute);
  std::tie(r, binary) = sc.Compile(&shader_info, ShaderMap());
  ASSERT_TRUE(r.IsSuccess());
  EXPECT_FALSE(binary.empty());
  EXPECT_EQ(0x07230203, binary[0]);  // Verify SPIR-V header present.
}

TEST_F(ShaderCompilerTest, FailsOnInvalidShader) {
  std::string contents = "Just Random\nText()\nThat doesn't work.";

  Shader shader(kShaderTypeVertex);
  shader.SetName("BadTestShader");
  shader.SetFormat(kShaderFormatGlsl);
  shader.SetData(contents);

  ShaderCompiler sc;
  Result r;
  std::vector<uint32_t> binary;
  Pipeline::ShaderInfo shader_info(&shader, kShaderTypeCompute);
  std::tie(r, binary) = sc.Compile(&shader_info, ShaderMap());
  ASSERT_FALSE(r.IsSuccess());
}

TEST_F(ShaderCompilerTest, ReturnsCachedShader) {
  // This shader would normally fail, but because we pull it from the cache,
  // we don't compile this so the test will pass.
  std::string contents = "Just Random\nText()\nThat doesn't work.";

  static const char kShaderName[] = "CachedShader";
  Shader shader(kShaderTypeVertex);
  shader.SetName(kShaderName);
  shader.SetFormat(kShaderFormatGlsl);
  shader.SetData(contents);

  std::vector<uint32_t> src_bytes = {1, 2, 3, 4, 5};

  ShaderMap map;
  map[kShaderName] = src_bytes;

  ShaderCompiler sc;
  Result r;
  std::vector<uint32_t> binary;
  Pipeline::ShaderInfo shader_info(&shader, kShaderTypeCompute);
  std::tie(r, binary) = sc.Compile(&shader_info, map);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  ASSERT_EQ(binary.size(), src_bytes.size());
  for (size_t i = 0; i < src_bytes.size(); ++i) {
    EXPECT_EQ(src_bytes[i], binary[i]);
  }
}

#if AMBER_ENABLE_CLSPV
TEST_F(ShaderCompilerTest, ClspvCompile) {
  Shader shader(kShaderTypeCompute);
  shader.SetName("TestShader");
  shader.SetFormat(kShaderFormatOpenCLC);
  shader.SetData(R"(
kernel void TestShader(global int* in, global int* out) {
  *out = *in;
}
  )");

  ShaderCompiler sc;
  Result r;
  std::vector<uint32_t> binary;
  Pipeline::ShaderInfo shader_info(&shader, kShaderTypeCompute);
  std::tie(r, binary) = sc.Compile(&shader_info, ShaderMap());
  ASSERT_TRUE(r.IsSuccess());
  EXPECT_FALSE(binary.empty());
  EXPECT_EQ(0x07230203, binary[0]);  // Verify SPIR-V header present.
}

TEST_F(ShaderCompilerTest, ClspvDisallowCaching) {
  Shader shader(kShaderTypeCompute);
  std::string name = "TestShader";
  shader.SetName(name);
  shader.SetFormat(kShaderFormatOpenCLC);
  shader.SetData(R"(
kernel void TestShader(global int* in, global int* out) {
  *out = *in;
}
  )");

  std::vector<uint32_t> src_bytes = {1, 2, 3, 4, 5};

  ShaderMap map;
  map[name] = src_bytes;

  ShaderCompiler sc;
  Result r;
  std::vector<uint32_t> binary;
  Pipeline::ShaderInfo shader_info(&shader, kShaderTypeCompute);
  std::tie(r, binary) = sc.Compile(&shader_info, map);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_TRUE(binary.empty());
}

TEST_F(ShaderCompilerTest, ClspvCompileOptions) {
  std::string data = R"(
kernel void TestShader(global int* in, global int* out, int m, int b) {
  *out = *in * m + b;
}
)";
  Shader shader(kShaderTypeCompute);
  shader.SetName("TestShader");
  shader.SetFormat(kShaderFormatOpenCLC);
  shader.SetData(data);

  ShaderCompiler sc;
  Result r;
  std::vector<uint32_t> binary;
  Pipeline::ShaderInfo shader_info1(&shader, kShaderTypeCompute);
  std::tie(r, binary) = sc.Compile(&shader_info1, ShaderMap());
  ASSERT_TRUE(r.IsSuccess());
  EXPECT_FALSE(binary.empty());
  EXPECT_EQ(0x07230203, binary[0]);  // Verify SPIR-V header present.
  auto iter = shader_info1.GetDescriptorMap().find("TestShader");
  ASSERT_NE(iter, shader_info1.GetDescriptorMap().end());
  uint32_t max_binding = 0;
  bool has_pod_ubo = 0;
  for (const auto& entry : iter->second) {
    max_binding = std::max(max_binding, entry.binding);
    has_pod_ubo =
        entry.kind == Pipeline::ShaderInfo::DescriptorMapEntry::Kind::POD_UBO;
  }
  EXPECT_EQ(3U, max_binding);
  EXPECT_FALSE(has_pod_ubo);

  binary.clear();
  Pipeline::ShaderInfo shader_info2(&shader, kShaderTypeCompute);
  shader_info2.SetCompileOptions({"-cluster-pod-kernel-args", "-pod-ubo"});
  std::tie(r, binary) = sc.Compile(&shader_info2, ShaderMap());
  ASSERT_TRUE(r.IsSuccess());
  EXPECT_FALSE(binary.empty());
  EXPECT_EQ(0x07230203, binary[0]);  // Verify SPIR-V header present.
  iter = shader_info2.GetDescriptorMap().find("TestShader");
  ASSERT_NE(iter, shader_info2.GetDescriptorMap().end());
  max_binding = 0;
  has_pod_ubo = 0;
  for (const auto& entry : iter->second) {
    max_binding = std::max(max_binding, entry.binding);
    has_pod_ubo =
        entry.kind == Pipeline::ShaderInfo::DescriptorMapEntry::Kind::POD_UBO;
  }
  EXPECT_EQ(2U, max_binding);
  EXPECT_TRUE(has_pod_ubo);
}
#endif  // AMBER_ENABLE_CLSPV

struct ParseSpvEnvCase {
  std::string env_str;
  bool ok;
  uint32_t target_env;
  uint32_t env_version;
  uint32_t spirv_version;
};

using ParseSpvEnvTest = ::testing::TestWithParam<ParseSpvEnvCase>;

TEST_P(ParseSpvEnvTest, Samples) {
  uint32_t target_env = 42u;
  uint32_t env_version = 43u;
  uint32_t spirv_version = 44u;
  auto r = amber::ParseSpvEnv(GetParam().env_str, &target_env, &env_version,
                              &spirv_version);
  if (GetParam().ok) {
    EXPECT_TRUE(r.IsSuccess());
    EXPECT_EQ(GetParam().target_env, target_env) << GetParam().env_str;
    EXPECT_EQ(GetParam().env_version, env_version) << GetParam().env_str;
    EXPECT_EQ(GetParam().spirv_version, spirv_version) << GetParam().env_str;
  } else {
    EXPECT_FALSE(r.IsSuccess());
  }
}

// See also shaderc/env.h
const uint32_t vulkan = 0;
const uint32_t vulkan_1_0 = ((uint32_t(1) << 22));
const uint32_t vulkan_1_1 = ((uint32_t(1) << 22) | (1 << 12));
const uint32_t spv_1_0 = uint32_t(0x10000);
const uint32_t spv_1_1 = uint32_t(0x10100);
const uint32_t spv_1_2 = uint32_t(0x10200);
const uint32_t spv_1_3 = uint32_t(0x10300);
const uint32_t spv_1_4 = uint32_t(0x10400);

INSTANTIATE_TEST_SUITE_P(ParseSpvEnvFailures,
                         ParseSpvEnvTest,
                         ::testing::ValuesIn(std::vector<ParseSpvEnvCase>{
                             {"foobar", false, 0u, 0u, 0u},
                             {"spv99", false, 0u, 0u, 0u},
                             {"spv99.9", false, 0u, 0u, 0u},
                             {"spv1.0.1", false, 0u, 0u, 0u},
                             {"spv1.0.1", false, 0u, 0u, 0u},
                             {"spv1.5", false, 0u, 0u, 0u},
                             {"vulkan99", false, 0u, 0u, 0u},
                             {"vulkan99.9", false, 0u, 0u, 0u},
                         }));

INSTANTIATE_TEST_SUITE_P(ParseSpvEnvSuccesses,
                         ParseSpvEnvTest,
                         ::testing::ValuesIn(std::vector<ParseSpvEnvCase>{
                             {"", true, vulkan, vulkan_1_0, spv_1_0},
                             {"spv1.0", true, vulkan, vulkan_1_0, spv_1_0},
                             {"spv1.1", true, vulkan, vulkan_1_1, spv_1_1},
                             {"spv1.2", true, vulkan, vulkan_1_1, spv_1_2},
                             {"spv1.3", true, vulkan, vulkan_1_1, spv_1_3},
                             {"spv1.4", true, vulkan, vulkan_1_1, spv_1_4},
                             {"vulkan1.0", true, vulkan, vulkan_1_0, spv_1_0},
                             {"vulkan1.1", true, vulkan, vulkan_1_1, spv_1_3},
                             {"vulkan1.1spv1.4", true, vulkan, vulkan_1_1,
                              spv_1_4},
                         }));

}  // namespace amber
