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

#include "src/vkscript/command_parser.h"

#include "gtest/gtest.h"
#include "src/pipeline.h"
#include "src/vkscript/section_parser.h"

namespace amber {
namespace vkscript {

using CommandParserTest = testing::Test;

TEST_F(CommandParserTest, MultipleCommands) {
  std::string data = R"(# this is the test data
draw rect 1.2 2.3 200 400.2
# another comment
clear color 255 128 1 100 # set clear color
clear
# done)";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto& cmds = cp.Commands();
  ASSERT_EQ(3U, cmds.size());
  ASSERT_TRUE(cmds[0]->IsDrawRect());

  auto* draw_cmd = cmds[0]->AsDrawRect();
  EXPECT_FALSE(draw_cmd->IsOrtho());
  EXPECT_FALSE(draw_cmd->IsPatch());
  EXPECT_FLOAT_EQ(1.2f, draw_cmd->GetX());
  EXPECT_FLOAT_EQ(2.3f, draw_cmd->GetY());
  EXPECT_FLOAT_EQ(200.0f, draw_cmd->GetWidth());
  EXPECT_FLOAT_EQ(400.2f, draw_cmd->GetHeight());

  ASSERT_TRUE(cmds[1]->IsClearColor());

  auto* clear_cmd = cmds[1]->AsClearColor();
  EXPECT_EQ(255, clear_cmd->GetR());
  EXPECT_EQ(128, clear_cmd->GetG());
  EXPECT_EQ(1, clear_cmd->GetB());
  EXPECT_EQ(100, clear_cmd->GetA());

  ASSERT_TRUE(cmds[2]->IsClear());
}

TEST_F(CommandParserTest, DISABLED_DrawArraysNonInstancedFollowedByCommand) {}

TEST_F(CommandParserTest, DISABLED_DrawArraysInstancedFollowedByCommand) {}

TEST_F(CommandParserTest, DISABLED_UnknownCommand) {}

TEST_F(CommandParserTest, DrawRect) {
  std::string data = "draw rect 1.2 2.3 200 400.2";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto& cmds = cp.Commands();
  ASSERT_EQ(1U, cmds.size());
  ASSERT_TRUE(cmds[0]->IsDrawRect());

  auto* cmd = cmds[0]->AsDrawRect();
  EXPECT_FALSE(cmd->IsOrtho());
  EXPECT_FALSE(cmd->IsPatch());
  EXPECT_FLOAT_EQ(1.2f, cmd->GetX());
  EXPECT_FLOAT_EQ(2.3f, cmd->GetY());
  EXPECT_FLOAT_EQ(200.0f, cmd->GetWidth());
  EXPECT_FLOAT_EQ(400.2f, cmd->GetHeight());
}

TEST_F(CommandParserTest, DrawRectWithOrth) {
  std::string data = "draw rect ortho 1.2 2.3 200 400.2";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto& cmds = cp.Commands();
  ASSERT_EQ(1U, cmds.size());
  ASSERT_TRUE(cmds[0]->IsDrawRect());

  auto* cmd = cmds[0]->AsDrawRect();
  EXPECT_TRUE(cmd->IsOrtho());
  EXPECT_FALSE(cmd->IsPatch());
  EXPECT_FLOAT_EQ(1.2f, cmd->GetX());
  EXPECT_FLOAT_EQ(2.3f, cmd->GetY());
  EXPECT_FLOAT_EQ(200.0f, cmd->GetWidth());
  EXPECT_FLOAT_EQ(400.2f, cmd->GetHeight());
}

TEST_F(CommandParserTest, DrawRectWithPatch) {
  std::string data = "draw rect patch 1.2 2.3 200 400.2";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto& cmds = cp.Commands();
  ASSERT_EQ(1U, cmds.size());
  ASSERT_TRUE(cmds[0]->IsDrawRect());

  auto* cmd = cmds[0]->AsDrawRect();
  EXPECT_FALSE(cmd->IsOrtho());
  EXPECT_TRUE(cmd->IsPatch());
  EXPECT_FLOAT_EQ(1.2f, cmd->GetX());
  EXPECT_FLOAT_EQ(2.3f, cmd->GetY());
  EXPECT_FLOAT_EQ(200.0f, cmd->GetWidth());
  EXPECT_FLOAT_EQ(400.2f, cmd->GetHeight());
}

TEST_F(CommandParserTest, DrawRectWithOrthAndPatch) {
  std::string data = "draw rect ortho patch 1.2 2.3 200 400.2";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto& cmds = cp.Commands();
  ASSERT_EQ(1U, cmds.size());
  ASSERT_TRUE(cmds[0]->IsDrawRect());

  auto* cmd = cmds[0]->AsDrawRect();
  EXPECT_TRUE(cmd->IsOrtho());
  EXPECT_TRUE(cmd->IsPatch());
  EXPECT_FLOAT_EQ(1.2f, cmd->GetX());
  EXPECT_FLOAT_EQ(2.3f, cmd->GetY());
  EXPECT_FLOAT_EQ(200.0f, cmd->GetWidth());
  EXPECT_FLOAT_EQ(400.2f, cmd->GetHeight());
}

TEST_F(CommandParserTest, DrawRectTooShort) {
  std::string data = "draw rect 1.2 2.3 400.2";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Invalid conversion to double", r.Error());
}

TEST_F(CommandParserTest, DrawRectExtraParameters) {
  std::string data = "draw rect ortho patch 1.2 2.3 200 400.2 EXTRA";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Extra parameter to draw rect command: EXTRA", r.Error());
}

TEST_F(CommandParserTest, DrawArrays) {
  std::string data = "draw arrays GL_LINES 2 4";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto& cmds = cp.Commands();
  ASSERT_EQ(1U, cmds.size());
  ASSERT_TRUE(cmds[0]->IsDrawArrays());

  auto* cmd = cmds[0]->AsDrawArrays();
  EXPECT_FALSE(cmd->IsIndexed());
  EXPECT_FALSE(cmd->IsInstanced());
  EXPECT_EQ(static_cast<uint32_t>(0U), cmd->GetInstanceCount());
  EXPECT_EQ(Topology::kLineList, cmd->GetTopology());
  EXPECT_EQ(2U, cmd->GetFirstVertexIndex());
  EXPECT_EQ(4U, cmd->GetVertexCount());
}

TEST_F(CommandParserTest, DrawArraysIndexed) {
  std::string data = "draw arrays indexed TRIANGLE_FAN 2 4";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto& cmds = cp.Commands();
  ASSERT_EQ(1U, cmds.size());
  ASSERT_TRUE(cmds[0]->IsDrawArrays());

  auto* cmd = cmds[0]->AsDrawArrays();
  EXPECT_TRUE(cmd->IsIndexed());
  EXPECT_FALSE(cmd->IsInstanced());
  EXPECT_EQ(static_cast<uint32_t>(0U), cmd->GetInstanceCount());
  EXPECT_EQ(Topology::kTriangleFan, cmd->GetTopology());
  EXPECT_EQ(2U, cmd->GetFirstVertexIndex());
  EXPECT_EQ(4U, cmd->GetVertexCount());
}

TEST_F(CommandParserTest, DrawArraysExtraParams) {
  std::string data = "draw arrays indexed TRIANGLE_FAN 2 4 EXTRA_PARAM";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Extra parameter to draw arrays command: EXTRA_PARAM",
            r.Error());
}

TEST_F(CommandParserTest, DrawArraysInstanced) {
  std::string data = "draw arrays instanced LINE_LIST_WITH_ADJACENCY 2 9";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto& cmds = cp.Commands();
  ASSERT_EQ(1U, cmds.size());
  ASSERT_TRUE(cmds[0]->IsDrawArrays());

  auto* cmd = cmds[0]->AsDrawArrays();
  EXPECT_FALSE(cmd->IsIndexed());
  EXPECT_TRUE(cmd->IsInstanced());
  EXPECT_EQ(static_cast<uint32_t>(0U), cmd->GetInstanceCount());
  EXPECT_EQ(Topology::kLineListWithAdjacency, cmd->GetTopology());
  EXPECT_EQ(2U, cmd->GetFirstVertexIndex());
  EXPECT_EQ(9U, cmd->GetVertexCount());
}

TEST_F(CommandParserTest, DrawArraysInstancedExtraParams) {
  std::string data =
      "draw arrays instanced LINE_LIST_WITH_ADJACENCY 2 9 4 EXTRA_COMMAND";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Extra parameter to draw arrays command: EXTRA_COMMAND",
            r.Error());
}

TEST_F(CommandParserTest, DrawArraysIndexedAndInstanced) {
  std::string data =
      "draw arrays indexed instanced LINE_LIST_WITH_ADJACENCY 3 9";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto& cmds = cp.Commands();
  ASSERT_EQ(1U, cmds.size());
  ASSERT_TRUE(cmds[0]->IsDrawArrays());

  auto* cmd = cmds[0]->AsDrawArrays();
  EXPECT_TRUE(cmd->IsIndexed());
  EXPECT_TRUE(cmd->IsInstanced());
  EXPECT_EQ(static_cast<uint32_t>(0U), cmd->GetInstanceCount());
  EXPECT_EQ(Topology::kLineListWithAdjacency, cmd->GetTopology());
  EXPECT_EQ(3U, cmd->GetFirstVertexIndex());
  EXPECT_EQ(9U, cmd->GetVertexCount());
}

TEST_F(CommandParserTest, DrawArraysInstancedWithCount) {
  std::string data = "draw arrays instanced LINE_LIST_WITH_ADJACENCY 3 9 12";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto& cmds = cp.Commands();
  ASSERT_EQ(1U, cmds.size());
  ASSERT_TRUE(cmds[0]->IsDrawArrays());

  auto* cmd = cmds[0]->AsDrawArrays();
  EXPECT_FALSE(cmd->IsIndexed());
  EXPECT_TRUE(cmd->IsInstanced());
  EXPECT_EQ(12U, cmd->GetInstanceCount());
  EXPECT_EQ(Topology::kLineListWithAdjacency, cmd->GetTopology());
  EXPECT_EQ(3U, cmd->GetFirstVertexIndex());
  EXPECT_EQ(9U, cmd->GetVertexCount());
}

TEST_F(CommandParserTest, DrawArraysBadTopology) {
  std::string data = "draw arrays UNKNOWN_TOPO 1 4";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Unknown parameter to draw arrays: UNKNOWN_TOPO", r.Error());
}

TEST_F(CommandParserTest, DrawArraysTooShort) {
  std::string data = "draw arrays PATCH_LIST 1";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Missing integer vertex count value for draw arrays: ",
            r.Error());
}

TEST_F(CommandParserTest, DrawArraysInstanceCountWithoutInstanced) {
  std::string data = "draw arrays PATCH_LIST 1 2 3";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Extra parameter to draw arrays command: 3", r.Error());
}

TEST_F(CommandParserTest, DrawArraysMissingTopology) {
  std::string data = "draw arrays 1 2";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Missing draw arrays topology", r.Error());
}

TEST_F(CommandParserTest, Compute) {
  std::string data = "compute 1 2 3";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto& cmds = cp.Commands();
  ASSERT_EQ(1U, cmds.size());
  ASSERT_TRUE(cmds[0]->IsCompute());

  auto* cmd = cmds[0]->AsCompute();
  EXPECT_EQ(1U, cmd->GetX());
  EXPECT_EQ(2U, cmd->GetY());
  EXPECT_EQ(3U, cmd->GetZ());
}

TEST_F(CommandParserTest, ComputeTooShort) {
  std::string data = "compute 1 2";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Missing integer value for compute Z entry: ", r.Error());
}

TEST_F(CommandParserTest, ComputeInvalidX) {
  std::string data = "compute 1.2 2 3";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Missing integer value for compute X entry: 1.2", r.Error());
}

TEST_F(CommandParserTest, ComputeInvalidY) {
  std::string data = "compute 1 a 3";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Missing integer value for compute Y entry: a", r.Error());
}

TEST_F(CommandParserTest, ComputeInvalidZ) {
  std::string data = "compute 1 2 1.5";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Missing integer value for compute Z entry: 1.5", r.Error());
}

TEST_F(CommandParserTest, ComputeExtraCommands) {
  std::string data = "compute 1 2 3 EXTRA";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Extra parameter to compute command: EXTRA", r.Error());
}

TEST_F(CommandParserTest, Clear) {
  std::string data = "clear";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto& cmds = cp.Commands();
  ASSERT_EQ(1U, cmds.size());
  ASSERT_TRUE(cmds[0]->IsClear());
}

TEST_F(CommandParserTest, ClearExtraParams) {
  std::string data = "clear EXTRA";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Extra parameter to clear command: EXTRA", r.Error());
}

TEST_F(CommandParserTest, ClearDepth) {
  std::string data = "clear depth 0.8";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto& cmds = cp.Commands();
  ASSERT_EQ(1U, cmds.size());
  ASSERT_TRUE(cmds[0]->IsClearDepth());

  auto* cmd = cmds[0]->AsClearDepth();
  EXPECT_FLOAT_EQ(0.8f, cmd->GetValue());
}

TEST_F(CommandParserTest, ClearDepthMissingValue) {
  std::string data = "clear depth";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Invalid conversion to double", r.Error());
}

TEST_F(CommandParserTest, ClearDepthExtraParameters) {
  std::string data = "clear depth 0.2 EXTRA";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Extra parameter to clear depth command: EXTRA", r.Error());
}

TEST_F(CommandParserTest, ClearStencil) {
  std::string data = "clear stencil 8";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto& cmds = cp.Commands();
  ASSERT_EQ(1U, cmds.size());
  ASSERT_TRUE(cmds[0]->IsClearStencil());

  auto* cmd = cmds[0]->AsClearStencil();
  EXPECT_EQ(8U, cmd->GetValue());
}

TEST_F(CommandParserTest, ClearStencilMissingValue) {
  std::string data = "clear stencil";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Missing stencil value for clear stencil command: ", r.Error());
}

TEST_F(CommandParserTest, ClearStencilExtraParameters) {
  std::string data = "clear stencil 2 EXTRA";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Extra parameter to clear stencil command: EXTRA", r.Error());
}

TEST_F(CommandParserTest, ClearStencilNotInteger) {
  std::string data = "clear stencil 2.3";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Invalid stencil value for clear stencil command: 2.3",
            r.Error());
}

TEST_F(CommandParserTest, ClearColor) {
  std::string data = "clear color 0.8 0.4 0.2 1.3";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto& cmds = cp.Commands();
  ASSERT_EQ(1U, cmds.size());
  ASSERT_TRUE(cmds[0]->IsClearColor());

  auto* cmd = cmds[0]->AsClearColor();
  EXPECT_FLOAT_EQ(0.8f, cmd->GetR());
  EXPECT_FLOAT_EQ(0.4f, cmd->GetG());
  EXPECT_FLOAT_EQ(0.2f, cmd->GetB());
  EXPECT_FLOAT_EQ(1.3f, cmd->GetA());
}

TEST_F(CommandParserTest, ClearColorMissingParams) {
  std::string data = "clear color 0.8 0.4 0.2";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Invalid conversion to double", r.Error());
}

TEST_F(CommandParserTest, ClearColorExtraParams) {
  std::string data = "clear color 0.8 0.4 0.2 1.3 EXTRA";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Extra parameter to clear color command: EXTRA", r.Error());
}

TEST_F(CommandParserTest, ClearColorBadR) {
  std::string data = "clear color a 0.4 0.2 0.4";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Invalid conversion to double", r.Error());
}

TEST_F(CommandParserTest, ClearColorBadG) {
  std::string data = "clear color 0.2 a 0.2 0.4";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Invalid conversion to double", r.Error());
}

TEST_F(CommandParserTest, ClearColorBadB) {
  std::string data = "clear color 0.2 0.4 a 0.2";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Invalid conversion to double", r.Error());
}

