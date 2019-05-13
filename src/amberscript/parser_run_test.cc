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
  ASSERT_EQ("12: RUN command requires compute pipeline", r.Error());
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
  ASSERT_EQ("12: RUN command requires graphics pipeline", r.Error());
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

TEST_F(AmberScriptParserTest, RunDrawArrays) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER vtex_buf DATA_TYPE vec3<float> DATA
1 2 3
4 5 6
7 8 9
END

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
  VERTEX_DATA vtex_buf LOCATION 0
END

RUN my_pipeline DRAW_ARRAY AS TRIANGLE_LIST START_IDX 1 COUNT 2)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& commands = script->GetCommands();
  ASSERT_EQ(1U, commands.size());

  ASSERT_TRUE(commands[0]->IsDrawArrays());

  auto* cmd = commands[0]->AsDrawArrays();
  EXPECT_FALSE(cmd->IsIndexed());
  EXPECT_FALSE(cmd->IsInstanced());
  EXPECT_EQ(static_cast<uint32_t>(0U), cmd->GetInstanceCount());
  EXPECT_EQ(Topology::kTriangleList, cmd->GetTopology());
  EXPECT_EQ(1U, cmd->GetFirstVertexIndex());
  EXPECT_EQ(2U, cmd->GetVertexCount());
}

TEST_F(AmberScriptParserTest, RunDrawArraysCountOmitted) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER vtex_buf DATA_TYPE vec3<float> DATA
1 2 3
4 5 6
7 8 9
END

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
  VERTEX_DATA vtex_buf LOCATION 0
END

RUN my_pipeline DRAW_ARRAY AS TRIANGLE_LIST START_IDX 1)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& commands = script->GetCommands();
  ASSERT_EQ(1U, commands.size());

  ASSERT_TRUE(commands[0]->IsDrawArrays());

  auto* cmd = commands[0]->AsDrawArrays();
  EXPECT_FALSE(cmd->IsIndexed());
  EXPECT_FALSE(cmd->IsInstanced());
  EXPECT_EQ(static_cast<uint32_t>(0U), cmd->GetInstanceCount());
  EXPECT_EQ(Topology::kTriangleList, cmd->GetTopology());
  EXPECT_EQ(1U, cmd->GetFirstVertexIndex());
  // There are 3 elements in the vertex buffer, but we start at element 1.
  EXPECT_EQ(2U, cmd->GetVertexCount());
}

TEST_F(AmberScriptParserTest, RunDrawArraysStartIdxAndCountOmitted) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER vtex_buf DATA_TYPE vec3<float> DATA
1 2 3
4 5 6
7 8 9
END

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
  VERTEX_DATA vtex_buf LOCATION 0
END

RUN my_pipeline DRAW_ARRAY AS TRIANGLE_LIST)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& commands = script->GetCommands();
  ASSERT_EQ(1U, commands.size());

  ASSERT_TRUE(commands[0]->IsDrawArrays());

  auto* cmd = commands[0]->AsDrawArrays();
  EXPECT_FALSE(cmd->IsIndexed());
  EXPECT_FALSE(cmd->IsInstanced());
  EXPECT_EQ(static_cast<uint32_t>(0U), cmd->GetInstanceCount());
  EXPECT_EQ(Topology::kTriangleList, cmd->GetTopology());
  EXPECT_EQ(static_cast<uint32_t>(0U), cmd->GetFirstVertexIndex());
  // There are 3 elements in the vertex buffer.
  EXPECT_EQ(3U, cmd->GetVertexCount());
}

TEST_F(AmberScriptParserTest, RunDrawArraysIndexed) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER vtex_buf DATA_TYPE vec3<float> DATA
1 2 3
4 5 6
7 8 9
END
BUFFER idx_buf DATA_TYPE vec3<float> DATA
9 8 7
6 5 4
3 2 1
END

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
  VERTEX_DATA vtex_buf LOCATION 0
  INDEX_DATA idx_buf
END

RUN my_pipeline DRAW_ARRAY AS TRIANGLE_LIST INDEXED)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& commands = script->GetCommands();
  ASSERT_EQ(1U, commands.size());

  ASSERT_TRUE(commands[0]->IsDrawArrays());

  auto* cmd = commands[0]->AsDrawArrays();
  EXPECT_TRUE(cmd->IsIndexed());
  EXPECT_FALSE(cmd->IsInstanced());
  EXPECT_EQ(static_cast<uint32_t>(0U), cmd->GetInstanceCount());
  EXPECT_EQ(Topology::kTriangleList, cmd->GetTopology());
  EXPECT_EQ(static_cast<uint32_t>(0U), cmd->GetFirstVertexIndex());
  // There are 3 elements in the vertex buffer.
  EXPECT_EQ(3U, cmd->GetVertexCount());
}

