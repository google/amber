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
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or parseried.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <vector>

#include "gtest/gtest.h"
#include "src/amberscript/parser.h"
#include "src/shader_data.h"

namespace amber {
namespace amberscript {
namespace {

struct NameData {
  const char* name;
};

struct ShaderTypeData {
  const char* name;
  ShaderType type;
};

struct ShaderFormatData {
  const char* name;
  ShaderFormat format;
};

struct BufferTypeData {
  const char* name;
  BufferType type;
};

struct BufferData {
  const char* name;
  DataType type;
  size_t row_count;
  size_t column_count;
};

}  // namespace

using AmberScriptParserTest = testing::Test;

TEST_F(AmberScriptParserTest, EmptyInput) {
  std::string in = "";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  ASSERT_TRUE(script != nullptr);
}

TEST_F(AmberScriptParserTest, InvalidStartToken) {
  std::string in = R"(#!amber
# Start comment
1234)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: expected string", r.Error());
}

TEST_F(AmberScriptParserTest, UnknownStartToken) {
  std::string in = "INVALID token";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: unknown token: INVALID", r.Error());
}

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
INSTANTIATE_TEST_CASE_P(
    AmberScriptParserShaderPassThroughTests,
    AmberScriptParserShaderPassThroughTest,
    testing::Values(NameData{"fragment"},
                    NameData{"geometry"},
                    NameData{"tessellation_evaluation"},
                    NameData{"tessellation_control"},
                    NameData{"compute"},
                    NameData{"multi"}), );  // NOLINT(whitespace/parens)

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
INSTANTIATE_TEST_CASE_P(
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
                       kShaderTypeMulti}), );  // NOLINT(whitespace/parens)

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
INSTANTIATE_TEST_CASE_P(
    AmberScriptParserTestsShaderFormat,
    AmberScriptParserShaderFormatTest,
    testing::Values(
        ShaderFormatData{"GLSL", kShaderFormatGlsl},
        ShaderFormatData{"SPIRV-ASM", kShaderFormatSpirvAsm},
        ShaderFormatData{
            "SPIRV-HEX",
            kShaderFormatSpirvHex}), );  // NOLINT(whitespace/parens)

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

TEST_F(AmberScriptParserTest, Pipeline) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  EXPECT_EQ(2U, script->GetShaders().size());

  const auto& pipelines = script->GetPipelines();
  ASSERT_EQ(1U, pipelines.size());

  const auto* pipeline = pipelines[0].get();
  EXPECT_EQ("my_pipeline", pipeline->GetName());
  EXPECT_EQ(PipelineType::kGraphics, pipeline->GetType());

  const auto& shaders = pipeline->GetShaders();
  ASSERT_EQ(2U, shaders.size());

  ASSERT_TRUE(shaders[0].GetShader() != nullptr);
  EXPECT_EQ("my_shader", shaders[0].GetShader()->GetName());
  EXPECT_EQ(kShaderTypeVertex, shaders[0].GetShader()->GetType());
  EXPECT_EQ(static_cast<uint32_t>(0),
            shaders[0].GetShaderOptimizations().size());

  ASSERT_TRUE(shaders[1].GetShader() != nullptr);
  EXPECT_EQ("my_fragment", shaders[1].GetShader()->GetName());
  EXPECT_EQ(kShaderTypeFragment, shaders[1].GetShader()->GetType());
  EXPECT_EQ(static_cast<uint32_t>(0),
            shaders[1].GetShaderOptimizations().size());
}

TEST_F(AmberScriptParserTest, PipelineMissingEnd) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
PIPELINE graphics my_pipeline
  ATTACH my_shader
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("5: PIPELINE missing END command", r.Error());
}

TEST_F(AmberScriptParserTest, PipelineWithExtraParams) {
  std::string in = R"(
PIPELINE graphics my_pipeline INVALID
  ATTACH my_shader
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("2: extra parameters after PIPELINE command", r.Error());
}

TEST_F(AmberScriptParserTest, PipelineInvalidType) {
  std::string in = "PIPELINE my_name\nEND";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: unknown pipeline type: my_name", r.Error());
}

TEST_F(AmberScriptParserTest, PipelineMissingName) {
  std::string in = "PIPELINE compute\nEND";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("2: invalid token when looking for pipeline name", r.Error());
}

TEST_F(AmberScriptParserTest, PipelineWithInvalidTokenType) {
  std::string in = "PIPELINE 123 my_pipeline\nEND";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: invalid token when looking for pipeline type", r.Error());
}

TEST_F(AmberScriptParserTest, PipelineWithInvalidTokenName) {
  std::string in = "PIPELINE compute 123\nEND";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: invalid token when looking for pipeline name", r.Error());
}

TEST_F(AmberScriptParserTest, PipelineEmpty) {
  std::string in = "PIPELINE compute my_pipeline\nEND";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("compute pipeline requires a compute shader", r.Error());
}

TEST_F(AmberScriptParserTest, PipelineWithUnknownShader) {
  std::string in = R"(
PIPELINE graphics my_pipeline
  ATTACH my_shader
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: unknown shader in ATTACH command", r.Error());
}

TEST_F(AmberScriptParserTest, PipelineWithUnknownCommand) {
  std::string in = R"(
PIPELINE compute my_pipeline
  SHADER vertex my_shader PASSTHROUGH
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: unknown token in pipeline block: SHADER", r.Error());
}

TEST_F(AmberScriptParserTest, DuplicatePipelineName) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# Fragment shader
END

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
END
PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("14: duplicate pipeline name provided", r.Error());
}

TEST_F(AmberScriptParserTest, DuplicateShadersInAPipeline) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_shader
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("6: can not add duplicate shader to pipeline", r.Error());
}

TEST_F(AmberScriptParserTest, AttachInvalidToken) {
  std::string in = R"(PIPELINE graphics my_pipeline
  ATTACH 1234
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("2: invalid token in ATTACH command", r.Error());
}

TEST_F(AmberScriptParserTest, AttachExtraParameter) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
PIPELINE graphics my_pipeline
  ATTACH my_shader INVALID
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("4: Unknown ATTACH parameter: INVALID", r.Error());
}

TEST_F(AmberScriptParserTest, AttachMissingValue) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
PIPELINE graphics my_pipeline
  ATTACH
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("5: invalid token in ATTACH command", r.Error());
}

TEST_F(AmberScriptParserTest, ComputeShaderInGraphicsPipeline) {
  std::string in = R"(SHADER compute my_shader GLSL
void main() {
  gl_FragColor = vec3(2, 3, 4);
}
END

PIPELINE graphics my_pipeline
  ATTACH my_shader
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("9: can not add a compute shader to a graphics pipeline",
            r.Error());
}

using AmberScriptParserPipelineAttachTest =
    testing::TestWithParam<ShaderTypeData>;
TEST_P(AmberScriptParserPipelineAttachTest, GraphicsShaderInComputePipeline) {
  auto test_data = GetParam();

  std::string in = "SHADER " + std::string(test_data.name) + R"( my_shader GLSL
void main() {
  gl_FragColor = vec3(2, 3, 4);
}
END

PIPELINE compute my_pipeline
  ATTACH my_shader
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("9: only compute shaders allowed in a compute pipeline", r.Error());
}
INSTANTIATE_TEST_CASE_P(
    AmberScriptParserPipelineAttachTests,
    AmberScriptParserPipelineAttachTest,
    testing::Values(
        ShaderTypeData{"vertex", kShaderTypeVertex},
        ShaderTypeData{"fragment", kShaderTypeFragment},
        ShaderTypeData{"geometry", kShaderTypeGeometry},
        ShaderTypeData{"tessellation_evaluation",
                       kShaderTypeTessellationEvaluation},
        ShaderTypeData{
            "tessellation_control",
            kShaderTypeTessellationControl}), );  // NOLINT(whitespace/parens)

TEST_F(AmberScriptParserTest, PipelineEntryPoint) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END

PIPELINE graphics my_pipeline
  ATTACH my_shader ENTRY_POINT green
  ATTACH my_fragment
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& pipelines = script->GetPipelines();
  ASSERT_EQ(1U, pipelines.size());

  const auto* pipeline = pipelines[0].get();
  const auto& shaders = pipeline->GetShaders();
  ASSERT_EQ(2U, shaders.size());

  ASSERT_TRUE(shaders[0].GetShader() != nullptr);
  EXPECT_EQ(kShaderTypeVertex, shaders[0].GetShader()->GetType());
  EXPECT_EQ("green", shaders[0].GetEntryPoint());

  ASSERT_TRUE(shaders[1].GetShader() != nullptr);
  EXPECT_EQ(kShaderTypeFragment, shaders[1].GetShader()->GetType());
  EXPECT_EQ("main", shaders[1].GetEntryPoint());
}

TEST_F(AmberScriptParserTest, PipelineEntryPointWithInvalidValue) {
  std::string in = R"(
SHADER compute my_compute GLSL
# Compute Shader
END
PIPELINE compute my_pipeline
  ATTACH my_compute ENTRY_POINT 1234
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("6: missing shader name in ATTACH ENTRY_POINT command", r.Error());
}

TEST_F(AmberScriptParserTest, PipelineEntryPointMissingValue) {
  std::string in = R"(
SHADER compute my_compute GLSL
# Compute Shader
END
PIPELINE compute my_pipeline
  ATTACH my_compute ENTRY_POINT
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("7: missing shader name in ATTACH ENTRY_POINT command", r.Error());
}

TEST_F(AmberScriptParserTest, PipelineEntryPointExtraParameter) {
  std::string in = R"(
SHADER compute my_compute GLSL
# Compute Shader
END
PIPELINE compute my_pipeline
  ATTACH my_compute ENTRY_POINT green INVALID
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("6: extra parameters after ATTACH command", r.Error());
}

TEST_F(AmberScriptParserTest, PiplineMultiShaderAttach) {
  std::string in = R"(
SHADER multi my_shader GLSL
# shaders
END
PIPELINE compute my_pipeline
  ATTACH my_shader TYPE compute ENTRY_POINT my_entry_point
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& pipelines = script->GetPipelines();
  ASSERT_EQ(1U, pipelines.size());

  const auto* pipeline = pipelines[0].get();
  const auto& shaders = pipeline->GetShaders();
  ASSERT_EQ(1U, shaders.size());

  ASSERT_TRUE(shaders[0].GetShader() != nullptr);
  EXPECT_EQ(kShaderTypeMulti, shaders[0].GetShader()->GetType());
  EXPECT_EQ(kShaderTypeCompute, shaders[0].GetShaderType());
  EXPECT_EQ("my_entry_point", shaders[0].GetEntryPoint());
}

