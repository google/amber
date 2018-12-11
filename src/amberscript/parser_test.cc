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
  ASSERT_TRUE(script->IsAmberScript());
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

  auto parent_script = parser.GetScript();
  auto script = ToAmberScript(parent_script.get());
  const auto& shaders = script->GetShaders();
  ASSERT_EQ(1U, shaders.size());

  const auto* shader = shaders[0].get();
  EXPECT_EQ("my_shader1", shader->GetName());
  EXPECT_EQ(ShaderType::kVertex, shader->GetType());
  EXPECT_EQ(ShaderFormat::kSpirvAsm, shader->GetFormat());
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
                    NameData{"compute"}), );  // NOLINT(whitespace/parens)

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

  auto parent_script = parser.GetScript();
  auto script = ToAmberScript(parent_script.get());
  const auto& shaders = script->GetShaders();
  ASSERT_EQ(1U, shaders.size());

  const auto* shader = shaders[0].get();
  EXPECT_EQ("shader_name", shader->GetName());
  EXPECT_EQ(ShaderType::kGeometry, shader->GetType());
  EXPECT_EQ(ShaderFormat::kGlsl, shader->GetFormat());
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

  std::string in = "SHADER " + std::string(test_data.name) + R"( my_shader GLSL
)" + shader_result +
                   "END";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto parent_script = parser.GetScript();
  auto script = ToAmberScript(parent_script.get());
  const auto& shaders = script->GetShaders();
  ASSERT_EQ(1U, shaders.size());

  const auto* shader = shaders[0].get();
  EXPECT_EQ("my_shader", shader->GetName());
  EXPECT_EQ(test_data.type, shader->GetType());
  EXPECT_EQ(ShaderFormat::kGlsl, shader->GetFormat());
  EXPECT_EQ(shader_result, shader->GetData());
}
INSTANTIATE_TEST_CASE_P(
    AmberScriptParserTestsShaderType,
    AmberScriptParserShaderTypeTest,
    testing::Values(ShaderTypeData{"vertex", ShaderType::kVertex},
                    ShaderTypeData{"fragment", ShaderType::kFragment},
                    ShaderTypeData{"geometry", ShaderType::kGeometry},
                    ShaderTypeData{"tessellation_evaluation",
                                   ShaderType::kTessellationEvaluation},
                    ShaderTypeData{"tessellation_control",
                                   ShaderType::kTessellationControl},
                    ShaderTypeData{
                        "compute",
                        ShaderType::kCompute}), );  // NOLINT(whitespace/parens)

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

  auto parent_script = parser.GetScript();
  auto script = ToAmberScript(parent_script.get());
  const auto& shaders = script->GetShaders();
  ASSERT_EQ(1U, shaders.size());

  const auto* shader = shaders[0].get();
  EXPECT_EQ("my_shader", shader->GetName());
  EXPECT_EQ(ShaderType::kVertex, shader->GetType());
  EXPECT_EQ(test_data.format, shader->GetFormat());
  EXPECT_EQ(shader_result, shader->GetData());
}
INSTANTIATE_TEST_CASE_P(
    AmberScriptParserTestsShaderFormat,
    AmberScriptParserShaderFormatTest,
    testing::Values(
        ShaderFormatData{"GLSL", ShaderFormat::kGlsl},
        ShaderFormatData{"SPIRV-ASM", ShaderFormat::kSpirvAsm},
        ShaderFormatData{
            "SPIRV-HEX",
            ShaderFormat::kSpirvHex}), );  // NOLINT(whitespace/parens)

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

  auto parent_script = parser.GetScript();
  auto script = ToAmberScript(parent_script.get());
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
  EXPECT_EQ(ShaderType::kVertex, shaders[0].GetShader()->GetType());
  EXPECT_EQ(static_cast<uint32_t>(0),
            shaders[0].GetShaderOptimizations().size());

  ASSERT_TRUE(shaders[1].GetShader() != nullptr);
  EXPECT_EQ("my_fragment", shaders[1].GetShader()->GetName());
  EXPECT_EQ(ShaderType::kFragment, shaders[1].GetShader()->GetType());
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
  EXPECT_EQ("2: compute pipeline requires a compute shader", r.Error());
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
  EXPECT_EQ("5: can not add duplicate shader to pipeline", r.Error());
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
  EXPECT_EQ("4: extra parameters after ATTACH command", r.Error());
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
  EXPECT_EQ("8: can not add a compute shader to a graphics pipeline",
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
  EXPECT_EQ("8: only compute shaders allowed in a compute pipeline", r.Error());
}
INSTANTIATE_TEST_CASE_P(
    AmberScriptParserPipelineAttachTests,
    AmberScriptParserPipelineAttachTest,
    testing::Values(
        ShaderTypeData{"vertex", ShaderType::kVertex},
        ShaderTypeData{"fragment", ShaderType::kFragment},
        ShaderTypeData{"geometry", ShaderType::kGeometry},
        ShaderTypeData{"tessellation_evaluation",
                       ShaderType::kTessellationEvaluation},
        ShaderTypeData{
            "tessellation_control",
            ShaderType::kTessellationControl}), );  // NOLINT(whitespace/parens)