TEST_F(CommandParserTest, ClearColorBadA) {
  std::string data = "clear color 0.2 0.4 0.2 a";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Invalid conversion to double", r.Error());
}

TEST_F(CommandParserTest, PatchParameterVertices) {
  std::string data = "patch parameter vertices 9";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto& cmds = cp.Commands();
  ASSERT_EQ(1U, cmds.size());
  ASSERT_TRUE(cmds[0]->IsPatchParameterVertices());

  auto* cmd = cmds[0]->AsPatchParameterVertices();
  EXPECT_EQ(9U, cmd->GetControlPointCount());
}

TEST_F(CommandParserTest, PatchParameterVerticesMissingParameter) {
  std::string data = "patch vertices 5";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Missing parameter flag to patch command: vertices", r.Error());
}

TEST_F(CommandParserTest, PatchParameterVerticesMissingVertices) {
  std::string data = "patch parameter 5";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Missing vertices flag to patch command: 5", r.Error());
}

TEST_F(CommandParserTest, PatchParameterVerticesMissingParam) {
  std::string data = "patch parameter vertices";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Invalid count parameter for patch parameter vertices: ",
            r.Error());
}

TEST_F(CommandParserTest, PatchParameterVerticesInvalidParam) {
  std::string data = "patch parameter vertices invalid";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Invalid count parameter for patch parameter vertices: invalid",
            r.Error());
}

TEST_F(CommandParserTest, PatchParameterVerticesExtraParam) {
  std::string data = "patch parameter vertices 3 EXTRA";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Extra parameter for patch parameter vertices command: EXTRA",
            r.Error());
}

struct EntryInfo {
  const char* name;
  ShaderType type;
};
static const EntryInfo kEntryPoints[] = {
    {"vertex", kShaderTypeVertex},
    {"fragment", kShaderTypeFragment},
    {"geometry", kShaderTypeGeometry},
    {"compute", kShaderTypeCompute},
    {"tessellation evaluation", kShaderTypeTessellationEvaluation},
    {"tessellation control", kShaderTypeTessellationControl},
};

TEST_F(CommandParserTest, EntryPoint) {
  for (const auto& ep : kEntryPoints) {
    std::string data = std::string(ep.name) + " entrypoint main";

    Pipeline pipeline(PipelineType::kGraphics);
    Script script;
    CommandParser cp(&script, &pipeline, 1, data);
    Result r = cp.Parse();
    ASSERT_TRUE(r.IsSuccess()) << r.Error();

    auto& cmds = cp.Commands();
    ASSERT_EQ(1U, cmds.size());
    ASSERT_TRUE(cmds[0]->IsEntryPoint());

    auto* cmd = cmds[0]->AsEntryPoint();
    EXPECT_EQ(ep.type, cmd->GetShaderType());
    EXPECT_EQ("main", cmd->GetEntryPointName());
  }
}

TEST_F(CommandParserTest, EntryPointNameMissing) {
  for (const auto& ep : kEntryPoints) {
    std::string data = std::string(ep.name) + " entrypoint";

    Pipeline pipeline(PipelineType::kGraphics);
    Script script;
    CommandParser cp(&script, &pipeline, 1, data);
    Result r = cp.Parse();
    ASSERT_FALSE(r.IsSuccess());
    EXPECT_EQ("1: Missing entrypoint name", r.Error());
  }
}

TEST_F(CommandParserTest, EntryPointEntryPointMissing) {
  for (const auto& ep : kEntryPoints) {
    // Skip compute because compute is also a command ....
    if (std::string(ep.name) == "compute")
      continue;

    std::string data = std::string(ep.name) + " main";

    Pipeline pipeline(PipelineType::kGraphics);
    Script script;
    CommandParser cp(&script, &pipeline, 1, data);
    Result r = cp.Parse();
    ASSERT_FALSE(r.IsSuccess());
    EXPECT_EQ("1: Unknown command: " + std::string(ep.name), r.Error());
  }
}

TEST_F(CommandParserTest, EntryPointExtraParam) {
  for (const auto& ep : kEntryPoints) {
    std::string data = std::string(ep.name) + " entrypoint main EXTRA";

    Pipeline pipeline(PipelineType::kGraphics);
    Script script;
    CommandParser cp(&script, &pipeline, 1, data);
    Result r = cp.Parse();
    ASSERT_FALSE(r.IsSuccess());
    EXPECT_EQ("1: Extra parameter for entrypoint command: EXTRA", r.Error());
  }
}

TEST_F(CommandParserTest, EntryPointInvalidValue) {
  for (const auto& ep : kEntryPoints) {
    std::string data = std::string(ep.name) + " entrypoint 123";

    Pipeline pipeline(PipelineType::kGraphics);
    Script script;
    CommandParser cp(&script, &pipeline, 1, data);
    Result r = cp.Parse();
    ASSERT_FALSE(r.IsSuccess());
    EXPECT_EQ("1: Entrypoint name must be a string: 123", r.Error());
  }
}

TEST_F(CommandParserTest, TessellationEntryPointRequiresASuffix) {
  std::string data = "tessellation entrypoint main";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ(
      "1: Tessellation entrypoint must have <evaluation|control> in name: "
      "entrypoint",
      r.Error());
}

TEST_F(CommandParserTest, TessellationEntryPointRequiresAKnownSuffix) {
  std::string data = "tessellation unknown entrypoint main";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ(
      "1: Tessellation entrypoint must have <evaluation|control> in name: "
      "unknown",
      r.Error());
}

TEST_F(CommandParserTest, InvalidEntryPoint) {
  std::string data = "unknown entrypoint main";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Unknown command: unknown", r.Error());
}

using CommandParserProbeTest = testing::TestWithParam<bool>;

TEST_P(CommandParserProbeTest, ProbeRgb) {
  bool is_relative = GetParam();

  std::string data = (is_relative ? std::string("relative ") : std::string()) +
                     "probe rgb 25 30 0.2 0.4 0.6";

  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();
  pipeline.AddColorAttachment(color_buf.get(), 0);

  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << data << std::endl << r.Error();

  auto& cmds = cp.Commands();
  ASSERT_EQ(1U, cmds.size());
  ASSERT_TRUE(cmds[0]->IsProbe());

  auto* cmd = cmds[0]->AsProbe();
  EXPECT_EQ(is_relative, cmd->IsRelative());
  EXPECT_FALSE(cmd->IsWholeWindow());
  EXPECT_FALSE(cmd->IsProbeRect());
  EXPECT_FALSE(cmd->IsRGBA());

  EXPECT_FLOAT_EQ(25U, cmd->GetX());
  EXPECT_FLOAT_EQ(30U, cmd->GetY());
  EXPECT_FLOAT_EQ(1U, cmd->GetWidth());
  EXPECT_FLOAT_EQ(1U, cmd->GetHeight());

  EXPECT_FLOAT_EQ(0.2f, cmd->GetR());
  EXPECT_FLOAT_EQ(0.4f, cmd->GetG());
  EXPECT_FLOAT_EQ(0.6f, cmd->GetB());
}

TEST_P(CommandParserProbeTest, ProbeRgba) {
  bool is_relative = GetParam();

  std::string data = (is_relative ? std::string("relative ") : std::string()) +
                     "probe rgba 25 30 1 255 9 4";

  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();
  pipeline.AddColorAttachment(color_buf.get(), 0);

  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << data << std::endl << r.Error();

  auto& cmds = cp.Commands();
  ASSERT_EQ(1U, cmds.size());
  ASSERT_TRUE(cmds[0]->IsProbe());

  auto* cmd = cmds[0]->AsProbe();
  EXPECT_EQ(is_relative, cmd->IsRelative());
  EXPECT_FALSE(cmd->IsWholeWindow());
  EXPECT_FALSE(cmd->IsProbeRect());
  EXPECT_TRUE(cmd->IsRGBA());

  EXPECT_FLOAT_EQ(25U, cmd->GetX());
  EXPECT_FLOAT_EQ(30U, cmd->GetY());
  EXPECT_FLOAT_EQ(1U, cmd->GetWidth());
  EXPECT_FLOAT_EQ(1U, cmd->GetHeight());

  EXPECT_FLOAT_EQ(1.0f, cmd->GetR());
  EXPECT_FLOAT_EQ(255.0f, cmd->GetG());
  EXPECT_FLOAT_EQ(9.0f, cmd->GetB());
  EXPECT_FLOAT_EQ(4.0f, cmd->GetA());
}

TEST_P(CommandParserProbeTest, ProbeRect) {
  bool is_relative = GetParam();

  std::string data = (is_relative ? std::string("relative ") : std::string()) +
                     "probe rect rgba 25 30 200 400 1 255 9 4";

  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();
  pipeline.AddColorAttachment(color_buf.get(), 0);

  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << data << std::endl << r.Error();

  auto& cmds = cp.Commands();
  ASSERT_EQ(1U, cmds.size());
  ASSERT_TRUE(cmds[0]->IsProbe());

  auto* cmd = cmds[0]->AsProbe();
  EXPECT_EQ(is_relative, cmd->IsRelative());
  EXPECT_FALSE(cmd->IsWholeWindow());
  EXPECT_TRUE(cmd->IsProbeRect());
  EXPECT_TRUE(cmd->IsRGBA());

  EXPECT_FLOAT_EQ(25U, cmd->GetX());
  EXPECT_FLOAT_EQ(30U, cmd->GetY());
  EXPECT_FLOAT_EQ(200U, cmd->GetWidth());
  EXPECT_FLOAT_EQ(400U, cmd->GetHeight());

  EXPECT_FLOAT_EQ(1.0f, cmd->GetR());
  EXPECT_FLOAT_EQ(255.0f, cmd->GetG());
  EXPECT_FLOAT_EQ(9.0f, cmd->GetB());
  EXPECT_FLOAT_EQ(4.0f, cmd->GetA());
}

TEST_P(CommandParserProbeTest, ProbeNotRect) {
  bool is_relative = GetParam();

  std::string data = (is_relative ? std::string("relative ") : std::string()) +
                     "probe rgba 25 30 1 255 9 4";

  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();
  pipeline.AddColorAttachment(color_buf.get(), 0);

  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << data << std::endl << r.Error();

  auto& cmds = cp.Commands();
  ASSERT_EQ(1U, cmds.size());
  ASSERT_TRUE(cmds[0]->IsProbe());

  auto* cmd = cmds[0]->AsProbe();
  EXPECT_EQ(is_relative, cmd->IsRelative());
  EXPECT_FALSE(cmd->IsWholeWindow());
  EXPECT_FALSE(cmd->IsProbeRect());
  EXPECT_TRUE(cmd->IsRGBA());

  EXPECT_FLOAT_EQ(25U, cmd->GetX());
  EXPECT_FLOAT_EQ(30U, cmd->GetY());
  EXPECT_FLOAT_EQ(1.0f, cmd->GetWidth());
  EXPECT_FLOAT_EQ(1.0f, cmd->GetHeight());

  EXPECT_FLOAT_EQ(1.0f, cmd->GetR());
  EXPECT_FLOAT_EQ(255.0f, cmd->GetG());
  EXPECT_FLOAT_EQ(9.0f, cmd->GetB());
  EXPECT_FLOAT_EQ(4.0f, cmd->GetA());
}

INSTANTIATE_TEST_SUITE_P(ProbeTests,
                         CommandParserProbeTest,
                         testing::Values(false,
                                         true));  // NOLINT(whitespace/parens)

TEST_F(CommandParserTest, ProbeAllRGB) {
  std::string data = "probe all rgb 0.2 0.3 0.4";

  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();
  pipeline.AddColorAttachment(color_buf.get(), 0);

  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto& cmds = cp.Commands();
  ASSERT_EQ(1U, cmds.size());
  ASSERT_TRUE(cmds[0]->IsProbe());

  auto* cmd = cmds[0]->AsProbe();
  EXPECT_FALSE(cmd->IsRelative());
  EXPECT_TRUE(cmd->IsWholeWindow());
  EXPECT_TRUE(cmd->IsProbeRect());
  EXPECT_FALSE(cmd->IsRGBA());

  EXPECT_FLOAT_EQ(0.2f, cmd->GetR());
  EXPECT_FLOAT_EQ(0.3f, cmd->GetG());
  EXPECT_FLOAT_EQ(0.4f, cmd->GetB());
}

TEST_F(CommandParserTest, ProbeAllRGBA) {
  std::string data = "probe all rgba 0.2 0.3 0.4 0.5";

  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();
  pipeline.AddColorAttachment(color_buf.get(), 0);

  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto& cmds = cp.Commands();
  ASSERT_EQ(1U, cmds.size());
  ASSERT_TRUE(cmds[0]->IsProbe());

  auto* cmd = cmds[0]->AsProbe();
  EXPECT_FALSE(cmd->IsRelative());
  EXPECT_TRUE(cmd->IsWholeWindow());
  EXPECT_TRUE(cmd->IsProbeRect());
  EXPECT_TRUE(cmd->IsRGBA());

  EXPECT_FLOAT_EQ(0.2f, cmd->GetR());
  EXPECT_FLOAT_EQ(0.3f, cmd->GetG());
  EXPECT_FLOAT_EQ(0.4f, cmd->GetB());
  EXPECT_FLOAT_EQ(0.5f, cmd->GetA());
}

TEST_F(CommandParserTest, ProbeCommandRectBrackets) {
  std::string data = "relative probe rect rgb (0.5, 0.6, 0.3, 0.4) 1 2 3";

  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();
  pipeline.AddColorAttachment(color_buf.get(), 0);

  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto& cmds = cp.Commands();
  ASSERT_EQ(1U, cmds.size());
  ASSERT_TRUE(cmds[0]->IsProbe());

  auto* cmd = cmds[0]->AsProbe();
  EXPECT_TRUE(cmd->IsRelative());
  EXPECT_FALSE(cmd->IsWholeWindow());
  EXPECT_TRUE(cmd->IsProbeRect());
  EXPECT_FALSE(cmd->IsRGBA());

  EXPECT_FLOAT_EQ(0.5f, cmd->GetX());
  EXPECT_FLOAT_EQ(0.6f, cmd->GetY());
  EXPECT_FLOAT_EQ(0.3f, cmd->GetWidth());
  EXPECT_FLOAT_EQ(0.4f, cmd->GetHeight());

  EXPECT_FLOAT_EQ(1.0f, cmd->GetR());
  EXPECT_FLOAT_EQ(2.0f, cmd->GetG());
  EXPECT_FLOAT_EQ(3.0f, cmd->GetB());
}

TEST_F(CommandParserTest, ProbeCommandNotRectBrackets) {
  std::string data = "relative probe rgb (0.5, 0.6) 1 2 3";

  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();
  pipeline.AddColorAttachment(color_buf.get(), 0);

  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto& cmds = cp.Commands();
  ASSERT_EQ(1U, cmds.size());
  ASSERT_TRUE(cmds[0]->IsProbe());

  auto* cmd = cmds[0]->AsProbe();
  EXPECT_TRUE(cmd->IsRelative());
  EXPECT_FALSE(cmd->IsWholeWindow());
  EXPECT_FALSE(cmd->IsProbeRect());
  EXPECT_FALSE(cmd->IsRGBA());

  EXPECT_FLOAT_EQ(0.5f, cmd->GetX());
  EXPECT_FLOAT_EQ(0.6f, cmd->GetY());
  EXPECT_FLOAT_EQ(1.0f, cmd->GetWidth());
  EXPECT_FLOAT_EQ(1.0f, cmd->GetHeight());

  EXPECT_FLOAT_EQ(1.0f, cmd->GetR());
  EXPECT_FLOAT_EQ(2.0f, cmd->GetG());
  EXPECT_FLOAT_EQ(3.0f, cmd->GetB());
}