TEST_F(AmberScriptParserTest,
       PipelineMultiShaderMismatchPipelineAndShaderType) {
  std::string in = R"(
SHADER multi my_shader GLSL
# shaders
END
PIPELINE graphics my_pipeline
  ATTACH my_shader TYPE compute ENTRY_POINT my_entry_point
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("6: can not add a compute shader to a graphics pipeline",
            r.Error());
}

TEST_F(AmberScriptParserTest, PipelineMultiShaderMissingEntryPoint) {
  std::string in = R"(
SHADER multi my_shader GLSL
# shaders
END
PIPELINE graphics my_pipeline
  ATTACH my_shader TYPE fragment
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("7: ATTACH TYPE requires an ENTRY_POINT", r.Error());
}

TEST_F(AmberScriptParserTest, PipelineMultiShaderMissingType) {
  std::string in = R"(
SHADER multi my_shader GLSL
# shaders
END
PIPELINE graphics my_pipeline
  ATTACH my_shader
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("7: multi shader ATTACH requires TYPE", r.Error());
}

TEST_F(AmberScriptParserTest, PipelineMultiShaderMissingTypeWithEntryPoint) {
  std::string in = R"(
SHADER multi my_shader GLSL
# shaders
END
PIPELINE graphics my_pipeline
  ATTACH my_shader ENTRY_POINT my_ep
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("6: ATTACH missing TYPE for multi shader", r.Error());
}

TEST_F(AmberScriptParserTest, PipelineShaderOptimization) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
SHADER geometry my_geom GLSL
# Geom shader
END
PIPELINE graphics my_pipeline
  ATTACH my_shader
  SHADER_OPTIMIZATION my_shader
    opt1
    opt_second
  END

  ATTACH my_fragment
  SHADER_OPTIMIZATION my_fragment
    another_optimization
    third
  END

  ATTACH my_geom
  SHADER_OPTIMIZATION my_geom
  END
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& pipelines = script->GetPipelines();
  ASSERT_EQ(1U, pipelines.size());

  const auto* pipeline = pipelines[0].get();
  const auto& shaders = pipeline->GetShaders();
  ASSERT_EQ(3U, shaders.size());

  ASSERT_TRUE(shaders[0].GetShader() != nullptr);
  EXPECT_EQ(kShaderTypeVertex, shaders[0].GetShader()->GetType());
  std::vector<std::string> my_shader_opts = {"opt1", "opt_second"};
  EXPECT_EQ(my_shader_opts, shaders[0].GetShaderOptimizations());

  ASSERT_TRUE(shaders[1].GetShader() != nullptr);
  EXPECT_EQ(kShaderTypeFragment, shaders[1].GetShader()->GetType());
  std::vector<std::string> my_fragment_opts = {"another_optimization", "third"};
  EXPECT_EQ(my_fragment_opts, shaders[1].GetShaderOptimizations());

  ASSERT_TRUE(shaders[2].GetShader() != nullptr);
  EXPECT_EQ(kShaderTypeGeometry, shaders[2].GetShader()->GetType());
  std::vector<std::string> my_geom_opts = {};
  EXPECT_EQ(my_geom_opts, shaders[2].GetShaderOptimizations());
}

TEST_F(AmberScriptParserTest, PipelineShaderOptmizationInvalidShader) {
  std::string in = R"(
PIPELINE graphics my_pipeline
SHADER_OPTIMIZATION invalid_shader
  opt1
  opt_second
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: unknown shader in SHADER_OPTIMIZATION command", r.Error());
}

TEST_F(AmberScriptParserTest, PipelineShaderOptmizationMissingShader) {
  std::string in = R"(
PIPELINE graphics my_pipeline
SHADER_OPTIMIZATION
  opt1
  opt_second
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("4: missing shader name in SHADER_OPTIMIZATION command", r.Error());
}

TEST_F(AmberScriptParserTest, PipelineShaderOptmizationnUnAttachedShader) {
  std::string in = R"(
SHADER vertex my_vertex PASSTHROUGH
PIPELINE graphics my_pipeline
  SHADER_OPTIMIZATION my_vertex
    opt1
    opt_second
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("7: unknown shader specified for optimizations: my_vertex",
            r.Error());
}

TEST_F(AmberScriptParserTest, PipelineShaderOptimizationMissingEnd) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
PIPELINE graphics my_pipeline
  ATTACH my_shader
  SHADER_OPTIMIZATION my_shader
    opt1
    opt_second)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("7: SHADER_OPTIMIZATION missing END command", r.Error());
}

TEST_F(AmberScriptParserTest, PipelineShaderOptimizationExtraParams) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
PIPELINE graphics my_pipeline
  ATTACH my_shader
  SHADER_OPTIMIZATION my_shader EXTRA
    opt1
    opt_second
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("5: extra parameters after SHADER_OPTIMIZATION command", r.Error());
}

TEST_F(AmberScriptParserTest, PipelineShaderOptimizationNonStringParam) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
PIPELINE graphics my_pipeline
  ATTACH my_shader
  SHADER_OPTIMIZATION my_shader
    123
    opt
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("6: SHADER_OPTIMIZATION options must be strings", r.Error());
}

TEST_F(AmberScriptParserTest, BufferData) {
  std::string in = R"(
BUFFER my_buffer DATA_TYPE uint32 DATA
1 2 3 4
55 99 1234
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& buffers = script->GetBuffers();
  ASSERT_EQ(1U, buffers.size());

  ASSERT_TRUE(buffers[0] != nullptr);
  EXPECT_EQ("my_buffer", buffers[0]->GetName());

  ASSERT_TRUE(buffers[0]->IsDataBuffer());
  auto* buffer = buffers[0]->AsDataBuffer();
  EXPECT_TRUE(buffer->GetDatumType().IsUint32());
  EXPECT_EQ(7U, buffer->GetSize());
  EXPECT_EQ(7U * sizeof(uint32_t), buffer->GetSizeInBytes());

  std::vector<uint32_t> results = {1, 2, 3, 4, 55, 99, 1234};
  const auto* data = buffer->GetValues<uint32_t>();
  ASSERT_EQ(results.size(), buffer->GetSize());
  for (size_t i = 0; i < results.size(); ++i) {
    EXPECT_EQ(results[i], data[i]);
  }
}

TEST_F(AmberScriptParserTest, BufferFill) {
  std::string in = "BUFFER my_buffer DATA_TYPE uint8 SIZE 5 FILL 5";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& buffers = script->GetBuffers();
  ASSERT_EQ(1U, buffers.size());

  ASSERT_TRUE(buffers[0] != nullptr);
  ASSERT_TRUE(buffers[0]->IsDataBuffer());
  auto* buffer = buffers[0]->AsDataBuffer();
  EXPECT_EQ("my_buffer", buffer->GetName());
  EXPECT_TRUE(buffer->GetDatumType().IsUint8());
  EXPECT_EQ(5U, buffer->GetSize());
  EXPECT_EQ(5U * sizeof(uint8_t), buffer->GetSizeInBytes());

  std::vector<uint32_t> results = {5, 5, 5, 5, 5};
  const auto* data = buffer->GetValues<uint8_t>();
  ASSERT_EQ(results.size(), buffer->GetSize());
  for (size_t i = 0; i < results.size(); ++i) {
    EXPECT_EQ(results[i], data[i]);
  }
}

TEST_F(AmberScriptParserTest, BufferFillFloat) {
  std::string in = "BUFFER my_buffer DATA_TYPE float SIZE 5 FILL 5.2";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& buffers = script->GetBuffers();
  ASSERT_EQ(1U, buffers.size());

  ASSERT_TRUE(buffers[0] != nullptr);
  ASSERT_TRUE(buffers[0]->IsDataBuffer());
  auto* buffer = buffers[0]->AsDataBuffer();
  EXPECT_EQ("my_buffer", buffer->GetName());
  EXPECT_TRUE(buffer->GetDatumType().IsFloat());
  EXPECT_EQ(5U, buffer->GetSize());
  EXPECT_EQ(5U * sizeof(float), buffer->GetSizeInBytes());

  std::vector<float> results = {5.2f, 5.2f, 5.2f, 5.2f, 5.2f};
  const auto* data = buffer->GetValues<float>();
  ASSERT_EQ(results.size(), buffer->GetSize());
  for (size_t i = 0; i < results.size(); ++i) {
    EXPECT_FLOAT_EQ(results[i], data[i]);
  }
}

TEST_F(AmberScriptParserTest, BufferSeries) {
  std::string in =
      "BUFFER my_buffer DATA_TYPE uint8 SIZE 5 SERIES_FROM 2 INC_BY 1";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& buffers = script->GetBuffers();
  ASSERT_EQ(1U, buffers.size());

  ASSERT_TRUE(buffers[0] != nullptr);
  ASSERT_TRUE(buffers[0]->IsDataBuffer());
  auto* buffer = buffers[0]->AsDataBuffer();
  EXPECT_EQ("my_buffer", buffer->GetName());
  EXPECT_TRUE(buffer->GetDatumType().IsUint8());
  EXPECT_EQ(5U, buffer->GetSize());
  EXPECT_EQ(5U * sizeof(uint8_t), buffer->GetSizeInBytes());

  std::vector<uint8_t> results = {2, 3, 4, 5, 6};
  const auto* data = buffer->GetValues<uint8_t>();
  ASSERT_EQ(results.size(), buffer->GetSize());
  for (size_t i = 0; i < results.size(); ++i) {
    EXPECT_EQ(results[i], data[i]);
  }
}