TEST_F(AmberScriptParserTest, PipelineEntryPoint) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  ENTRY_POINT my_shader green
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto parent_script = parser.GetScript();
  auto script = ToAmberScript(parent_script.get());
  const auto& pipelines = script->GetPipelines();
  ASSERT_EQ(1U, pipelines.size());

  const auto* pipeline = pipelines[0].get();
  const auto& shaders = pipeline->GetShaders();
  ASSERT_EQ(2U, shaders.size());

  ASSERT_TRUE(shaders[0].GetShader() != nullptr);
  EXPECT_EQ(ShaderType::kVertex, shaders[0].GetShader()->GetType());
  EXPECT_EQ("green", shaders[0].GetEntryPoint());

  ASSERT_TRUE(shaders[1].GetShader() != nullptr);
  EXPECT_EQ(ShaderType::kFragment, shaders[1].GetShader()->GetType());
  EXPECT_EQ("main", shaders[1].GetEntryPoint());
}

TEST_F(AmberScriptParserTest, PipelineEntryPointWithInvalidValue) {
  std::string in = R"(
SHADER compute my_compute GLSL
# Compute Shader
END
PIPELINE compute my_pipeline
  ATTACH my_compute
  ENTRY_POINT my_compute 1234
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("7: invalid value in ENTRY_POINT command", r.Error());
}

TEST_F(AmberScriptParserTest, PipelineEntryPointMissingValue) {
  std::string in = R"(
SHADER compute my_compute GLSL
# Compute Shader
END
PIPELINE compute my_pipeline
  ATTACH my_compute
  ENTRY_POINT
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("8: missing shader name in ENTRY_POINT command", r.Error());
}

TEST_F(AmberScriptParserTest, PipelineEntryPointMissingEntryPointName) {
  std::string in = R"(
SHADER compute my_compute GLSL
# Compute Shader
END
PIPELINE compute my_pipeline
  ATTACH my_compute
  ENTRY_POINT my_compute
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("8: invalid value in ENTRY_POINT command", r.Error());
}

TEST_F(AmberScriptParserTest, PipelineEntryPointExtraParameter) {
  std::string in = R"(
SHADER compute my_compute GLSL
# Compute Shader
END
PIPELINE compute my_pipeline
  ATTACH my_compute
  ENTRY_POINT my_compute green INVALID
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("7: extra parameters after ENTRY_POINT command", r.Error());
}

TEST_F(AmberScriptParserTest, PipelineMultipleEntryPointsForOneShader) {
  std::string in = R"(
SHADER compute my_compute GLSL
# Compute Shader
END
PIPELINE compute my_pipeline
  ATTACH my_compute
  ENTRY_POINT my_compute green
  ENTRY_POINT my_compute red
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("8: multiple entry points given for the same shader", r.Error());
}

