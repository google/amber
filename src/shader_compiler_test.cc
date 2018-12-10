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

#include <string>

#include "gtest/gtest.h"
#include "src/shader_compiler.h"
#include "src/shader_data.h"

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

  Shader shader(ShaderType::kVertex);
  shader.SetName("TestShader");
  shader.SetFormat(ShaderFormat::kGlsl);
  shader.SetData(contents);

  ShaderCompiler sc;
  Result r;
  std::vector<uint32_t> binary;
  std::tie(r, binary) = sc.Compile(&shader, ShaderMap());
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  EXPECT_FALSE(binary.empty());
  EXPECT_EQ(0x07230203, binary[0]);  // Verify SPIR-V header present.
}
#endif  // AMBER_ENABLE_SHADERC

#if AMBER_ENABLE_SPIRV_TOOLS
TEST_F(ShaderCompilerTest, CompilesSpirvAsm) {
  Shader shader(ShaderType::kVertex);
  shader.SetName("TestShader");
  shader.SetFormat(ShaderFormat::kSpirvAsm);
  shader.SetData(kPassThroughShader);

  ShaderCompiler sc;
  Result r;
  std::vector<uint32_t> binary;
  std::tie(r, binary) = sc.Compile(&shader, ShaderMap());
  ASSERT_TRUE(r.IsSuccess());
  EXPECT_FALSE(binary.empty());
  EXPECT_EQ(0x07230203, binary[0]);  // Verify SPIR-V header present.
}

TEST_F(ShaderCompilerTest, InvalidSpirvHex) {
  std::string contents = kHexShader;
  contents[3] = '0';

  Shader shader(ShaderType::kVertex);
  shader.SetName("BadTestShader");
  shader.SetFormat(ShaderFormat::kSpirvHex);
  shader.SetData(contents);

  ShaderCompiler sc;
  Result r;
  std::vector<uint32_t> binary;
  std::tie(r, binary) = sc.Compile(&shader, ShaderMap());
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("Invalid shader: error: line 0: Invalid SPIR-V magic number.\n",
            r.Error());
}

TEST_F(ShaderCompilerTest, InvalidHex) {
  Shader shader(ShaderType::kVertex);
  shader.SetName("BadTestShader");
  shader.SetFormat(ShaderFormat::kSpirvHex);
  shader.SetData("aaaaaaaaaa");

  ShaderCompiler sc;
  Result r;
  std::vector<uint32_t> binary;
  std::tie(r, binary) = sc.Compile(&shader, ShaderMap());
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("Invalid shader: error: line 0: Invalid SPIR-V magic number.\n",
            r.Error());
}
#endif  // AMBER_ENABLE_SPIRV_TOOLS

TEST_F(ShaderCompilerTest, CompilesSpirvHex) {
  Shader shader(ShaderType::kVertex);
  shader.SetName("TestShader");
  shader.SetFormat(ShaderFormat::kSpirvHex);
  shader.SetData(kHexShader);

  ShaderCompiler sc;
  Result r;
  std::vector<uint32_t> binary;
  std::tie(r, binary) = sc.Compile(&shader, ShaderMap());
  ASSERT_TRUE(r.IsSuccess());
  EXPECT_FALSE(binary.empty());
  EXPECT_EQ(0x07230203, binary[0]);  // Verify SPIR-V header present.
}

TEST_F(ShaderCompilerTest, FailsOnInvalidShader) {
  std::string contents = "Just Random\nText()\nThat doesn't work.";

  Shader shader(ShaderType::kVertex);
  shader.SetName("BadTestShader");
  shader.SetFormat(ShaderFormat::kGlsl);
  shader.SetData(contents);

  ShaderCompiler sc;
  Result r;
  std::vector<uint32_t> binary;
  std::tie(r, binary) = sc.Compile(&shader, ShaderMap());
  ASSERT_FALSE(r.IsSuccess());
}

TEST_F(ShaderCompilerTest, ReturnsCachedShader) {
  // This shader would normally fail, but because we pull it from the cache,
  // we don't compile this so the test will pass.
  std::string contents = "Just Random\nText()\nThat doesn't work.";

  static const char kShaderName[] = "CachedShader";
  Shader shader(ShaderType::kVertex);
  shader.SetName(kShaderName);
  shader.SetFormat(ShaderFormat::kGlsl);
  shader.SetData(contents);

  std::vector<uint32_t> src_bytes = {1, 2, 3, 4, 5};

  ShaderMap map;
  map[kShaderName] = src_bytes;

  ShaderCompiler sc;
  Result r;
  std::vector<uint32_t> binary;
  std::tie(r, binary) = sc.Compile(&shader, map);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  ASSERT_EQ(binary.size(), src_bytes.size());
  for (size_t i = 0; i < src_bytes.size(); ++i) {
    EXPECT_EQ(src_bytes[i], binary[i]);
  }
}

}  // namespace amber