TEST_F(AmberScriptParserTest, BufferSeriesFloat) {
  std::string in =
      "BUFFER my_buffer DATA_TYPE float SIZE 5 SERIES_FROM 2.2 INC_BY "
      "1.1";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& buffers = script->GetBuffers();
  ASSERT_EQ(1U, buffers.size());

  ASSERT_TRUE(buffers[0] != nullptr);
  ASSERT_TRUE(buffers[0]->IsDataBuffer());
  auto* buffer = buffers[0]->AsDataBuffer();
  EXPECT_EQ("my_buffer", buffer->GetName());
  EXPECT_TRUE(buffer->GetDatumType().IsFloat());
  EXPECT_EQ(5U, buffer->GetSize());
  EXPECT_EQ(5U * sizeof(float), buffer->GetSizeInBytes());

  std::vector<float> results = {2.2f, 3.3f, 4.4f, 5.5f, 6.6f};
  const auto* data = buffer->GetValues<float>();
  ASSERT_EQ(results.size(), buffer->GetSize());
  for (size_t i = 0; i < results.size(); ++i) {
    EXPECT_FLOAT_EQ(results[i], data[i]);
  }
}

TEST_F(AmberScriptParserTest, BufferMultipleBuffers) {
  std::string in = R"(
BUFFER color_buffer DATA_TYPE uint8 SIZE 5 FILL 5
BUFFER storage_buffer DATA_TYPE uint32 DATA
1 2 3 4
55 99 1234
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& buffers = script->GetBuffers();
  ASSERT_EQ(2U, buffers.size());

  ASSERT_TRUE(buffers[0] != nullptr);
  ASSERT_TRUE(buffers[0]->IsDataBuffer());
  auto* buffer = buffers[0]->AsDataBuffer();
  EXPECT_EQ("color_buffer", buffer->GetName());
  EXPECT_TRUE(buffer->GetDatumType().IsUint8());
  EXPECT_EQ(5U, buffer->GetSize());
  EXPECT_EQ(5U * sizeof(uint8_t), buffer->GetSizeInBytes());

  std::vector<uint32_t> results0 = {5, 5, 5, 5, 5};
  const auto* data0 = buffer->GetValues<uint8_t>();
  ASSERT_EQ(results0.size(), buffer->GetSize());
  for (size_t i = 0; i < results0.size(); ++i) {
    EXPECT_EQ(results0[i], data0[i]);
  }

  ASSERT_TRUE(buffers[1] != nullptr);
  ASSERT_TRUE(buffers[1]->IsDataBuffer());
  buffer = buffers[1]->AsDataBuffer();
  EXPECT_EQ("storage_buffer", buffer->GetName());
  EXPECT_TRUE(buffer->GetDatumType().IsUint32());
  EXPECT_EQ(7U, buffer->GetSize());
  EXPECT_EQ(7U * sizeof(uint32_t), buffer->GetSizeInBytes());

  std::vector<uint32_t> results1 = {1, 2, 3, 4, 55, 99, 1234};
  const auto* data1 = buffer->GetValues<uint32_t>();
  ASSERT_EQ(results1.size(), buffer->GetSize());
  for (size_t i = 0; i < results1.size(); ++i) {
    EXPECT_EQ(results1[i], data1[i]);
  }
}

TEST_F(AmberScriptParserTest, BufferFillMultiRow) {
  std::string in = R"(
BUFFER my_index_buffer DATA_TYPE vec2<int32> SIZE 5 FILL 2)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& buffers = script->GetBuffers();
  ASSERT_EQ(1U, buffers.size());

  ASSERT_TRUE(buffers[0] != nullptr);
  ASSERT_TRUE(buffers[0]->IsDataBuffer());
  auto* buffer = buffers[0]->AsDataBuffer();
  EXPECT_EQ("my_index_buffer", buffer->GetName());
  EXPECT_TRUE(buffer->GetDatumType().IsInt32());
  EXPECT_EQ(5U, buffer->GetSize());
  EXPECT_EQ(5U * 2 * sizeof(int32_t), buffer->GetSizeInBytes());

  std::vector<int32_t> results0 = {2, 2, 2, 2, 2, 2, 2, 2, 2, 2};
  const auto* data0 = buffer->GetValues<int32_t>();
  for (size_t i = 0; i < results0.size(); ++i) {
    EXPECT_EQ(results0[i], data0[i]);
  }
}

TEST_F(AmberScriptParserTest, BufferDataMultiRow) {
  std::string in = R"(
BUFFER my_index_buffer DATA_TYPE vec2<int32> DATA
2 3
4 5
6 7
8 9
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& buffers = script->GetBuffers();
  ASSERT_EQ(1U, buffers.size());

  ASSERT_TRUE(buffers[0] != nullptr);
  ASSERT_TRUE(buffers[0]->IsDataBuffer());
  auto* buffer = buffers[0]->AsDataBuffer();
  EXPECT_EQ("my_index_buffer", buffer->GetName());
  EXPECT_TRUE(buffer->GetDatumType().IsInt32());
  EXPECT_EQ(4U, buffer->GetSize());
  EXPECT_EQ(4U * 2 * sizeof(int32_t), buffer->GetSizeInBytes());

  std::vector<int32_t> results0 = {2, 3, 4, 5, 6, 7, 8, 9};
  const auto* data0 = buffer->GetValues<int32_t>();
  for (size_t i = 0; i < results0.size(); ++i) {
    EXPECT_EQ(results0[i], data0[i]);
  }
}

TEST_F(AmberScriptParserTest, BufferDataHex) {
  std::string in = R"(
BUFFER my_index_buffer DATA_TYPE uint32 DATA
0xff000000
0x00ff0000
0x0000ff00
0x000000ff
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& buffers = script->GetBuffers();
  ASSERT_EQ(1U, buffers.size());

  ASSERT_TRUE(buffers[0] != nullptr);
  ASSERT_TRUE(buffers[0]->IsDataBuffer());
  auto* buffer = buffers[0]->AsDataBuffer();
  EXPECT_EQ("my_index_buffer", buffer->GetName());
  EXPECT_TRUE(buffer->GetDatumType().IsUint32());
  EXPECT_EQ(4U, buffer->GetSize());
  EXPECT_EQ(4U * sizeof(uint32_t), buffer->GetSizeInBytes());

  std::vector<uint32_t> results0 = {4278190080, 16711680, 65280, 255};
  const auto* data0 = buffer->GetValues<uint32_t>();
  ASSERT_EQ(results0.size(), buffer->GetSize());
  for (size_t i = 0; i < results0.size(); ++i) {
    EXPECT_EQ(results0[i], data0[i]);
  }
}

TEST_F(AmberScriptParserTest, BufferFormat) {
  std::string in = "BUFFER my_buf FORMAT R32G32B32A32_SINT";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& buffers = script->GetBuffers();
  ASSERT_EQ(1U, buffers.size());

  ASSERT_TRUE(buffers[0] != nullptr);
  ASSERT_TRUE(buffers[0]->IsFormatBuffer());
  auto* buffer = buffers[0]->AsFormatBuffer();
  EXPECT_EQ("my_buf", buffer->GetName());

  auto& fmt = buffer->GetFormat();
  auto& comps = fmt.GetComponents();
  ASSERT_EQ(4U, comps.size());

  EXPECT_EQ(FormatComponentType::kR, comps[0].type);
  EXPECT_EQ(FormatMode::kSInt, comps[0].mode);
  EXPECT_EQ(32U, comps[0].num_bits);
  EXPECT_EQ(FormatComponentType::kG, comps[1].type);
  EXPECT_EQ(FormatMode::kSInt, comps[1].mode);
  EXPECT_EQ(32U, comps[1].num_bits);
  EXPECT_EQ(FormatComponentType::kB, comps[2].type);
  EXPECT_EQ(FormatMode::kSInt, comps[2].mode);
  EXPECT_EQ(32U, comps[2].num_bits);
  EXPECT_EQ(FormatComponentType::kA, comps[3].type);
  EXPECT_EQ(FormatMode::kSInt, comps[3].mode);
  EXPECT_EQ(32U, comps[3].num_bits);
}

struct BufferParseError {
  const char* in;
  const char* err;
};
using AmberScriptParserBufferParseErrorTest =
    testing::TestWithParam<BufferParseError>;
TEST_P(AmberScriptParserBufferParseErrorTest, Test) {
  auto test_data = GetParam();

  Parser parser;
  Result r = parser.Parse(test_data.in);
  ASSERT_FALSE(r.IsSuccess()) << test_data.in;
  EXPECT_EQ(std::string(test_data.err), r.Error()) << test_data.in;
}