TEST_F(AmberScriptParserTest, PipelineEntryPointForInvalidShader) {
  std::string in = R"(
SHADER compute my_compute GLSL
# Compute Shader
END
PIPELINE compute my_pipeline
  ATTACH my_compute
  ENTRY_POINT INVALID green
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("7: unknown shader in ENTRY_POINT command", r.Error());
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

  auto parent_script = parser.GetScript();
  auto script = ToAmberScript(parent_script.get());
  const auto& pipelines = script->GetPipelines();
  ASSERT_EQ(1U, pipelines.size());

  const auto* pipeline = pipelines[0].get();
  const auto& shaders = pipeline->GetShaders();
  ASSERT_EQ(3U, shaders.size());

  ASSERT_TRUE(shaders[0].GetShader() != nullptr);
  EXPECT_EQ(ShaderType::kVertex, shaders[0].GetShader()->GetType());
  std::vector<std::string> my_shader_opts = {"opt1", "opt_second"};
  EXPECT_EQ(my_shader_opts, shaders[0].GetShaderOptimizations());

  ASSERT_TRUE(shaders[1].GetShader() != nullptr);
  EXPECT_EQ(ShaderType::kFragment, shaders[1].GetShader()->GetType());
  std::vector<std::string> my_fragment_opts = {"another_optimization", "third"};
  EXPECT_EQ(my_fragment_opts, shaders[1].GetShaderOptimizations());

  ASSERT_TRUE(shaders[2].GetShader() != nullptr);
  EXPECT_EQ(ShaderType::kGeometry, shaders[2].GetShader()->GetType());
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
BUFFER storage my_buffer DATA_TYPE uint32 DATA
1 2 3 4
55 99 1234
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto parent_script = parser.GetScript();
  auto script = ToAmberScript(parent_script.get());
  const auto& buffers = script->GetBuffers();
  ASSERT_EQ(1U, buffers.size());

  ASSERT_TRUE(buffers[0] != nullptr);
  EXPECT_EQ("my_buffer", buffers[0]->GetName());
  EXPECT_EQ(BufferType::kStorage, buffers[0]->GetBufferType());

  ASSERT_TRUE(buffers[0]->IsDataBuffer());
  auto* buffer = buffers[0]->AsDataBuffer();
  EXPECT_TRUE(buffer->GetDatumType().IsUint32());
  EXPECT_EQ(7U, buffer->GetSize());
  EXPECT_EQ(7U * sizeof(uint32_t), buffer->GetSizeInBytes());

  std::vector<uint32_t> results = {1, 2, 3, 4, 55, 99, 1234};
  const auto& data = buffer->GetData();
  ASSERT_EQ(results.size(), data.size());
  for (size_t i = 0; i < results.size(); ++i) {
    ASSERT_TRUE(data[i].IsInteger());
    EXPECT_EQ(results[i], data[i].AsUint32());
  }
}

TEST_F(AmberScriptParserTest, BufferFill) {
  std::string in = "BUFFER color my_buffer DATA_TYPE uint8 SIZE 5 FILL 5";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto parent_script = parser.GetScript();
  auto script = ToAmberScript(parent_script.get());
  const auto& buffers = script->GetBuffers();
  ASSERT_EQ(1U, buffers.size());

  ASSERT_TRUE(buffers[0] != nullptr);
  ASSERT_TRUE(buffers[0]->IsDataBuffer());
  auto* buffer = buffers[0]->AsDataBuffer();
  EXPECT_EQ("my_buffer", buffer->GetName());
  EXPECT_EQ(BufferType::kColor, buffer->GetBufferType());
  EXPECT_TRUE(buffer->GetDatumType().IsUint8());
  EXPECT_EQ(5U, buffer->GetSize());
  EXPECT_EQ(5U * sizeof(uint8_t), buffer->GetSizeInBytes());

  std::vector<uint32_t> results = {5, 5, 5, 5, 5};
  const auto& data = buffer->GetData();
  ASSERT_EQ(results.size(), data.size());
  for (size_t i = 0; i < results.size(); ++i) {
    ASSERT_TRUE(data[i].IsInteger());
    EXPECT_EQ(results[i], data[i].AsUint8());
  }
}

TEST_F(AmberScriptParserTest, BufferFillFloat) {
  std::string in = "BUFFER color my_buffer DATA_TYPE float SIZE 5 FILL 5.2";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto parent_script = parser.GetScript();
  auto script = ToAmberScript(parent_script.get());
  const auto& buffers = script->GetBuffers();
  ASSERT_EQ(1U, buffers.size());

  ASSERT_TRUE(buffers[0] != nullptr);
  ASSERT_TRUE(buffers[0]->IsDataBuffer());
  auto* buffer = buffers[0]->AsDataBuffer();
  EXPECT_EQ("my_buffer", buffer->GetName());
  EXPECT_EQ(BufferType::kColor, buffer->GetBufferType());
  EXPECT_TRUE(buffer->GetDatumType().IsFloat());
  EXPECT_EQ(5U, buffer->GetSize());
  EXPECT_EQ(5U * sizeof(float), buffer->GetSizeInBytes());

  std::vector<float> results = {5.2f, 5.2f, 5.2f, 5.2f, 5.2f};
  const auto& data = buffer->GetData();
  ASSERT_EQ(results.size(), data.size());
  for (size_t i = 0; i < results.size(); ++i) {
    ASSERT_TRUE(data[i].IsFloat());
    EXPECT_FLOAT_EQ(results[i], data[i].AsFloat());
  }
}

