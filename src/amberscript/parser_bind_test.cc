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
  EXPECT_EQ(250 * 250, buf_info.buffer->ElementCount());
  EXPECT_EQ(250 * 250 * 4, buf_info.buffer->ValueCount());
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
  EXPECT_EQ(90 * 180, buf1.buffer->ElementCount());
  EXPECT_EQ(90 * 180 * 4, buf1.buffer->ValueCount());
  EXPECT_EQ(90 * 180 * 4 * sizeof(float), buf1.buffer->GetSizeInBytes());

  pipeline = pipelines[1].get();
  const auto& color_buffers2 = pipeline->GetColorAttachments();
  const auto& buf2 = color_buffers2[0];
  ASSERT_TRUE(buf2.buffer != nullptr);
  EXPECT_EQ(9, buf2.location);
  EXPECT_EQ(256 * 300, buf2.buffer->ElementCount());
  EXPECT_EQ(256 * 300 * 4, buf2.buffer->ValueCount());
  EXPECT_EQ(256 * 300 * 4 * sizeof(uint8_t), buf2.buffer->GetSizeInBytes());
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
  EXPECT_EQ(90 * 180, buf1.buffer->ElementCount());
  EXPECT_EQ(90 * 180 * 4, buf1.buffer->ValueCount());
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
  EXPECT_EQ(90 * 180, buf1.buffer->ElementCount());
  EXPECT_EQ(90 * 180 * 4, buf1.buffer->ValueCount());
  EXPECT_EQ(90 * 180 * 4 * sizeof(float), buf1.buffer->GetSizeInBytes());
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
  EXPECT_EQ(90 * 180, buf.buffer->ElementCount());
  EXPECT_EQ(90 * 180 * 4, buf.buffer->ValueCount());
  EXPECT_EQ(90 * 180 * 4 * sizeof(float), buf.buffer->GetSizeInBytes());
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
BUFFER my_buf DATA_TYPE int8 SIZE 5 FILL 5
BUFFER my_buf2 DATA_TYPE int8 SIZE 5 FILL 5

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
  EXPECT_EQ(0, info1.location);

  const auto& info2 = vertex_buffers[1];
  ASSERT_TRUE(info2.buffer != nullptr);
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
            bufs[0].buffer->GetFormat()->GetFormatType());
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

  BIND BUFFER my_buf AS uniform DESCRIPTOR_SET 1
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("13: missing BINDING for BIND command", r.Error());
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
  EXPECT_EQ("12: missing DESCRIPTOR_SET or KERNEL for BIND command", r.Error());
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

  BIND BUFFER my_buf AS INVALID DESCRIPTOR_SET 1 BINDING 2
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("12: unknown buffer_type: INVALID", r.Error());
}

struct BufferTypeData {
  const char* name;
  BufferType type;
};

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
INSTANTIATE_TEST_SUITE_P(
    AmberScriptParserBufferTypeTestSamples,
    AmberScriptParserBufferTypeTest,
    testing::Values(BufferTypeData{"uniform", BufferType::kUniform},
                    BufferTypeData{
                        "storage",
                        BufferType::kStorage}));  // NOLINT(whitespace/parens)

TEST_F(AmberScriptParserTest, BindPushConstants) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_buf DATA_TYPE float SIZE 20 FILL 5

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_buf AS push_constant
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& pipelines = script->GetPipelines();
  ASSERT_EQ(1U, pipelines.size());

  const auto* pipeline = pipelines[0].get();
  const auto& buf = pipeline->GetPushConstantBuffer();
  ASSERT_TRUE(buf.buffer != nullptr);
  EXPECT_EQ(20, buf.buffer->ElementCount());
  EXPECT_EQ(20, buf.buffer->ValueCount());
  EXPECT_EQ(20 * sizeof(float), buf.buffer->GetSizeInBytes());
}

TEST_F(AmberScriptParserTest, BindPushConstantsExtraParams) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_buf DATA_TYPE float SIZE 20 FILL 5

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_buf AS push_constant EXTRA
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("12: extra parameters after BIND command", r.Error());
}