INSTANTIATE_TEST_CASE_P(
    AmberScriptParserBufferParseErrorTest,
    AmberScriptParserBufferParseErrorTest,
    testing::Values(
        BufferParseError{"BUFFER my_buf FORMAT 123",
                         "1: BUFFER FORMAT must be a string"},
        BufferParseError{"BUFFER my_buf FORMAT A23A32",
                         "1: invalid BUFFER FORMAT"},
        BufferParseError{"BUFFER my_buf FORMAT",
                         "1: BUFFER FORMAT must be a string"},
        BufferParseError{"BUFFER my_buffer FORMAT R32G32B32A32_SFLOAT EXTRA",
                         "1: unknown token: EXTRA"},
        BufferParseError{"BUFFER 1234 DATA_TYPE uint8 SIZE 5 FILL 5",
                         "1: invalid BUFFER name provided"},
        BufferParseError{"BUFFER DATA_TYPE uint8 SIZE 5 FILL 5",
                         "1: missing BUFFER name"},

        BufferParseError{"BUFFER my_buf 1234",
                         "1: invalid BUFFER command provided"},
        BufferParseError{"BUFFER my_buf INVALID",
                         "1: unknown BUFFER command provided: INVALID"},
        BufferParseError{"BUFFER my_buf DATA_TYPE uint8 SIZE INVALID FILL 5",
                         "1: BUFFER size invalid"},
        BufferParseError{"BUFFER my_buf DATA_TYPE uint8 SIZE FILL 5",
                         "1: BUFFER size invalid"},
        BufferParseError{"BUFFER my_buf DATA_TYPE uint8 SIZE 5 FILL",
                         "1: missing BUFFER fill value"},
        BufferParseError{"BUFFER my_buf DATA_TYPE uint8 SIZE 5 FILL INVALID",
                         "1: invalid BUFFER fill value"},
        BufferParseError{"BUFFER my_buf DATA_TYPE uint8 SIZE 5 INVALID 5",
                         "1: invalid BUFFER initializer provided"},
        BufferParseError{"BUFFER my_buf DATA_TYPE uint8 SIZE 5 FILL INVALID",
                         "1: invalid BUFFER fill value"},
        BufferParseError{"BUFFER my_buf DATA_TYPE uint8 SIZE 5 FILL",
                         "1: missing BUFFER fill value"},
        BufferParseError{
            "BUFFER my_buf DATA_TYPE uint8 SIZE 5 SERIES_FROM INC_BY 2",
            "1: invalid BUFFER series_from value"},
        BufferParseError{"BUFFER my_buf DATA_TYPE uint8 SIZE 5 SERIES_FROM 2",
                         "1: missing BUFFER series_from inc_by"},
        BufferParseError{
            "BUFFER my_buf DATA_TYPE uint8 SIZE 5 SERIES_FROM 2 INC_BY",
            "1: missing BUFFER series_from inc_by value"},
        BufferParseError{"BUFFER my_buf DATA_TYPE uint8 SIZE 5 "
                         "SERIES_FROM INVALID INC_BY 2",
                         "1: invalid BUFFER series_from value"},
        BufferParseError{"BUFFER my_buf DATA_TYPE uint8 SIZE 5 "
                         "SERIES_FROM 1 INC_BY INVALID",
                         "1: invalid BUFFER series_from inc_by value"},
        BufferParseError{"BUFFER my_buf DATA_TYPE uint8 SIZE 5 "
                         "SERIES_FROM 1 INVALID 2",
                         "1: BUFFER series_from invalid command"},
        BufferParseError{
            "BUFFER my_index_buffer DATA_TYPE int32 DATA\n1.234\nEND",
            "2: invalid BUFFER data value"},
        BufferParseError{
            "BUFFER my_index_buffer DATA_TYPE int32 DATA\nINVALID\nEND",
            "2: invalid BUFFER data value"},
        BufferParseError{
            "BUFFER my_index_buffer DATA_TYPE int32 DATA INVALID\n123\nEND",
            "1: extra parameters after BUFFER data command"},
        BufferParseError{"BUFFER my_index_buffer DATA_TYPE int32 SIZE 256 FILL "
                         "5 INVALID\n123\nEND",
                         "1: extra parameters after BUFFER fill command"},
        BufferParseError{
            "BUFFER my_buffer DATA_TYPE int32 SIZE 256 SERIES_FROM 2 "
            "INC_BY 5 "
            "INVALID",
            "1: extra parameters after BUFFER series_from command"},
        BufferParseError{"BUFFER my_buf DATA_TYPE int32 SIZE 5 FILL 5\nBUFFER "
                         "my_buf DATA_TYPE int16 SIZE 5 FILL 2",
                         // NOLINTNEXTLINE(whitespace/parens)
                         "2: duplicate buffer name provided"}), );

using AmberScriptParserBufferDataTypeTest = testing::TestWithParam<BufferData>;
TEST_P(AmberScriptParserBufferDataTypeTest, BufferTypes) {
  auto test_data = GetParam();

  std::string in = std::string("BUFFER my_buf DATA_TYPE ") + test_data.name +
                   " SIZE 2 FILL 5";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << test_data.name << " :" << r.Error();

  auto script = parser.GetScript();
  const auto& buffers = script->GetBuffers();
  ASSERT_EQ(1U, buffers.size());

  ASSERT_TRUE(buffers[0] != nullptr);
  ASSERT_TRUE(buffers[0]->IsDataBuffer());
  auto* buffer = buffers[0]->AsDataBuffer();
  auto& datum = buffer->GetDatumType();
  EXPECT_EQ(test_data.type, datum.GetType());
  EXPECT_EQ(test_data.row_count, datum.RowCount());
  EXPECT_EQ(test_data.column_count, datum.ColumnCount());
}
INSTANTIATE_TEST_CASE_P(
    AmberScriptParserTestsDataType,
    AmberScriptParserBufferDataTypeTest,
    testing::Values(BufferData{"int8", DataType::kInt8, 1, 1},
                    BufferData{"int16", DataType::kInt16, 1, 1},
                    BufferData{"int32", DataType::kInt32, 1, 1},
                    BufferData{"int64", DataType::kInt64, 1, 1},
                    BufferData{"uint8", DataType::kUint8, 1, 1},
                    BufferData{"uint16", DataType::kUint16, 1, 1},
                    BufferData{"uint32", DataType::kUint32, 1, 1},
                    BufferData{"uint64", DataType::kUint64, 1, 1},
                    BufferData{"float", DataType::kFloat, 1, 1},
                    BufferData{"double", DataType::kDouble, 1, 1},
                    BufferData{"vec2<int8>", DataType::kInt8, 2, 1},
                    BufferData{"vec3<float>", DataType::kFloat, 3, 1},
                    BufferData{"vec4<uint32>", DataType::kUint32, 4, 1},
                    BufferData{"mat2x4<int32>", DataType::kInt32, 2, 4},
                    BufferData{"mat3x3<float>", DataType::kFloat, 3, 3},
                    BufferData{"mat4x2<uint16>", DataType::kUint16, 4,
                               2}), );  // NOLINT(whitespace/parens)

using AmberScriptParserBufferDataTypeInvalidTest =
    testing::TestWithParam<NameData>;
TEST_P(AmberScriptParserBufferDataTypeInvalidTest, BufferTypes) {
  auto test_data = GetParam();

  std::string in = std::string("BUFFER my_buf DATA_TYPE ") + test_data.name +
                   " SIZE 4 FILL 5";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess()) << test_data.name;
  EXPECT_EQ("1: invalid data_type provided", r.Error()) << test_data.name;
}
INSTANTIATE_TEST_CASE_P(
    AmberScriptParserBufferDataTypeInvalidTest,
    AmberScriptParserBufferDataTypeInvalidTest,
    testing::Values(NameData{"int17"},
                    NameData{"uintt0"},
                    NameData{"vec7<uint8>"},
                    NameData{"vec27<uint8>"},
                    NameData{"vec2<vec2<float>>"},
                    NameData{"vec2<mat2x2<float>>"},
                    NameData{"vec2float>"},
                    NameData{"vec2<uint32"},
                    NameData{"vec2<uint4>"},
                    NameData{"vec2<>"},
                    NameData{"vec2"},
                    NameData{"mat1x1<double>"},
                    NameData{"mat5x2<double>"},
                    NameData{"mat2x5<double>"},
                    NameData{"mat22x22<double>"},
                    NameData{"matx5<double>"},
                    NameData{"mat2<double>"},
                    NameData{"mat2x<double>"},
                    NameData{"mat2x2<vec4<float>>"},
                    NameData{"mat2x2<mat3x3<double>>"},
                    NameData{"mat2x2<unit7>"},
                    NameData{"mat2x2"},
                    NameData{"mat2x2<>"}), );  // NOLINT(whitespace/parens)

TEST_F(AmberScriptParserTest, FramebufferDefaultSize) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& pipelines = script->GetPipelines();
  ASSERT_EQ(1U, pipelines.size());

  const auto* pipeline = pipelines[0].get();
  EXPECT_EQ(250, pipeline->GetFramebufferWidth());
  EXPECT_EQ(250, pipeline->GetFramebufferHeight());
}

TEST_F(AmberScriptParserTest, FramebufferSize) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
  FRAMEBUFFER_SIZE 256 246
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& pipelines = script->GetPipelines();
  ASSERT_EQ(1U, pipelines.size());

  const auto* pipeline = pipelines[0].get();
  EXPECT_EQ(256, pipeline->GetFramebufferWidth());
  EXPECT_EQ(246, pipeline->GetFramebufferHeight());
}

TEST_F(AmberScriptParserTest, FramebufferSizeMissingSize) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
  FRAMEBUFFER_SIZE
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());

  EXPECT_EQ("10: missing size for FRAMEBUFFER_SIZE command", r.Error());
}

TEST_F(AmberScriptParserTest, FramebufferSizeMissingHeight) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
  FRAMEBUFFER_SIZE 222
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());

  EXPECT_EQ("10: missing height for FRAMEBUFFER_SIZE command", r.Error());
}

TEST_F(AmberScriptParserTest, FramebufferSizeExtraParams) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
  FRAMEBUFFER_SIZE 222 233 INVALID
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());

  EXPECT_EQ("9: extra parameters after FRAMEBUFFER_SIZE command", r.Error());
}

TEST_F(AmberScriptParserTest, FramebufferInvalidWidth) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
  FRAMEBUFFER_SIZE INVALID 245
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());

  EXPECT_EQ("9: invalid width for FRAMEBUFFER_SIZE command", r.Error());
}

TEST_F(AmberScriptParserTest, FramebufferInvalidHeight) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
  FRAMEBUFFER_SIZE 245 INVALID
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());

  EXPECT_EQ("9: invalid height for FRAMEBUFFER_SIZE command", r.Error());
}

TEST_F(AmberScriptParserTest, BindColorBuffer) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& pipelines = script->GetPipelines();
  ASSERT_EQ(1U, pipelines.size());

  const auto* pipeline = pipelines[0].get();
  const auto& color_buffers = pipeline->GetColorAttachments();
  ASSERT_EQ(1U, color_buffers.size());

  const auto& buf_info = color_buffers[0];
  ASSERT_TRUE(buf_info.buffer != nullptr);
  EXPECT_EQ(0, buf_info.location);
  EXPECT_EQ(250 * 250, buf_info.buffer->GetSize());
  EXPECT_EQ(250 * 250 * 4 * sizeof(float), buf_info.buffer->GetSizeInBytes());
}

TEST_F(AmberScriptParserTest, BindColorBufferTwice) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0
  BIND BUFFER my_fb AS color LOCATION 1
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("13: color buffer may only be bound to a PIPELINE once", r.Error());
}

TEST_F(AmberScriptParserTest, BindColorBufferMissingBuffer) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER AS color LOCATION 0
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("12: unknown buffer: AS", r.Error());
}

TEST_F(AmberScriptParserTest, BindColorBufferNonDeclaredBuffer) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("11: unknown buffer: my_fb", r.Error());
}

TEST_F(AmberScriptParserTest, BindColorBufferMissingLocation) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("13: BIND missing LOCATION", r.Error());
}

TEST_F(AmberScriptParserTest, BindColorBufferMissingLocationIndex) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("13: invalid value for BIND LOCATION", r.Error());
}