TEST_F(AmberScriptParserTest, BufferSeries) {
  std::string in =
      "BUFFER color my_buffer DATA_TYPE uint8 SIZE 5 SERIES_FROM 2 INC_BY 1";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto parent_script = parser.GetScript();
  auto script = ToAmberScript(parent_script.get());
  const auto& buffers = script->GetBuffers();
  ASSERT_EQ(1U, buffers.size());

  ASSERT_TRUE(buffers[0] != nullptr);
  ASSERT_TRUE(buffers[0]->IsDataBuffer());
  auto* buffer = buffers[0]->AsDataBuffer();
  EXPECT_EQ("my_buffer", buffer->GetName());
  EXPECT_EQ(BufferType::kColor, buffer->GetBufferType());
  EXPECT_TRUE(buffer->GetDatumType().IsUint8());
  EXPECT_EQ(5U, buffer->GetSize());
  EXPECT_EQ(5U * sizeof(uint8_t), buffer->GetSizeInBytes());

  std::vector<uint8_t> results = {2, 3, 4, 5, 6};
  const auto& data = buffer->GetData();
  ASSERT_EQ(results.size(), data.size());
  for (size_t i = 0; i < results.size(); ++i) {
    ASSERT_TRUE(data[i].IsInteger());
    EXPECT_EQ(results[i], data[i].AsUint8());
  }
}

TEST_F(AmberScriptParserTest, BufferSeriesFloat) {
  std::string in =
      "BUFFER color my_buffer DATA_TYPE float SIZE 5 SERIES_FROM 2.2 INC_BY "
      "1.1";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto parent_script = parser.GetScript();
  auto script = ToAmberScript(parent_script.get());
  const auto& buffers = script->GetBuffers();
  ASSERT_EQ(1U, buffers.size());

  ASSERT_TRUE(buffers[0] != nullptr);
  ASSERT_TRUE(buffers[0]->IsDataBuffer());
  auto* buffer = buffers[0]->AsDataBuffer();
  EXPECT_EQ("my_buffer", buffer->GetName());
  EXPECT_EQ(BufferType::kColor, buffer->GetBufferType());
  EXPECT_TRUE(buffer->GetDatumType().IsFloat());
  EXPECT_EQ(5U, buffer->GetSize());
  EXPECT_EQ(5U * sizeof(float), buffer->GetSizeInBytes());

  std::vector<float> results = {2.2f, 3.3f, 4.4f, 5.5f, 6.6f};
  const auto& data = buffer->GetData();
  ASSERT_EQ(results.size(), data.size());
  for (size_t i = 0; i < results.size(); ++i) {
    ASSERT_TRUE(data[i].IsFloat());
    EXPECT_FLOAT_EQ(results[i], data[i].AsFloat());
  }
}

TEST_F(AmberScriptParserTest, BufferFramebuffer) {
  std::string in = "BUFFER framebuffer my_buffer DIMS 800 600";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto parent_script = parser.GetScript();
  auto script = ToAmberScript(parent_script.get());
  const auto& buffers = script->GetBuffers();
  ASSERT_EQ(1U, buffers.size());

  ASSERT_TRUE(buffers[0] != nullptr);
  ASSERT_TRUE(buffers[0]->IsDataBuffer());
  auto* buffer = buffers[0]->AsDataBuffer();
  EXPECT_EQ("my_buffer", buffer->GetName());
  EXPECT_EQ(BufferType::kFramebuffer, buffer->GetBufferType());
  EXPECT_TRUE(buffer->GetDatumType().IsUint32());
  EXPECT_EQ(4U, buffer->GetDatumType().ColumnCount());
  EXPECT_EQ(800U * 600U, buffer->GetSize());
  EXPECT_EQ(800U * 600U * 4U * sizeof(uint32_t), buffer->GetSizeInBytes());
}