TEST_F(AmberScriptParserTest, BindBufferOpenCLArgName) {
  std::string in = R"(
SHADER compute my_shader OPENCL-C
#shader
END
BUFFER my_buf DATA_TYPE uint32 DATA 1 END

PIPELINE compute my_pipeline
  ATTACH my_shader
  BIND BUFFER my_buf AS storage KERNEL ARG_NAME arg
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess());
}

TEST_F(AmberScriptParserTest, BindBufferOpenCLArgNumber) {
  std::string in = R"(
SHADER compute my_shader OPENCL-C
#shader
END
BUFFER my_buf DATA_TYPE uint32 DATA 1 END

PIPELINE compute my_pipeline
  ATTACH my_shader
  BIND BUFFER my_buf AS storage KERNEL ARG_NUMBER 0
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess());
}

TEST_F(AmberScriptParserTest, BindBufferOpenCLArgNameTypeless) {
  std::string in = R"(
SHADER compute my_shader OPENCL-C
#shader
END
BUFFER my_buf DATA_TYPE uint32 DATA 1 END

PIPELINE compute my_pipeline
  ATTACH my_shader
  BIND BUFFER my_buf KERNEL ARG_NAME arg
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess());
}

TEST_F(AmberScriptParserTest, BindBufferOpenCLArgNumberTypeless) {
  std::string in = R"(
SHADER compute my_shader OPENCL-C
#shader
END
BUFFER my_buf DATA_TYPE uint32 DATA 1 END

PIPELINE compute my_pipeline
  ATTACH my_shader
  BIND BUFFER my_buf KERNEL ARG_NUMBER 0
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess());
}

TEST_F(AmberScriptParserTest, BindBufferOpenCLMissingKernel) {
  std::string in = R"(
SHADER compute my_shader OPENCL-C
#shader
END
BUFFER my_buf DATA_TYPE uint32 DATA 1 END

PIPELINE compute my_pipeline
  ATTACH my_shader
  BIND BUFFER my_buf AS storage ARG_NAME arg
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("9: missing DESCRIPTOR_SET or KERNEL for BIND command", r.Error());
}

TEST_F(AmberScriptParserTest, BindBufferOpenCLMissingArg) {
  std::string in = R"(
SHADER compute my_shader OPENCL-C
#shader
END
BUFFER my_buf DATA_TYPE uint32 DATA 1 END

PIPELINE compute my_pipeline
  ATTACH my_shader
  BIND BUFFER my_buf AS storage KERNEL arg
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("9: missing ARG_NAME or ARG_NUMBER keyword", r.Error());
}

TEST_F(AmberScriptParserTest, BindBufferOpenCLMissingArgName) {
  std::string in = R"(
SHADER compute my_shader OPENCL-C
#shader
END
BUFFER my_buf DATA_TYPE uint32 DATA 1 END

PIPELINE compute my_pipeline
  ATTACH my_shader
  BIND BUFFER my_buf KERNEL ARG_NAME
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("10: expected argument identifier", r.Error());
}

TEST_F(AmberScriptParserTest, BindBufferOpenCLMissingArgNumber) {
  std::string in = R"(
SHADER compute my_shader OPENCL-C
#shader
END
BUFFER my_buf DATA_TYPE uint32 DATA 1 END

PIPELINE compute my_pipeline
  ATTACH my_shader
  BIND BUFFER my_buf AS storage KERNEL ARG_NUMBER
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("10: expected argument number", r.Error());
}

TEST_F(AmberScriptParserTest, BindBufferOpenCLArgNameNotString) {
  std::string in = R"(
SHADER compute my_shader OPENCL-C
#shader
END
BUFFER my_buf DATA_TYPE uint32 DATA 1 END

PIPELINE compute my_pipeline
  ATTACH my_shader
  BIND BUFFER my_buf AS storage KERNEL ARG_NAME 0
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("9: expected argument identifier", r.Error());
}

TEST_F(AmberScriptParserTest, BindBufferOpenCLArgNumberNotInteger) {
  std::string in = R"(
SHADER compute my_shader OPENCL-C
#shader
END
BUFFER my_buf DATA_TYPE uint32 DATA 1 END

PIPELINE compute my_pipeline
  ATTACH my_shader
  BIND BUFFER my_buf KERNEL ARG_NUMBER in
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("9: expected argument number", r.Error());
}

}  // namespace amberscript
}  // namespace amber