TEST_F(AmberScriptParserTest, BindColorBufferInvalidLocationIndex) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION INVALID
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("12: invalid value for BIND LOCATION", r.Error());
}

TEST_F(AmberScriptParserTest, BindColorBufferExtraParams) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0 EXTRA
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("12: extra parameters after BIND command", r.Error());
}

TEST_F(AmberScriptParserTest, BindColorBufferNonFormatBuffer) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb DATA_TYPE int32 SIZE 500 FILL 0

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("12: color buffer must be a FORMAT buffer", r.Error());
}

TEST_F(AmberScriptParserTest, BindColorBufferDuplicateLocation) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT
BUFFER sec_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0
  BIND BUFFER sec_fb AS color LOCATION 0
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("14: can not bind two color buffers to the same LOCATION",
            r.Error());
}

TEST_F(AmberScriptParserTest, BindColorToTwoPipelinesRequiresMatchingSize) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0
END
PIPELINE graphics second_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0
  FRAMEBUFFER_SIZE 256 300
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("shared framebuffer must have same size over all PIPELINES",
            r.Error());
}

TEST_F(AmberScriptParserTest, BindColorTwoPipelines) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT
BUFFER second_fb FORMAT R8G8B8A8_UINT
BUFFER depth_1 FORMAT D32_SFLOAT_S8_UINT
BUFFER depth_2 FORMAT D32_SFLOAT_S8_UINT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0
  BIND BUFFER depth_1 AS depth_stencil
  FRAMEBUFFER_SIZE 90 180
END
PIPELINE graphics second_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER second_fb AS color LOCATION 9
  BIND BUFFER depth_2 AS depth_stencil
  FRAMEBUFFER_SIZE 256 300
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& pipelines = script->GetPipelines();
  ASSERT_EQ(2U, pipelines.size());

  const auto* pipeline = pipelines[0].get();
  const auto& color_buffers1 = pipeline->GetColorAttachments();
  ASSERT_EQ(1U, color_buffers1.size());

  const auto& buf1 = color_buffers1[0];
  ASSERT_TRUE(buf1.buffer != nullptr);
  EXPECT_EQ(0, buf1.location);
  EXPECT_EQ(90 * 180, buf1.buffer->GetSize());
  EXPECT_EQ(90 * 180 * 4 * sizeof(float), buf1.buffer->GetSizeInBytes());

  pipeline = pipelines[1].get();
  const auto& color_buffers2 = pipeline->GetColorAttachments();
  const auto& buf2 = color_buffers2[0];
  ASSERT_TRUE(buf2.buffer != nullptr);
  EXPECT_EQ(9, buf2.location);
  EXPECT_EQ(256 * 300, buf2.buffer->GetSize());
  EXPECT_EQ(256 * 300 * sizeof(uint32_t), buf2.buffer->GetSizeInBytes());
}

TEST_F(AmberScriptParserTest, BindColorFBSizeSetBeforeBuffer) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  FRAMEBUFFER_SIZE 90 180
  BIND BUFFER my_fb AS color LOCATION 0
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& pipelines = script->GetPipelines();
  ASSERT_EQ(1U, pipelines.size());

  const auto* pipeline = pipelines[0].get();
  const auto& color_buffers1 = pipeline->GetColorAttachments();
  ASSERT_EQ(1U, color_buffers1.size());

  const auto& buf1 = color_buffers1[0];
  ASSERT_TRUE(buf1.buffer != nullptr);
  EXPECT_EQ(0, buf1.location);
  EXPECT_EQ(90 * 180, buf1.buffer->GetSize());
  EXPECT_EQ(90 * 180 * 4 * sizeof(float), buf1.buffer->GetSizeInBytes());
}

TEST_F(AmberScriptParserTest, BindColorFBSizeSetAfterBuffer) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0
  FRAMEBUFFER_SIZE 90 180
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& pipelines = script->GetPipelines();
  ASSERT_EQ(1U, pipelines.size());

  const auto* pipeline = pipelines[0].get();
  const auto& color_buffers1 = pipeline->GetColorAttachments();
  ASSERT_EQ(1U, color_buffers1.size());

  const auto& buf1 = color_buffers1[0];
  ASSERT_TRUE(buf1.buffer != nullptr);
  EXPECT_EQ(0, buf1.location);
  EXPECT_EQ(90 * 180, buf1.buffer->GetSize());
  EXPECT_EQ(90 * 180 * 4 * sizeof(float), buf1.buffer->GetSizeInBytes());
}

TEST_F(AmberScriptParserTest, PipelineDefaultColorBuffer) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
END
PIPELINE graphics my_pipeline2
  ATTACH my_shader
  ATTACH my_fragment
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& pipelines = script->GetPipelines();
  ASSERT_EQ(2U, pipelines.size());

  ASSERT_EQ(1U, pipelines[0]->GetColorAttachments().size());
  const auto& buf1 = pipelines[0]->GetColorAttachments()[0];
  ASSERT_TRUE(buf1.buffer != nullptr);

  Buffer* buffer1 = buf1.buffer;
  ASSERT_TRUE(buffer1->IsFormatBuffer());
  EXPECT_EQ(FormatType::kB8G8R8A8_UNORM,
            buffer1->AsFormatBuffer()->GetFormat().GetFormatType());
  EXPECT_EQ(0, buf1.location);
  EXPECT_EQ(250 * 250, buffer1->GetSize());
  EXPECT_EQ(250 * 250 * sizeof(uint32_t), buffer1->GetSizeInBytes());

  ASSERT_EQ(1U, pipelines[1]->GetColorAttachments().size());
  const auto& buf2 = pipelines[1]->GetColorAttachments()[0];
  ASSERT_TRUE(buf2.buffer != nullptr);
  ASSERT_EQ(buffer1, buf2.buffer);
  EXPECT_EQ(0, buf2.location);
  EXPECT_EQ(FormatType::kB8G8R8A8_UNORM,
            buf2.buffer->AsFormatBuffer()->GetFormat().GetFormatType());
  EXPECT_EQ(250 * 250, buf2.buffer->GetSize());
  EXPECT_EQ(250 * 250 * sizeof(uint32_t), buf2.buffer->GetSizeInBytes());
}

TEST_F(AmberScriptParserTest, PipelineDefaultColorBufferMismatchSize) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
END
PIPELINE graphics my_pipeline2
  ATTACH my_shader
  ATTACH my_fragment
  FRAMEBUFFER_SIZE 256 256
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());

  EXPECT_EQ("shared framebuffer must have same size over all PIPELINES",
            r.Error());
}

TEST_F(AmberScriptParserTest, PipelineDefaultDepthBuffer) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& pipelines = script->GetPipelines();
  ASSERT_EQ(1U, pipelines.size());

  const auto* pipeline = pipelines[0].get();
  const auto& buf = pipeline->GetDepthBuffer();

  ASSERT_TRUE(buf.buffer != nullptr);
  EXPECT_EQ(FormatType::kD32_SFLOAT_S8_UINT,
            buf.buffer->AsFormatBuffer()->GetFormat().GetFormatType());
  EXPECT_EQ(250 * 250, buf.buffer->GetSize());
  EXPECT_EQ(250 * 250 * (sizeof(float) + sizeof(uint8_t)),
            buf.buffer->GetSizeInBytes());
}

TEST_F(AmberScriptParserTest, BindDepthBuffer) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_buf FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_buf AS depth_stencil
  FRAMEBUFFER_SIZE 90 180
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& pipelines = script->GetPipelines();
  ASSERT_EQ(1U, pipelines.size());

  const auto* pipeline = pipelines[0].get();
  const auto& buf = pipeline->GetDepthBuffer();
  ASSERT_TRUE(buf.buffer != nullptr);
  EXPECT_EQ(90 * 180, buf.buffer->GetSize());
  EXPECT_EQ(90 * 180 * 4 * sizeof(float), buf.buffer->GetSizeInBytes());
}

TEST_F(AmberScriptParserTest, BindDepthBufferNonFormatBuffer) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_buf DATA_TYPE int32 SIZE 500 FILL 0

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_buf AS depth_stencil
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("12: depth buffer must be a FORMAT buffer", r.Error());
}

TEST_F(AmberScriptParserTest, BindDepthBufferExtraParams) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_buf FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_buf AS depth_stencil EXTRA
  FRAMEBUFFER_SIZE 90 180
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("12: extra parameters after BIND command", r.Error());
}

TEST_F(AmberScriptParserTest, BindBufferMissingBufferName) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_buf FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER AS depth_stencil
  FRAMEBUFFER_SIZE 90 180
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("12: unknown buffer: AS", r.Error());
}

TEST_F(AmberScriptParserTest, BindBufferAsMissingType) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_buf FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_buf AS
  FRAMEBUFFER_SIZE 90 180
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("13: invalid token for BUFFER type", r.Error());
}

TEST_F(AmberScriptParserTest, BindBufferAsInvalidType) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_buf FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_buf AS
  FRAMEBUFFER_SIZE 90 180
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("13: invalid token for BUFFER type", r.Error());
}

TEST_F(AmberScriptParserTest, BindDepthBufferUnknownBuffer) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_buf AS depth_stencil
  FRAMEBUFFER_SIZE 90 180
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("11: unknown buffer: my_buf", r.Error());
}

TEST_F(AmberScriptParserTest, BindBufferMultipleDepthBuffers) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_buf FORMAT R32G32B32A32_SFLOAT
BUFFER my_buf2 FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_buf AS depth_stencil
  BIND BUFFER my_buf AS depth_stencil
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("14: can only bind one depth buffer in a PIPELINE", r.Error());
}

TEST_F(AmberScriptParserTest, BindVertexData) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_buf DATA_TYPE int8 SIZE 50 FILL 5
BUFFER my_buf2 DATA_TYPE int8 SIZE 50 FILL 5

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  VERTEX_DATA my_buf LOCATION 0
  VERTEX_DATA my_buf2 LOCATION 1
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& pipelines = script->GetPipelines();
  ASSERT_EQ(1U, pipelines.size());

  const auto* pipeline = pipelines[0].get();
  const auto& vertex_buffers = pipeline->GetVertexBuffers();
  ASSERT_EQ(2, vertex_buffers.size());

  const auto& info1 = vertex_buffers[0];
  ASSERT_TRUE(info1.buffer != nullptr);
  EXPECT_TRUE(info1.buffer->IsDataBuffer());
  EXPECT_EQ(0, info1.location);

  const auto& info2 = vertex_buffers[1];
  ASSERT_TRUE(info2.buffer != nullptr);
  EXPECT_TRUE(info2.buffer->IsDataBuffer());
  EXPECT_EQ(1, info2.location);
}