TEST_F(AmberScriptParserTest, BufferMultipleBuffers) {
  std::string in = R"(
BUFFER color color_buffer DATA_TYPE uint8 SIZE 5 FILL 5
BUFFER storage storage_buffer DATA_TYPE uint32 DATA
1 2 3 4
55 99 1234
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto parent_script = parser.GetScript();
  auto script = ToAmberScript(parent_script.get());
  const auto& buffers = script->GetBuffers();
  ASSERT_EQ(2U, buffers.size());

  ASSERT_TRUE(buffers[0] != nullptr);
  ASSERT_TRUE(buffers[0]->IsDataBuffer());
  auto* buffer = buffers[0]->AsDataBuffer();
  EXPECT_EQ("color_buffer", buffer->GetName());
  EXPECT_EQ(BufferType::kColor, buffer->GetBufferType());
  EXPECT_TRUE(buffer->GetDatumType().IsUint8());
  EXPECT_EQ(5U, buffer->GetSize());
  EXPECT_EQ(5U * sizeof(uint8_t), buffer->GetSizeInBytes());

  std::vector<uint32_t> results0 = {5, 5, 5, 5, 5};
  const auto& data0 = buffer->GetData();
  ASSERT_EQ(results0.size(), data0.size());
  for (size_t i = 0; i < results0.size(); ++i) {
    ASSERT_TRUE(data0[i].IsInteger());
    EXPECT_EQ(results0[i], data0[i].AsUint8());
  }

  ASSERT_TRUE(buffers[1] != nullptr);
  ASSERT_TRUE(buffers[1]->IsDataBuffer());
  buffer = buffers[1]->AsDataBuffer();
  EXPECT_EQ("storage_buffer", buffer->GetName());
  EXPECT_EQ(BufferType::kStorage, buffer->GetBufferType());
  EXPECT_TRUE(buffer->GetDatumType().IsUint32());
  EXPECT_EQ(7U, buffer->GetSize());
  EXPECT_EQ(7U * sizeof(uint32_t), buffer->GetSizeInBytes());

  std::vector<uint32_t> results1 = {1, 2, 3, 4, 55, 99, 1234};
  const auto& data1 = buffer->GetData();
  ASSERT_EQ(results1.size(), data1.size());
  for (size_t i = 0; i < results1.size(); ++i) {
    ASSERT_TRUE(data1[i].IsInteger());
    EXPECT_EQ(results1[i], data1[i].AsUint32());
  }
}

TEST_F(AmberScriptParserTest, BufferFillMultiRow) {
  std::string in = R"(
BUFFER index my_index_buffer DATA_TYPE vec2<int32> SIZE 5 FILL 2)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto parent_script = parser.GetScript();
  auto script = ToAmberScript(parent_script.get());
  const auto& buffers = script->GetBuffers();
  ASSERT_EQ(1U, buffers.size());

  ASSERT_TRUE(buffers[0] != nullptr);
  ASSERT_TRUE(buffers[0]->IsDataBuffer());
  auto* buffer = buffers[0]->AsDataBuffer();
  EXPECT_EQ("my_index_buffer", buffer->GetName());
  EXPECT_EQ(BufferType::kIndex, buffer->GetBufferType());
  EXPECT_TRUE(buffer->GetDatumType().IsInt32());
  EXPECT_EQ(5U, buffer->GetSize());
  EXPECT_EQ(5U * 2 * sizeof(int32_t), buffer->GetSizeInBytes());

  std::vector<int32_t> results0 = {2, 2, 2, 2, 2, 2, 2, 2, 2, 2};
  const auto& data0 = buffer->GetData();
  ASSERT_EQ(results0.size(), data0.size());
  for (size_t i = 0; i < results0.size(); ++i) {
    ASSERT_TRUE(data0[i].IsInteger());
    EXPECT_EQ(results0[i], data0[i].AsInt32());
  }
}