TEST_F(CommandParserTest, ProbeCommandColorBrackets) {
  std::string data = "relative probe rect rgb 0.5 0.6 0.3 0.4 (1, 2, 3)";

  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();
  pipeline.AddColorAttachment(color_buf.get(), 0);

  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto& cmds = cp.Commands();
  ASSERT_EQ(1U, cmds.size());
  ASSERT_TRUE(cmds[0]->IsProbe());

  auto* cmd = cmds[0]->AsProbe();
  EXPECT_TRUE(cmd->IsRelative());
  EXPECT_FALSE(cmd->IsWholeWindow());
  EXPECT_TRUE(cmd->IsProbeRect());
  EXPECT_FALSE(cmd->IsRGBA());

  EXPECT_FLOAT_EQ(0.5f, cmd->GetX());
  EXPECT_FLOAT_EQ(0.6f, cmd->GetY());
  EXPECT_FLOAT_EQ(0.3f, cmd->GetWidth());
  EXPECT_FLOAT_EQ(0.4f, cmd->GetHeight());

  EXPECT_FLOAT_EQ(1.0f, cmd->GetR());
  EXPECT_FLOAT_EQ(2.0f, cmd->GetG());
  EXPECT_FLOAT_EQ(3.0f, cmd->GetB());
}

TEST_F(CommandParserTest, ProbeCommandColorOptionalCommas) {
  std::string data = "relative probe rect rgb 0.5, 0.6, 0.3 0.4 1 2 3";

  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();
  pipeline.AddColorAttachment(color_buf.get(), 0);

  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto& cmds = cp.Commands();
  ASSERT_EQ(1U, cmds.size());
  ASSERT_TRUE(cmds[0]->IsProbe());

  auto* cmd = cmds[0]->AsProbe();
  EXPECT_TRUE(cmd->IsRelative());
  EXPECT_FALSE(cmd->IsWholeWindow());
  EXPECT_TRUE(cmd->IsProbeRect());
  EXPECT_FALSE(cmd->IsRGBA());

  EXPECT_FLOAT_EQ(0.5f, cmd->GetX());
  EXPECT_FLOAT_EQ(0.6f, cmd->GetY());
  EXPECT_FLOAT_EQ(0.3f, cmd->GetWidth());
  EXPECT_FLOAT_EQ(0.4f, cmd->GetHeight());

  EXPECT_FLOAT_EQ(1.0f, cmd->GetR());
  EXPECT_FLOAT_EQ(2.0f, cmd->GetG());
  EXPECT_FLOAT_EQ(3.0f, cmd->GetB());
}

TEST_F(CommandParserTest, ProbeErrors) {
  struct {
    const char* str;
    const char* err;
  } probes[] = {
      {"probe rgba ab 30 0.2 0.3 0.4 0.5", "Invalid conversion to double"},
      {"relative probe rgba ab 30 0.2 0.3 0.4 0.5",
       "Invalid conversion to double"},
      {"probe rect rgba ab 30 2 3 0.2 0.3 0.4 0.5",
       "Invalid conversion to double"},
      {"relative probe rect rgba ab 30 2 3 0.2 0.3 0.4 0.5",
       "Invalid conversion to double"},

      {"probe rgba 30 ab 0.2 0.3 0.4 0.5", "Invalid conversion to double"},
      {"relative probe rgba 30 ab 0.2 0.3 0.4 0.5",
       "Invalid conversion to double"},
      {"probe rect rgba 30 ab 2 3 0.2 0.3 0.4 0.5",
       "Invalid conversion to double"},
      {"relative probe rect rgba 30 ab 2 3 0.2 0.3 0.4 0.5",
       "Invalid conversion to double"},

      {"probe rect rgba 30 40 ab 3 0.2 0.3 0.4 0.5",
       "Invalid conversion to double"},
      {"relative probe rect rgba 30 40 ab 3 0.2 0.3 0.4 0.5",
       "Invalid conversion to double"},

      {"probe rect rgba 30 40 3 ab 0.2 0.3 0.4 0.5",
       "Invalid conversion to double"},
      {"relative probe rect rgba 30 40 3 ab 0.2 0.3 0.4 0.5",
       "Invalid conversion to double"},

      {"probe rgba 10 30 ab 0.3 0.4 0.5", "Invalid conversion to double"},
      {"relative probe rgba 10 30 ab 0.3 0.4 0.5",
       "Invalid conversion to double"},
      {"probe rect rgba 10 30 2 3 ab 0.3 0.4 0.5",
       "Invalid conversion to double"},
      {"relative probe rect rgba 10 30 2 3 ab 0.3 0.4 0.5",
       "Invalid conversion to double"},

      {"probe rgba 10 30 0.2 ab 0.4 0.5", "Invalid conversion to double"},
      {"relative probe rgba 10 30 0.2 ab 0.4 0.5",
       "Invalid conversion to double"},
      {"probe rect rgba 10 30 2 3 0.2 ab 0.4 0.5",
       "Invalid conversion to double"},
      {"relative probe rect rgba 10 30 2 3 0.2 ab 0.4 0.5",
       "Invalid conversion to double"},

      {"probe rgba 10 30 0.2 0.3 ab 0.5", "Invalid conversion to double"},
      {"relative probe rgba 10 30 0.2 0.3 ab 0.5",
       "Invalid conversion to double"},
      {"probe rect rgba 10 30 2 3 0.2 0.3 ab 0.5",
       "Invalid conversion to double"},
      {"relative probe rect rgba 10 30 2 3 0.2 0.3 ab 0.5",
       "Invalid conversion to double"},

      {"probe rgba 10 30 0.2 0.3 0.4 ab", "Invalid conversion to double"},
      {"relative probe rgba 10 30 0.2 0.3 0.4 ab",
       "Invalid conversion to double"},
      {"probe rect rgba 10 30 2 3 0.2 0.3 0.4 ab",
       "Invalid conversion to double"},
      {"relative probe rect rgba 10 30 2 3 0.2 0.3 0.4 ab",
       "Invalid conversion to double"},

      {"probe all rgb ab 2 3", "Invalid conversion to double"},
      {"probe all rgb 2 ab 4", "Invalid conversion to double"},
      {"probe all rgb 2 3 ab", "Invalid conversion to double"},

      {"probe all rgba ab 2 3 4", "Invalid conversion to double"},
      {"probe all rgba 2 ab 4 5", "Invalid conversion to double"},
      {"probe all rgba 2 3 ab 5", "Invalid conversion to double"},
      {"probe all rgba 2 3 4 ab", "Invalid conversion to double"},

      {"probe rgb 10 30 0.2 0.3 0.4 extra",
       "Extra parameter to probe command: extra"},
      {"probe rgba 10 30 0.2 0.3 0.4 0.4 extra",
       "Extra parameter to probe command: extra"},
      {"relative probe rgb 10 30 0.2 0.3 0.4 extra",
       "Extra parameter to probe command: extra"},
      {"relative probe rgba 10 30 0.2 0.3 0.4 0.4 extra",
       "Extra parameter to probe command: extra"},
      {"probe rect rgb 10 30 40 50 0.2 0.3 0.4 extra",
       "Extra parameter to probe command: extra"},
      {"probe rect rgba 10 30 40 50 0.2 0.3 0.4 0.4 extra",
       "Extra parameter to probe command: extra"},
      {"relative probe rect rgb 10 30 40 50 0.2 0.3 0.4 extra",
       "Extra parameter to probe command: extra"},
      {"relative probe rect rgba 10 30 40 50 0.2 0.3 0.4 0.4 extra",
       "Extra parameter to probe command: extra"},
      {"probe all rgb 2 3 4 extra", "Extra parameter to probe command: extra"},
      {"probe all rgba 2 3 4 5 extra",
       "Extra parameter to probe command: extra"},

      {"relative probe rect rgb 0.5 0.6 0.3 0.4 1 2 3)",
       "Missing open bracket for probe command"},
      {"relative probe rect rgb (0.5 0.6 0.3 0.4 1 2 3",
       "Missing close bracket for probe command"},
      {"relative probe rect rgb 0.5 0.6 0.3 0.4) 1 2 3",
       "Missing open bracket for probe command"},
      {"relative probe rect rgb 0.5 0.6 0.3 0.4 (1, 2, 3",
       "Missing close bracket for probe command"},
      {"relative probe rect rgb (0.5, 0.6, 0.3, 0.4, 1, 2, 3)",
       "Missing close bracket for probe command"},
  };

  for (const auto& probe : probes) {
    Pipeline pipeline(PipelineType::kGraphics);
    auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();
    pipeline.AddColorAttachment(color_buf.get(), 0);

    Script script;
    CommandParser cp(&script, &pipeline, 1, probe.str);
    Result r = cp.Parse();
    EXPECT_FALSE(r.IsSuccess()) << probe.str;
    EXPECT_EQ(std::string("1: ") + probe.err, r.Error()) << probe.str;
  }
}

TEST_F(CommandParserTest, RelativeWithoutProbe) {
  std::string data = "relative unknown";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: relative must be used with probe: unknown", r.Error());
}

TEST_F(CommandParserTest, ProbeWithInvalidRGBA) {
  std::string data = "probe 1";

  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();
  pipeline.AddColorAttachment(color_buf.get(), 0);

  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Invalid token in probe command: 1", r.Error());
}

TEST_F(CommandParserTest, ProbeWithRectAndInvalidRGB) {
  std::string data = "probe rect 1";

  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();
  pipeline.AddColorAttachment(color_buf.get(), 0);

  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Invalid token in probe command: 1", r.Error());
}

TEST_F(CommandParserTest, ProbeWithRectMissingFormat) {
  std::string data = "probe rect unknown";

  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();
  pipeline.AddColorAttachment(color_buf.get(), 0);

  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Invalid format specified to probe command: unknown", r.Error());
}

TEST_F(CommandParserTest, ProbeAllMissingFormat) {
  std::string data = "probe all unknown";

  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();
  pipeline.AddColorAttachment(color_buf.get(), 0);

  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Invalid format specified to probe command: unknown", r.Error());
}

TEST_F(CommandParserTest, ProbeAlWithInvalidRGB) {
  std::string data = "probe all unknown";

  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();
  pipeline.AddColorAttachment(color_buf.get(), 0);

  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Invalid format specified to probe command: unknown", r.Error());
}

struct TopologyTestData {
  const char* name;
  Topology value;
};
using CommandDataPipelineTopologyParser =
    testing::TestWithParam<TopologyTestData>;

TEST_P(CommandDataPipelineTopologyParser, Topology) {
  const auto& test_data = GetParam();

  std::string data = "topology " + std::string(test_data.name);

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  EXPECT_EQ(test_data.value, cp.PipelineDataForTesting()->GetTopology());
}

INSTANTIATE_TEST_SUITE_P(
    TopologyTests,
    CommandDataPipelineTopologyParser,
    testing::Values(
        TopologyTestData{"VK_PRIMITIVE_TOPOLOGY_PATCH_LIST",
                         Topology::kPatchList},
        TopologyTestData{"VK_PRIMITIVE_TOPOLOGY_POINT_LIST",
                         Topology::kPointList},
        TopologyTestData{"VK_PRIMITIVE_TOPOLOGY_LINE_LIST",
                         Topology::kLineList},
        TopologyTestData{"VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY",
                         Topology::kLineListWithAdjacency},
        TopologyTestData{"VK_PRIMITIVE_TOPOLOGY_LINE_STRIP",
                         Topology::kLineStrip},
        TopologyTestData{"VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY",
                         Topology::kLineStripWithAdjacency},
        TopologyTestData{"VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN",
                         Topology::kTriangleFan},
        TopologyTestData{"VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST",
                         Topology::kTriangleList},
        TopologyTestData{"VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY",
                         Topology::kTriangleListWithAdjacency},
        TopologyTestData{"VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP",
                         Topology::kTriangleStrip},
        TopologyTestData{
            "VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY",
            Topology::
                kTriangleStripWithAdjacency}));  // NOLINT(whitespace/parens)

struct PipelineDataInvalidTest {
  const char* name;
  const char* arg;
};
using CommandDataPipelineDataInvalidParser =
    testing::TestWithParam<PipelineDataInvalidTest>;

TEST_P(CommandDataPipelineDataInvalidParser, InvalidPipelineParamValue) {
  const auto& test_data = GetParam();

  std::string data = std::string(test_data.name) + " 123";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ(
      std::string("1: Invalid value for ") + test_data.name + " command: 123",
      r.Error());
}

TEST_P(CommandDataPipelineDataInvalidParser, MissingTopologyValue) {
  const auto& test_data = GetParam();

  std::string data = test_data.name;

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ(std::string("1: Missing value for ") + test_data.name + " command",
            r.Error());
}

TEST_P(CommandDataPipelineDataInvalidParser, UnknownPipelineParamValue) {
  const auto& test_data = GetParam();

  std::string data = std::string(test_data.name) + " UNKNOWN";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ(std::string("1: Unknown value for ") + test_data.name +
                " command: UNKNOWN",
            r.Error());
}

TEST_P(CommandDataPipelineDataInvalidParser, ExtraPipelineParamValue) {
  const auto& test_data = GetParam();

  // CullMode consumes all parameters, so skip this test.
  if (std::string(test_data.name) == "cullMode")
    return;

  std::string data =
      std::string(test_data.name) + " " + test_data.arg + " EXTRA";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ(std::string("1: Extra parameter for ") + test_data.name +
                " command: EXTRA",
            r.Error());
}

INSTANTIATE_TEST_SUITE_P(
    PipelineDataInvalidTests,
    CommandDataPipelineDataInvalidParser,
    testing::Values(
        PipelineDataInvalidTest{"topology", "VK_PRIMITIVE_TOPOLOGY_POINT_LIST"},
        PipelineDataInvalidTest{"polygonMode", "VK_POLYGON_MODE_POINT"},
        PipelineDataInvalidTest{"cullMode", "VK_CULL_MODE_BACK_BIT"},
        PipelineDataInvalidTest{"frontFace", "VK_FRONT_FACE_COUNTER_CLOCKWISE"},
        PipelineDataInvalidTest{
            "logicOp", "VK_LOGIC_OP_NO_OP"}));  // NOLINT(whitespace/parens)

TEST_F(CommandParserTest, BooleanTrue) {
  struct {
    const char* name;
  } data[] = {{"TRUE"}, {"true"}, {"TRuE"}};

  for (const auto& d : data) {
    Pipeline pipeline(PipelineType::kGraphics);
    Script script;
    CommandParser cp(&script, &pipeline, 1, "unused");

    bool value = false;
    Result r = cp.ParseBooleanForTesting(d.name, &value);
    EXPECT_TRUE(r.IsSuccess()) << r.Error();
    EXPECT_TRUE(value);
  }
}

TEST_F(CommandParserTest, BooleanFalse) {
  struct {
    const char* name;
  } data[] = {{"FALSE"}, {"false"}, {"FAlsE"}};

  for (const auto& d : data) {
    Pipeline pipeline(PipelineType::kGraphics);
    Script script;
    CommandParser cp(&script, &pipeline, 1, "unused");

    bool value = true;
    Result r = cp.ParseBooleanForTesting(d.name, &value);
    EXPECT_TRUE(r.IsSuccess()) << d.name << " " << r.Error();
    EXPECT_FALSE(value);
  }
}

TEST_F(CommandParserTest, BooleanInvalid) {
  struct {
    const char* name;
  } data[] = {{""}, {"Invalid"}};

  for (const auto& d : data) {
    Pipeline pipeline(PipelineType::kGraphics);
    Script script;
    CommandParser cp(&script, &pipeline, 1, "unused");

    bool value = true;
    Result r = cp.ParseBooleanForTesting(d.name, &value);
    ASSERT_FALSE(r.IsSuccess()) << d.name;
    EXPECT_EQ(
        std::string("Invalid value passed as a boolean string: ") + d.name,
        r.Error());
  }
}

