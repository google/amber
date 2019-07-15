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
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or parseried.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "gtest/gtest.h"
#include "src/amberscript/parser.h"
#include "src/shader_data.h"

namespace amber {
namespace amberscript {

using AmberScriptParserTest = testing::Test;

TEST_F(AmberScriptParserTest, ShaderPassThrough) {
  std::string in = "SHADER vertex my_shader1 PASSTHROUGH";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& shaders = script->GetShaders();
  ASSERT_EQ(1U, shaders.size());

  const auto* shader = shaders[0].get();
  EXPECT_EQ("my_shader1", shader->GetName());
  EXPECT_EQ(kShaderTypeVertex, shader->GetType());
  EXPECT_EQ(kShaderFormatSpirvAsm, shader->GetFormat());
  EXPECT_EQ(kPassThroughShader, shader->GetData());
}

TEST_F(AmberScriptParserTest, ShaderInvalidShaderTypeToken) {
  std::string in = "SHADER 1234 my_shader PASSTHROUGH";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: invalid token when looking for shader type", r.Error());
}

TEST_F(AmberScriptParserTest, ShaderInvalidShaderNameToken) {
  std::string in = "SHADER vertex 12345 PASSTHROUGH";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: invalid token when looking for shader name", r.Error());
}

TEST_F(AmberScriptParserTest, ShaderInvalidShaderFormatToken) {
  std::string in = "SHADER vertex my_shader 1234";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: invalid token when looking for shader format", r.Error());
}

struct NameData {
  const char* name;
};

using AmberScriptParserShaderPassThroughTest = testing::TestWithParam<NameData>;
TEST_P(AmberScriptParserShaderPassThroughTest, ShaderPassThroughWithoutVertex) {
  auto test_data = GetParam();

  std::string in =
      "SHADER " + std::string(test_data.name) + " my_shader PASSTHROUGH";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ(
      "1: invalid shader type for PASSTHROUGH. Only vertex PASSTHROUGH "
      "allowed",
      r.Error());
}
INSTANTIATE_TEST_SUITE_P(
    AmberScriptParserShaderPassThroughTests,
    AmberScriptParserShaderPassThroughTest,
    testing::Values(NameData{"fragment"},
                    NameData{"geometry"},
                    NameData{"tessellation_evaluation"},
                    NameData{"tessellation_control"},
                    NameData{"compute"},
                    NameData{"multi"}));  // NOLINT(whitespace/parens)

TEST_F(AmberScriptParserTest, ShaderPassThroughUnknownShaderType) {
  std::string in = "SHADER UNKNOWN my_shader PASSTHROUGH";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: unknown shader type: UNKNOWN", r.Error());
}

TEST_F(AmberScriptParserTest, ShaderPassThroughMissingName) {
  std::string in = "SHADER vertex PASSTHROUGH";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: invalid token when looking for shader format", r.Error());
}

TEST_F(AmberScriptParserTest, ShaderPassThroughExtraParameters) {
  std::string in = "SHADER vertex my_shader PASSTHROUGH INVALID";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: extra parameters after SHADER PASSTHROUGH", r.Error());
}

TEST_F(AmberScriptParserTest, Shader) {
  std::string shader_result = R"(
# Shader has a comment in it.
void main() {
  gl_FragColor = vec3(2, 3, 4);
}
)";

  std::string in = R"(#!amber
SHADER geometry shader_name GLSL
)" + shader_result +
                   "END";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& shaders = script->GetShaders();
  ASSERT_EQ(1U, shaders.size());

  const auto* shader = shaders[0].get();
  EXPECT_EQ("shader_name", shader->GetName());
  EXPECT_EQ(kShaderTypeGeometry, shader->GetType());
  EXPECT_EQ(kShaderFormatGlsl, shader->GetFormat());
  EXPECT_EQ(shader_result, shader->GetData());
}

TEST_F(AmberScriptParserTest, ShaderInvalidFormat) {
  std::string in = R"(#!amber
SHADER geometry shader_name INVALID
# Shader has a comment in it.
void main() {
  gl_FragColor = vec3(2, 3, 4);
}
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("2: unknown shader format: INVALID", r.Error());
}

TEST_F(AmberScriptParserTest, ShaderMissingFormat) {
  std::string in = R"(#!amber
SHADER geometry shader_name
# Shader has a comment in it.
void main() {
  gl_FragColor = vec3(2, 3, 4);
}
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: invalid token when looking for shader format", r.Error());
}

TEST_F(AmberScriptParserTest, ShaderEmpty) {
  std::string in = R"(#!amber
SHADER geometry shader_name GLSL
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: SHADER must not be empty", r.Error());
}