TEST_F(AmberScriptParserTest, BufferDataMultiRow) {
  std::string in = R"(
BUFFER index my_index_buffer DATA_TYPE vec2<int32> DATA
2 3
4 5
6 7
8 9
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto parent_script = parser.GetScript();
  auto script = ToAmberScript(parent_script.get());
  const auto& buffers = script->GetBuffers();
  ASSERT_EQ(1U, buffers.size());

  ASSERT_TRUE(buffers[0] != nullptr);
  ASSERT_TRUE(buffers[0]->IsDataBuffer());
  auto* buffer = buffers[0]->AsDataBuffer();
  EXPECT_EQ("my_index_buffer", buffer->GetName());
  EXPECT_EQ(BufferType::kIndex, buffer->GetBufferType());
  EXPECT_TRUE(buffer->GetDatumType().IsInt32());
  EXPECT_EQ(4U, buffer->GetSize());
  EXPECT_EQ(4U * 2 * sizeof(int32_t), buffer->GetSizeInBytes());

  std::vector<int32_t> results0 = {2, 3, 4, 5, 6, 7, 8, 9};
  const auto& data0 = buffer->GetData();
  ASSERT_EQ(results0.size(), data0.size());
  for (size_t i = 0; i < results0.size(); ++i) {
    ASSERT_TRUE(data0[i].IsInteger());
    EXPECT_EQ(results0[i], data0[i].AsInt32());
  }
}

TEST_F(AmberScriptParserTest, BufferDataHex) {
  std::string in = R"(
BUFFER index my_index_buffer DATA_TYPE uint32 DATA
0xff000000
0x00ff0000
0x0000ff00
0x000000ff
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto parent_script = parser.GetScript();
  auto script = ToAmberScript(parent_script.get());
  const auto& buffers = script->GetBuffers();
  ASSERT_EQ(1U, buffers.size());

  ASSERT_TRUE(buffers[0] != nullptr);
  ASSERT_TRUE(buffers[0]->IsDataBuffer());
  auto* buffer = buffers[0]->AsDataBuffer();
  EXPECT_EQ("my_index_buffer", buffer->GetName());
  EXPECT_EQ(BufferType::kIndex, buffer->GetBufferType());
  EXPECT_TRUE(buffer->GetDatumType().IsUint32());
  EXPECT_EQ(4U, buffer->GetSize());
  EXPECT_EQ(4U * sizeof(uint32_t), buffer->GetSizeInBytes());

  std::vector<uint32_t> results0 = {4278190080, 16711680, 65280, 255};
  const auto& data0 = buffer->GetData();
  ASSERT_EQ(results0.size(), data0.size());
  for (size_t i = 0; i < results0.size(); ++i) {
    ASSERT_TRUE(data0[i].IsInteger());
    EXPECT_EQ(results0[i], data0[i].AsUint32());
  }
}

TEST_F(AmberScriptParserTest, BufferInvalidType) {
  std::string in = "BUFFER 1234 color_buffer DATA_TYPE uint8 SIZE 5 FILL 5";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: invalid BUFFER type provided", r.Error());
}

TEST_F(AmberScriptParserTest, BufferUnknownType) {
  std::string in = "BUFFER UNKNOWN color_buffer DATA_TYPE uint8 SIZE 5 FILL 5";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: unknown BUFFER type provided: UNKNOWN", r.Error());
}

TEST_F(AmberScriptParserTest, BufferInvalidName) {
  std::string in = "BUFFER color 1234 DATA_TYPE uint8 SIZE 5 FILL 5";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: invalid BUFFER name provided", r.Error());
}

TEST_F(AmberScriptParserTest, BufferMissingName) {
  std::string in = "BUFFER color DATA_TYPE uint8 SIZE 5 FILL 5";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: missing BUFFER name", r.Error());
}

TEST_F(AmberScriptParserTest, BufferInvalidCommand) {
  std::string in = "BUFFER color my_buf 1234";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: invalid BUFFER command provided", r.Error());
}

TEST_F(AmberScriptParserTest, BufferUnknownCommand) {
  std::string in = "BUFFER color my_buf INVALID";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: unknown BUFFER command provided: INVALID", r.Error());
}

TEST_F(AmberScriptParserTest, BufferDimsWithoutFramebuffer) {
  std::string in = "BUFFER color my_buf DIMS 256 256";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: BUFFER DIMS can only be used with a framebuffer", r.Error());
}

TEST_F(AmberScriptParserTest, BufferFramebufferWithoutDims) {
  std::string in = "BUFFER framebuffer my_buf DATA_TYPE int32 FILL 0";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: BUFFER framebuffer must be used with DIMS", r.Error());
}

TEST_F(AmberScriptParserTest, BufferInvalidSize) {
  std::string in = "BUFFER color my_buf DATA_TYPE uint8 SIZE INVALID FILL 5";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: BUFFER size invalid", r.Error());
}

TEST_F(AmberScriptParserTest, BufferMissingSizeValue) {
  std::string in = "BUFFER color my_buf DATA_TYPE uint8 SIZE FILL 5";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: BUFFER size invalid", r.Error());
}