TEST_F(CommandParserTest, PrimitiveRestartEnable) {
  std::string data = "primitiveRestartEnable true";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  EXPECT_TRUE(cp.PipelineDataForTesting()->GetEnablePrimitiveRestart());
}

TEST_F(CommandParserTest, PrimitiveRestartDisable) {
  std::string data = "primitiveRestartEnable false";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  EXPECT_FALSE(cp.PipelineDataForTesting()->GetEnablePrimitiveRestart());
}

TEST_F(CommandParserTest, DepthClampEnable) {
  std::string data = "depthClampEnable true";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  EXPECT_TRUE(cp.PipelineDataForTesting()->GetEnableDepthClamp());
}

TEST_F(CommandParserTest, DepthClampDisable) {
  std::string data = "depthClampEnable false";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  EXPECT_FALSE(cp.PipelineDataForTesting()->GetEnableDepthClamp());
}

TEST_F(CommandParserTest, RasterizerDiscardEnable) {
  std::string data = "rasterizerDiscardEnable true";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  EXPECT_TRUE(cp.PipelineDataForTesting()->GetEnableRasterizerDiscard());
}

TEST_F(CommandParserTest, RasterizerDiscardDisable) {
  std::string data = "rasterizerDiscardEnable false";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  EXPECT_FALSE(cp.PipelineDataForTesting()->GetEnableRasterizerDiscard());
}

TEST_F(CommandParserTest, DepthBiasEnable) {
  std::string data = "depthBiasEnable true";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  EXPECT_TRUE(cp.PipelineDataForTesting()->GetEnableDepthBias());
}

TEST_F(CommandParserTest, DepthBiasDisable) {
  std::string data = "depthBiasEnable false";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  EXPECT_FALSE(cp.PipelineDataForTesting()->GetEnableDepthBias());
}

TEST_F(CommandParserTest, LogicOpEnable) {
  std::string data = "logicOpEnable true";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  EXPECT_TRUE(cp.PipelineDataForTesting()->GetEnableLogicOp());
}

TEST_F(CommandParserTest, LogicOpDisable) {
  std::string data = "logicOpEnable false";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  EXPECT_FALSE(cp.PipelineDataForTesting()->GetEnableLogicOp());
}

TEST_F(CommandParserTest, BlendEnable) {
  std::string data = "blendEnable true";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  EXPECT_TRUE(cp.PipelineDataForTesting()->GetEnableBlend());
}

TEST_F(CommandParserTest, BlendDisable) {
  std::string data = "blendEnable false";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  EXPECT_FALSE(cp.PipelineDataForTesting()->GetEnableBlend());
}

TEST_F(CommandParserTest, DepthTestEnable) {
  std::string data = "depthTestEnable true";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  EXPECT_TRUE(cp.PipelineDataForTesting()->GetEnableDepthTest());
}

TEST_F(CommandParserTest, DepthTestDisable) {
  std::string data = "depthTestEnable false";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  EXPECT_FALSE(cp.PipelineDataForTesting()->GetEnableDepthTest());
}

TEST_F(CommandParserTest, DepthWriteEnable) {
  std::string data = "depthWriteEnable true";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  EXPECT_TRUE(cp.PipelineDataForTesting()->GetEnableDepthWrite());
}

TEST_F(CommandParserTest, DepthWriteDisable) {
  std::string data = "depthWriteEnable false";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  EXPECT_FALSE(cp.PipelineDataForTesting()->GetEnableDepthWrite());
}

TEST_F(CommandParserTest, DepthBoundsTestEnable) {
  std::string data = "depthBoundsTestEnable true";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  EXPECT_TRUE(cp.PipelineDataForTesting()->GetEnableDepthBoundsTest());
}

TEST_F(CommandParserTest, DepthBoundsTestDisable) {
  std::string data = "depthBoundsTestEnable false";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  EXPECT_FALSE(cp.PipelineDataForTesting()->GetEnableDepthBoundsTest());
}

TEST_F(CommandParserTest, StencilTestEnable) {
  std::string data = "stencilTestEnable true";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  EXPECT_TRUE(cp.PipelineDataForTesting()->GetEnableStencilTest());
}

TEST_F(CommandParserTest, StencilTestDisable) {
  std::string data = "stencilTestEnable false";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  EXPECT_FALSE(cp.PipelineDataForTesting()->GetEnableStencilTest());
}

struct BooleanTest {
  const char* name;
};
using CommandParserBooleanTests = testing::TestWithParam<BooleanTest>;

TEST_P(CommandParserBooleanTests, MissingParam) {
  const auto& test_data = GetParam();

  std::string data = std::string(test_data.name);

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ(std::string("1: Missing value for ") + test_data.name + " command",
            r.Error());
}

TEST_P(CommandParserBooleanTests, IllegalParam) {
  const auto& test_data = GetParam();

  std::string data = std::string(test_data.name) + " 123";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ(
      std::string("1: Invalid value for ") + test_data.name + " command: 123",
      r.Error());
}

TEST_P(CommandParserBooleanTests, ExtraParam) {
  const auto& test_data = GetParam();

  std::string data = std::string(test_data.name) + " true EXTRA";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ(std::string("1: Extra parameter for ") + test_data.name +
                " command: EXTRA",
            r.Error());
}

INSTANTIATE_TEST_SUITE_P(
    BooleanTests,
    CommandParserBooleanTests,
    testing::Values(BooleanTest{"primitiveRestartEnable"},
                    BooleanTest{"depthClampEnable"},
                    BooleanTest{"rasterizerDiscardEnable"},
                    BooleanTest{"depthBiasEnable"},
                    BooleanTest{"logicOpEnable"},
                    BooleanTest{"blendEnable"},
                    BooleanTest{"depthTestEnable"},
                    BooleanTest{"depthWriteEnable"},
                    BooleanTest{"depthBoundsTestEnable"},
                    BooleanTest{
                        "stencilTestEnable"}));  // NOLINT(whitespace/parens)

struct PolygonModeTestData {
  const char* name;
  PolygonMode value;
};
using CommandDataPipelinePolygonModeParser =
    testing::TestWithParam<PolygonModeTestData>;

TEST_P(CommandDataPipelinePolygonModeParser, PolygonMode) {
  const auto& test_data = GetParam();

  std::string data = "polygonMode " + std::string(test_data.name);

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  EXPECT_EQ(test_data.value, cp.PipelineDataForTesting()->GetPolygonMode());
}

INSTANTIATE_TEST_SUITE_P(
    PolygonModeTests,
    CommandDataPipelinePolygonModeParser,
    testing::Values(
        PolygonModeTestData{"VK_POLYGON_MODE_FILL", PolygonMode::kFill},
        PolygonModeTestData{"VK_POLYGON_MODE_LINE", PolygonMode::kLine},
        PolygonModeTestData{
            "VK_POLYGON_MODE_POINT",
            PolygonMode::kPoint}));  // NOLINT(whitespace/parens)

struct CullModeTestData {
  const char* name;
  CullMode value;
};
using CommandDataPipelineCullModeParser =
    testing::TestWithParam<CullModeTestData>;

TEST_P(CommandDataPipelineCullModeParser, CullMode) {
  const auto& test_data = GetParam();

  std::string data = "cullMode " + std::string(test_data.name);

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  EXPECT_EQ(test_data.value, cp.PipelineDataForTesting()->GetCullMode());
}

INSTANTIATE_TEST_SUITE_P(
    CullModeTests,
    CommandDataPipelineCullModeParser,
    testing::Values(
        CullModeTestData{"VK_CULL_MODE_NONE", CullMode::kNone},
        CullModeTestData{"VK_CULL_MODE_FRONT_BIT", CullMode::kFront},
        CullModeTestData{"VK_CULL_MODE_BACK_BIT", CullMode::kBack},
        CullModeTestData{"VK_CULL_MODE_BACK_BIT | VK_CULL_MODE_FRONT_BIT",
                         CullMode::kFrontAndBack},
        CullModeTestData{"VK_CULL_MODE_FRONT_BIT | VK_CULL_MODE_BACK_BIT",
                         CullMode::kFrontAndBack},
        CullModeTestData{
            "VK_CULL_MODE_FRONT_AND_BACK",
            CullMode::kFrontAndBack}));  // NOLINT(whitespace/parens)

struct FrontFaceTestData {
  const char* name;
  FrontFace value;
};
using CommandDataPipelineFrontFaceParser =
    testing::TestWithParam<FrontFaceTestData>;

TEST_P(CommandDataPipelineFrontFaceParser, FrontFace) {
  const auto& test_data = GetParam();

  std::string data = "frontFace " + std::string(test_data.name);

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  EXPECT_EQ(test_data.value, cp.PipelineDataForTesting()->GetFrontFace());
}

INSTANTIATE_TEST_SUITE_P(
    FrontFaceTests,
    CommandDataPipelineFrontFaceParser,
    testing::Values(FrontFaceTestData{"VK_FRONT_FACE_COUNTER_CLOCKWISE",
                                      FrontFace::kCounterClockwise},
                    FrontFaceTestData{
                        "VK_FRONT_FACE_CLOCKWISE",
                        FrontFace::kClockwise}));  // NOLINT(whitespace/parens)

struct LogicOpTestData {
  const char* name;
  LogicOp value;
};
using CommandDataPipelineLogicOpParser =
    testing::TestWithParam<LogicOpTestData>;

TEST_P(CommandDataPipelineLogicOpParser, LogicOp) {
  const auto& test_data = GetParam();

  std::string data = "logicOp " + std::string(test_data.name);

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  EXPECT_EQ(test_data.value, cp.PipelineDataForTesting()->GetLogicOp());
}

INSTANTIATE_TEST_SUITE_P(
    LogicOpTests,
    CommandDataPipelineLogicOpParser,
    testing::Values(
        LogicOpTestData{"VK_LOGIC_OP_CLEAR", LogicOp::kClear},
        LogicOpTestData{"VK_LOGIC_OP_AND", LogicOp::kAnd},
        LogicOpTestData{"VK_LOGIC_OP_AND_REVERSE", LogicOp::kAndReverse},
        LogicOpTestData{"VK_LOGIC_OP_COPY", LogicOp::kCopy},
        LogicOpTestData{"VK_LOGIC_OP_AND_INVERTED", LogicOp::kAndInverted},
        LogicOpTestData{"VK_LOGIC_OP_NO_OP", LogicOp::kNoOp},
        LogicOpTestData{"VK_LOGIC_OP_XOR", LogicOp::kXor},
        LogicOpTestData{"VK_LOGIC_OP_OR", LogicOp::kOr},
        LogicOpTestData{"VK_LOGIC_OP_NOR", LogicOp::kNor},
        LogicOpTestData{"VK_LOGIC_OP_EQUIVALENT", LogicOp::kEquivalent},
        LogicOpTestData{"VK_LOGIC_OP_INVERT", LogicOp::kInvert},
        LogicOpTestData{"VK_LOGIC_OP_OR_REVERSE", LogicOp::kOrReverse},
        LogicOpTestData{"VK_LOGIC_OP_COPY_INVERTED", LogicOp::kCopyInverted},
        LogicOpTestData{"VK_LOGIC_OP_OR_INVERTED", LogicOp::kOrInverted},
        LogicOpTestData{"VK_LOGIC_OP_NAND", LogicOp::kNand},
        LogicOpTestData{"VK_LOGIC_OP_SET",
                        LogicOp::kSet}));  // NOLINT(whitespace/parens)

TEST_F(CommandParserTest, DepthBiasConstantFactor) {
  std::string data = "depthBiasConstantFactor 3.4";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  EXPECT_FLOAT_EQ(3.4f,
                  cp.PipelineDataForTesting()->GetDepthBiasConstantFactor());
}

TEST_F(CommandParserTest, DepthBiasClamp) {
  std::string data = "depthBiasClamp 3.4";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  EXPECT_FLOAT_EQ(3.4f, cp.PipelineDataForTesting()->GetDepthBiasClamp());
}

TEST_F(CommandParserTest, DepthBiasSlopeFactor) {
  std::string data = "depthBiasSlopeFactor 3.4";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  EXPECT_FLOAT_EQ(3.4f, cp.PipelineDataForTesting()->GetDepthBiasSlopeFactor());
}

TEST_F(CommandParserTest, LineWidth) {
  std::string data = "lineWidth 3.4";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  EXPECT_FLOAT_EQ(3.4f, cp.PipelineDataForTesting()->GetLineWidth());
}

TEST_F(CommandParserTest, MinDepthBounds) {
  std::string data = "minDepthBounds 3.4";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  EXPECT_FLOAT_EQ(3.4f, cp.PipelineDataForTesting()->GetMinDepthBounds());
}

TEST_F(CommandParserTest, MaxDepthBounds) {
  std::string data = "maxDepthBounds 3.4";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  EXPECT_FLOAT_EQ(3.4f, cp.PipelineDataForTesting()->GetMaxDepthBounds());
}

struct FloatTest {
  const char* name;
};
using CommandParserFloatTests = testing::TestWithParam<FloatTest>;

TEST_P(CommandParserFloatTests, MissingParam) {
  const auto& test_data = GetParam();

  std::string data = std::string(test_data.name);

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ(std::string("1: Missing value for ") + test_data.name + " command",
            r.Error());
}

TEST_P(CommandParserFloatTests, IllegalParam) {
  const auto& test_data = GetParam();

  std::string data = std::string(test_data.name) + " INVALID";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Invalid conversion to double", r.Error());
}

TEST_P(CommandParserFloatTests, ExtraParam) {
  const auto& test_data = GetParam();

  std::string data = std::string(test_data.name) + " 3.2 EXTRA";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ(std::string("1: Extra parameter for ") + test_data.name +
                " command: EXTRA",
            r.Error());
}

INSTANTIATE_TEST_SUITE_P(
    FloatTests,
    CommandParserFloatTests,
    testing::Values(FloatTest{"depthBiasConstantFactor"},
                    FloatTest{"lineWidth"},
                    FloatTest{"depthBiasClamp"},
                    FloatTest{"depthBiasSlopeFactor"},
                    FloatTest{"minDepthBounds"},
                    FloatTest{"maxDepthBounds"}));  // NOLINT(whitespace/parens)

TEST_F(CommandParserTest, SrcColorBlendFactor) {
  std::string data = "srcColorBlendFactor VK_BLEND_FACTOR_DST_COLOR";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  EXPECT_EQ(BlendFactor::kDstColor,
            cp.PipelineDataForTesting()->GetSrcColorBlendFactor());
}

TEST_F(CommandParserTest, DstColorBlendFactor) {
  std::string data = "dstColorBlendFactor VK_BLEND_FACTOR_DST_COLOR";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  EXPECT_EQ(BlendFactor::kDstColor,
            cp.PipelineDataForTesting()->GetDstColorBlendFactor());
}

TEST_F(CommandParserTest, SrcAlphaBlendFactor) {
  std::string data = "srcAlphaBlendFactor VK_BLEND_FACTOR_DST_COLOR";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  EXPECT_EQ(BlendFactor::kDstColor,
            cp.PipelineDataForTesting()->GetSrcAlphaBlendFactor());
}

TEST_F(CommandParserTest, DstAlphaBlendFactor) {
  std::string data = "dstAlphaBlendFactor VK_BLEND_FACTOR_DST_COLOR";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  EXPECT_EQ(BlendFactor::kDstColor,
            cp.PipelineDataForTesting()->GetDstAlphaBlendFactor());
}

struct BlendFactorData {
  const char* name;
  BlendFactor type;
};
using CommandParserBlendFactorParsing = testing::TestWithParam<BlendFactorData>;

TEST_P(CommandParserBlendFactorParsing, Parse) {
  const auto& test_data = GetParam();

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, "unused");
  BlendFactor factor = BlendFactor::kZero;
  Result r = cp.ParseBlendFactorNameForTesting(test_data.name, &factor);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  EXPECT_EQ(test_data.type, factor);
}