TEST_F(AmberScriptParserTest, BindVertexDataDuplicateLocation) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_buf DATA_TYPE int8 SIZE 50 FILL 5
BUFFER my_buf2 DATA_TYPE int8 SIZE 50 FILL 5

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  VERTEX_DATA my_buf LOCATION 0
  VERTEX_DATA my_buf2 LOCATION 0
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("14: can not bind two vertex buffers to the same LOCATION",
            r.Error());
}

TEST_F(AmberScriptParserTest, BindVertexDataDuplicateBinding) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_buf DATA_TYPE int8 SIZE 50 FILL 5

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  VERTEX_DATA my_buf LOCATION 0
  VERTEX_DATA my_buf LOCATION 1
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("13: vertex buffer may only be bound to a PIPELINE once",
            r.Error());
}

TEST_F(AmberScriptParserTest, BindVertexDataMissingBuffer) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_buf DATA_TYPE int8 SIZE 50 FILL 5

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  VERTEX_DATA LOCATION 0
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("12: unknown buffer: LOCATION", r.Error());
}

TEST_F(AmberScriptParserTest, BindVertexDataUnknownBuffer) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  VERTEX_DATA my_buf LOCATION 0
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("11: unknown buffer: my_buf", r.Error());
}

TEST_F(AmberScriptParserTest, BindVertexDataMissingLocation) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_buf DATA_TYPE int8 SIZE 50 FILL 5

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  VERTEX_DATA my_buf
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("13: VERTEX_DATA missing LOCATION", r.Error());
}

TEST_F(AmberScriptParserTest, BindVertexDataMissingLocationValue) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_buf DATA_TYPE int8 SIZE 50 FILL 5

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  VERTEX_DATA my_buf LOCATION
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("13: invalid value for VERTEX_DATA LOCATION", r.Error());
}

TEST_F(AmberScriptParserTest, BindVertexDataExtraParameters) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_buf DATA_TYPE int8 SIZE 50 FILL 5

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  VERTEX_DATA my_buf LOCATION 0 EXTRA
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("12: extra parameters after VERTEX_DATA command", r.Error());
}

TEST_F(AmberScriptParserTest, BindIndexData) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_buf DATA_TYPE int8 SIZE 50 FILL 5

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  INDEX_DATA my_buf
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& pipelines = script->GetPipelines();
  ASSERT_EQ(1U, pipelines.size());

  const auto* pipeline = pipelines[0].get();
  const auto* buf = pipeline->GetIndexBuffer();
  ASSERT_TRUE(buf != nullptr);
  EXPECT_TRUE(buf->IsDataBuffer());
}

TEST_F(AmberScriptParserTest, BindIndexataMissingBuffer) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_buf FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  INDEX_DATA
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("13: missing buffer name in INDEX_DATA command", r.Error());
}

TEST_F(AmberScriptParserTest, BindIndexDataUnknownBuffer) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  INDEX_DATA my_buf
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("11: unknown buffer: my_buf", r.Error());
}

TEST_F(AmberScriptParserTest, BindIndexDataExtraParameters) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_buf FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  INDEX_DATA my_buf EXTRA
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("12: extra parameters after INDEX_DATA command", r.Error());
}

TEST_F(AmberScriptParserTest, BindIndexDataMultiple) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_buf FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  INDEX_DATA my_buf
  INDEX_DATA my_buf
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("13: can only bind one INDEX_DATA buffer in a pipeline", r.Error());
}

TEST_F(AmberScriptParserTest, BindBuffer) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_buf FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_buf AS uniform DESCRIPTOR_SET 1 BINDING 2
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& pipelines = script->GetPipelines();
  ASSERT_EQ(1U, pipelines.size());

  const auto* pipeline = pipelines[0].get();
  const auto& bufs = pipeline->GetBuffers();
  ASSERT_EQ(1U, bufs.size());
  EXPECT_EQ(BufferType::kUniform, bufs[0].buffer->GetBufferType());
  EXPECT_EQ(1U, bufs[0].descriptor_set);
  EXPECT_EQ(2U, bufs[0].binding);
  EXPECT_EQ(static_cast<uint32_t>(0), bufs[0].location);
  EXPECT_EQ(FormatType::kR32G32B32A32_SFLOAT,
            bufs[0].buffer->AsFormatBuffer()->GetFormat().GetFormatType());
}

TEST_F(AmberScriptParserTest, BindBufferWithIdx) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_buf FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_buf AS uniform DESCRIPTOR_SET 1 BINDING 2 IDX 5
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& pipelines = script->GetPipelines();
  ASSERT_EQ(1U, pipelines.size());

  const auto* pipeline = pipelines[0].get();
  const auto& bufs = pipeline->GetBuffers();
  ASSERT_EQ(1U, bufs.size());
  EXPECT_EQ(BufferType::kUniform, bufs[0].buffer->GetBufferType());
  EXPECT_EQ(1U, bufs[0].descriptor_set);
  EXPECT_EQ(2U, bufs[0].binding);
  EXPECT_EQ(5U, bufs[0].location);
  EXPECT_TRUE(bufs[0].buffer->IsFormatBuffer());
  EXPECT_EQ(FormatType::kR32G32B32A32_SFLOAT,
            bufs[0].buffer->AsFormatBuffer()->GetFormat().GetFormatType());
}

TEST_F(AmberScriptParserTest, BindBufferMissingIdxValue) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_buf FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_buf AS uniform DESCRIPTOR_SET 1 BINDING 2 IDX
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("13: invalid value for IDX in BIND command", r.Error());
}

TEST_F(AmberScriptParserTest, BindBufferMissingBindingValue) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_buf FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_buf AS uniform DESCRIPTOR_SET 1 BINDING
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("13: invalid value for BINDING in BIND command", r.Error());
}

TEST_F(AmberScriptParserTest, BindBufferMissingBinding) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_buf FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_buf AS uniform DESCRIPTOR_SET 1 IDX 5
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("12: missing BINDING for BIND command", r.Error());
}

TEST_F(AmberScriptParserTest, BindBufferMissingDescriptorSetValue) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_buf FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_buf AS uniform DESCRIPTOR_SET BINDING 2
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("12: invalid value for DESCRIPTOR_SET in BIND command", r.Error());
}

TEST_F(AmberScriptParserTest, BindBufferMissingDescriptorSet) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_buf FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_buf AS uniform BINDING 2
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("12: missing DESCRIPTOR_SET for BIND command", r.Error());
}

TEST_F(AmberScriptParserTest, BindingBufferExtraParams) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_buf FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_buf AS uniform DESCRIPTOR_SET 1 BINDING 2 EXTRA
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("12: extra parameters after BIND command", r.Error());
}

TEST_F(AmberScriptParserTest, BindingBufferIdxExtraParams) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_buf FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_buf AS uniform DESCRIPTOR_SET 1 BINDING 2 IDX 5 EXTRA
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("12: extra parameters after BIND command", r.Error());
}

TEST_F(AmberScriptParserTest, BindingBufferInvalidIdxValue) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_buf FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_buf AS uniform DESCRIPTOR_SET 1 BINDING 2 IDX INVALID
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("12: invalid value for IDX in BIND command", r.Error());
}

TEST_F(AmberScriptParserTest, BindingBufferInvalidBindingValue) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_buf FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_buf AS uniform DESCRIPTOR_SET 1 BINDING INVALID
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("12: invalid value for BINDING in BIND command", r.Error());
}

TEST_F(AmberScriptParserTest, BindingBufferInvalidDescriptorSetValue) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_buf FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_buf AS uniform DESCRIPTOR_SET INVALID BINDING 2
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("12: invalid value for DESCRIPTOR_SET in BIND command", r.Error());
}

TEST_F(AmberScriptParserTest, BindingBufferInvalidBufferType) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_buf FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_buf AS INVALID DESCRIPTOR_SET 1 BINDING 2 IDX INVALID
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("12: unknown buffer_type: INVALID", r.Error());
}

using AmberScriptParserBufferTypeTest = testing::TestWithParam<BufferTypeData>;
TEST_P(AmberScriptParserBufferTypeTest, BufferType) {
  auto test_data = GetParam();

  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_buf FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_buf AS )" +
                   std::string(test_data.name) +
                   " DESCRIPTOR_SET 0 BINDING 0\nEND";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& pipelines = script->GetPipelines();
  ASSERT_EQ(1U, pipelines.size());

  const auto* pipeline = pipelines[0].get();
  const auto& bufs = pipeline->GetBuffers();
  ASSERT_EQ(1U, bufs.size());
  EXPECT_EQ(test_data.type, bufs[0].buffer->GetBufferType());
}
INSTANTIATE_TEST_CASE_P(
    AmberScriptParserBufferTypeTest,
    AmberScriptParserBufferTypeTest,
    testing::Values(BufferTypeData{"push_constant", BufferType::kPushConstant},
                    BufferTypeData{"uniform", BufferType::kUniform},
                    BufferTypeData{
                        "storage",
                        BufferType::kStorage}), );  // NOLINT(whitespace/parens)

TEST_F(AmberScriptParserTest, RunCompute) {
  std::string in = R"(
SHADER compute my_shader GLSL
void main() {
  gl_FragColor = vec3(2, 3, 4);
}
END

PIPELINE compute my_pipeline
  ATTACH my_shader
END

RUN my_pipeline 2 4 5
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& commands = script->GetCommands();
  ASSERT_EQ(1U, commands.size());

  auto* cmd = commands[0].get();
  ASSERT_TRUE(cmd->IsCompute());
  EXPECT_EQ(2U, cmd->AsCompute()->GetX());
  EXPECT_EQ(4U, cmd->AsCompute()->GetY());
  EXPECT_EQ(5U, cmd->AsCompute()->GetZ());
}