TEST_F(AmberScriptParserTest, BufferMissingFillValue) {
  std::string in = "BUFFER color my_buf DATA_TYPE uint8 SIZE 5 FILL";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: missing BUFFER fill value", r.Error());
}

TEST_F(AmberScriptParserTest, BufferInvalidFillValue) {
  std::string in = "BUFFER color my_buf DATA_TYPE uint8 SIZE 5 FILL INVALID";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: invalid BUFFER fill value", r.Error());
}

TEST_F(AmberScriptParserTest, BufferInvalidInitializer) {
  std::string in = "BUFFER color my_buf DATA_TYPE uint8 SIZE 5 INVALID 5";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: invalid BUFFER initializer provided", r.Error());
}

TEST_F(AmberScriptParserTest, BufferFillInvalidValue) {
  std::string in = "BUFFER color my_buf DATA_TYPE uint8 SIZE 5 FILL INVALID";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: invalid BUFFER fill value", r.Error());
}

TEST_F(AmberScriptParserTest, BufferFillingMissingValue) {
  std::string in = "BUFFER color my_buf DATA_TYPE uint8 SIZE 5 FILL";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: missing BUFFER fill value", r.Error());
}

TEST_F(AmberScriptParserTest, BufferSeriesMissingValue) {
  std::string in =
      "BUFFER color my_buf DATA_TYPE uint8 SIZE 5 SERIES_FROM INC_BY 2";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: invalid BUFFER series_from value", r.Error());
}

TEST_F(AmberScriptParserTest, BufferSeriesMissingInc) {
  std::string in = "BUFFER color my_buf DATA_TYPE uint8 SIZE 5 SERIES_FROM 2";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: missing BUFFER series_from inc_by", r.Error());
}

TEST_F(AmberScriptParserTest, BufferSeriesMissingIncValue) {
  std::string in =
      "BUFFER color my_buf DATA_TYPE uint8 SIZE 5 SERIES_FROM 2 INC_BY";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: missing BUFFER series_from inc_by value", r.Error());
}

TEST_F(AmberScriptParserTest, BufferSeriesInvalidStart) {
  std::string in =
      "BUFFER color my_buf DATA_TYPE uint8 SIZE 5 SERIES_FROM INVALID INC_BY 2";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: invalid BUFFER series_from value", r.Error());
}

TEST_F(AmberScriptParserTest, BufferSeriesInvalidInc) {
  std::string in =
      "BUFFER color my_buf DATA_TYPE uint8 SIZE 5 SERIES_FROM 1 INC_BY INVALID";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: invalid BUFFER series_from inc_by value", r.Error());
}

TEST_F(AmberScriptParserTest, BufferSeriesInvalidSuffix) {
  std::string in =
      "BUFFER color my_buf DATA_TYPE uint8 SIZE 5 SERIES_FROM 1 INVALID 2";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: BUFFER series_from invalid command", r.Error());
}

TEST_F(AmberScriptParserTest, BufferFramebufferMissingDims) {
  std::string in = "BUFFER framebuffer my_frame";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: invalid BUFFER command provided", r.Error());
}

TEST_F(AmberScriptParserTest, BufferFramebufferMissingDimValues) {
  std::string in = "BUFFER framebuffer my_frame DIMS";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: BUFFER framebuffer missing DIMS values", r.Error());
}

TEST_F(AmberScriptParserTest, BufferFramebufferMissingHeight) {
  std::string in = "BUFFER framebuffer my_frame DIMS 256";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: BUFFER framebuffer missing height value", r.Error());
}

TEST_F(AmberScriptParserTest, BufferFramebufferInvalidWidth) {
  std::string in = "BUFFER framebuffer my_frame DIMS INVALID 256";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: BUFFER framebuffer invalid width value", r.Error());
}

TEST_F(AmberScriptParserTest, BufferFramebufferFloatWidth) {
  std::string in = "BUFFER framebuffer my_frame DIMS 2.4 256";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: BUFFER framebuffer invalid width value", r.Error());
}

TEST_F(AmberScriptParserTest, BufferFramebufferInvalidHeight) {
  std::string in = "BUFFER framebuffer my_frame DIMS 256 INVALID";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: BUFFER framebuffer invalid height value", r.Error());
}