INSTANTIATE_TEST_SUITE_P(
    BlendFactorParsingTests,
    CommandParserBlendFactorParsing,
    testing::Values(
        BlendFactorData{"VK_BLEND_FACTOR_ZERO", BlendFactor::kZero},
        BlendFactorData{"VK_BLEND_FACTOR_ONE", BlendFactor::kOne},
        BlendFactorData{"VK_BLEND_FACTOR_SRC_COLOR", BlendFactor::kSrcColor},
        BlendFactorData{"VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR",
                        BlendFactor::kOneMinusSrcColor},
        BlendFactorData{"VK_BLEND_FACTOR_DST_COLOR", BlendFactor::kDstColor},
        BlendFactorData{"VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR",
                        BlendFactor::kOneMinusDstColor},
        BlendFactorData{"VK_BLEND_FACTOR_SRC_ALPHA", BlendFactor::kSrcAlpha},
        BlendFactorData{"VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA",
                        BlendFactor::kOneMinusSrcAlpha},
        BlendFactorData{"VK_BLEND_FACTOR_DST_ALPHA", BlendFactor::kDstAlpha},
        BlendFactorData{"VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA",
                        BlendFactor::kOneMinusDstAlpha},
        BlendFactorData{"VK_BLEND_FACTOR_CONSTANT_COLOR",
                        BlendFactor::kConstantColor},
        BlendFactorData{"VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR",
                        BlendFactor::kOneMinusConstantColor},
        BlendFactorData{"VK_BLEND_FACTOR_CONSTANT_ALPHA",
                        BlendFactor::kConstantAlpha},
        BlendFactorData{"VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA",
                        BlendFactor::kOneMinusConstantAlpha},
        BlendFactorData{"VK_BLEND_FACTOR_SRC_ALPHA_SATURATE",
                        BlendFactor::kSrcAlphaSaturate},
        BlendFactorData{"VK_BLEND_FACTOR_SRC1_COLOR", BlendFactor::kSrc1Color},
        BlendFactorData{"VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR",
                        BlendFactor::kOneMinusSrc1Color},
        BlendFactorData{"VK_BLEND_FACTOR_SRC1_ALPHA", BlendFactor::kSrc1Alpha},
        BlendFactorData{
            "VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA",
            BlendFactor::kOneMinusSrc1Alpha}));  // NOLINT(whitespace/parens)

TEST_F(CommandParserTest, BlendFactorParsingInvalid) {
  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, "unused");
  BlendFactor factor = BlendFactor::kZero;
  Result r = cp.ParseBlendFactorNameForTesting("INVALID", &factor);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("Unknown BlendFactor provided: INVALID", r.Error());
}

struct BlendFactorTest {
  const char* name;
};
using CommandParserBlendFactorTests = testing::TestWithParam<BlendFactorTest>;

TEST_P(CommandParserBlendFactorTests, MissingParam) {
  const auto& test_data = GetParam();

  std::string data = std::string(test_data.name);

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ(
      std::string("1: Missing parameter for ") + test_data.name + " command",
      r.Error());
}

TEST_P(CommandParserBlendFactorTests, IllegalParam) {
  const auto& test_data = GetParam();

  std::string data = std::string(test_data.name) + " 1.23";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ(std::string("1: Invalid parameter for ") + test_data.name +
                " command: 1.23",
            r.Error());
}

TEST_P(CommandParserBlendFactorTests, ExtraParam) {
  const auto& test_data = GetParam();

  std::string data = std::string(test_data.name) + " VK_BLEND_FACTOR_ONE EXTRA";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ(std::string("1: Extra parameter for ") + test_data.name +
                " command: EXTRA",
            r.Error());
}

INSTANTIATE_TEST_SUITE_P(
    BlendFactorTests,
    CommandParserBlendFactorTests,
    testing::Values(BlendFactorTest{"srcColorBlendFactor"},
                    BlendFactorTest{"dstColorBlendFactor"},
                    BlendFactorTest{"srcAlphaBlendFactor"},
                    BlendFactorTest{
                        "dstAlphaBlendFactor"}));  // NOLINT(whitespace/parens)

TEST_F(CommandParserTest, ColorBlendOp) {
  std::string data = "colorBlendOp VK_BLEND_OP_XOR_EXT";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  EXPECT_EQ(BlendOp::kXor, cp.PipelineDataForTesting()->GetColorBlendOp());
}

TEST_F(CommandParserTest, AlphaBlendOp) {
  std::string data = "alphaBlendOp VK_BLEND_OP_XOR_EXT";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  EXPECT_EQ(BlendOp::kXor, cp.PipelineDataForTesting()->GetAlphaBlendOp());
}

struct BlendOpData {
  const char* name;
  BlendOp type;
};
using CommandParserBlendOpParsing = testing::TestWithParam<BlendOpData>;

TEST_P(CommandParserBlendOpParsing, Parse) {
  const auto& test_data = GetParam();

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, "unused");
  BlendOp op = BlendOp::kAdd;
  Result r = cp.ParseBlendOpNameForTesting(test_data.name, &op);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  EXPECT_EQ(test_data.type, op);
}

INSTANTIATE_TEST_SUITE_P(
    BlendOpParsingTests1,
    CommandParserBlendOpParsing,
    testing::Values(
        BlendOpData{"VK_BLEND_OP_ADD", BlendOp::kAdd},
        BlendOpData{"VK_BLEND_OP_SUBTRACT", BlendOp::kSubtract},
        BlendOpData{"VK_BLEND_OP_REVERSE_SUBTRACT", BlendOp::kReverseSubtract},
        BlendOpData{"VK_BLEND_OP_MIN", BlendOp::kMin},
        BlendOpData{"VK_BLEND_OP_MAX", BlendOp::kMax},
        BlendOpData{"VK_BLEND_OP_ZERO_EXT", BlendOp::kZero},
        BlendOpData{"VK_BLEND_OP_SRC_EXT", BlendOp::kSrc},
        BlendOpData{"VK_BLEND_OP_DST_EXT", BlendOp::kDst},
        BlendOpData{"VK_BLEND_OP_SRC_OVER_EXT", BlendOp::kSrcOver},
        BlendOpData{"VK_BLEND_OP_DST_OVER_EXT", BlendOp::kDstOver},
        BlendOpData{"VK_BLEND_OP_SRC_IN_EXT", BlendOp::kSrcIn},
        BlendOpData{"VK_BLEND_OP_DST_IN_EXT", BlendOp::kDstIn},
        BlendOpData{"VK_BLEND_OP_SRC_OUT_EXT", BlendOp::kSrcOut},
        BlendOpData{"VK_BLEND_OP_DST_OUT_EXT", BlendOp::kDstOut},
        BlendOpData{"VK_BLEND_OP_SRC_ATOP_EXT", BlendOp::kSrcAtop},
        BlendOpData{"VK_BLEND_OP_DST_ATOP_EXT", BlendOp::kDstAtop},
        BlendOpData{"VK_BLEND_OP_XOR_EXT", BlendOp::kXor},
        BlendOpData{"VK_BLEND_OP_MULTIPLY_EXT", BlendOp::kMultiply},
        BlendOpData{"VK_BLEND_OP_SCREEN_EXT", BlendOp::kScreen},
        BlendOpData{"VK_BLEND_OP_OVERLAY_EXT", BlendOp::kOverlay},
        BlendOpData{"VK_BLEND_OP_DARKEN_EXT", BlendOp::kDarken},
        BlendOpData{"VK_BLEND_OP_LIGHTEN_EXT", BlendOp::kLighten},
        BlendOpData{"VK_BLEND_OP_COLORDODGE_EXT", BlendOp::kColorDodge},
        BlendOpData{"VK_BLEND_OP_COLORBURN_EXT", BlendOp::kColorBurn},
        BlendOpData{"VK_BLEND_OP_HARDLIGHT_EXT", BlendOp::kHardLight},
        BlendOpData{"VK_BLEND_OP_SOFTLIGHT_EXT", BlendOp::kSoftLight},
        BlendOpData{"VK_BLEND_OP_DIFFERENCE_EXT", BlendOp::kDifference},
        BlendOpData{"VK_BLEND_OP_EXCLUSION_EXT", BlendOp::kExclusion},
        BlendOpData{"VK_BLEND_OP_INVERT_EXT",
                    BlendOp::kInvert}));  // NOLINT(whitespace/parens)

INSTANTIATE_TEST_SUITE_P(
    BlendOpParsingTests2,
    CommandParserBlendOpParsing,
    testing::Values(
        BlendOpData{"VK_BLEND_OP_INVERT_RGB_EXT", BlendOp::kInvertRGB},
        BlendOpData{"VK_BLEND_OP_LINEARDODGE_EXT", BlendOp::kLinearDodge},
        BlendOpData{"VK_BLEND_OP_LINEARBURN_EXT", BlendOp::kLinearBurn},
        BlendOpData{"VK_BLEND_OP_VIVIDLIGHT_EXT", BlendOp::kVividLight},
        BlendOpData{"VK_BLEND_OP_LINEARLIGHT_EXT", BlendOp::kLinearLight},
        BlendOpData{"VK_BLEND_OP_PINLIGHT_EXT", BlendOp::kPinLight},
        BlendOpData{"VK_BLEND_OP_HARDMIX_EXT", BlendOp::kHardMix},
        BlendOpData{"VK_BLEND_OP_HSL_HUE_EXT", BlendOp::kHslHue},
        BlendOpData{"VK_BLEND_OP_HSL_SATURATION_EXT", BlendOp::kHslSaturation},
        BlendOpData{"VK_BLEND_OP_HSL_COLOR_EXT", BlendOp::kHslColor},
        BlendOpData{"VK_BLEND_OP_HSL_LUMINOSITY_EXT", BlendOp::kHslLuminosity},
        BlendOpData{"VK_BLEND_OP_PLUS_EXT", BlendOp::kPlus},
        BlendOpData{"VK_BLEND_OP_PLUS_CLAMPED_EXT", BlendOp::kPlusClamped},
        BlendOpData{"VK_BLEND_OP_PLUS_CLAMPED_ALPHA_EXT",
                    BlendOp::kPlusClampedAlpha},
        BlendOpData{"VK_BLEND_OP_PLUS_DARKER_EXT", BlendOp::kPlusDarker},
        BlendOpData{"VK_BLEND_OP_MINUS_EXT", BlendOp::kMinus},
        BlendOpData{"VK_BLEND_OP_MINUS_CLAMPED_EXT", BlendOp::kMinusClamped},
        BlendOpData{"VK_BLEND_OP_CONTRAST_EXT", BlendOp::kContrast},
        BlendOpData{"VK_BLEND_OP_INVERT_OVG_EXT", BlendOp::kInvertOvg},
        BlendOpData{"VK_BLEND_OP_RED_EXT", BlendOp::kRed},
        BlendOpData{"VK_BLEND_OP_GREEN_EXT", BlendOp::kGreen},
        BlendOpData{"VK_BLEND_OP_BLUE_EXT",
                    BlendOp::kBlue}));  // NOLINT(whitespace/parens)

TEST_F(CommandParserTest, BlendOpParsingInvalid) {
  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, "unused");
  BlendOp op = BlendOp::kAdd;
  Result r = cp.ParseBlendOpNameForTesting("INVALID", &op);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("Unknown BlendOp provided: INVALID", r.Error());
}

struct BlendOpTest {
  const char* name;
};
using CommandParserBlendOpTests = testing::TestWithParam<BlendOpTest>;

TEST_P(CommandParserBlendOpTests, MissingParam) {
  const auto& test_data = GetParam();

  std::string data = std::string(test_data.name);

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ(
      std::string("1: Missing parameter for ") + test_data.name + " command",
      r.Error());
}

TEST_P(CommandParserBlendOpTests, IllegalParam) {
  const auto& test_data = GetParam();

  std::string data = std::string(test_data.name) + " 1.23";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ(std::string("1: Invalid parameter for ") + test_data.name +
                " command: 1.23",
            r.Error());
}

TEST_P(CommandParserBlendOpTests, ExtraParam) {
  const auto& test_data = GetParam();

  std::string data = std::string(test_data.name) + " VK_BLEND_OP_MAX EXTRA";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ(std::string("1: Extra parameter for ") + test_data.name +
                " command: EXTRA",
            r.Error());
}

INSTANTIATE_TEST_SUITE_P(
    BlendOpTests,
    CommandParserBlendOpTests,
    testing::Values(BlendOpTest{"colorBlendOp"},
                    BlendOpTest{"alphaBlendOp"}));  // NOLINT(whitespace/parens)

TEST_F(CommandParserTest, DepthCompareOp) {
  std::string data = "depthCompareOp VK_COMPARE_OP_EQUAL";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  EXPECT_EQ(CompareOp::kEqual,
            cp.PipelineDataForTesting()->GetDepthCompareOp());
}

TEST_F(CommandParserTest, FrontCompareOp) {
  std::string data = "front.compareOp VK_COMPARE_OP_EQUAL";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  EXPECT_EQ(CompareOp::kEqual,
            cp.PipelineDataForTesting()->GetFrontCompareOp());
}

TEST_F(CommandParserTest, BackCompareOp) {
  std::string data = "back.compareOp VK_COMPARE_OP_EQUAL";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  EXPECT_EQ(CompareOp::kEqual, cp.PipelineDataForTesting()->GetBackCompareOp());
}

struct CompareOpData {
  const char* name;
  CompareOp type;
};
using CommandParserCompareOpParsing = testing::TestWithParam<CompareOpData>;

TEST_P(CommandParserCompareOpParsing, Parse) {
  const auto& test_data = GetParam();

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, "unused");
  CompareOp op = CompareOp::kNever;
  Result r = cp.ParseCompareOpNameForTesting(test_data.name, &op);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  EXPECT_EQ(test_data.type, op);
}

INSTANTIATE_TEST_SUITE_P(
    CompareOpParsingTests,
    CommandParserCompareOpParsing,
    testing::Values(
        CompareOpData{"VK_COMPARE_OP_NEVER", CompareOp::kNever},
        CompareOpData{"VK_COMPARE_OP_LESS", CompareOp::kLess},
        CompareOpData{"VK_COMPARE_OP_EQUAL", CompareOp::kEqual},
        CompareOpData{"VK_COMPARE_OP_LESS_OR_EQUAL", CompareOp::kLessOrEqual},
        CompareOpData{"VK_COMPARE_OP_GREATER", CompareOp::kGreater},
        CompareOpData{"VK_COMPARE_OP_NOT_EQUAL", CompareOp::kNotEqual},
        CompareOpData{"VK_COMPARE_OP_GREATER_OR_EQUAL",
                      CompareOp::kGreaterOrEqual},
        CompareOpData{"VK_COMPARE_OP_ALWAYS",
                      CompareOp::kAlways}));  // NOLINT(whitespace/parens)

TEST_F(CommandParserTest, CompareOpParsingInvalid) {
  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, "unused");
  CompareOp op = CompareOp::kNever;
  Result r = cp.ParseCompareOpNameForTesting("INVALID", &op);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("Unknown CompareOp provided: INVALID", r.Error());
}

struct CompareOpTest {
  const char* name;
};
using CommandParserCompareOpTests = testing::TestWithParam<CompareOpTest>;

TEST_P(CommandParserCompareOpTests, MissingParam) {
  const auto& test_data = GetParam();

  std::string data = std::string(test_data.name);

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ(
      std::string("1: Missing parameter for ") + test_data.name + " command",
      r.Error());
}

TEST_P(CommandParserCompareOpTests, IllegalParam) {
  const auto& test_data = GetParam();

  std::string data = std::string(test_data.name) + " 1.23";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ(std::string("1: Invalid parameter for ") + test_data.name +
                " command: 1.23",
            r.Error());
}

TEST_P(CommandParserCompareOpTests, ExtraParam) {
  const auto& test_data = GetParam();

  std::string data =
      std::string(test_data.name) + " VK_COMPARE_OP_ALWAYS EXTRA";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ(std::string("1: Extra parameter for ") + test_data.name +
                " command: EXTRA",
            r.Error());
}