TEST_F(AmberScriptParserTest, RunWithoutPipeline) {
  std::string in = R"(RUN 2 4 5)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  ASSERT_EQ("1: missing pipeline name for RUN command", r.Error());
}

TEST_F(AmberScriptParserTest, RunWithInvalidPipeline) {
  std::string in = R"(RUN unknown_pipeline 2 4 5)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  ASSERT_EQ("1: unknown pipeline for RUN command: unknown_pipeline", r.Error());
}

TEST_F(AmberScriptParserTest, RunComputeWithGraphicsPipeline) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
END

RUN my_pipeline 2 4 5)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  ASSERT_EQ("12: RUN command requires compute pipeline, got graphics",
            r.Error());
}

TEST_F(AmberScriptParserTest, RunComputeMissingParams) {
  std::string in = R"(
SHADER compute my_shader GLSL
void main() {
  gl_FragColor = vec3(2, 3, 4);
}
END

PIPELINE compute my_pipeline
  ATTACH my_shader
END

RUN my_pipeline)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  ASSERT_EQ("12: RUN command requires parameters", r.Error());
}

TEST_F(AmberScriptParserTest, RunComputeExtraParams) {
  std::string in = R"(
SHADER compute my_shader GLSL
void main() {
  gl_FragColor = vec3(2, 3, 4);
}
END

PIPELINE compute my_pipeline
  ATTACH my_shader
END

RUN my_pipeline 2 4 5 EXTRA)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  ASSERT_EQ("12: extra parameters after RUN command", r.Error());
}

TEST_F(AmberScriptParserTest, RunComputeInvalidZ) {
  std::string in = R"(
SHADER compute my_shader GLSL
void main() {
  gl_FragColor = vec3(2, 3, 4);
}
END

PIPELINE compute my_pipeline
  ATTACH my_shader
END

RUN my_pipeline 2 4 INVALID)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  ASSERT_EQ("12: invalid parameter for RUN command: INVALID", r.Error());
}

TEST_F(AmberScriptParserTest, RunComputeInvalidY) {
  std::string in = R"(
SHADER compute my_shader GLSL
void main() {
  gl_FragColor = vec3(2, 3, 4);
}
END

PIPELINE compute my_pipeline
  ATTACH my_shader
END

RUN my_pipeline 2 INVALID 5)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  ASSERT_EQ("12: invalid parameter for RUN command: INVALID", r.Error());
}

TEST_F(AmberScriptParserTest, RunComputeInvalidX) {
  std::string in = R"(
SHADER compute my_shader GLSL
void main() {
  gl_FragColor = vec3(2, 3, 4);
}
END

PIPELINE compute my_pipeline
  ATTACH my_shader
END

RUN my_pipeline INVALID 4 5)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  ASSERT_EQ("12: invalid token in RUN command: INVALID", r.Error());
}

TEST_F(AmberScriptParserTest, RunDrawRect) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
END

RUN my_pipeline DRAW_RECT POS 2 4 SIZE 10 20)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& commands = script->GetCommands();
  ASSERT_EQ(1U, commands.size());

  auto* cmd = commands[0].get();
  ASSERT_TRUE(cmd->IsDrawRect());
  EXPECT_TRUE(cmd->AsDrawRect()->IsOrtho());
  EXPECT_FALSE(cmd->AsDrawRect()->IsPatch());
  EXPECT_FLOAT_EQ(2.f, cmd->AsDrawRect()->GetX());
  EXPECT_FLOAT_EQ(4.f, cmd->AsDrawRect()->GetY());
  EXPECT_FLOAT_EQ(10.f, cmd->AsDrawRect()->GetWidth());
  EXPECT_FLOAT_EQ(20.f, cmd->AsDrawRect()->GetHeight());
}

TEST_F(AmberScriptParserTest, RunDrawRectWithComputePipelineInvalid) {
  std::string in = R"(
SHADER compute my_shader GLSL
void main() {
  gl_FragColor = vec3(2, 3, 4);
}
END

PIPELINE compute my_pipeline
  ATTACH my_shader
END

RUN my_pipeline DRAW_RECT POS 2 4 SIZE 10 20)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  ASSERT_EQ("12: RUN command requires graphics pipeline, got compute",
            r.Error());
}

TEST_F(AmberScriptParserTest, RunDrawRectWithMissingPipeline) {
  std::string in = R"(RUN my_pipeline DRAW_RECT POS 2 4 SIZE 10 20)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  ASSERT_EQ("1: unknown pipeline for RUN command: my_pipeline", r.Error());
}

TEST_F(AmberScriptParserTest, RunDrawRectMissingValues) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
END

RUN my_pipeline DRAW_RECT)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  ASSERT_EQ("12: RUN DRAW_RECT command requires parameters", r.Error());
}

TEST_F(AmberScriptParserTest, RunDrawRectMissingPOS) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
END

RUN my_pipeline DRAW_RECT 2 4 SIZE 10 20)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  ASSERT_EQ("12: invalid token in RUN command: 2; expected POS", r.Error());
}

TEST_F(AmberScriptParserTest, RunDrawRectPOSMissingValues) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
END

RUN my_pipeline DRAW_RECT POS SIZE 10 20)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  ASSERT_EQ("12: missing X position for RUN command", r.Error());
}

TEST_F(AmberScriptParserTest, RunDrawRectMissingPOSY) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
END

RUN my_pipeline DRAW_RECT POS 2 SIZE 10 20)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  ASSERT_EQ("12: missing Y position for RUN command", r.Error());
}

TEST_F(AmberScriptParserTest, RunDrawRectInvalidPOSX) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
END

RUN my_pipeline DRAW_RECT POS INVALID 4 SIZE 10 20)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  ASSERT_EQ("12: missing X position for RUN command", r.Error());
}

TEST_F(AmberScriptParserTest, RunDrawRectInavlidPOSY) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
END

RUN my_pipeline DRAW_RECT POS 2 INVALID SIZE 10 20)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  ASSERT_EQ("12: missing Y position for RUN command", r.Error());
}

TEST_F(AmberScriptParserTest, RunDrawRectMissingSize) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
END

RUN my_pipeline DRAW_RECT POS 2 4 10 20)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  ASSERT_EQ("12: invalid token in RUN command: 10; expected SIZE", r.Error());
}

TEST_F(AmberScriptParserTest, RunDrawRectMissingSizeValues) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
END

RUN my_pipeline DRAW_RECT POS 2 4 SIZE)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  ASSERT_EQ("12: missing width value for RUN command", r.Error());
}

TEST_F(AmberScriptParserTest, RunDrawRectMissingSizeHeight) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
END

RUN my_pipeline DRAW_RECT POS 2 4 SIZE 10)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  ASSERT_EQ("12: missing height value for RUN command", r.Error());
}

TEST_F(AmberScriptParserTest, RunDrawRectInvalidSizeWidth) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
END

RUN my_pipeline DRAW_RECT POS 2 4 SIZE INVALID 20)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  ASSERT_EQ("12: missing width value for RUN command", r.Error());
}

TEST_F(AmberScriptParserTest, RunDrawRectInvalidSizeHeight) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
END

RUN my_pipeline DRAW_RECT POS 2 4 SIZE 10 INVALID)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  ASSERT_EQ("12: missing height value for RUN command", r.Error());
}

TEST_F(AmberScriptParserTest, RunDrawRectExtraCommands) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
END

RUN my_pipeline DRAW_RECT POS 2 4 SIZE 10 20 EXTRA)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  ASSERT_EQ("12: extra parameters after RUN command", r.Error());
}

TEST_F(AmberScriptParserTest, Clear) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
END

CLEAR my_pipeline)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& commands = script->GetCommands();
  ASSERT_EQ(1U, commands.size());

  auto* cmd = commands[0].get();
  ASSERT_TRUE(cmd->IsClear());
}

TEST_F(AmberScriptParserTest, ClearMissingPipeline) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
END

CLEAR)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("12: missing pipeline name for CLEAR command", r.Error());
}

TEST_F(AmberScriptParserTest, ClearInvalidPipeline) {
  std::string in = R"(CLEAR other_pipeline)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: unknown pipeline for CLEAR command: other_pipeline", r.Error());
}

TEST_F(AmberScriptParserTest, ClearComputePipeline) {
  std::string in = R"(
SHADER compute my_shader GLSL
void main() {
  gl_FragColor = vec3(2, 3, 4);
}
END

PIPELINE compute my_pipeline
  ATTACH my_shader
END

CLEAR my_pipeline)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  ASSERT_EQ("12: CLEAR command requires graphics pipeline, got compute",
            r.Error());
}

TEST_F(AmberScriptParserTest, ClearExtraParams) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
END

CLEAR my_pipeline EXTRA)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("12: extra parameters after CLEAR command", r.Error());
}

TEST_F(AmberScriptParserTest, ExpectRGB) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0
END

EXPECT my_fb IDX 5 6 SIZE 250 150 EQ_RGB 2 128 255)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& commands = script->GetCommands();
  ASSERT_EQ(1U, commands.size());

  auto* cmd = commands[0].get();
  ASSERT_TRUE(cmd->IsProbe());

  auto* probe = cmd->AsProbe();
  EXPECT_FALSE(probe->IsRGBA());
  EXPECT_TRUE(probe->IsProbeRect());
  EXPECT_FALSE(probe->IsRelative());
  EXPECT_FALSE(probe->IsWholeWindow());
  EXPECT_EQ(5U, probe->GetX());
  EXPECT_EQ(6U, probe->GetY());
  EXPECT_EQ(250U, probe->GetWidth());
  EXPECT_EQ(150U, probe->GetHeight());
  EXPECT_EQ(2U, probe->GetR());
  EXPECT_EQ(128U, probe->GetG());
  EXPECT_EQ(255U, probe->GetB());
}

TEST_F(AmberScriptParserTest, ExpectRGBA) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0
END