TEST_F(AmberScriptParserTest, ShaderMissingName) {
  std::string in = R"(#!amber
SHADER geometry GLSL
# Shader has a comment in it.
void main() {
  gl_FragColor = vec3(2, 3, 4);
}
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: invalid token when looking for shader format", r.Error());
}

TEST_F(AmberScriptParserTest, ShaderMissingEnd) {
  std::string in = R"(#!amber
SHADER geometry shader_name GLSL
# Shader has a comment in it.
void main() {
  gl_FragColor = vec3(2, 3, 4);
})";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("6: SHADER missing END command", r.Error());
}

TEST_F(AmberScriptParserTest, ShaderExtraParameter) {
  std::string in = R"(#!amber
SHADER geometry shader_name GLSL INVALID
# Shader has a comment in it.
void main() {
  gl_FragColor = vec3(2, 3, 4);
}
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("2: extra parameters after SHADER command", r.Error());
}

struct ShaderTypeData {
  const char* name;
  ShaderType type;
};

using AmberScriptParserShaderTypeTest = testing::TestWithParam<ShaderTypeData>;
TEST_P(AmberScriptParserShaderTypeTest, ShaderFormats) {
  auto test_data = GetParam();

  std::string shader_result = R"(
void main() {
  gl_FragColor = vec3(2, 3, 4);
}
)";

  std::string in = "SHADER " + std::string(test_data.name) +
                   R"( my_shader GLSL
)" + shader_result +
                   "END";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& shaders = script->GetShaders();
  ASSERT_EQ(1U, shaders.size());

  const auto* shader = shaders[0].get();
  EXPECT_EQ("my_shader", shader->GetName());
  EXPECT_EQ(test_data.type, shader->GetType());
  EXPECT_EQ(kShaderFormatGlsl, shader->GetFormat());
  EXPECT_EQ(shader_result, shader->GetData());
}
INSTANTIATE_TEST_SUITE_P(
    AmberScriptParserTestsShaderType,
    AmberScriptParserShaderTypeTest,
    testing::Values(
        ShaderTypeData{"vertex", kShaderTypeVertex},
        ShaderTypeData{"fragment", kShaderTypeFragment},
        ShaderTypeData{"geometry", kShaderTypeGeometry},
        ShaderTypeData{"tessellation_evaluation",
                       kShaderTypeTessellationEvaluation},
        ShaderTypeData{"tessellation_control", kShaderTypeTessellationControl},
        ShaderTypeData{"compute", kShaderTypeCompute},
        ShaderTypeData{"multi",
                       kShaderTypeMulti}));  // NOLINT(whitespace/parens)

struct ShaderFormatData {
  const char* name;
  ShaderFormat format;
};

using AmberScriptParserShaderFormatTest =
    testing::TestWithParam<ShaderFormatData>;
TEST_P(AmberScriptParserShaderFormatTest, ShaderFormats) {
  auto test_data = GetParam();

  std::string shader_result = R"(void main() {
  gl_FragColor = vec3(2, 3, 4);
}
)";

  std::string in = "SHADER vertex my_shader " + std::string(test_data.name) +
                   "\n" + shader_result + "END";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& shaders = script->GetShaders();
  ASSERT_EQ(1U, shaders.size());

  const auto* shader = shaders[0].get();
  EXPECT_EQ("my_shader", shader->GetName());
  EXPECT_EQ(kShaderTypeVertex, shader->GetType());
  EXPECT_EQ(test_data.format, shader->GetFormat());
  EXPECT_EQ(shader_result, shader->GetData());
}
INSTANTIATE_TEST_SUITE_P(
    AmberScriptParserTestsShaderFormat,
    AmberScriptParserShaderFormatTest,
    testing::Values(ShaderFormatData{"GLSL", kShaderFormatGlsl},
                    ShaderFormatData{"SPIRV-ASM", kShaderFormatSpirvAsm},
                    ShaderFormatData{
                        "SPIRV-HEX",
                        kShaderFormatSpirvHex}));  // NOLINT(whitespace/parens)

TEST_F(AmberScriptParserTest, DuplicateShaderName) {
  std::string in = R"(
SHADER vertex my_shader GLSL
# shader
END
SHADER fragment my_shader GLSL
# another shader
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("7: duplicate shader name provided", r.Error());
}

TEST_F(AmberScriptParserTest, OpenCLCKernel) {
  std::string in = R"(
SHADER compute my_shader OPENCL-C
# shader
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess());
}

TEST_F(AmberScriptParserTest, OpenCLCMultiKernel) {
  std::string in = R"(
SHADER multi my_shader OPENCL-C
# shader
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess());
}

}  // namespace amberscript
}  // namespace amber