INSTANTIATE_TEST_SUITE_P(
    CompareOpTests,
    CommandParserCompareOpTests,
    testing::Values(CompareOpTest{"depthCompareOp"},
                    CompareOpTest{"front.compareOp"},
                    CompareOpTest{
                        "back.compareOp"}));  // NOLINT(whitespace/parens)

TEST_F(CommandParserTest, FrontFailOp) {
  std::string data = "front.failOp VK_STENCIL_OP_REPLACE";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  EXPECT_EQ(StencilOp::kReplace, cp.PipelineDataForTesting()->GetFrontFailOp());
}

TEST_F(CommandParserTest, FrontPassOp) {
  std::string data = "front.passOp VK_STENCIL_OP_REPLACE";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  EXPECT_EQ(StencilOp::kReplace, cp.PipelineDataForTesting()->GetFrontPassOp());
}

TEST_F(CommandParserTest, FrontDepthFailOp) {
  std::string data = "front.depthFailOp VK_STENCIL_OP_REPLACE";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  EXPECT_EQ(StencilOp::kReplace,
            cp.PipelineDataForTesting()->GetFrontDepthFailOp());
}

TEST_F(CommandParserTest, BackFailOp) {
  std::string data = "back.failOp VK_STENCIL_OP_REPLACE";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  EXPECT_EQ(StencilOp::kReplace, cp.PipelineDataForTesting()->GetBackFailOp());
}

TEST_F(CommandParserTest, BackPassOp) {
  std::string data = "back.passOp VK_STENCIL_OP_REPLACE";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  EXPECT_EQ(StencilOp::kReplace, cp.PipelineDataForTesting()->GetBackPassOp());
}

TEST_F(CommandParserTest, BackDepthFailOp) {
  std::string data = "back.depthFailOp VK_STENCIL_OP_REPLACE";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  EXPECT_EQ(StencilOp::kReplace,
            cp.PipelineDataForTesting()->GetBackDepthFailOp());
}

struct StencilOpData {
  const char* name;
  StencilOp type;
};
using CommandParserStencilOpParsing = testing::TestWithParam<StencilOpData>;

TEST_P(CommandParserStencilOpParsing, Parse) {
  const auto& test_data = GetParam();

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, "unused");
  StencilOp op = StencilOp::kKeep;
  Result r = cp.ParseStencilOpNameForTesting(test_data.name, &op);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  EXPECT_EQ(test_data.type, op);
}

INSTANTIATE_TEST_SUITE_P(
    CompareOpParsingTests,
    CommandParserStencilOpParsing,
    testing::Values(
        StencilOpData{"VK_STENCIL_OP_KEEP", StencilOp::kKeep},
        StencilOpData{"VK_STENCIL_OP_ZERO", StencilOp::kZero},
        StencilOpData{"VK_STENCIL_OP_REPLACE", StencilOp::kReplace},
        StencilOpData{"VK_STENCIL_OP_INCREMENT_AND_CLAMP",
                      StencilOp::kIncrementAndClamp},
        StencilOpData{"VK_STENCIL_OP_DECREMENT_AND_CLAMP",
                      StencilOp::kDecrementAndClamp},
        StencilOpData{"VK_STENCIL_OP_INVERT", StencilOp::kInvert},
        StencilOpData{"VK_STENCIL_OP_INCREMENT_AND_WRAP",
                      StencilOp::kIncrementAndWrap},
        StencilOpData{
            "VK_STENCIL_OP_DECREMENT_AND_WRAP",
            StencilOp::kDecrementAndWrap}));  // NOLINT(whitespace/parens)

TEST_F(CommandParserTest, StencilOpParsingInvalid) {
  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, "unused");
  StencilOp op = StencilOp::kKeep;
  Result r = cp.ParseStencilOpNameForTesting("INVALID", &op);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("Unknown StencilOp provided: INVALID", r.Error());
}

struct StencilOpTest {
  const char* name;
};
using CommandParserStencilOpTests = testing::TestWithParam<StencilOpTest>;

TEST_P(CommandParserStencilOpTests, MissingParam) {
  const auto& test_data = GetParam();

  std::string data = std::string(test_data.name);

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ(
      std::string("1: Missing parameter for ") + test_data.name + " command",
      r.Error());
}

TEST_P(CommandParserStencilOpTests, IllegalParam) {
  const auto& test_data = GetParam();

  std::string data = std::string(test_data.name) + " 1.23";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ(std::string("1: Invalid parameter for ") + test_data.name +
                " command: 1.23",
            r.Error());
}

TEST_P(CommandParserStencilOpTests, ExtraParam) {
  const auto& test_data = GetParam();

  std::string data =
      std::string(test_data.name) + " VK_STENCIL_OP_REPLACE EXTRA";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ(std::string("1: Extra parameter for ") + test_data.name +
                " command: EXTRA",
            r.Error());
}

INSTANTIATE_TEST_SUITE_P(
    StencilOpTests,
    CommandParserStencilOpTests,
    testing::Values(StencilOpTest{"front.passOp"},
                    StencilOpTest{"front.failOp"},
                    StencilOpTest{"front.depthFailOp"},
                    StencilOpTest{"back.passOp"},
                    StencilOpTest{"back.failOp"},
                    StencilOpTest{
                        "back.depthFailOp"}));  // NOLINT(whitespace/parens)

TEST_F(CommandParserTest, FrontCompareMask) {
  std::string data = "front.compareMask 123";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: front.compareMask not implemented", r.Error());
}

TEST_F(CommandParserTest, FrontWriteMask) {
  std::string data = "front.writeMask 123";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: front.writeMask not implemented", r.Error());
}

TEST_F(CommandParserTest, BackCompareMask) {
  std::string data = "back.compareMask 123";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: back.compareMask not implemented", r.Error());
}

TEST_F(CommandParserTest, BackWriteMask) {
  std::string data = "back.writeMask 123";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: back.writeMask not implemented", r.Error());
}

TEST_F(CommandParserTest, FrontReference) {
  std::string data = "front.reference 10";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  EXPECT_EQ(10U, cp.PipelineDataForTesting()->GetFrontReference());
}

TEST_F(CommandParserTest, BackReference) {
  std::string data = "back.reference 10";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  EXPECT_EQ(10U, cp.PipelineDataForTesting()->GetBackReference());
}

struct ReferenceData {
  const char* name;
};

using CommandParserReferenceTests = testing::TestWithParam<ReferenceData>;

TEST_P(CommandParserReferenceTests, FrontReferenceMissingValue) {
  const auto& test_data = GetParam();
  std::string data = std::string(test_data.name);

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ(
      std::string("1: Missing parameter for ") + test_data.name + " command",
      r.Error());
}

TEST_P(CommandParserReferenceTests, FrontReferenceExtraParameters) {
  const auto& test_data = GetParam();
  std::string data = std::string(test_data.name) + " 10 EXTRA";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ(std::string("1: Extra parameter for ") + test_data.name +
                " command: EXTRA",
            r.Error());
}

TEST_P(CommandParserReferenceTests, FrontReferenceInvalidParameters) {
  const auto& test_data = GetParam();
  std::string data = std::string(test_data.name) + " INVALID";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ(std::string("1: Invalid parameter for ") + test_data.name +
                " command: INVALID",
            r.Error());
}

INSTANTIATE_TEST_SUITE_P(
    ReferenceTest,
    CommandParserReferenceTests,
    testing::Values(ReferenceData{"front.reference"},
                    ReferenceData{
                        "back.reference"}));  // NOLINT(whitespace/parens)

struct ColorMaskData {
  const char* input;
  uint8_t result;
};
using CommandParserColorMaskTests = testing::TestWithParam<ColorMaskData>;

TEST_P(CommandParserColorMaskTests, ColorWriteMask) {
  const auto& test_data = GetParam();
  std::string data = "colorWriteMask " + std::string(test_data.input);

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  EXPECT_EQ(test_data.result, cp.PipelineDataForTesting()->GetColorWriteMask());
}

INSTANTIATE_TEST_SUITE_P(
    ColorMaskTests,
    CommandParserColorMaskTests,
    testing::Values(
        ColorMaskData{"VK_COLOR_COMPONENT_R_BIT", kColorMaskR},
        ColorMaskData{"VK_COLOR_COMPONENT_G_BIT", kColorMaskG},
        ColorMaskData{"VK_COLOR_COMPONENT_B_BIT", kColorMaskB},
        ColorMaskData{"VK_COLOR_COMPONENT_A_BIT", kColorMaskA},
        ColorMaskData{"VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | "
                      "VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT",
                      kColorMaskR | kColorMaskG | kColorMaskB | kColorMaskA},
        ColorMaskData{"VK_COLOR_COMPONENT_A_BIT | VK_COLOR_COMPONENT_B_BIT | "
                      "VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT",
                      kColorMaskR | kColorMaskG | kColorMaskB |
                          kColorMaskA}));  // NOLINT(whitespace/parens)

TEST_F(CommandParserTest, ColorWriteMaskInvalid) {
  std::string data = "colorWriteMask INVALID";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Unknown parameter for colorWriteMask command: INVALID",
            r.Error());
}

TEST_F(CommandParserTest, ColorWriteMaskInvalidAfterValid) {
  std::string data = "colorWriteMask VK_COLOR_COMPONENT_G_BIT | INVALID";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Unknown parameter for colorWriteMask command: INVALID",
            r.Error());
}

TEST_F(CommandParserTest, ColorWriteMaskMissingParam) {
  std::string data = "colorWriteMask";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Missing parameter for colorWriteMask command", r.Error());
}

TEST_F(CommandParserTest, ColorWriteMaskExtraParam) {
  std::string data =
      "colorWriteMask VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_B_BIT "
      "EXTRA";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Unknown parameter for colorWriteMask command: EXTRA",
            r.Error());
}

TEST_F(CommandParserTest, SSBO) {
  std::string data = "ssbo 5 40";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto& cmds = cp.Commands();
  ASSERT_EQ(1U, cmds.size());
  ASSERT_TRUE(cmds[0]->IsBuffer());

  auto* cmd = cmds[0]->AsBuffer();
  EXPECT_TRUE(cmd->IsSSBO());
  EXPECT_EQ(static_cast<uint32_t>(0), cmd->GetDescriptorSet());
  EXPECT_EQ(5U, cmd->GetBinding());
  EXPECT_EQ(40U, cmd->GetBuffer()->ElementCount());
}

TEST_F(CommandParserTest, SSBOWithDescriptorSet) {
  std::string data = "ssbo 9:5 40";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto& cmds = cp.Commands();
  ASSERT_EQ(1U, cmds.size());
  ASSERT_TRUE(cmds[0]->IsBuffer());

  auto* cmd = cmds[0]->AsBuffer();
  EXPECT_TRUE(cmd->IsSSBO());
  EXPECT_EQ(9U, cmd->GetDescriptorSet());
  EXPECT_EQ(5U, cmd->GetBinding());
  EXPECT_EQ(40U, cmd->GetBuffer()->ElementCount());
}

TEST_F(CommandParserTest, SSBOExtraParameter) {
  std::string data = "ssbo 5 40 EXTRA";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Extra parameter for ssbo command: EXTRA", r.Error());
}

TEST_F(CommandParserTest, SSBOInvalidFloatBinding) {
  std::string data = "ssbo 5.0 40";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Invalid binding value for ssbo command", r.Error());
}

TEST_F(CommandParserTest, SSBOInvalidBinding) {
  std::string data = "ssbo abc 40";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Invalid binding value for ssbo command", r.Error());
}

TEST_F(CommandParserTest, SSBOInvalidFloatSize) {
  std::string data = "ssbo 5 40.0";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Invalid size value for ssbo command: 40.0", r.Error());
}

TEST_F(CommandParserTest, SSBOInvalidSize) {
  std::string data = "ssbo 5 abc";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Invalid value for ssbo command: abc", r.Error());
}

TEST_F(CommandParserTest, SSBOMissingSize) {
  std::string data = "ssbo 5";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Missing size value for ssbo command: ", r.Error());
}

TEST_F(CommandParserTest, SSBOMissingBinding) {
  std::string data = "ssbo";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Missing binding and size values for ssbo command", r.Error());
}

TEST_F(CommandParserTest, SSBOSubdataWithFloat) {
  std::string data = "ssbo 6 subdata vec3 16 2.3 4.2 1.2";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;

  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto& cmds = cp.Commands();
  ASSERT_EQ(1U, cmds.size());
  ASSERT_TRUE(cmds[0]->IsBuffer());

  auto* cmd = cmds[0]->AsBuffer();
  EXPECT_TRUE(cmd->IsSSBO());
  EXPECT_EQ(static_cast<uint32_t>(0), cmd->GetDescriptorSet());
  EXPECT_EQ(6U, cmd->GetBinding());
  EXPECT_EQ(16U, cmd->GetOffset());
  ASSERT_TRUE(cmd->IsSubdata());

  auto* fmt = cmd->GetBuffer()->GetFormat();
  ASSERT_TRUE(fmt->GetType()->IsNumber());

  auto n = fmt->GetType()->AsNumber();
  EXPECT_TRUE(type::Type::IsFloat32(n->GetFormatMode(), n->NumBits()));
  EXPECT_EQ(1U, fmt->GetType()->ColumnCount());
  EXPECT_EQ(3U, fmt->GetType()->RowCount());

  const auto& values = cmd->GetValues();
  std::vector<float> results = {2.3f, 4.2f, 1.2f};
  ASSERT_EQ(results.size(), values.size());
  for (size_t i = 0; i < results.size(); ++i) {
    EXPECT_FLOAT_EQ(results[i], values[i].AsFloat());
  }
}

TEST_F(CommandParserTest, SSBOSubdataWithNegativeOffset) {
  std::string data = "ssbo 6 subdata vec3 -2 -4 -5 -6";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: offset for SSBO must be positive, got: -2", r.Error());
}

TEST_F(CommandParserTest, SSBOSubdataWithDescriptorSet) {
  std::string data = "ssbo 5:6 subdata vec3 16 2.3 4.2 1.2";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto& cmds = cp.Commands();
  ASSERT_EQ(1U, cmds.size());
  ASSERT_TRUE(cmds[0]->IsBuffer());

  auto* cmd = cmds[0]->AsBuffer();
  EXPECT_TRUE(cmd->IsSSBO());
  ASSERT_TRUE(cmd->IsSubdata());
  EXPECT_EQ(5U, cmd->GetDescriptorSet());
  EXPECT_EQ(6U, cmd->GetBinding());
  EXPECT_EQ(16U, cmd->GetOffset());

  auto* fmt = cmd->GetBuffer()->GetFormat();
  ASSERT_TRUE(fmt->GetType()->IsNumber());

  auto n = fmt->GetType()->AsNumber();
  EXPECT_TRUE(type::Type::IsFloat32(n->GetFormatMode(), n->NumBits()));
  EXPECT_EQ(1U, fmt->GetType()->ColumnCount());
  EXPECT_EQ(3U, fmt->GetType()->RowCount());

  const auto& values = cmd->GetValues();
  std::vector<float> results = {2.3f, 4.2f, 1.2f};
  ASSERT_EQ(results.size(), values.size());
  for (size_t i = 0; i < results.size(); ++i) {
    EXPECT_FLOAT_EQ(results[i], values[i].AsFloat());
  }
}