TEST_F(AmberScriptParserTest, RunDrawArraysIndexedMissingIndexData) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER vtex_buf DATA_TYPE vec3<float> DATA
1 2 3
4 5 6
7 8 9
END

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
  VERTEX_DATA vtex_buf LOCATION 0
END

RUN my_pipeline DRAW_ARRAY AS TRIANGLE_LIST INDEXED)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());

  EXPECT_EQ("18: RUN DRAW_ARRAYS INDEXED requires attached index buffer",
            r.Error());
}

TEST_F(AmberScriptParserTest, RunDrawArraysMissingVertexBuffer) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
END

RUN my_pipeline DRAW_ARRAY TRIANGLE_LIST START_IDX 1 COUNT 2)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("12: RUN DRAW_ARRAY requires attached vertex buffer", r.Error());
}

TEST_F(AmberScriptParserTest, RunDrawArraysMissingAS) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER vtex_buf DATA_TYPE vec3<float> DATA
1 2 3
4 5 6
7 8 9
END

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
  VERTEX_DATA vtex_buf LOCATION 0
END

RUN my_pipeline DRAW_ARRAY TRIANGLE_LIST START_IDX 1 COUNT 2)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("18: missing AS for RUN command", r.Error());
}

TEST_F(AmberScriptParserTest, RunDrawArraysMissingTopology) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER vtex_buf DATA_TYPE vec3<float> DATA
1 2 3
4 5 6
7 8 9
END

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
  VERTEX_DATA vtex_buf LOCATION 0
END

RUN my_pipeline DRAW_ARRAY AS START_IDX 1 COUNT 2)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("18: invalid topology for RUN command: START_IDX", r.Error());
}

TEST_F(AmberScriptParserTest, RunDrawArraysInvalidTopologyFormat) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER vtex_buf DATA_TYPE vec3<float> DATA
1 2 3
4 5 6
7 8 9
END

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
  VERTEX_DATA vtex_buf LOCATION 0
END

RUN my_pipeline DRAW_ARRAY AS 1234 START_IDX 1 COUNT 2)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("18: invalid topology for RUN command: 1234", r.Error());
}

TEST_F(AmberScriptParserTest, RunDrawArraysInvalidTopology) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER vtex_buf DATA_TYPE vec3<float> DATA
1 2 3
4 5 6
7 8 9
END

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
  VERTEX_DATA vtex_buf LOCATION 0
END

RUN my_pipeline DRAW_ARRAY AS INVALID_TOPOLOGY START_IDX 1 COUNT 2)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("18: invalid topology for RUN command: INVALID_TOPOLOGY",
            r.Error());
}

TEST_F(AmberScriptParserTest, RunDrawArraysMissingStartIdx) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER vtex_buf DATA_TYPE vec3<float> DATA
1 2 3
4 5 6
7 8 9
END

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
  VERTEX_DATA vtex_buf LOCATION 0
END

RUN my_pipeline DRAW_ARRAY AS TRIANGLE_LIST 1 COUNT 2)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("18: missing START_IDX for RUN command", r.Error());
}

TEST_F(AmberScriptParserTest, RunDrawArraysMissingStartIdxValue) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER vtex_buf DATA_TYPE vec3<float> DATA
1 2 3
4 5 6
7 8 9
END

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
  VERTEX_DATA vtex_buf LOCATION 0
END

RUN my_pipeline DRAW_ARRAY AS TRIANGLE_LIST START_IDX COUNT 2)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("18: invalid START_IDX value for RUN command: COUNT", r.Error());
}

TEST_F(AmberScriptParserTest, RunDrawArraysInvalidStartIdxValueFormat) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER vtex_buf DATA_TYPE vec3<float> DATA
1 2 3
4 5 6
7 8 9
END

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
  VERTEX_DATA vtex_buf LOCATION 0
END

RUN my_pipeline DRAW_ARRAY AS TRIANGLE_LIST START_IDX INVALID COUNT 2)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("18: invalid START_IDX value for RUN command: INVALID", r.Error());
}

TEST_F(AmberScriptParserTest, RunDrawArraysInvalidStartIdxValue) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER vtex_buf DATA_TYPE vec3<float> DATA
1 2 3
4 5 6
7 8 9
END

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
  VERTEX_DATA vtex_buf LOCATION 0