EXPECT my_fb IDX 2 7 SIZE 20 88 EQ_RGBA 2 128 255 99)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& commands = script->GetCommands();
  ASSERT_EQ(1U, commands.size());

  auto* cmd = commands[0].get();
  ASSERT_TRUE(cmd->IsProbe());

  auto* probe = cmd->AsProbe();
  EXPECT_TRUE(probe->IsRGBA());
  EXPECT_TRUE(probe->IsProbeRect());
  EXPECT_FALSE(probe->IsRelative());
  EXPECT_FALSE(probe->IsWholeWindow());
  EXPECT_EQ(2U, probe->GetX());
  EXPECT_EQ(7U, probe->GetY());
  EXPECT_EQ(20U, probe->GetWidth());
  EXPECT_EQ(88U, probe->GetHeight());
  EXPECT_EQ(2U, probe->GetR());
  EXPECT_EQ(128U, probe->GetG());
  EXPECT_EQ(255U, probe->GetB());
  EXPECT_EQ(99U, probe->GetA());
}

TEST_F(AmberScriptParserTest, ExpectMissingBufferName) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0
END

EXPECT IDX 0 0 SIZE 250 250 EQ_RGB 0 128 255)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("15: unknown buffer name for EXPECT command", r.Error());
}

TEST_F(AmberScriptParserTest, ExpectInvalidBufferName) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0
END

EXPECT unknown_buffer IDX 0 0 SIZE 250 250 EQ_RGB 0 128 255)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("15: unknown buffer name for EXPECT command", r.Error());
}

TEST_F(AmberScriptParserTest, ExpectMissingIDX) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0
END

EXPECT my_fb 0 0 SIZE 250 250 EQ_RGB 0 128 255)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("15: missing IDX in EXPECT command", r.Error());
}

TEST_F(AmberScriptParserTest, ExpectMissingIDXValues) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0
END

EXPECT my_fb IDX SIZE 250 250 EQ_RGB 0 128 255)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("15: invalid X value in EXPECT command", r.Error());
}

TEST_F(AmberScriptParserTest, ExpectMissingIdxY) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0
END

EXPECT my_fb IDX 0 SIZE 250 250 EQ_RGB 0 128 255)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("15: invalid Y value in EXPECT command", r.Error());
}

TEST_F(AmberScriptParserTest, ExpectIdxInvalidX) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0
END

EXPECT my_fb IDX INVAILD 0 SIZE 250 250 EQ_RGB 0 128 255)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("15: invalid X value in EXPECT command", r.Error());
}

TEST_F(AmberScriptParserTest, ExpectIdxInvalidY) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0
END

EXPECT my_fb IDX 0 INVALID SIZE 250 250 EQ_RGB 0 128 255)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("15: unexpected token in EXPECT command: INVALID", r.Error());
}

TEST_F(AmberScriptParserTest, ExpectRGBMissingSize) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0
END

EXPECT my_fb IDX 0 0 250 250 EQ_RGB 0 128 255)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("15: unexpected token in EXPECT command: 250", r.Error());
}

TEST_F(AmberScriptParserTest, ExpectSizeMissingValues) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0
END

EXPECT my_fb IDX 0 0 SIZE EQ_RGB 0 128 255)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("15: invalid width in EXPECT command", r.Error());
}

TEST_F(AmberScriptParserTest, ExpectSizeMissingHeight) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0
END

EXPECT my_fb IDX 0 0 SIZE 250 EQ_RGB 0 128 255)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("15: invalid height in EXPECT command", r.Error());
}

TEST_F(AmberScriptParserTest, ExpectSizeInvalidWidth) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0
END

EXPECT my_fb IDX 0 0 SIZE INVALID 250 EQ_RGB 0 128 255)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("15: invalid width in EXPECT command", r.Error());
}

TEST_F(AmberScriptParserTest, ExpectSizeInvalidHeight) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0
END

EXPECT my_fb IDX 0 0 SIZE 250 INVALID EQ_RGB 0 128 255)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("15: invalid height in EXPECT command", r.Error());
}

TEST_F(AmberScriptParserTest, ExpectSizeInvalidComparitor) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0
END

EXPECT my_fb IDX 0 0 SIZE 250 250 INVALID 0 128 255)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("15: unknown comparator type in EXPECT: INVALID", r.Error());
}

TEST_F(AmberScriptParserTest, ExpectRGBMissingValues) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0
END

EXPECT my_fb IDX 0 0 SIZE 250 250 EQ_RGB)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("15: invalid R value in EXPECT command", r.Error());
}

TEST_F(AmberScriptParserTest, ExpectRGBMissingB) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0
END

EXPECT my_fb IDX 0 0 SIZE 250 250 EQ_RGB 0 128)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("15: invalid B value in EXPECT command", r.Error());
}

TEST_F(AmberScriptParserTest, ExpectRGBMissingG) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0
END

EXPECT my_fb IDX 0 0 SIZE 250 250 EQ_RGB 0)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("15: invalid G value in EXPECT command", r.Error());
}

TEST_F(AmberScriptParserTest, ExpectRGBAMissingA) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0
END

EXPECT my_fb IDX 0 0 SIZE 250 250 EQ_RGBA 0 128 255)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("15: invalid A value in EXPECT command", r.Error());
}

TEST_F(AmberScriptParserTest, ExpectRGBInvalidR) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0
END

EXPECT my_fb IDX 0 0 SIZE 250 250 EQ_RGB INVALID 128 255)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("15: invalid R value in EXPECT command", r.Error());
}

TEST_F(AmberScriptParserTest, ExpectRGBInvalidG) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0
END

EXPECT my_fb IDX 0 0 SIZE 250 250 EQ_RGB 0 INVALID 255)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("15: invalid G value in EXPECT command", r.Error());
}

TEST_F(AmberScriptParserTest, ExpectRGBInvalidB) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0
END

EXPECT my_fb IDX 0 0 SIZE 250 250 EQ_RGB 0 128 INVALID)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("15: invalid B value in EXPECT command", r.Error());
}

TEST_F(AmberScriptParserTest, ExpectRGBAInvalidA) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0
END

EXPECT my_fb IDX 0 0 SIZE 250 250 EQ_RGBA 0 128 255 INVALID)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("15: invalid A value in EXPECT command", r.Error());
}

TEST_F(AmberScriptParserTest, ExpectRGBExtraParam) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0
END

EXPECT my_fb IDX 0 0 SIZE 250 250 EQ_RGB 0 128 255 EXTRA)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("15: extra parameters after EXPECT command", r.Error());
}

TEST_F(AmberScriptParserTest, ExpectRGBAExtraParam) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0
END

EXPECT my_fb IDX 0 0 SIZE 250 250 EQ_RGBA 0 128 255 99 EXTRA)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("15: extra parameters after EXPECT command", r.Error());
}

TEST_F(AmberScriptParserTest, ExpectEQ) {
  std::string in = R"(
BUFFER orig_buf DATA_TYPE int32 SIZE 100 FILL 11
EXPECT orig_buf IDX 5 EQ 11)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& commands = script->GetCommands();
  ASSERT_EQ(1U, commands.size());

  auto* cmd = commands[0].get();
  ASSERT_TRUE(cmd->IsProbeSSBO());

  auto* probe = cmd->AsProbeSSBO();
  EXPECT_EQ(ProbeSSBOCommand::Comparator::kEqual, probe->GetComparator());
  EXPECT_EQ(5U, probe->GetOffset());
  EXPECT_TRUE(probe->GetDatumType().IsInt32());
  ASSERT_EQ(1U, probe->GetValues().size());
  EXPECT_EQ(11U, probe->GetValues()[0].AsInt32());
}

TEST_F(AmberScriptParserTest, ExpectEqMissingValue) {
  std::string in = R"(
BUFFER orig_buf DATA_TYPE int32 SIZE 100 FILL 11
EXPECT orig_buf IDX 5 EQ)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: missing comparison values for EXPECT command", r.Error());
}

TEST_F(AmberScriptParserTest, ExpectEQExtraParams) {
  std::string in = R"(
BUFFER orig_buf DATA_TYPE int32 SIZE 100 FILL 11
EXPECT orig_buf IDX 5 EQ 11 EXTRA)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: Invalid value provided to EXPECT command: EXTRA", r.Error());
}

TEST_F(AmberScriptParserTest, MultipleExpect) {
  std::string in = R"(
BUFFER orig_buf DATA_TYPE int32 SIZE 100 FILL 11
BUFFER dest_buf DATA_TYPE int32 SIZE 100 FILL 22

EXPECT orig_buf IDX 0 EQ 11
EXPECT dest_buf IDX 0 EQ 22
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
}

TEST_F(AmberScriptParserTest, Copy) {
  std::string in = R"(
BUFFER from FORMAT R32G32B32A32_SFLOAT
BUFFER dest FORMAT R32G32B32A32_SFLOAT
COPY from TO dest)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& commands = script->GetCommands();
  ASSERT_EQ(1U, commands.size());

  auto* cmd = commands[0].get();
  ASSERT_TRUE(cmd->IsCopy());
}

TEST_F(AmberScriptParserTest, CopyUndeclaredOriginBuffer) {
  std::string in = R"(
COPY from)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("2: COPY origin buffer was not declared", r.Error());
}

TEST_F(AmberScriptParserTest, CopyInvalidOriginBufferName) {
  std::string in = R"(
COPY 123)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("2: invalid buffer name after COPY", r.Error());
}

TEST_F(AmberScriptParserTest, CopyUndeclaredDestinationBuffer) {
  std::string in = R"(
BUFFER from FORMAT R32G32B32A32_SFLOAT
COPY from TO dest)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: COPY destination buffer was not declared", r.Error());
}

TEST_F(AmberScriptParserTest, CopyMissingOriginBuffer) {
  std::string in = R"(
COPY)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("2: missing buffer name after COPY", r.Error());
}

TEST_F(AmberScriptParserTest, CopyMissingDestinationBuffer) {
  std::string in = R"(
BUFFER from FORMAT R32G32B32A32_SFLOAT
COPY from TO)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: missing buffer name after TO", r.Error());
}

TEST_F(AmberScriptParserTest, CopyToSameBuffer) {
  std::string in = R"(
BUFFER from FORMAT R32G32B32A32_SFLOAT
COPY from TO from)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: COPY origin and destination buffers are identical", r.Error());
}

TEST_F(AmberScriptParserTest, CopyMissingToKeyword) {
  std::string in = R"(
BUFFER from FORMAT R32G32B32A32_SFLOAT
BUFFER dest FORMAT R32G32B32A32_SFLOAT
COPY from dest)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("4: expected 'TO' after COPY and buffer name", r.Error());
}

}  // namespace amberscript
}  // namespace amber