TEST_F(CommandParserTest, SSBOSubdataWithInts) {
  std::string data = "ssbo 6 subdata i16vec3 8 2 4 1";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto& cmds = cp.Commands();
  ASSERT_EQ(1U, cmds.size());
  ASSERT_TRUE(cmds[0]->IsBuffer());

  auto* cmd = cmds[0]->AsBuffer();
  EXPECT_TRUE(cmd->IsSSBO());
  ASSERT_TRUE(cmd->IsSubdata());
  EXPECT_EQ(static_cast<uint32_t>(0), cmd->GetDescriptorSet());
  EXPECT_EQ(6U, cmd->GetBinding());
  EXPECT_EQ(8U, cmd->GetOffset());

  auto* fmt = cmd->GetBuffer()->GetFormat();
  ASSERT_TRUE(fmt->GetType()->IsNumber());

  auto n = fmt->GetType()->AsNumber();
  EXPECT_TRUE(type::Type::IsInt16(n->GetFormatMode(), n->NumBits()));
  EXPECT_EQ(1U, fmt->GetType()->ColumnCount());
  EXPECT_EQ(3U, fmt->GetType()->RowCount());

  const auto& values = cmd->GetValues();
  std::vector<int16_t> results = {2, 4, 1};
  ASSERT_EQ(results.size(), values.size());
  for (size_t i = 0; i < results.size(); ++i) {
    EXPECT_FLOAT_EQ(results[i], values[i].AsInt16());
  }
}

TEST_F(CommandParserTest, SSBOSubdataWithMultipleVectors) {
  std::string data = "ssbo 6 subdata i16vec3 8 2 4 1 3 6 8";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto& cmds = cp.Commands();
  ASSERT_EQ(1U, cmds.size());
  ASSERT_TRUE(cmds[0]->IsBuffer());

  auto* cmd = cmds[0]->AsBuffer();
  EXPECT_TRUE(cmd->IsSSBO());
  ASSERT_TRUE(cmd->IsSubdata());
  EXPECT_EQ(static_cast<uint32_t>(0), cmd->GetDescriptorSet());
  EXPECT_EQ(6U, cmd->GetBinding());
  EXPECT_EQ(8U, cmd->GetOffset());

  auto* fmt = cmd->GetBuffer()->GetFormat();
  ASSERT_TRUE(fmt->GetType()->IsNumber());

  auto n = fmt->GetType()->AsNumber();
  EXPECT_TRUE(type::Type::IsInt16(n->GetFormatMode(), n->NumBits()));
  EXPECT_EQ(1U, fmt->GetType()->ColumnCount());
  EXPECT_EQ(3U, fmt->GetType()->RowCount());

  const auto& values = cmd->GetValues();
  std::vector<int16_t> results = {2, 4, 1, 3, 6, 8};
  ASSERT_EQ(results.size(), values.size());
  for (size_t i = 0; i < results.size(); ++i) {
    EXPECT_FLOAT_EQ(results[i], values[i].AsInt16());
  }
}

TEST_F(CommandParserTest, SSBOSubdataMissingBinding) {
  std::string data = "ssbo subdata i16vec3 0 2 3 2";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Invalid binding value for ssbo command", r.Error());
}

TEST_F(CommandParserTest, SSBOSubdataWithInvalidBinding) {
  std::string data = "ssbo INVALID subdata i16vec3 2 2 3 4";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Invalid binding value for ssbo command", r.Error());
}

TEST_F(CommandParserTest, SSBOSubdataMissingSubdataCommand) {
  std::string data = "ssbo 6 INVALID i16vec3 2 2";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Invalid value for ssbo command: INVALID", r.Error());
}

TEST_F(CommandParserTest, SSBOSubdataWithBadType) {
  std::string data = "ssbo 0 subdata INVALID 2 2 3 4";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Invalid type provided: INVALID", r.Error());
}

TEST_F(CommandParserTest, SSBOSubdataWithInvalidFloatOffset) {
  std::string data = "ssbo 0 subdata vec2 2.0 3 2 4";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Invalid offset for ssbo command: 2.0", r.Error());
}

TEST_F(CommandParserTest, SSBOSubdataWithInvalidStringOffset) {
  std::string data = "ssbo 0 subdata vec2 asdf 3 2 4";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Invalid offset for ssbo command: asdf", r.Error());
}

TEST_F(CommandParserTest, SSBOSubdataWithMissingData) {
  std::string data = "ssbo 6 subdata i16vec3 0 2";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Incorrect number of values provided to ssbo command",
            r.Error());
}

TEST_F(CommandParserTest, SSBOSubdataWithMissingAllData) {
  std::string data = "ssbo 6 subdata i16vec3 8";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Incorrect number of values provided to ssbo command",
            r.Error());
}

TEST_F(CommandParserTest, SSBOSubdataWithNonDataTypeSizedOffset) {
  std::string data = "ssbo 6 subdata i16vec3 2";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: offset for SSBO must be a multiple of the data size expected 8",
            r.Error());
}

TEST_F(CommandParserTest, Uniform) {
  std::string data = "uniform vec3 32 2.1 3.2 4.3";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto& cmds = cp.Commands();
  ASSERT_EQ(1U, cmds.size());
  ASSERT_TRUE(cmds[0]->IsBuffer());

  auto* cmd = cmds[0]->AsBuffer();
  EXPECT_TRUE(cmd->IsPushConstant());
  EXPECT_EQ(32U, cmd->GetOffset());

  auto* fmt = cmd->GetBuffer()->GetFormat();
  ASSERT_TRUE(fmt->GetType()->IsNumber());

  auto n = fmt->GetType()->AsNumber();
  EXPECT_TRUE(type::Type::IsFloat32(n->GetFormatMode(), n->NumBits()));
  EXPECT_EQ(1U, fmt->GetType()->ColumnCount());
  EXPECT_EQ(3U, fmt->GetType()->RowCount());

  const auto* buf = cmd->GetBuffer();
  const auto* values = buf->GetValues<float>();
  std::vector<float> results = {2.1f, 3.2f, 4.3f, 0.f};
  for (size_t i = 0; i < results.size(); ++i) {
    EXPECT_FLOAT_EQ(results[i], values[i]);
  }
}

TEST_F(CommandParserTest, UniformOffsetMustBePositive) {
  std::string data = "uniform vec3 -2 2.1 3.2 4.3";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: offset for uniform must be positive, got: -2", r.Error());
}

TEST_F(CommandParserTest, UniformWithContinuation) {
  std::string data = "uniform vec3 16 2.1 3.2 4.3 \\\n5.4 6.7 8.9";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto& cmds = cp.Commands();
  ASSERT_EQ(1U, cmds.size());
  ASSERT_TRUE(cmds[0]->IsBuffer());

  auto* cmd = cmds[0]->AsBuffer();
  EXPECT_TRUE(cmd->IsPushConstant());
  EXPECT_EQ(16U, cmd->GetOffset());

  auto* fmt = cmd->GetBuffer()->GetFormat();
  ASSERT_TRUE(fmt->GetType()->IsNumber());

  auto n = fmt->GetType()->AsNumber();
  EXPECT_TRUE(type::Type::IsFloat32(n->GetFormatMode(), n->NumBits()));
  EXPECT_EQ(1U, fmt->GetType()->ColumnCount());
  EXPECT_EQ(3U, fmt->GetType()->RowCount());

  const auto* buf = cmd->GetBuffer();
  const auto* values = buf->GetValues<float>();
  std::vector<float> results = {2.1f, 3.2f, 4.3f, 0.f, 5.4f, 6.7f, 8.9f, 0.f};
  for (size_t i = 0; i < results.size(); ++i) {
    EXPECT_FLOAT_EQ(results[i], values[i]);
  }
}

TEST_F(CommandParserTest, UniformInvalidType) {
  std::string data = "uniform INVALID 0 2.1 3.2 4.3";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Invalid type provided: INVALID", r.Error());
}

TEST_F(CommandParserTest, UniformInvalidFloatOffset) {
  std::string data = "uniform vec3 5.5 2.1 3.2 4.3";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Invalid offset value for uniform command: 5.5", r.Error());
}

TEST_F(CommandParserTest, UniformInvalidStringOffset) {
  std::string data = "uniform vec3 INVALID 2.1 3.2 4.3";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Invalid offset value for uniform command: INVALID", r.Error());
}

TEST_F(CommandParserTest, UniformMissingValues) {
  std::string data = "uniform vec3 0 2.1 3.2 4.3 5.5";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Incorrect number of values provided to uniform command",
            r.Error());
}

TEST_F(CommandParserTest, UniformUBO) {
  std::string data = "uniform ubo 2 vec3 0 2.1 3.2 4.3";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto& cmds = cp.Commands();
  ASSERT_EQ(1U, cmds.size());
  ASSERT_TRUE(cmds[0]->IsBuffer());

  auto* cmd = cmds[0]->AsBuffer();
  EXPECT_TRUE(cmd->IsUniform());
  EXPECT_EQ(static_cast<uint32_t>(0), cmd->GetDescriptorSet());
  EXPECT_EQ(2U, cmd->GetBinding());
  EXPECT_EQ(static_cast<uint32_t>(0), cmd->GetOffset());

  auto* fmt = cmd->GetBuffer()->GetFormat();
  ASSERT_TRUE(fmt->GetType()->IsNumber());

  auto n = fmt->GetType()->AsNumber();
  EXPECT_TRUE(type::Type::IsFloat32(n->GetFormatMode(), n->NumBits()));
  EXPECT_EQ(1U, fmt->GetType()->ColumnCount());
  EXPECT_EQ(3U, fmt->GetType()->RowCount());

  const auto& values = cmd->GetValues();
  std::vector<float> results = {2.1f, 3.2f, 4.3f};
  ASSERT_EQ(results.size(), values.size());
  for (size_t i = 0; i < results.size(); ++i) {
    EXPECT_FLOAT_EQ(results[i], values[i].AsFloat());
  }
}

TEST_F(CommandParserTest, UniformUBODisallowUpdatingInMiddleOfElement) {
  std::string data = "uniform ubo 2 vec3 4 2.1 3.2 4.3";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());

  EXPECT_EQ("1: offset for uniform must be multiple of data size", r.Error());
}

TEST_F(CommandParserTest, UniformUBOOffsetMustBePositive) {
  std::string data = "uniform ubo 2 vec3 -1 2.1 3.2 4.3";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: offset for uniform must be positive, got: -1", r.Error());
}

TEST_F(CommandParserTest, UniformUBOWithDescriptorSet) {
  std::string data = "uniform ubo 3:2 vec3 16 2.1 3.2 4.3";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto& cmds = cp.Commands();
  ASSERT_EQ(1U, cmds.size());
  ASSERT_TRUE(cmds[0]->IsBuffer());

  auto* cmd = cmds[0]->AsBuffer();
  EXPECT_TRUE(cmd->IsUniform());
  EXPECT_EQ(3U, cmd->GetDescriptorSet());
  EXPECT_EQ(2U, cmd->GetBinding());
  EXPECT_EQ(16U, cmd->GetOffset());

  auto* fmt = cmd->GetBuffer()->GetFormat();
  ASSERT_TRUE(fmt->GetType()->IsNumber());

  auto n = fmt->GetType()->AsNumber();
  EXPECT_TRUE(type::Type::IsFloat32(n->GetFormatMode(), n->NumBits()));
  EXPECT_EQ(1U, fmt->GetType()->ColumnCount());
  EXPECT_EQ(3U, fmt->GetType()->RowCount());

  const auto& values = cmd->GetValues();
  std::vector<float> results = {2.1f, 3.2f, 4.3f};
  ASSERT_EQ(results.size(), values.size());
  for (size_t i = 0; i < results.size(); ++i) {
    EXPECT_FLOAT_EQ(results[i], values[i].AsFloat());
  }
}

TEST_F(CommandParserTest, UniformUBOInvalidFloatBinding) {
  std::string data = "uniform ubo 0.0 vec3 0 2.1 3.2 4.3";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Invalid binding value for uniform ubo command: 0.0", r.Error());
}

TEST_F(CommandParserTest, UniformUBOInvalidStringBinding) {
  std::string data = "uniform ubo INVALID vec3 0 2.1 3.2 4.3";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Invalid binding value for uniform ubo command: INVALID",
            r.Error());
}

TEST_F(CommandParserTest, UniformUBOInvalidType) {
  std::string data = "uniform ubo 0 INVALID 0 2.1 3.2 4.3";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Invalid type provided: INVALID", r.Error());
}

TEST_F(CommandParserTest, UniformUBOInvalidFloatOffset) {
  std::string data = "uniform ubo 0 vec3 5.5 2.1 3.2 4.3";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Invalid offset value for uniform command: 5.5", r.Error());
}

TEST_F(CommandParserTest, UniformUBOInvalidStringOffset) {
  std::string data = "uniform ubo 0 vec3 INVALID 2.1 3.2 4.3";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Invalid offset value for uniform command: INVALID", r.Error());
}

TEST_F(CommandParserTest, UniformUBOMissingValues) {
  std::string data = "uniform ubo 0 vec3 0 2.1 3.2 4.3 5.5";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Incorrect number of values provided to uniform command",
            r.Error());
}

TEST_F(CommandParserTest, ToleranceSingleFloatValue) {
  std::string data = "tolerance 0.5";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto& tolerances = cp.TolerancesForTesting();
  ASSERT_EQ(1U, tolerances.size());
  EXPECT_FALSE(tolerances[0].is_percent);
  EXPECT_DOUBLE_EQ(0.5, tolerances[0].value);
}

TEST_F(CommandParserTest, ToleranceSingleFloatPercent) {
  std::string data = "tolerance 0.5%";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto& tolerances = cp.TolerancesForTesting();
  ASSERT_EQ(1U, tolerances.size());
  EXPECT_TRUE(tolerances[0].is_percent);
  EXPECT_DOUBLE_EQ(0.5, tolerances[0].value);
}

TEST_F(CommandParserTest, ToleranceSingleIntValue) {
  std::string data = "tolerance 5";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto& tolerances = cp.TolerancesForTesting();
  ASSERT_EQ(1U, tolerances.size());
  EXPECT_FALSE(tolerances[0].is_percent);
  EXPECT_DOUBLE_EQ(5.0, tolerances[0].value);
}

TEST_F(CommandParserTest, ToleranceSingleIntPercent) {
  std::string data = "tolerance 5%";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto& tolerances = cp.TolerancesForTesting();
  ASSERT_EQ(1U, tolerances.size());
  EXPECT_TRUE(tolerances[0].is_percent);
  EXPECT_DOUBLE_EQ(5.0, tolerances[0].value);
}

TEST_F(CommandParserTest, ToleranceMultiFloatValue) {
  std::string data = "tolerance 0.5 2.4 3.9 99.7";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto& tolerances = cp.TolerancesForTesting();
  std::vector<double> results = {0.5, 2.4, 3.9, 99.7};
  ASSERT_EQ(results.size(), tolerances.size());
  for (size_t i = 0; i < results.size(); ++i) {
    EXPECT_FALSE(tolerances[0].is_percent);
    EXPECT_DOUBLE_EQ(results[i], tolerances[i].value);
  }
}

TEST_F(CommandParserTest, ToleranceMultiFloatValueWithPercent) {
  std::string data = "tolerance 0.5% 2.4 3.9% 99.7";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto& tolerances = cp.TolerancesForTesting();
  std::vector<double> results = {0.5, 2.4, 3.9, 99.7};
  ASSERT_EQ(results.size(), tolerances.size());
  for (size_t i = 0; i < results.size(); ++i) {
    if (i % 2 == 0)
      EXPECT_TRUE(tolerances[i].is_percent);
    else
      EXPECT_FALSE(tolerances[i].is_percent);

    EXPECT_DOUBLE_EQ(results[i], tolerances[i].value);
  }
}

TEST_F(CommandParserTest, ToleranceMultiIntValue) {
  std::string data = "tolerance 5 4 3 99";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto& tolerances = cp.TolerancesForTesting();
  std::vector<double> results = {5.0, 4.0, 3.0, 99.0};
  ASSERT_EQ(results.size(), tolerances.size());
  for (size_t i = 0; i < results.size(); ++i) {
    EXPECT_FALSE(tolerances[0].is_percent);
    EXPECT_DOUBLE_EQ(results[i], tolerances[i].value);
  }
}

