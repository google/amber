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

namespace amber {
namespace amberscript {

using AmberScriptParserTest = testing::Test;

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
  EXPECT_EQ("4: unknown ATTACH parameter: INVALID", r.Error());
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

struct ShaderTypeData {
  const char* name;
  ShaderType type;
};

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
INSTANTIATE_TEST_SUITE_P(
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
            kShaderTypeTessellationControl}));  // NOLINT(whitespace/parens)

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
  EXPECT_EQ("6: unknown ATTACH parameter: INVALID", r.Error());
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

TEST_F(AmberScriptParserTest, PipelineSpecializationUint32) {
  std::string in = R"(
SHADER compute my_shader GLSL
#shaders
END
PIPELINE compute my_pipeline
  ATTACH my_shader TYPE compute ENTRY_POINT my_ep SPECIALIZE 1 AS uint32 4
END)";

  Parser parser;
  Result r = parser.Parse(in);
  EXPECT_EQ(r.Error(), "");
  ASSERT_TRUE(r.IsSuccess());

  auto script = parser.GetScript();
  const auto& pipelines = script->GetPipelines();
  ASSERT_EQ(1U, pipelines.size());

  const auto* pipeline = pipelines[0].get();
  const auto& shaders = pipeline->GetShaders();
  ASSERT_EQ(1U, shaders.size());

  EXPECT_EQ(1, shaders[0].GetSpecialization().size());
  EXPECT_EQ(4, shaders[0].GetSpecialization().at(1));
}

TEST_F(AmberScriptParserTest, PipelineSpecializationInt32) {
  std::string in = R"(
SHADER compute my_shader GLSL
#shaders
END
PIPELINE compute my_pipeline
  ATTACH my_shader TYPE compute ENTRY_POINT my_ep SPECIALIZE 2 AS int32 -1
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess());

  auto script = parser.GetScript();
  const auto& pipelines = script->GetPipelines();
  ASSERT_EQ(1U, pipelines.size());

  const auto* pipeline = pipelines[0].get();
  const auto& shaders = pipeline->GetShaders();
  ASSERT_EQ(1U, shaders.size());

  EXPECT_EQ(1, shaders[0].GetSpecialization().size());
  EXPECT_EQ(0xffffffff, shaders[0].GetSpecialization().at(2));
}

TEST_F(AmberScriptParserTest, PipelineSpecializationFloat) {
  std::string in = R"(
SHADER compute my_shader GLSL
#shaders
END
PIPELINE compute my_pipeline
  ATTACH my_shader TYPE compute ENTRY_POINT my_ep SPECIALIZE 3 AS float 1.1
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess());

  auto script = parser.GetScript();
  const auto& pipelines = script->GetPipelines();
  ASSERT_EQ(1U, pipelines.size());

  const auto* pipeline = pipelines[0].get();
  const auto& shaders = pipeline->GetShaders();
  ASSERT_EQ(1U, shaders.size());

  EXPECT_EQ(1, shaders[0].GetSpecialization().size());
  EXPECT_EQ(0x3f8ccccd, shaders[0].GetSpecialization().at(3));
}

TEST_F(AmberScriptParserTest, PipelineSpecializationIDIsString) {
  std::string in = R"(
SHADER compute my_shader GLSL
#shaders
END
PIPELINE compute my_pipeline
  ATTACH my_shader TYPE compute ENTRY_POINT my_ep SPECIALIZE s3 AS float 1.1
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("6: specialization ID must be an integer", r.Error());
}

TEST_F(AmberScriptParserTest, PipelineSpecializationNoAS) {
  std::string in = R"(
SHADER compute my_shader GLSL
#shaders
END
PIPELINE compute my_pipeline
  ATTACH my_shader TYPE compute ENTRY_POINT my_ep SPECIALIZE 1 ASa float 1.1
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("6: expected AS as next token", r.Error());
}

TEST_F(AmberScriptParserTest, PipelineSpecializationNotDataType) {
  std::string in = R"(
SHADER compute my_shader GLSL
#shaders
END
PIPELINE compute my_pipeline
  ATTACH my_shader TYPE compute ENTRY_POINT my_ep SPECIALIZE 1 AS uint 1.1
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("6: invalid data type 'uint' provided", r.Error());
}

TEST_F(AmberScriptParserTest, PipelineSpecializationBadDataType) {
  std::string in = R"(
SHADER compute my_shader GLSL
#shaders
END
PIPELINE compute my_pipeline
  ATTACH my_shader ENTRY_POINT my_ep SPECIALIZE 1 AS uint8 1.1
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ(
      "6: only 32-bit types are currently accepted for specialization values",
      r.Error());
}

TEST_F(AmberScriptParserTest, PipelineSpecializationMultipleSpecializations) {
  std::string in = R"(
SHADER compute my_shader GLSL
#shaders
END
PIPELINE compute my_pipeline
  ATTACH my_shader TYPE compute ENTRY_POINT my_ep \
      SPECIALIZE 1 AS uint32 4 \
      SPECIALIZE 2 AS uint32 5 \
      SPECIALIZE 5 AS uint32 1
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess());

  auto script = parser.GetScript();
  const auto& pipelines = script->GetPipelines();
  ASSERT_EQ(1U, pipelines.size());

  const auto* pipeline = pipelines[0].get();
  const auto& shaders = pipeline->GetShaders();
  ASSERT_EQ(1U, shaders.size());

  EXPECT_EQ(3, shaders[0].GetSpecialization().size());
  EXPECT_EQ(4, shaders[0].GetSpecialization().at(1));
  EXPECT_EQ(5, shaders[0].GetSpecialization().at(2));
  EXPECT_EQ(1, shaders[0].GetSpecialization().at(5));
}

TEST_F(AmberScriptParserTest, PipelineSpecializationNoType) {
  std::string in = R"(
SHADER compute my_shader GLSL
#shaders
END
PIPELINE compute my_pipeline
  ATTACH my_shader SPECIALIZE 1 AS uint32 4
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess());

  auto script = parser.GetScript();
  const auto& pipelines = script->GetPipelines();
  ASSERT_EQ(1U, pipelines.size());

  const auto* pipeline = pipelines[0].get();
  const auto& shaders = pipeline->GetShaders();
  ASSERT_EQ(1U, shaders.size());

  EXPECT_EQ(1, shaders[0].GetSpecialization().size());
  EXPECT_EQ(4, shaders[0].GetSpecialization().at(1));
}

}  // namespace amberscript
}  // namespace amber