TEST_F(AmberScriptParserTest, BufferFramebufferFloatHeight) {
  std::string in = "BUFFER framebuffer my_frame DIMS 256 2.4";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: BUFFER framebuffer invalid height value", r.Error());
}

TEST_F(AmberScriptParserTest, BufferFramebufferTooLarge) {
  std::string in =
      "BUFFER framebuffer my_frame DIMS 99999999999999 9999999999999";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: BUFFER framebuffer size too large", r.Error());
}

TEST_F(AmberScriptParserTest, BufferDataInvalidValueFloatForInt) {
  std::string in = R"(
BUFFER index my_index_buffer DATA_TYPE int32 DATA
1.234
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: invalid BUFFER data value", r.Error());
}

TEST_F(AmberScriptParserTest, BufferDataInvalidValueString) {
  std::string in = R"(
BUFFER index my_index_buffer DATA_TYPE int32 DATA
INVALID
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: invalid BUFFER data value", r.Error());
}

TEST_F(AmberScriptParserTest, BufferDataExtraParams) {
  std::string in = R"(
BUFFER index my_index_buffer DATA_TYPE int32 DATA INVALID
123
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("2: extra parameters after BUFFER data command", r.Error());
}

TEST_F(AmberScriptParserTest, BufferFillExtraParams) {
  std::string in = R"(
BUFFER index my_index_buffer DATA_TYPE int32 SIZE 256 FILL 5 INVALID
123
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("2: extra parameters after BUFFER fill command", r.Error());
}

TEST_F(AmberScriptParserTest, BufferSeriesExtraParams) {
  std::string in =
      "BUFFER index my_buffer DATA_TYPE int32 SIZE 256 SERIES_FROM 2 INC_BY 5 "
      "INVALID";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: extra parameters after BUFFER series_from command", r.Error());
}

TEST_F(AmberScriptParserTest, BufferFramebufferExtraParams) {
  std::string in = "BUFFER framebuffer my_buffer DIMS 256 256 INVALID";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: extra parameters after BUFFER framebuffer command", r.Error());
}

TEST_F(AmberScriptParserTest, BufferDuplicateName) {
  std::string in = R"(
BUFFER vertex my_buf DATA_TYPE int32 SIZE 5 FILL 5
BUFFER index my_buf DATA_TYPE int16 SIZE 5 FILL 2)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: duplicate buffer name provided", r.Error());
}

using AmberScriptParserBufferTypeTest = testing::TestWithParam<BufferTypeData>;
TEST_P(AmberScriptParserBufferTypeTest, BufferTypes) {
  auto test_data = GetParam();

  std::string in = std::string("BUFFER ") + test_data.name +
                   " my_buf DATA_TYPE int32 SIZE 2 FILL 5";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto parent_script = parser.GetScript();
  auto script = ToAmberScript(parent_script.get());
  const auto& buffers = script->GetBuffers();
  ASSERT_EQ(1U, buffers.size());

  ASSERT_TRUE(buffers[0] != nullptr);
  EXPECT_EQ(test_data.type, buffers[0]->GetBufferType());
}
INSTANTIATE_TEST_CASE_P(
    AmberScriptParserTestsBufferType,
    AmberScriptParserBufferTypeTest,
    testing::Values(BufferTypeData{"uniform", BufferType::kUniform},
                    BufferTypeData{"storage", BufferType::kStorage},
                    BufferTypeData{"vertex", BufferType::kVertex},
                    BufferTypeData{"index", BufferType::kIndex},
                    BufferTypeData{"sampled", BufferType::kSampled},
                    BufferTypeData{"color", BufferType::kColor},
                    BufferTypeData{
                        "depth",
                        BufferType::kDepth}), );  // NOLINT(whitespace/parens)

using AmberScriptParserBufferDataTypeTest = testing::TestWithParam<BufferData>;
TEST_P(AmberScriptParserBufferDataTypeTest, BufferTypes) {
  auto test_data = GetParam();

  std::string in = std::string("BUFFER vertex my_buf DATA_TYPE ") +
                   test_data.name + " SIZE 2 FILL 5";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << test_data.name << " :" << r.Error();

  auto parent_script = parser.GetScript();
  auto script = ToAmberScript(parent_script.get());
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

  std::string in = std::string("BUFFER vertex my_buf DATA_TYPE ") +
                   test_data.name + " SIZE 4 FILL 5";

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

}  // namespace amberscript
}  // namespace amber