TEST_F(CommandParserTest, ToleranceMultiIntValueWithPercent) {
  std::string data = "tolerance 5% 4 3% 99";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto& tolerances = cp.TolerancesForTesting();
  std::vector<double> results = {5.0, 4.0, 3.0, 99.0};
  ASSERT_EQ(results.size(), tolerances.size());
  for (size_t i = 0; i < results.size(); ++i) {
    if (i % 2 == 0)
      EXPECT_TRUE(tolerances[i].is_percent);
    else
      EXPECT_FALSE(tolerances[i].is_percent);

    EXPECT_DOUBLE_EQ(results[i], tolerances[i].value);
  }
}

TEST_F(CommandParserTest, ToleranceInvalidValue1) {
  std::string data = "tolerance INVALID";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Invalid value for tolerance command: INVALID", r.Error());
}

TEST_F(CommandParserTest, ToleranceInvalidJustPercent) {
  std::string data = "tolerance %";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Invalid value for tolerance command: %", r.Error());
}

TEST_F(CommandParserTest, ToleranceInvalidValue2) {
  std::string data = "tolerance 1 INVALID 3 4";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Invalid value for tolerance command: INVALID", r.Error());
}

TEST_F(CommandParserTest, ToleranceInvalidValue3) {
  std::string data = "tolerance 1 2 INVALID 4";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Invalid value for tolerance command: INVALID", r.Error());
}

TEST_F(CommandParserTest, ToleranceInvalidValue4) {
  std::string data = "tolerance 1 2 3 INVALID";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Invalid value for tolerance command: INVALID", r.Error());
}

TEST_F(CommandParserTest, ToleranceMissingValues) {
  std::string data = "tolerance";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Missing value for tolerance command", r.Error());
}

TEST_F(CommandParserTest, ToleranceTooManyValues) {
  std::string data = "tolerance 1 2 3 4 5";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Extra parameter for tolerance command: 5", r.Error());
}

TEST_F(CommandParserTest, ToleranceInvalidWithNumber) {
  std::string data = "tolerance 1INVALID";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Invalid value for tolerance command: INVALID", r.Error());
}

TEST_F(CommandParserTest, ToleranceInvalidWithMissingValue) {
  std::string data = "tolerance 1, , 3, 4";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Invalid number of tolerance parameters provided", r.Error());
}

TEST_F(CommandParserTest, ToleranceWithCommas) {
  std::string data = "tolerance 1,2, 3 ,4";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto& tolerances = cp.TolerancesForTesting();
  std::vector<double> results = {1.0, 2.0, 3.0, 4.0};
  ASSERT_EQ(results.size(), tolerances.size());
  for (size_t i = 0; i < results.size(); ++i) {
    EXPECT_FALSE(tolerances[0].is_percent);
    EXPECT_DOUBLE_EQ(results[i], tolerances[i].value);
  }
}

TEST_F(CommandParserTest, ProbeSSBOWithTolerance) {
  std::string data = R"(
ssbo 3:6 3
tolerance 2 3 4 5
probe ssbo vec3 3:6 2 >= 2.3 4.2 1.2)";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto& cmds = cp.Commands();
  ASSERT_EQ(2U, cmds.size());
  ASSERT_TRUE(cmds[1]->IsProbeSSBO());

  auto* cmd = cmds[1]->AsProbeSSBO();
  ASSERT_TRUE(cmd->HasTolerances());

  auto& tolerances = cmd->GetTolerances();
  std::vector<double> vals = {2, 3, 4, 5};
  ASSERT_EQ(vals.size(), tolerances.size());
  for (size_t i = 0; i < vals.size(); ++i) {
    EXPECT_FALSE(tolerances[i].is_percent);
    EXPECT_DOUBLE_EQ(vals[i], tolerances[i].value);
  }
}

TEST_F(CommandParserTest, ProbeWithTolerance) {
  std::string data = R"(
tolerance 2% 3% 4% 5%
probe all rgba 0.2 0.3 0.4 0.5)";

  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();
  pipeline.AddColorAttachment(color_buf.get(), 0);

  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto& cmds = cp.Commands();
  ASSERT_EQ(1U, cmds.size());
  ASSERT_TRUE(cmds[0]->IsProbe());

  auto* cmd = cmds[0]->AsProbe();
  ASSERT_TRUE(cmd->HasTolerances());

  auto& tolerances = cmd->GetTolerances();
  std::vector<double> vals = {2, 3, 4, 5};
  ASSERT_EQ(vals.size(), tolerances.size());
  for (size_t i = 0; i < vals.size(); ++i) {
    EXPECT_TRUE(tolerances[i].is_percent);
    EXPECT_DOUBLE_EQ(vals[i], tolerances[i].value);
  }
}

TEST_F(CommandParserTest, ProbeSSBOWithDescriptorSet) {
  std::string data = R"(
ssbo 3:6 2
probe ssbo vec3 3:6 2 >= 2.3 4.2 1.2)";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto& cmds = cp.Commands();
  ASSERT_EQ(2U, cmds.size());
  ASSERT_TRUE(cmds[1]->IsProbeSSBO());

  auto* cmd = cmds[1]->AsProbeSSBO();
  EXPECT_EQ(3U, cmd->GetDescriptorSet());
  EXPECT_EQ(6U, cmd->GetBinding());
  EXPECT_EQ(2U, cmd->GetOffset());
  EXPECT_EQ(ProbeSSBOCommand::Comparator::kGreaterOrEqual,
            cmd->GetComparator());

  auto* fmt = cmd->GetFormat();
  ASSERT_TRUE(fmt->GetType()->IsNumber());

  auto n = fmt->GetType()->AsNumber();
  EXPECT_TRUE(type::Type::IsFloat32(n->GetFormatMode(), n->NumBits()));
  EXPECT_EQ(1U, fmt->GetType()->ColumnCount());
  EXPECT_EQ(3U, fmt->GetType()->RowCount());

  const auto& values = cmd->GetValues();
  std::vector<float> results = {2.3f, 4.2f, 1.2f};
  ASSERT_EQ(results.size(), values.size());
  for (size_t i = 0; i < results.size(); ++i) {
    EXPECT_FLOAT_EQ(results[i], values[i].AsFloat());
  }
}

TEST_F(CommandParserTest, ProbeSSBOWithFloats) {
  std::string data = R"(
ssbo 6 2
probe ssbo vec3 6 2 >= 2.3 4.2 1.2)";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto& cmds = cp.Commands();
  ASSERT_EQ(2U, cmds.size());
  ASSERT_TRUE(cmds[1]->IsProbeSSBO());

  auto* cmd = cmds[1]->AsProbeSSBO();
  EXPECT_EQ(static_cast<uint32_t>(0), cmd->GetDescriptorSet());
  EXPECT_EQ(6U, cmd->GetBinding());
  EXPECT_EQ(2U, cmd->GetOffset());
  EXPECT_EQ(ProbeSSBOCommand::Comparator::kGreaterOrEqual,
            cmd->GetComparator());

  auto* fmt = cmd->GetFormat();
  ASSERT_TRUE(fmt->GetType()->IsNumber());

  auto n = fmt->GetType()->AsNumber();
  EXPECT_TRUE(type::Type::IsFloat32(n->GetFormatMode(), n->NumBits()));
  EXPECT_EQ(1U, fmt->GetType()->ColumnCount());
  EXPECT_EQ(3U, fmt->GetType()->RowCount());

  const auto& values = cmd->GetValues();
  std::vector<float> results = {2.3f, 4.2f, 1.2f};
  ASSERT_EQ(results.size(), values.size());
  for (size_t i = 0; i < results.size(); ++i) {
    EXPECT_FLOAT_EQ(results[i], values[i].AsFloat());
  }
}

TEST_F(CommandParserTest, MultiProbeSSBOWithFloats) {
  std::string data = R"(
ssbo 6 2
probe ssbo vec3 6 2 >= 2.3 4.2 1.2
probe ssbo vec3 6 2 >= 2.3 4.2 1.2)";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto& cmds = cp.Commands();
  ASSERT_EQ(3U, cmds.size());
  ASSERT_TRUE(cmds[1]->IsProbeSSBO());

  auto* cmd = cmds[1]->AsProbeSSBO();
  EXPECT_EQ(6U, cmd->GetBinding());
  EXPECT_EQ(2U, cmd->GetOffset());
  EXPECT_EQ(ProbeSSBOCommand::Comparator::kGreaterOrEqual,
            cmd->GetComparator());

  auto* fmt = cmd->GetFormat();
  ASSERT_TRUE(fmt->GetType()->IsNumber());

  auto n = fmt->GetType()->AsNumber();
  EXPECT_TRUE(type::Type::IsFloat32(n->GetFormatMode(), n->NumBits()));
  EXPECT_EQ(1U, fmt->GetType()->ColumnCount());
  EXPECT_EQ(3U, fmt->GetType()->RowCount());

  const auto& values = cmd->GetValues();
  std::vector<float> results = {2.3f, 4.2f, 1.2f};
  ASSERT_EQ(results.size(), values.size());
  for (size_t i = 0; i < results.size(); ++i) {
    EXPECT_FLOAT_EQ(results[i], values[i].AsFloat());
  }
}

TEST_F(CommandParserTest, ProbeSSBOWithInts) {
  std::string data = R"(
ssbo 6 2
probe ssbo i16vec3 6 2 <= 2 4 1)";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto& cmds = cp.Commands();
  ASSERT_EQ(2U, cmds.size());
  ASSERT_TRUE(cmds[1]->IsProbeSSBO());

  auto* cmd = cmds[1]->AsProbeSSBO();
  EXPECT_EQ(static_cast<uint32_t>(0), cmd->GetDescriptorSet());
  EXPECT_EQ(6U, cmd->GetBinding());
  EXPECT_EQ(2U, cmd->GetOffset());
  EXPECT_EQ(ProbeSSBOCommand::Comparator::kLessOrEqual, cmd->GetComparator());

  auto* fmt = cmd->GetFormat();
  ASSERT_TRUE(fmt->GetType()->IsNumber());

  auto n = fmt->GetType()->AsNumber();
  EXPECT_TRUE(type::Type::IsInt16(n->GetFormatMode(), n->NumBits()));
  EXPECT_EQ(1U, fmt->GetType()->ColumnCount());
  EXPECT_EQ(3U, fmt->GetType()->RowCount());

  const auto& values = cmd->GetValues();
  std::vector<int16_t> results = {2, 4, 1};
  ASSERT_EQ(results.size(), values.size());
  for (size_t i = 0; i < results.size(); ++i) {
    EXPECT_FLOAT_EQ(results[i], values[i].AsInt16());
  }
}

TEST_F(CommandParserTest, ProbeSSBOWithMultipleVectors) {
  std::string data = R"(
ssbo 6 2
probe ssbo i16vec3 6 2 == 2 4 1 3 6 8)";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto& cmds = cp.Commands();
  ASSERT_EQ(2U, cmds.size());
  ASSERT_TRUE(cmds[1]->IsProbeSSBO());

  auto* cmd = cmds[1]->AsProbeSSBO();
  EXPECT_EQ(static_cast<uint32_t>(0), cmd->GetDescriptorSet());
  EXPECT_EQ(6U, cmd->GetBinding());
  EXPECT_EQ(2U, cmd->GetOffset());
  EXPECT_EQ(ProbeSSBOCommand::Comparator::kEqual, cmd->GetComparator());

  auto* fmt = cmd->GetFormat();
  ASSERT_TRUE(fmt->GetType()->IsNumber());

  auto n = fmt->GetType()->AsNumber();
  EXPECT_TRUE(type::Type::IsInt16(n->GetFormatMode(), n->NumBits()));
  EXPECT_EQ(1U, fmt->GetType()->ColumnCount());
  EXPECT_EQ(3U, fmt->GetType()->RowCount());

  const auto& values = cmd->GetValues();
  std::vector<int16_t> results = {2, 4, 1, 3, 6, 8};
  ASSERT_EQ(results.size(), values.size());
  for (size_t i = 0; i < results.size(); ++i) {
    EXPECT_FLOAT_EQ(results[i], values[i].AsInt16());
  }
}

TEST_F(CommandParserTest, ProbeSSBOMissingBinding) {
  std::string data = "probe ssbo i16vec3 2 == 2 3 2";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Invalid value for probe ssbo command: ==", r.Error());
}

TEST_F(CommandParserTest, ProbeSSBOWithInvalidBinding) {
  std::string data = "probe ssbo i16vec3 INVALID 2 == 2 3 4";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Invalid binding value for probe ssbo command: INVALID",
            r.Error());
}

TEST_F(CommandParserTest, ProbeSSBOWithBadType) {
  std::string data = "probe ssbo INVALID 0 2 == 2 3 4";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Invalid type provided: INVALID", r.Error());
}

TEST_F(CommandParserTest, ProbeSSBOWithInvalidFloatOffset) {
  std::string data = R"(
ssbo 0 2
probe ssbo vec2 0 2.0 == 3 2 4)";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: Invalid offset for probe ssbo command: 2.0", r.Error());
}

TEST_F(CommandParserTest, ProbeSSBOWithInvalidStringOffset) {
  std::string data = "probe ssbo vec2 0 INVALID == 3 2 4";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Invalid value for probe ssbo command: INVALID", r.Error());
}

TEST_F(CommandParserTest, ProbeSSBOWithInvalidComparator) {
  std::string data = R"(
ssbo 6 2
probe ssbo vec2 6 2 INVALID 3 2 4)";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: Invalid comparator: INVALID", r.Error());
}

TEST_F(CommandParserTest, ProbeSSBOWithMissingData) {
  std::string data = R"(
ssbo 6 2
probe ssbo i16vec3 6 2 == 2)";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: Incorrect number of values provided to probe ssbo command",
            r.Error());
}

TEST_F(CommandParserTest, ProbeSSBOWithMissingAllData) {
  std::string data = R"(
ssbo 6 2
probe ssbo i16vec3 6 2 ==)";

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, data);
  Result r = cp.Parse();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: Incorrect number of values provided to probe ssbo command",
            r.Error());
}

struct ComparatorTest {
  const char* name;
  ProbeSSBOCommand::Comparator op;
};
using CommandParserComparatorTests = testing::TestWithParam<ComparatorTest>;

TEST_P(CommandParserComparatorTests, Comparator) {
  const auto& test_data = GetParam();

  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, "unused");
  ProbeSSBOCommand::Comparator result;
  Result r = cp.ParseComparatorForTesting(test_data.name, &result);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  EXPECT_EQ(test_data.op, result);
}

INSTANTIATE_TEST_SUITE_P(
    ComparatorTests,
    CommandParserComparatorTests,
    testing::Values(
        ComparatorTest{"==", ProbeSSBOCommand::Comparator::kEqual},
        ComparatorTest{"!=", ProbeSSBOCommand::Comparator::kNotEqual},
        ComparatorTest{"~=", ProbeSSBOCommand::Comparator::kFuzzyEqual},
        ComparatorTest{"<", ProbeSSBOCommand::Comparator::kLess},
        ComparatorTest{"<=", ProbeSSBOCommand::Comparator::kLessOrEqual},
        ComparatorTest{">", ProbeSSBOCommand::Comparator::kGreater},
        ComparatorTest{">=",
                       ProbeSSBOCommand::Comparator::
                           kGreaterOrEqual}));  // NOLINT(whitespace/parens)

TEST_F(CommandParserTest, ComparatorInvalid) {
  Pipeline pipeline(PipelineType::kGraphics);
  Script script;
  CommandParser cp(&script, &pipeline, 1, "unused");
  ProbeSSBOCommand::Comparator result;
  Result r = cp.ParseComparatorForTesting("INVALID", &result);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("Invalid comparator: INVALID", r.Error());
}

}  // namespace vkscript
}  // namespace amber
