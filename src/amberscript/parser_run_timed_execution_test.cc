// Copyright 2024 The Amber Authors.
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

TEST_F(AmberScriptParserTest, RunComputeTimedExecution) {
  std::string in = R"(
SHADER compute my_shader GLSL
void main() {
  gl_FragColor = vec3(2, 3, 4);
}
END

PIPELINE compute my_pipeline
  ATTACH my_shader
END

RUN TIMED_EXECUTION my_pipeline 2 4 5
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
  EXPECT_TRUE(cmd->AsCompute()->IsTimedExecution());
}

TEST_F(AmberScriptParserTest, RunComputeNoTimedExecution) {
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
  EXPECT_FALSE(cmd->AsCompute()->IsTimedExecution());
}

TEST_F(AmberScriptParserTest, RunDrawRectTimedExecution) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
END

RUN TIMED_EXECUTION my_pipeline DRAW_RECT POS 2 4 SIZE 10 20)";

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
  EXPECT_TRUE(cmd->AsDrawRect()->IsTimedExecution());
}

TEST_F(AmberScriptParserTest, RunDrawGridTimedExecution) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
END

RUN TIMED_EXECUTION my_pipeline DRAW_GRID POS 2 4 SIZE 10 20 CELLS 4 5)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& commands = script->GetCommands();
  ASSERT_EQ(1U, commands.size());

  auto* cmd = commands[0].get();
  ASSERT_TRUE(cmd->IsDrawGrid());
  EXPECT_FLOAT_EQ(2.f, cmd->AsDrawGrid()->GetX());
  EXPECT_FLOAT_EQ(4.f, cmd->AsDrawGrid()->GetY());
  EXPECT_FLOAT_EQ(10.f, cmd->AsDrawGrid()->GetWidth());
  EXPECT_FLOAT_EQ(20.f, cmd->AsDrawGrid()->GetHeight());
  EXPECT_EQ(4u, cmd->AsDrawGrid()->GetColumns());
  EXPECT_EQ(5u, cmd->AsDrawGrid()->GetRows());
  EXPECT_TRUE(cmd->AsDrawGrid()->IsTimedExecution());
}

TEST_F(AmberScriptParserTest, RunDrawArraysTimedExecution) {
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

RUN TIMED_EXECUTION my_pipeline DRAW_ARRAY AS TRIANGLE_LIST START_IDX 1 COUNT 2)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& commands = script->GetCommands();
  ASSERT_EQ(1U, commands.size());

  ASSERT_TRUE(commands[0]->IsDrawArrays());

  auto* cmd = commands[0]->AsDrawArrays();
  EXPECT_FALSE(cmd->IsIndexed());
  EXPECT_EQ(static_cast<uint32_t>(1U), cmd->GetInstanceCount());
  EXPECT_EQ(static_cast<uint32_t>(0U), cmd->GetFirstInstance());
  EXPECT_EQ(Topology::kTriangleList, cmd->GetTopology());
  EXPECT_EQ(1U, cmd->GetFirstVertexIndex());
  EXPECT_EQ(2U, cmd->GetVertexCount());
  EXPECT_TRUE(cmd->IsTimedExecution());
}

TEST_F(AmberScriptParserTest, RunDrawArraysInstancedTimedExecution) {
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

RUN TIMED_EXECUTION my_pipeline DRAW_ARRAY AS TRIANGLE_LIST START_IDX 1 COUNT 2 START_INSTANCE 2 INSTANCE_COUNT 10)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& commands = script->GetCommands();
  ASSERT_EQ(1U, commands.size());

  ASSERT_TRUE(commands[0]->IsDrawArrays());

  auto* cmd = commands[0]->AsDrawArrays();
  EXPECT_FALSE(cmd->IsIndexed());
  EXPECT_EQ(static_cast<uint32_t>(10U), cmd->GetInstanceCount());
  EXPECT_EQ(static_cast<uint32_t>(2U), cmd->GetFirstInstance());
  EXPECT_EQ(Topology::kTriangleList, cmd->GetTopology());
  EXPECT_EQ(1U, cmd->GetFirstVertexIndex());
  EXPECT_EQ(2U, cmd->GetVertexCount());
  EXPECT_TRUE(cmd->IsTimedExecution());
}

TEST_F(AmberScriptParserTest, RunDrawArraysIndexedTimedExecution) {
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

RUN TIMED_EXECUTION my_pipeline DRAW_ARRAY AS TRIANGLE_LIST INDEXED)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& commands = script->GetCommands();
  ASSERT_EQ(1U, commands.size());

  ASSERT_TRUE(commands[0]->IsDrawArrays());

  auto* cmd = commands[0]->AsDrawArrays();
  EXPECT_TRUE(cmd->IsIndexed());
  EXPECT_EQ(static_cast<uint32_t>(1U), cmd->GetInstanceCount());
  EXPECT_EQ(static_cast<uint32_t>(0U), cmd->GetFirstInstance());
  EXPECT_EQ(Topology::kTriangleList, cmd->GetTopology());
  EXPECT_EQ(static_cast<uint32_t>(0U), cmd->GetFirstVertexIndex());
  // There are 3 elements in the vertex buffer.
  EXPECT_EQ(3U, cmd->GetVertexCount());
  EXPECT_TRUE(cmd->IsTimedExecution());
}

}  // namespace amberscript
}  // namespace amber