END

RUN my_pipeline DRAW_ARRAY AS TRIANGLE_LIST START_IDX 1.3 COUNT 2)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("18: invalid START_IDX value for RUN command: 1.3", r.Error());
}

TEST_F(AmberScriptParserTest, RunDrawArraysNegativeStartIdxValue) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER vtex_buf DATA_TYPE vec3<float> DATA
1 2 3
4 5 6
7 8 9
END

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
  VERTEX_DATA vtex_buf LOCATION 0
END

RUN my_pipeline DRAW_ARRAY AS TRIANGLE_LIST START_IDX -1 COUNT 2)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("18: START_IDX value must be >= 0 for RUN command", r.Error());
}

TEST_F(AmberScriptParserTest, RunDrawArraysMissingCount) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER vtex_buf DATA_TYPE vec3<float> DATA
1 2 3
4 5 6
7 8 9
END

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
  VERTEX_DATA vtex_buf LOCATION 0
END

RUN my_pipeline DRAW_ARRAY AS TRIANGLE_LIST START_IDX 1 2)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("18: missing COUNT for RUN command", r.Error());
}

TEST_F(AmberScriptParserTest, RunDrawArraysMissingCountValue) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER vtex_buf DATA_TYPE vec3<float> DATA
1 2 3
4 5 6
7 8 9
END

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
  VERTEX_DATA vtex_buf LOCATION 0
END

RUN my_pipeline DRAW_ARRAY AS TRIANGLE_LIST START_IDX 1 COUNT)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("18: invalid COUNT value for RUN command: ", r.Error());
}

TEST_F(AmberScriptParserTest, RunDrawArraysStartIdxTooLarge) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER vtex_buf DATA_TYPE vec3<float> DATA
1 2 3
4 5 6
7 8 9
END

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
  VERTEX_DATA vtex_buf LOCATION 0
END

RUN my_pipeline DRAW_ARRAY AS TRIANGLE_LIST START_IDX 9 COUNT 1)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("18: START_IDX plus COUNT exceeds vertex buffer data size",
            r.Error());
}

TEST_F(AmberScriptParserTest, RunDrawArraysCountTooLarge) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER vtex_buf DATA_TYPE vec3<float> DATA
1 2 3
4 5 6
7 8 9
END

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
  VERTEX_DATA vtex_buf LOCATION 0
END

RUN my_pipeline DRAW_ARRAY AS TRIANGLE_LIST START_IDX 1 COUNT 9)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("18: START_IDX plus COUNT exceeds vertex buffer data size",
            r.Error());
}

TEST_F(AmberScriptParserTest, RunDrawArraysInvalidCountValueFormat) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER vtex_buf DATA_TYPE vec3<float> DATA
1 2 3
4 5 6
7 8 9
END

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
  VERTEX_DATA vtex_buf LOCATION 0
END

RUN my_pipeline DRAW_ARRAY AS TRIANGLE_LIST START_IDX 1 COUNT INVALID)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("18: invalid COUNT value for RUN command: INVALID", r.Error());
}

TEST_F(AmberScriptParserTest, RunDrawArraysInvalidCountValue) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER vtex_buf DATA_TYPE vec3<float> DATA
1 2 3
4 5 6
7 8 9
END

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
  VERTEX_DATA vtex_buf LOCATION 0
END

RUN my_pipeline DRAW_ARRAY AS TRIANGLE_LIST START_IDX 1 COUNT 2.4)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("18: invalid COUNT value for RUN command: 2.4", r.Error());
}

TEST_F(AmberScriptParserTest, RunDrawArraysNegativeCountValue) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER vtex_buf DATA_TYPE vec3<float> DATA
1 2 3
4 5 6
7 8 9
END

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
  VERTEX_DATA vtex_buf LOCATION 0
END

RUN my_pipeline DRAW_ARRAY AS TRIANGLE_LIST START_IDX 1 COUNT -2)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("18: COUNT value must be > 0 for RUN command", r.Error());
}

TEST_F(AmberScriptParserTest, RunDrawArraysZeroCountValue) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER vtex_buf DATA_TYPE vec3<float> DATA
1 2 3
4 5 6
7 8 9
END

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
  VERTEX_DATA vtex_buf LOCATION 0
END

RUN my_pipeline DRAW_ARRAY AS TRIANGLE_LIST START_IDX 1 COUNT 0)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("18: COUNT value must be > 0 for RUN command", r.Error());
}

}  // namespace amberscript
}  // namespace amber
