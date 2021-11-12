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
  EXPECT_EQ(0u, buf_info.location);
  EXPECT_EQ(250u * 250u, buf_info.buffer->ElementCount());
  EXPECT_EQ(250u * 250u * 4u, buf_info.buffer->ValueCount());
  EXPECT_EQ(250u * 250u * 4u * sizeof(float),
            buf_info.buffer->GetSizeInBytes());
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
  EXPECT_EQ("12: extra parameters after BIND command: EXTRA", r.Error());
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
  EXPECT_EQ(0u, buf1.location);
  EXPECT_EQ(90u * 180u, buf1.buffer->ElementCount());
  EXPECT_EQ(90u * 180u * 4u, buf1.buffer->ValueCount());
  EXPECT_EQ(90u * 180u * 4u * sizeof(float), buf1.buffer->GetSizeInBytes());

  pipeline = pipelines[1].get();
  const auto& color_buffers2 = pipeline->GetColorAttachments();
  const auto& buf2 = color_buffers2[0];
  ASSERT_TRUE(buf2.buffer != nullptr);
  EXPECT_EQ(9u, buf2.location);
  EXPECT_EQ(256u * 300u, buf2.buffer->ElementCount());
  EXPECT_EQ(256u * 300u * 4u, buf2.buffer->ValueCount());
  EXPECT_EQ(256u * 300u * 4u * sizeof(uint8_t), buf2.buffer->GetSizeInBytes());
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
  EXPECT_EQ(0u, buf1.location);
  EXPECT_EQ(90u * 180u, buf1.buffer->ElementCount());
  EXPECT_EQ(90u * 180u * 4u, buf1.buffer->ValueCount());
  EXPECT_EQ(90u * 180u * 4u * sizeof(float), buf1.buffer->GetSizeInBytes());
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
  EXPECT_EQ(0u, buf1.location);
  EXPECT_EQ(90u * 180u, buf1.buffer->ElementCount());
  EXPECT_EQ(90u * 180u * 4u, buf1.buffer->ValueCount());
  EXPECT_EQ(90u * 180u * 4u * sizeof(float), buf1.buffer->GetSizeInBytes());
}

TEST_F(AmberScriptParserTest, BindColorBaseMipLevel) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT MIP_LEVELS 2

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0 BASE_MIP_LEVEL 1
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
  EXPECT_EQ(1u, buf1.base_mip_level);
}

TEST_F(AmberScriptParserTest, BindColorMissingBaseMipLevel) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT MIP_LEVELS 2

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0 BASE_MIP_LEVEL
  FRAMEBUFFER_SIZE 90 180
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("13: invalid value for BASE_MIP_LEVEL", r.Error());
}

TEST_F(AmberScriptParserTest, BindColorBaseMipLevelTooLarge) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT MIP_LEVELS 2

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0 BASE_MIP_LEVEL 2
  FRAMEBUFFER_SIZE 90 180
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ(
      "12: base mip level (now 2) needs to be larger than the number of buffer "
      "mip maps (2)",
      r.Error());
}

TEST_F(AmberScriptParserTest, BindColorTooManyMipLevels) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT MIP_LEVELS 20

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0
  FRAMEBUFFER_SIZE 90 180
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ(
      "color attachment with 20 mip levels would have zero width for level 7",
      r.Error());
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
  const auto& buf = pipeline->GetDepthStencilBuffer();
  ASSERT_TRUE(buf.buffer != nullptr);
  EXPECT_EQ(90u * 180u, buf.buffer->ElementCount());
  EXPECT_EQ(90u * 180u * 4u, buf.buffer->ValueCount());
  EXPECT_EQ(90u * 180u * 4u * sizeof(float), buf.buffer->GetSizeInBytes());
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
  EXPECT_EQ("12: extra parameters after BIND command: EXTRA", r.Error());
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
  EXPECT_EQ("14: can only bind one depth/stencil buffer in a PIPELINE",
            r.Error());
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
  ASSERT_EQ(2u, vertex_buffers.size());

  const auto& info1 = vertex_buffers[0];
  ASSERT_TRUE(info1.buffer != nullptr);
  EXPECT_EQ(0u, info1.location);
  EXPECT_EQ(InputRate::kVertex, info1.input_rate);

  const auto& info2 = vertex_buffers[1];
  ASSERT_TRUE(info2.buffer != nullptr);
  EXPECT_EQ(1u, info2.location);
  EXPECT_EQ(InputRate::kVertex, info2.input_rate);
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
  VERTEX_DATA my_buf LOCATION 1 OFFSET 10
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess());

  auto script = parser.GetScript();
  const auto& pipelines = script->GetPipelines();
  ASSERT_EQ(1U, pipelines.size());

  const auto* pipeline = pipelines[0].get();
  const auto& vertex_buffers = pipeline->GetVertexBuffers();
  ASSERT_EQ(2u, vertex_buffers.size());

  const auto& info1 = vertex_buffers[0];
  ASSERT_TRUE(info1.buffer != nullptr);
  EXPECT_EQ(0u, info1.location);
  EXPECT_EQ(0u, info1.offset);

  const auto& info2 = vertex_buffers[1];
  ASSERT_TRUE(info2.buffer != nullptr);
  EXPECT_EQ(1u, info2.location);
  EXPECT_EQ(10u, info2.offset);
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
  EXPECT_EQ("12: unexpected identifier for VERTEX_DATA command: EXTRA",
            r.Error());
}

TEST_F(AmberScriptParserTest, BindVertexDataInputRate) {
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

  VERTEX_DATA my_buf LOCATION 0 RATE vertex
  VERTEX_DATA my_buf2 LOCATION 1 RATE instance
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& pipelines = script->GetPipelines();
  ASSERT_EQ(1U, pipelines.size());

  const auto* pipeline = pipelines[0].get();
  const auto& vertex_buffers = pipeline->GetVertexBuffers();
  ASSERT_EQ(2u, vertex_buffers.size());

  const auto& info1 = vertex_buffers[0];
  ASSERT_TRUE(info1.buffer != nullptr);
  EXPECT_EQ(0u, info1.location);
  EXPECT_EQ(InputRate::kVertex, info1.input_rate);

  const auto& info2 = vertex_buffers[1];
  ASSERT_TRUE(info2.buffer != nullptr);
  EXPECT_EQ(1u, info2.location);
  EXPECT_EQ(InputRate::kInstance, info2.input_rate);
}

TEST_F(AmberScriptParserTest, BindVertexDataInputRateMissingValue) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_buf DATA_TYPE int8 SIZE 5 FILL 5

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  VERTEX_DATA my_buf LOCATION 0 RATE
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("13: missing input rate value for RATE", r.Error());
}

TEST_F(AmberScriptParserTest, BindVertexDataInputRateInvalidValue) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_buf DATA_TYPE int8 SIZE 5 FILL 5

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  VERTEX_DATA my_buf LOCATION 0 RATE foo
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("12: expecting 'vertex' or 'instance' for RATE value", r.Error());
}

TEST_F(AmberScriptParserTest, BindVertexDataOffset) {
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

  VERTEX_DATA my_buf LOCATION 0 OFFSET 5
  VERTEX_DATA my_buf2 LOCATION 1 OFFSET 10
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& pipelines = script->GetPipelines();
  ASSERT_EQ(1U, pipelines.size());

  const auto* pipeline = pipelines[0].get();
  const auto& vertex_buffers = pipeline->GetVertexBuffers();
  ASSERT_EQ(2u, vertex_buffers.size());

  const auto& info1 = vertex_buffers[0];
  ASSERT_TRUE(info1.buffer != nullptr);
  EXPECT_EQ(0u, info1.location);
  EXPECT_EQ(5u, info1.offset);

  const auto& info2 = vertex_buffers[1];
  ASSERT_TRUE(info2.buffer != nullptr);
  EXPECT_EQ(1u, info2.location);
  EXPECT_EQ(10u, info2.offset);
}

TEST_F(AmberScriptParserTest, BindVertexDataOffsetMissingValue) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_buf DATA_TYPE int8 SIZE 5 FILL 5

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  VERTEX_DATA my_buf LOCATION 0 OFFSET
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("13: expected unsigned integer for OFFSET", r.Error());
}

TEST_F(AmberScriptParserTest, BindVertexDataOffsetIncorrectValue) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_buf DATA_TYPE int8 SIZE 5 FILL 5

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  VERTEX_DATA my_buf LOCATION 0 OFFSET foo
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("12: expected unsigned integer for OFFSET", r.Error());
}

TEST_F(AmberScriptParserTest, BindVertexDataStride) {
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

  VERTEX_DATA my_buf LOCATION 0 STRIDE 5
  VERTEX_DATA my_buf2 LOCATION 1 STRIDE 10
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& pipelines = script->GetPipelines();
  ASSERT_EQ(1U, pipelines.size());

  const auto* pipeline = pipelines[0].get();
  const auto& vertex_buffers = pipeline->GetVertexBuffers();
  ASSERT_EQ(2u, vertex_buffers.size());

  const auto& info1 = vertex_buffers[0];
  ASSERT_TRUE(info1.buffer != nullptr);
  EXPECT_EQ(0u, info1.location);
  EXPECT_EQ(5u, info1.stride);

  const auto& info2 = vertex_buffers[1];
  ASSERT_TRUE(info2.buffer != nullptr);
  EXPECT_EQ(1u, info2.location);
  EXPECT_EQ(10u, info2.stride);
}

TEST_F(AmberScriptParserTest, BindVertexDataStrideFromFormat) {
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
  VERTEX_DATA my_buf2 LOCATION 1 FORMAT R16_UINT
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& pipelines = script->GetPipelines();
  ASSERT_EQ(1U, pipelines.size());

  const auto* pipeline = pipelines[0].get();
  const auto& vertex_buffers = pipeline->GetVertexBuffers();
  ASSERT_EQ(2u, vertex_buffers.size());

  const auto& info1 = vertex_buffers[0];
  ASSERT_TRUE(info1.buffer != nullptr);
  EXPECT_EQ(0u, info1.location);
  EXPECT_EQ(1u, info1.stride);

  const auto& info2 = vertex_buffers[1];
  ASSERT_TRUE(info2.buffer != nullptr);
  EXPECT_EQ(1u, info2.location);
  EXPECT_EQ(2u, info2.stride);
}

TEST_F(AmberScriptParserTest, BindVertexDataStrideMissingValue) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_buf DATA_TYPE int8 SIZE 5 FILL 5

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  VERTEX_DATA my_buf LOCATION 0 STRIDE
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("13: expected unsigned integer for STRIDE", r.Error());
}

TEST_F(AmberScriptParserTest, BindVertexDataStrideIncorrectValue) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_buf DATA_TYPE int8 SIZE 5 FILL 5

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  VERTEX_DATA my_buf LOCATION 0 STRIDE foo
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("12: expected unsigned integer for STRIDE", r.Error());
}

TEST_F(AmberScriptParserTest, BindVertexDataStrideZero) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_buf DATA_TYPE int8 SIZE 5 FILL 5

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  VERTEX_DATA my_buf LOCATION 0 STRIDE 0
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("12: STRIDE needs to be larger than zero", r.Error());
}

TEST_F(AmberScriptParserTest, BindVertexDataFormat) {
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

  VERTEX_DATA my_buf LOCATION 0 FORMAT R8G8_UNORM
  VERTEX_DATA my_buf2 LOCATION 1 FORMAT R8_SRGB
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& pipelines = script->GetPipelines();
  ASSERT_EQ(1U, pipelines.size());

  const auto* pipeline = pipelines[0].get();
  const auto& vertex_buffers = pipeline->GetVertexBuffers();
  ASSERT_EQ(2u, vertex_buffers.size());

  const auto& info1 = vertex_buffers[0];
  ASSERT_TRUE(info1.buffer != nullptr);
  EXPECT_EQ(0u, info1.location);
  EXPECT_EQ(FormatType::kR8G8_UNORM, info1.format->GetFormatType());

  const auto& info2 = vertex_buffers[1];
  ASSERT_TRUE(info2.buffer != nullptr);
  EXPECT_EQ(1u, info2.location);
  EXPECT_EQ(FormatType::kR8_SRGB, info2.format->GetFormatType());
}

TEST_F(AmberScriptParserTest, BindVertexDataFormatMissingValue) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_buf DATA_TYPE int8 SIZE 5 FILL 5

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  VERTEX_DATA my_buf LOCATION 0 FORMAT
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("13: vertex data FORMAT must be an identifier", r.Error());
}

TEST_F(AmberScriptParserTest, BindVertexDataFormatIncorrectValue) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_buf DATA_TYPE int8 SIZE 5 FILL 5

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  VERTEX_DATA my_buf LOCATION 0 FORMAT foo
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("12: invalid vertex data FORMAT", r.Error());
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
  EXPECT_EQ("12: extra parameters after INDEX_DATA command: EXTRA", r.Error());
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
  EXPECT_EQ(BufferType::kUniform, bufs[0].type);
  EXPECT_EQ(1U, bufs[0].descriptor_set);
  EXPECT_EQ(2U, bufs[0].binding);
  EXPECT_EQ(0U, bufs[0].descriptor_offset);
  EXPECT_EQ(~0ULL, bufs[0].descriptor_range);
  EXPECT_EQ(static_cast<uint32_t>(0), bufs[0].location);
  EXPECT_EQ(FormatType::kR32G32B32A32_SFLOAT,
            bufs[0].buffer->GetFormat()->GetFormatType());
}

TEST_F(AmberScriptParserTest, BindBufferDescriptorOffsetAndRange) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_buf FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_buf AS uniform DESCRIPTOR_SET 1 BINDING 2 DESCRIPTOR_OFFSET 256 DESCRIPTOR_RANGE 512
  BIND BUFFER my_buf AS uniform DESCRIPTOR_SET 1 BINDING 3 DESCRIPTOR_OFFSET 256 DESCRIPTOR_RANGE -1
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
  ASSERT_EQ(2U, bufs.size());
  EXPECT_EQ(BufferType::kUniform, bufs[0].type);
  EXPECT_EQ(1U, bufs[0].descriptor_set);
  EXPECT_EQ(2U, bufs[0].binding);
  EXPECT_EQ(256U, bufs[0].descriptor_offset);
  EXPECT_EQ(512U, bufs[0].descriptor_range);
  EXPECT_EQ(static_cast<uint32_t>(0), bufs[0].location);
  EXPECT_EQ(FormatType::kR32G32B32A32_SFLOAT,
            bufs[0].buffer->GetFormat()->GetFormatType());
  // Verify the descriptor range from the second buffer.
  EXPECT_EQ(~0ULL, bufs[1].descriptor_range);
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
  EXPECT_EQ("12: extra parameters after BIND command: EXTRA", r.Error());
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
  EXPECT_EQ(test_data.type, bufs[0].type);
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
  EXPECT_EQ(20u, buf.buffer->ElementCount());
  EXPECT_EQ(20u, buf.buffer->ValueCount());
  EXPECT_EQ(20u * sizeof(float), buf.buffer->GetSizeInBytes());
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
  EXPECT_EQ("12: extra parameters after BIND command: EXTRA", r.Error());
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

TEST_F(AmberScriptParserTest, BindSamplerOpenCLMissingKernel) {
  std::string in = R"(
SHADER compute my_shader OPENCL-C
#shader
END
SAMPLER s

PIPELINE compute my_pipeline
  ATTACH my_shader
  BIND SAMPLER s
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("10: expected a string token for BIND command", r.Error());
}

TEST_F(AmberScriptParserTest, BindSamplerOpenCLInvalidKernel) {
  std::string in = R"(
SHADER compute my_shader OPENCL-C
#shader
END
SAMPLER s

PIPELINE compute my_pipeline
  ATTACH my_shader
  BIND SAMPLER s INVALID
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("9: missing DESCRIPTOR_SET or KERNEL for BIND command", r.Error());
}

TEST_F(AmberScriptParserTest, BindSamplerOpenCLMissingArgument) {
  std::string in = R"(
SHADER compute my_shader OPENCL-C
#shader
END
SAMPLER s

PIPELINE compute my_pipeline
  ATTACH my_shader
  BIND SAMPLER s KERNEL
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("10: missing kernel arg identifier", r.Error());
}

TEST_F(AmberScriptParserTest, BindSamplerOpenCLMissingArgumentName) {
  std::string in = R"(
SHADER compute my_shader OPENCL-C
#shader
END
SAMPLER s

PIPELINE compute my_pipeline
  ATTACH my_shader
  BIND SAMPLER s KERNEL ARG_NAME
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("10: expected argument identifier", r.Error());
}

TEST_F(AmberScriptParserTest, BindSamplerOpenCLArgumentNameNotString) {
  std::string in = R"(
SHADER compute my_shader OPENCL-C
#shader
END
SAMPLER s

PIPELINE compute my_pipeline
  ATTACH my_shader
  BIND SAMPLER s KERNEL ARG_NAME 0
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("9: expected argument identifier", r.Error());
}

TEST_F(AmberScriptParserTest, BindSamplerOpenCLMissingArgumentNumber) {
  std::string in = R"(
SHADER compute my_shader OPENCL-C
#shader
END
SAMPLER s

PIPELINE compute my_pipeline
  ATTACH my_shader
  BIND SAMPLER s KERNEL ARG_NUMBER
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("10: expected argument number", r.Error());
}

TEST_F(AmberScriptParserTest, BindSamplerOpenCLArgumentNumberNotInteger) {
  std::string in = R"(
SHADER compute my_shader OPENCL-C
#shader
END
SAMPLER s

PIPELINE compute my_pipeline
  ATTACH my_shader
  BIND SAMPLER s KERNEL ARG_NUMBER a
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("9: expected argument number", r.Error());
}

TEST_F(AmberScriptParserTest, BindBufferStorageImageCompute) {
  std::string in = R"(
SHADER compute compute_shader GLSL
# GLSL Shader
END

BUFFER texture FORMAT R8G8B8A8_UNORM

PIPELINE compute pipeline
  ATTACH compute_shader
  BIND BUFFER texture AS storage_image DESCRIPTOR_SET 0 BINDING 0
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
  EXPECT_EQ(BufferType::kStorageImage, bufs[0].type);
}

TEST_F(AmberScriptParserTest, BindBufferStorageImageGraphics) {
  std::string in = R"(
SHADER vertex vert_shader PASSTHROUGH
SHADER fragment frag_shader GLSL
# GLSL Shader
END

BUFFER texture FORMAT R8G8B8A8_UNORM
BUFFER framebuffer FORMAT R8G8B8A8_UNORM

PIPELINE graphics pipeline
  ATTACH vert_shader
  ATTACH frag_shader
  BIND BUFFER texture AS storage_image DESCRIPTOR_SET 0 BINDING 0
  BIND BUFFER framebuffer AS color LOCATION 0
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
  EXPECT_EQ(BufferType::kStorageImage, bufs[0].type);
}

TEST_F(AmberScriptParserTest, BindBufferStorageImageMissingDescriptorSetValue) {
  std::string in = R"(
SHADER compute compute_shader GLSL
# GLSL Shader
END

BUFFER texture FORMAT R8G8B8A8_UNORM

PIPELINE compute pipeline
  ATTACH compute_shader
  BIND BUFFER texture AS storage_image DESCRIPTOR_SET BINDING 0
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("10: invalid value for DESCRIPTOR_SET in BIND command", r.Error());
}

TEST_F(AmberScriptParserTest, BindBufferStorageImageMissingDescriptorSet) {
  std::string in = R"(
SHADER compute compute_shader GLSL
# GLSL Shader
END

BUFFER texture FORMAT R8G8B8A8_UNORM

PIPELINE compute pipeline
  ATTACH compute_shader
  BIND BUFFER texture AS storage_image BINDING 0
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("10: missing DESCRIPTOR_SET or KERNEL for BIND command", r.Error());
}

TEST_F(AmberScriptParserTest, BindBufferStorageImageMissingBindingValue) {
  std::string in = R"(
SHADER compute compute_shader GLSL
# GLSL Shader
END

BUFFER texture FORMAT R8G8B8A8_UNORM

PIPELINE compute pipeline
  ATTACH compute_shader
  BIND BUFFER texture AS storage_image DESCRIPTOR_SET 0 BINDING
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("11: invalid value for BINDING in BIND command", r.Error());
}

TEST_F(AmberScriptParserTest, BindBufferStorageImageMissingBinding) {
  std::string in = R"(
SHADER compute compute_shader GLSL
# GLSL Shader
END

BUFFER texture FORMAT R8G8B8A8_UNORM

PIPELINE compute pipeline
  ATTACH compute_shader
  BIND BUFFER texture AS storage_image DESCRIPTOR_SET 0
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("11: missing BINDING for BIND command", r.Error());
}

TEST_F(AmberScriptParserTest, BindBufferSampledImage) {
  std::string in = R"(
SHADER vertex vert_shader PASSTHROUGH
SHADER fragment frag_shader GLSL
# GLSL Shader
END

BUFFER texture FORMAT R8G8B8A8_UNORM
BUFFER framebuffer FORMAT R8G8B8A8_UNORM

PIPELINE graphics pipeline
  ATTACH vert_shader
  ATTACH frag_shader
  BIND BUFFER texture AS sampled_image DESCRIPTOR_SET 0 BINDING 0
  BIND BUFFER framebuffer AS color LOCATION 0
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
  EXPECT_EQ(BufferType::kSampledImage, bufs[0].type);
}

TEST_F(AmberScriptParserTest, BindBufferSampledImageMissingDescriptorSetValue) {
  std::string in = R"(
SHADER vertex vert_shader PASSTHROUGH
SHADER fragment frag_shader GLSL
# GLSL Shader
END

BUFFER texture FORMAT R8G8B8A8_UNORM

PIPELINE graphics pipeline
  ATTACH vert_shader
  ATTACH frag_shader
  BIND BUFFER texture AS sampled_image DESCRIPTOR_SET BINDING 0
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("12: invalid value for DESCRIPTOR_SET in BIND command", r.Error());
}

TEST_F(AmberScriptParserTest, BindBufferSampledImageMissingDescriptorSet) {
  std::string in = R"(
SHADER vertex vert_shader PASSTHROUGH
SHADER fragment frag_shader GLSL
# GLSL Shader
END

BUFFER texture FORMAT R8G8B8A8_UNORM

PIPELINE graphics pipeline
  ATTACH vert_shader
  ATTACH frag_shader
  BIND BUFFER texture AS sampled_image BINDING 0
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("12: missing DESCRIPTOR_SET or KERNEL for BIND command", r.Error());
}

TEST_F(AmberScriptParserTest, BindBufferSampledImageMissingBindingValue) {
  std::string in = R"(
SHADER vertex vert_shader PASSTHROUGH
SHADER fragment frag_shader GLSL
# GLSL Shader
END

BUFFER texture FORMAT R8G8B8A8_UNORM

PIPELINE graphics pipeline
  ATTACH vert_shader
  ATTACH frag_shader
  BIND BUFFER texture AS sampled_image DESCRIPTOR_SET 0 BINDING
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("13: invalid value for BINDING in BIND command", r.Error());
}

TEST_F(AmberScriptParserTest, BindBufferSampledImageMissingBinding) {
  std::string in = R"(
SHADER vertex vert_shader PASSTHROUGH
SHADER fragment frag_shader GLSL
# GLSL Shader
END

BUFFER texture FORMAT R8G8B8A8_UNORM

PIPELINE graphics pipeline
  ATTACH vert_shader
  ATTACH frag_shader
  BIND BUFFER texture AS sampled_image DESCRIPTOR_SET 0
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("13: missing BINDING for BIND command", r.Error());
}

TEST_F(AmberScriptParserTest, BindBufferCombinedImageSampler) {
  std::string in = R"(
SHADER vertex vert_shader PASSTHROUGH
SHADER fragment frag_shader GLSL
# GLSL Shader
END

BUFFER texture FORMAT R8G8B8A8_UNORM
BUFFER framebuffer FORMAT R8G8B8A8_UNORM
SAMPLER sampler

PIPELINE graphics pipeline
  ATTACH vert_shader
  ATTACH frag_shader
  BIND BUFFER texture AS combined_image_sampler SAMPLER sampler DESCRIPTOR_SET 0 BINDING 0
  BIND BUFFER framebuffer AS color LOCATION 0
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
  EXPECT_EQ(BufferType::kCombinedImageSampler, bufs[0].type);
}

TEST_F(AmberScriptParserTest,
       BindBufferCombinedImageSamplerMissingDescriptorSetValue) {
  std::string in = R"(
SHADER vertex vert_shader PASSTHROUGH
SHADER fragment frag_shader GLSL
# GLSL Shader
END

BUFFER texture FORMAT R8G8B8A8_UNORM
SAMPLER sampler

PIPELINE graphics pipeline
  ATTACH vert_shader
  ATTACH frag_shader
  BIND BUFFER texture AS combined_image_sampler SAMPLER sampler DESCRIPTOR_SET BINDING 0
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("13: invalid value for DESCRIPTOR_SET in BIND command", r.Error());
}

TEST_F(AmberScriptParserTest,
       BindBufferCombinedImageSamplerMissingDescriptorSet) {
  std::string in = R"(
SHADER vertex vert_shader PASSTHROUGH
SHADER fragment frag_shader GLSL
# GLSL Shader
END

BUFFER texture FORMAT R8G8B8A8_UNORM
SAMPLER sampler

PIPELINE graphics pipeline
  ATTACH vert_shader
  ATTACH frag_shader
  BIND BUFFER texture AS combined_image_sampler SAMPLER sampler BINDING 0
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("13: missing DESCRIPTOR_SET or KERNEL for BIND command", r.Error());
}

TEST_F(AmberScriptParserTest,
       BindBufferCombinedImageSamplerMissingBindingValue) {
  std::string in = R"(
SHADER vertex vert_shader PASSTHROUGH
SHADER fragment frag_shader GLSL
# GLSL Shader
END

BUFFER texture FORMAT R8G8B8A8_UNORM
SAMPLER sampler

PIPELINE graphics pipeline
  ATTACH vert_shader
  ATTACH frag_shader
  BIND BUFFER texture AS combined_image_sampler SAMPLER sampler DESCRIPTOR_SET 0 BINDING
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("14: invalid value for BINDING in BIND command", r.Error());
}

TEST_F(AmberScriptParserTest, BindBufferCombinedImageSamplerMissingBinding) {
  std::string in = R"(
SHADER vertex vert_shader PASSTHROUGH
SHADER fragment frag_shader GLSL
# GLSL Shader
END

BUFFER texture FORMAT R8G8B8A8_UNORM
SAMPLER sampler

PIPELINE graphics pipeline
  ATTACH vert_shader
  ATTACH frag_shader
  BIND BUFFER texture AS combined_image_sampler SAMPLER sampler DESCRIPTOR_SET 0
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("14: missing BINDING for BIND command", r.Error());
}

TEST_F(AmberScriptParserTest, BindBufferCombinedImageSamplerMissingSampler) {
  std::string in = R"(
SHADER vertex vert_shader PASSTHROUGH
SHADER fragment frag_shader GLSL
# GLSL Shader
END

BUFFER texture FORMAT R8G8B8A8_UNORM

PIPELINE graphics pipeline
  ATTACH vert_shader
  ATTACH frag_shader
  BIND BUFFER texture AS combined_image_sampler DESCRIPTOR_SET 0 BINDING 0
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("12: expecting SAMPLER for combined image sampler", r.Error());
}

TEST_F(AmberScriptParserTest, BindBufferCombinedImageSamplerUnknownSampler) {
  std::string in = R"(
SHADER vertex vert_shader PASSTHROUGH
SHADER fragment frag_shader GLSL
# GLSL Shader
END

BUFFER texture FORMAT R8G8B8A8_UNORM

PIPELINE graphics pipeline
  ATTACH vert_shader
  ATTACH frag_shader
  BIND BUFFER texture AS combined_image_sampler SAMPLER foo DESCRIPTOR_SET 0 BINDING 0
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("12: unknown sampler: foo", r.Error());
}

TEST_F(AmberScriptParserTest, BindBufferCombinedImageSamplerBaseMipLevel) {
  std::string in = R"(
SHADER vertex vert_shader PASSTHROUGH
SHADER fragment frag_shader GLSL
# GLSL Shader
END

BUFFER texture FORMAT R8G8B8A8_UNORM MIP_LEVELS 4
BUFFER framebuffer FORMAT R8G8B8A8_UNORM
SAMPLER sampler MAX_LOD 4.0

PIPELINE graphics pipeline
  ATTACH vert_shader
  ATTACH frag_shader
  BIND BUFFER texture AS combined_image_sampler SAMPLER sampler DESCRIPTOR_SET 0 BINDING 0 BASE_MIP_LEVEL 2
  BIND BUFFER framebuffer AS color LOCATION 0
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
  EXPECT_EQ(2U, bufs[0].base_mip_level);
}

TEST_F(AmberScriptParserTest,
       BindBufferCombinedImageSamplerMissingBaseMipLevel) {
  std::string in = R"(
SHADER vertex vert_shader PASSTHROUGH
SHADER fragment frag_shader GLSL
# GLSL Shader
END

BUFFER texture FORMAT R8G8B8A8_UNORM MIP_LEVELS 4
BUFFER framebuffer FORMAT R8G8B8A8_UNORM
SAMPLER sampler MAX_LOD 4.0

PIPELINE graphics pipeline
  ATTACH vert_shader
  ATTACH frag_shader
  BIND BUFFER texture AS combined_image_sampler SAMPLER sampler DESCRIPTOR_SET 0 BINDING 0 BASE_MIP_LEVEL
  BIND BUFFER framebuffer AS color LOCATION 0
END)";

  Parser parser;
  Result r = parser.Parse(in);

  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("15: invalid value for BASE_MIP_LEVEL", r.Error());
}

TEST_F(AmberScriptParserTest,
       BindBufferCombinedImageSamplerBaseMipLevelTooLarge) {
  std::string in = R"(
SHADER vertex vert_shader PASSTHROUGH
SHADER fragment frag_shader GLSL
# GLSL Shader
END

BUFFER texture FORMAT R8G8B8A8_UNORM MIP_LEVELS 2
BUFFER framebuffer FORMAT R8G8B8A8_UNORM
SAMPLER sampler MAX_LOD 2.0

PIPELINE graphics pipeline
  ATTACH vert_shader
  ATTACH frag_shader
  BIND BUFFER texture AS combined_image_sampler SAMPLER sampler DESCRIPTOR_SET 0 BINDING 0 BASE_MIP_LEVEL 3
  BIND BUFFER framebuffer AS color LOCATION 0
END)";

  Parser parser;
  Result r = parser.Parse(in);

  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ(
      "14: base mip level (now 3) needs to be larger than the number of buffer "
      "mip maps (2)",
      r.Error());
}

TEST_F(AmberScriptParserTest, BindBufferUniformTexelBufferCompute) {
  std::string in = R"(
SHADER compute compute_shader GLSL
# GLSL Shader
END

BUFFER texture FORMAT R8G8B8A8_UNORM

PIPELINE compute pipeline
  ATTACH compute_shader
  BIND BUFFER texture AS uniform_texel_buffer DESCRIPTOR_SET 0 BINDING 0
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
  EXPECT_EQ(BufferType::kUniformTexelBuffer, bufs[0].type);
}

TEST_F(AmberScriptParserTest, BindBufferUniformTexelBufferGraphics) {
  std::string in = R"(
SHADER vertex vert_shader PASSTHROUGH
SHADER fragment frag_shader GLSL
# GLSL Shader
END

BUFFER texture FORMAT R8G8B8A8_UNORM
BUFFER framebuffer FORMAT R8G8B8A8_UNORM

PIPELINE graphics pipeline
  ATTACH vert_shader
  ATTACH frag_shader
  BIND BUFFER texture AS uniform_texel_buffer DESCRIPTOR_SET 0 BINDING 0
  BIND BUFFER framebuffer AS color LOCATION 0
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
  EXPECT_EQ(BufferType::kUniformTexelBuffer, bufs[0].type);
}

TEST_F(AmberScriptParserTest,
       BindBufferUniformTexelBufferMissingDescriptorSetValue) {
  std::string in = R"(
SHADER compute compute_shader GLSL
# GLSL Shader
END

BUFFER texture FORMAT R8G8B8A8_UNORM

PIPELINE compute pipeline
  ATTACH compute_shader
  BIND BUFFER texture AS uniform_texel_buffer DESCRIPTOR_SET BINDING 0
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("10: invalid value for DESCRIPTOR_SET in BIND command", r.Error());
}

TEST_F(AmberScriptParserTest,
       BindBufferUniformTexelBufferMissingDescriptorSet) {
  std::string in = R"(
SHADER compute compute_shader GLSL
# GLSL Shader
END

BUFFER texture FORMAT R8G8B8A8_UNORM

PIPELINE compute pipeline
  ATTACH compute_shader
  BIND BUFFER texture AS uniform_texel_buffer BINDING 0
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("10: missing DESCRIPTOR_SET or KERNEL for BIND command", r.Error());
}

TEST_F(AmberScriptParserTest, BindBufferUniformTexelBufferMissingBindingValue) {
  std::string in = R"(
SHADER compute compute_shader GLSL
# GLSL Shader
END

BUFFER texture FORMAT R8G8B8A8_UNORM

PIPELINE compute pipeline
  ATTACH compute_shader
  BIND BUFFER texture AS uniform_texel_buffer DESCRIPTOR_SET 0 BINDING
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("11: invalid value for BINDING in BIND command", r.Error());
}

TEST_F(AmberScriptParserTest, BindBufferUniformTexelBufferMissingBinding) {
  std::string in = R"(
SHADER compute compute_shader GLSL
# GLSL Shader
END

BUFFER texture FORMAT R8G8B8A8_UNORM

PIPELINE compute pipeline
  ATTACH compute_shader
  BIND BUFFER texture AS uniform_texel_buffer DESCRIPTOR_SET 0
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("11: missing BINDING for BIND command", r.Error());
}

TEST_F(AmberScriptParserTest, BindBufferStorageTexelBufferCompute) {
  std::string in = R"(
SHADER compute compute_shader GLSL
# GLSL Shader
END

BUFFER texture FORMAT R8G8B8A8_UNORM

PIPELINE compute pipeline
  ATTACH compute_shader
  BIND BUFFER texture AS storage_texel_buffer DESCRIPTOR_SET 0 BINDING 0
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
  EXPECT_EQ(BufferType::kStorageTexelBuffer, bufs[0].type);
}

TEST_F(AmberScriptParserTest, BindBufferStorageTexelBufferGraphics) {
  std::string in = R"(
SHADER vertex vert_shader PASSTHROUGH
SHADER fragment frag_shader GLSL
# GLSL Shader
END

BUFFER texture FORMAT R8G8B8A8_UNORM
BUFFER framebuffer FORMAT R8G8B8A8_UNORM

PIPELINE graphics pipeline
  ATTACH vert_shader
  ATTACH frag_shader
  BIND BUFFER texture AS storage_texel_buffer DESCRIPTOR_SET 0 BINDING 0
  BIND BUFFER framebuffer AS color LOCATION 0
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
  EXPECT_EQ(BufferType::kStorageTexelBuffer, bufs[0].type);
}

TEST_F(AmberScriptParserTest,
       BindBufferStorageTexelBufferMissingDescriptorSetValue) {
  std::string in = R"(
SHADER compute compute_shader GLSL
# GLSL Shader
END

BUFFER texture FORMAT R8G8B8A8_UNORM

PIPELINE compute pipeline
  ATTACH compute_shader
  BIND BUFFER texture AS storage_texel_buffer DESCRIPTOR_SET BINDING 0
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("10: invalid value for DESCRIPTOR_SET in BIND command", r.Error());
}

TEST_F(AmberScriptParserTest,
       BindBufferStorageTexelBufferMissingDescriptorSet) {
  std::string in = R"(
SHADER compute compute_shader GLSL
# GLSL Shader
END

BUFFER texture FORMAT R8G8B8A8_UNORM

PIPELINE compute pipeline
  ATTACH compute_shader
  BIND BUFFER texture AS storage_texel_buffer BINDING 0
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("10: missing DESCRIPTOR_SET or KERNEL for BIND command", r.Error());
}

TEST_F(AmberScriptParserTest, BindBufferStorageTexelBufferMissingBindingValue) {
  std::string in = R"(
SHADER compute compute_shader GLSL
# GLSL Shader
END

BUFFER texture FORMAT R8G8B8A8_UNORM

PIPELINE compute pipeline
  ATTACH compute_shader
  BIND BUFFER texture AS storage_texel_buffer DESCRIPTOR_SET 0 BINDING
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("11: invalid value for BINDING in BIND command", r.Error());
}

TEST_F(AmberScriptParserTest, BindBufferStorageTexelBufferMissingBinding) {
  std::string in = R"(
SHADER compute compute_shader GLSL
# GLSL Shader
END

BUFFER texture FORMAT R8G8B8A8_UNORM

PIPELINE compute pipeline
  ATTACH compute_shader
  BIND BUFFER texture AS storage_texel_buffer DESCRIPTOR_SET 0
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("11: missing BINDING for BIND command", r.Error());
}

TEST_F(AmberScriptParserTest, BindSampler) {
  std::string in = R"(
SHADER vertex vert_shader PASSTHROUGH
SHADER fragment frag_shader GLSL
# GLSL Shader
END

SAMPLER sampler
BUFFER framebuffer FORMAT R8G8B8A8_UNORM

PIPELINE graphics pipeline
  ATTACH vert_shader
  ATTACH frag_shader
  BIND SAMPLER sampler DESCRIPTOR_SET 0 BINDING 0
  BIND BUFFER framebuffer AS color LOCATION 0
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& pipelines = script->GetPipelines();
  ASSERT_EQ(1U, pipelines.size());

  const auto* pipeline = pipelines[0].get();
  const auto& samplers = pipeline->GetSamplers();
  ASSERT_EQ(1U, samplers.size());
}

TEST_F(AmberScriptParserTest, BindSamplerMissingDescriptorSetValue) {
  std::string in = R"(
SHADER vertex vert_shader PASSTHROUGH
SHADER fragment frag_shader GLSL
# GLSL Shader
END

SAMPLER sampler

PIPELINE graphics pipeline
  ATTACH vert_shader
  ATTACH frag_shader
  BIND SAMPLER sampler DESCRIPTOR_SET BINDING 0
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("12: invalid value for DESCRIPTOR_SET in BIND command", r.Error());
}

TEST_F(AmberScriptParserTest, BindSamplerMissingDescriptorSet) {
  std::string in = R"(
SHADER vertex vert_shader PASSTHROUGH
SHADER fragment frag_shader GLSL
# GLSL Shader
END

SAMPLER sampler

PIPELINE graphics pipeline
  ATTACH vert_shader
  ATTACH frag_shader
  BIND SAMPLER sampler BINDING 0
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("12: missing DESCRIPTOR_SET or KERNEL for BIND command", r.Error());
}

TEST_F(AmberScriptParserTest, BindSamplerMissingBindingValue) {
  std::string in = R"(
SHADER vertex vert_shader PASSTHROUGH
SHADER fragment frag_shader GLSL
# GLSL Shader
END

SAMPLER sampler

PIPELINE graphics pipeline
  ATTACH vert_shader
  ATTACH frag_shader
  BIND SAMPLER sampler DESCRIPTOR_SET 0 BINDING
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("13: invalid value for BINDING in BIND command", r.Error());
}

TEST_F(AmberScriptParserTest, BindSamplerMissingBinding) {
  std::string in = R"(
SHADER vertex vert_shader PASSTHROUGH
SHADER fragment frag_shader GLSL
# GLSL Shader
END

SAMPLER sampler

PIPELINE graphics pipeline
  ATTACH vert_shader
  ATTACH frag_shader
  BIND SAMPLER sampler DESCRIPTOR_SET 0
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("13: missing BINDING for BIND command", r.Error());
}

TEST_F(AmberScriptParserTest, BindDescriptorOffsetWithUnsupportedBufferType) {
  std::string unsupported_buffer_types[] = {"uniform_texel_buffer",
                                            "storage_texel_buffer",
                                            "sampled_image", "storage_image"};

  for (const auto& buffer_type : unsupported_buffer_types) {
    std::ostringstream in;
    in << R"(
SHADER compute compute_shader GLSL
# GLSL Shader
END

BUFFER texture FORMAT R8G8B8A8_UNORM

PIPELINE compute pipeline
  ATTACH compute_shader
  BIND BUFFER texture AS )"
       << buffer_type << R"( DESCRIPTOR_SET 0 BINDING 0 DESCRIPTOR_OFFSET 0
END)";
    Parser parser;
    Result r = parser.Parse(in.str());
    ASSERT_FALSE(r.IsSuccess());
    EXPECT_EQ("10: extra parameters after BIND command: DESCRIPTOR_OFFSET",
              r.Error());
  }
}

TEST_F(AmberScriptParserTest, BindBufferArray) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_buf1 FORMAT R32G32B32A32_SFLOAT
BUFFER my_buf2 FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER_ARRAY my_buf1 my_buf2 AS uniform DESCRIPTOR_SET 1 BINDING 2
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
  ASSERT_EQ(2U, bufs.size());
  for (size_t i = 0; i < 2; i++) {
    EXPECT_EQ(BufferType::kUniform, bufs[i].type);
    EXPECT_EQ(1U, bufs[i].descriptor_set);
    EXPECT_EQ(2U, bufs[i].binding);
    EXPECT_EQ(0U, bufs[i].descriptor_offset);
    EXPECT_EQ(~0ULL, bufs[i].descriptor_range);
    EXPECT_EQ(static_cast<uint32_t>(0), bufs[i].location);
    EXPECT_EQ(FormatType::kR32G32B32A32_SFLOAT,
              bufs[i].buffer->GetFormat()->GetFormatType());
  }
}

TEST_F(AmberScriptParserTest, BindBufferArrayWithDescriptorOffsetAndRange) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_buf1 FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER_ARRAY my_buf1 my_buf1 AS uniform DESCRIPTOR_SET 1 BINDING 2 DESCRIPTOR_OFFSET 256 512 DESCRIPTOR_RANGE 1024 2048
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
  ASSERT_EQ(2U, bufs.size());
  for (size_t i = 0; i < 2; i++) {
    EXPECT_EQ(BufferType::kUniform, bufs[i].type);
    EXPECT_EQ(1U, bufs[i].descriptor_set);
    EXPECT_EQ(2U, bufs[i].binding);
    EXPECT_EQ(256U * (i + 1), bufs[i].descriptor_offset);
    EXPECT_EQ(1024U * (i + 1), bufs[i].descriptor_range);
    EXPECT_EQ(static_cast<uint32_t>(0), bufs[i].location);
    EXPECT_EQ(FormatType::kR32G32B32A32_SFLOAT,
              bufs[i].buffer->GetFormat()->GetFormatType());
  }
}

TEST_F(AmberScriptParserTest,
       BindDynamicBufferArrayWithDescriptorOffsetAndRange) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_buf1 FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER_ARRAY my_buf1 my_buf1 AS uniform_dynamic DESCRIPTOR_SET 1 BINDING 2 OFFSET 16 32 DESCRIPTOR_OFFSET 256 512 DESCRIPTOR_RANGE 1024 2048
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
  ASSERT_EQ(2U, bufs.size());
  for (size_t i = 0; i < 2; i++) {
    EXPECT_EQ(BufferType::kUniformDynamic, bufs[i].type);
    EXPECT_EQ(1U, bufs[i].descriptor_set);
    EXPECT_EQ(2U, bufs[i].binding);
    EXPECT_EQ(16U * (i + 1), bufs[i].dynamic_offset);
    EXPECT_EQ(256U * (i + 1), bufs[i].descriptor_offset);
    EXPECT_EQ(1024U * (i + 1), bufs[i].descriptor_range);
    EXPECT_EQ(static_cast<uint32_t>(0), bufs[i].location);
    EXPECT_EQ(FormatType::kR32G32B32A32_SFLOAT,
              bufs[i].buffer->GetFormat()->GetFormatType());
  }
}

TEST_F(AmberScriptParserTest, BindBufferArrayOnlyOneDescriptorOffset) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_buf FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER_ARRAY my_buf my_buf AS uniform DESCRIPTOR_SET 1 BINDING 2 DESCRIPTOR_OFFSET 256
END
)";

  Parser parser;
  Result r = parser.Parse(in);

  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ(
      "13: expecting a DESCRIPTOR_OFFSET value for each buffer in the array",
      r.Error());
}

TEST_F(AmberScriptParserTest, BindBufferArrayOnlyOneDescriptorRange) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_buf FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER_ARRAY my_buf my_buf AS uniform DESCRIPTOR_SET 1 BINDING 2 DESCRIPTOR_RANGE 256
END
)";

  Parser parser;
  Result r = parser.Parse(in);

  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ(
      "13: expecting a DESCRIPTOR_RANGE value for each buffer in the array",
      r.Error());
}

TEST_F(AmberScriptParserTest, BindUniformBufferEmptyDescriptorOffset) {
  std::string in = R"(
BUFFER my_buf FORMAT R32G32B32A32_SFLOAT
PIPELINE graphics my_pipeline
  BIND BUFFER my_buf AS uniform DESCRIPTOR_SET 1 BINDING 2 DESCRIPTOR_OFFSET
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("5: expecting an integer value for DESCRIPTOR_OFFSET", r.Error());
}

TEST_F(AmberScriptParserTest, BindUniformBufferInvalidDescriptorOffset) {
  std::string in = R"(
BUFFER my_buf FORMAT R32G32B32A32_SFLOAT
PIPELINE graphics my_pipeline
  BIND BUFFER my_buf AS uniform DESCRIPTOR_SET 1 BINDING 2 DESCRIPTOR_OFFSET foo
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("4: expecting an integer value for DESCRIPTOR_OFFSET", r.Error());
}

TEST_F(AmberScriptParserTest, BindUniformBufferEmptyDescriptorRange) {
  std::string in = R"(
BUFFER my_buf FORMAT R32G32B32A32_SFLOAT
PIPELINE graphics my_pipeline
  BIND BUFFER my_buf AS uniform DESCRIPTOR_SET 1 BINDING 2 DESCRIPTOR_RANGE
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("5: expecting an integer value for DESCRIPTOR_RANGE", r.Error());
}

TEST_F(AmberScriptParserTest, BindUniformBufferInvalidDescriptorRange) {
  std::string in = R"(
BUFFER my_buf FORMAT R32G32B32A32_SFLOAT
PIPELINE graphics my_pipeline
  BIND BUFFER my_buf AS uniform DESCRIPTOR_SET 1 BINDING 2 DESCRIPTOR_RANGE foo
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("4: expecting an integer value for DESCRIPTOR_RANGE", r.Error());
}

TEST_F(AmberScriptParserTest, BindBufferArrayOnlyOneBuffer) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_buf FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER_ARRAY my_buf AS uniform DESCRIPTOR_SET 1 BINDING 2
END
)";

  Parser parser;
  Result r = parser.Parse(in);

  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("12: expecting multiple buffer names for BUFFER_ARRAY", r.Error());
}

TEST_F(AmberScriptParserTest, BindSamplerArray) {
  std::string in = R"(
SHADER vertex vert_shader PASSTHROUGH
SHADER fragment frag_shader GLSL
# GLSL Shader
END

SAMPLER sampler1
SAMPLER sampler2
BUFFER framebuffer FORMAT R8G8B8A8_UNORM

PIPELINE graphics pipeline
  ATTACH vert_shader
  ATTACH frag_shader
  BIND SAMPLER_ARRAY sampler1 sampler2 DESCRIPTOR_SET 0 BINDING 0
  BIND BUFFER framebuffer AS color LOCATION 0
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& pipelines = script->GetPipelines();
  ASSERT_EQ(1U, pipelines.size());

  const auto* pipeline = pipelines[0].get();
  const auto& samplers = pipeline->GetSamplers();
  ASSERT_EQ(2U, samplers.size());
}

TEST_F(AmberScriptParserTest, BindSamplerArrayOnlyOneSampler) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
SAMPLER sampler

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND SAMPLER_ARRAY sampler DESCRIPTOR_SET 1 BINDING 2
END
)";

  Parser parser;
  Result r = parser.Parse(in);

  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("12: expecting multiple sampler names for SAMPLER_ARRAY",
            r.Error());
}

TEST_F(AmberScriptParserTest, BindUniformBufferDynamic) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_buf FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_buf AS uniform_dynamic DESCRIPTOR_SET 1 BINDING 2 OFFSET 8
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
  EXPECT_EQ(BufferType::kUniformDynamic, bufs[0].type);
  EXPECT_EQ(1U, bufs[0].descriptor_set);
  EXPECT_EQ(2U, bufs[0].binding);
  EXPECT_EQ(8u, bufs[0].dynamic_offset);
  EXPECT_EQ(static_cast<uint32_t>(0), bufs[0].location);
  EXPECT_EQ(FormatType::kR32G32B32A32_SFLOAT,
            bufs[0].buffer->GetFormat()->GetFormatType());
}

TEST_F(AmberScriptParserTest, BindUniformBufferArrayDynamic) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER buf0 FORMAT R32G32B32A32_SFLOAT
BUFFER buf1 FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER_ARRAY buf0 buf1 AS uniform_dynamic DESCRIPTOR_SET 1 BINDING 2 OFFSET 8 16
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
  ASSERT_EQ(2U, bufs.size());
  EXPECT_EQ(BufferType::kUniformDynamic, bufs[0].type);
  EXPECT_EQ(1U, bufs[0].descriptor_set);
  EXPECT_EQ(2U, bufs[0].binding);
  EXPECT_EQ(8u, bufs[0].dynamic_offset);
  EXPECT_EQ(static_cast<uint32_t>(0), bufs[0].location);
  EXPECT_EQ(FormatType::kR32G32B32A32_SFLOAT,
            bufs[0].buffer->GetFormat()->GetFormatType());
  EXPECT_EQ(1U, bufs[1].descriptor_set);
  EXPECT_EQ(2U, bufs[1].binding);
  EXPECT_EQ(16u, bufs[1].dynamic_offset);
  EXPECT_EQ(static_cast<uint32_t>(0), bufs[1].location);
  EXPECT_EQ(FormatType::kR32G32B32A32_SFLOAT,
            bufs[1].buffer->GetFormat()->GetFormatType());
}

TEST_F(AmberScriptParserTest, BindUniformBufferDynamicMissingOffset) {
  std::string in = R"(
BUFFER my_buf FORMAT R32G32B32A32_SFLOAT
PIPELINE graphics my_pipeline
  BIND BUFFER my_buf AS uniform_dynamic DESCRIPTOR_SET 1 BINDING 2
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("5: expecting an OFFSET for dynamic buffer type", r.Error());
}

TEST_F(AmberScriptParserTest, BindUniformBufferDynamicEmptyOffset) {
  std::string in = R"(
BUFFER my_buf FORMAT R32G32B32A32_SFLOAT
PIPELINE graphics my_pipeline
  BIND BUFFER my_buf AS uniform_dynamic DESCRIPTOR_SET 1 BINDING 2 OFFSET
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("5: expecting an integer value for OFFSET", r.Error());
}

TEST_F(AmberScriptParserTest, BindUniformBufferDynamicInvalidOffset) {
  std::string in = R"(
BUFFER my_buf FORMAT R32G32B32A32_SFLOAT
PIPELINE graphics my_pipeline
  BIND BUFFER my_buf AS uniform_dynamic DESCRIPTOR_SET 1 BINDING 2 OFFSET foo
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("4: expecting an integer value for OFFSET", r.Error());
}

TEST_F(AmberScriptParserTest, BindUniformBufferArrayDynamicNotEnoughOffsets) {
  std::string in = R"(
BUFFER buf0 FORMAT R32G32B32A32_SFLOAT
BUFFER buf1 FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  BIND BUFFER_ARRAY buf0 buf1 AS uniform_dynamic DESCRIPTOR_SET 1 BINDING 2 OFFSET 8
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("7: expecting an OFFSET value for each buffer in the array",
            r.Error());
}

TEST_F(AmberScriptParserTest, BindStorageBufferDynamic) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_buf FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_buf AS storage_dynamic DESCRIPTOR_SET 1 BINDING 2 OFFSET 8
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
  EXPECT_EQ(BufferType::kStorageDynamic, bufs[0].type);
  EXPECT_EQ(1U, bufs[0].descriptor_set);
  EXPECT_EQ(2U, bufs[0].binding);
  EXPECT_EQ(8u, bufs[0].dynamic_offset);
  EXPECT_EQ(static_cast<uint32_t>(0), bufs[0].location);
  EXPECT_EQ(FormatType::kR32G32B32A32_SFLOAT,
            bufs[0].buffer->GetFormat()->GetFormatType());
}

TEST_F(AmberScriptParserTest, BindStorageBufferArrayDynamic) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER buf0 FORMAT R32G32B32A32_SFLOAT
BUFFER buf1 FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER_ARRAY buf0 buf1 AS storage_dynamic DESCRIPTOR_SET 1 BINDING 2 OFFSET 8 16
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
  ASSERT_EQ(2U, bufs.size());
  EXPECT_EQ(BufferType::kStorageDynamic, bufs[0].type);
  EXPECT_EQ(1U, bufs[0].descriptor_set);
  EXPECT_EQ(2U, bufs[0].binding);
  EXPECT_EQ(8u, bufs[0].dynamic_offset);
  EXPECT_EQ(static_cast<uint32_t>(0), bufs[0].location);
  EXPECT_EQ(FormatType::kR32G32B32A32_SFLOAT,
            bufs[0].buffer->GetFormat()->GetFormatType());
  EXPECT_EQ(1U, bufs[1].descriptor_set);
  EXPECT_EQ(2U, bufs[1].binding);
  EXPECT_EQ(16u, bufs[1].dynamic_offset);
  EXPECT_EQ(static_cast<uint32_t>(0), bufs[1].location);
  EXPECT_EQ(FormatType::kR32G32B32A32_SFLOAT,
            bufs[1].buffer->GetFormat()->GetFormatType());
}

TEST_F(AmberScriptParserTest, BindStorageBufferDynamicMissingOffset) {
  std::string in = R"(
BUFFER my_buf FORMAT R32G32B32A32_SFLOAT
PIPELINE graphics my_pipeline
  BIND BUFFER my_buf AS storage_dynamic DESCRIPTOR_SET 1 BINDING 2
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("5: expecting an OFFSET for dynamic buffer type", r.Error());
}

TEST_F(AmberScriptParserTest, BindStorageBufferDynamicEmptyOffset) {
  std::string in = R"(
BUFFER my_buf FORMAT R32G32B32A32_SFLOAT
PIPELINE graphics my_pipeline
  BIND BUFFER my_buf AS storage_dynamic DESCRIPTOR_SET 1 BINDING 2 OFFSET
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("5: expecting an integer value for OFFSET", r.Error());
}

TEST_F(AmberScriptParserTest, BindStorageBufferDynamicInvalidOffset) {
  std::string in = R"(
BUFFER my_buf FORMAT R32G32B32A32_SFLOAT
PIPELINE graphics my_pipeline
  BIND BUFFER my_buf AS storage_dynamic DESCRIPTOR_SET 1 BINDING 2 OFFSET foo
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("4: expecting an integer value for OFFSET", r.Error());
}

TEST_F(AmberScriptParserTest, BindStorageBufferArrayDynamicNotEnoughOffsets) {
  std::string in = R"(
BUFFER buf0 FORMAT R32G32B32A32_SFLOAT
BUFFER buf1 FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  BIND BUFFER_ARRAY buf0 buf1 AS storage_dynamic DESCRIPTOR_SET 1 BINDING 2 OFFSET 8
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("7: expecting an OFFSET value for each buffer in the array",
            r.Error());
}

TEST_F(AmberScriptParserTest, BindResolveTarget) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
IMAGE my_fb_ms DIM_2D WIDTH 64 HEIGHT 64 FORMAT R32G32B32A32_SFLOAT SAMPLES 4
IMAGE my_fb DIM_2D WIDTH 64 HEIGHT 64 FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  FRAMEBUFFER_SIZE 64 64
  BIND BUFFER my_fb_ms AS color LOCATION 0
  BIND BUFFER my_fb AS resolve
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& pipelines = script->GetPipelines();
  ASSERT_EQ(1U, pipelines.size());

  const auto* pipeline = pipelines[0].get();
  const auto& resolve_targets = pipeline->GetResolveTargets();
  ASSERT_EQ(1U, resolve_targets.size());

  const auto& buf_info = resolve_targets[0];
  ASSERT_TRUE(buf_info.buffer != nullptr);
  EXPECT_EQ(64u * 64u, buf_info.buffer->ElementCount());
  EXPECT_EQ(64u * 64u * 4u, buf_info.buffer->ValueCount());
  EXPECT_EQ(64u * 64u * 4u * sizeof(float), buf_info.buffer->GetSizeInBytes());
}

TEST_F(AmberScriptParserTest, BindResolveTargetMissingBuffer) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER AS resolve
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("12: unknown buffer: AS", r.Error());
}

TEST_F(AmberScriptParserTest, BindResolveTargetNonDeclaredBuffer) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
IMAGE my_fb_ms DIM_2D WIDTH 64 HEIGHT 64 FORMAT R32G32B32A32_SFLOAT SAMPLES 4

PIPELINE graphics my_pipeline
ATTACH my_shader
ATTACH my_fragment

FRAMEBUFFER_SIZE 64 64
BIND BUFFER my_fb_ms AS color LOCATION 0
BIND BUFFER my_fb AS resolve
END)";
  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("14: unknown buffer: my_fb", r.Error());
}

TEST_F(AmberScriptParserTest, BindMultipleResolveTargets) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
IMAGE my_fb_ms0 DIM_2D WIDTH 64 HEIGHT 64 FORMAT R32G32B32A32_SFLOAT SAMPLES 4
IMAGE my_fb_ms1 DIM_2D WIDTH 64 HEIGHT 64 FORMAT R32G32B32A32_SFLOAT SAMPLES 4
IMAGE my_fb_ms2 DIM_2D WIDTH 64 HEIGHT 64 FORMAT R32G32B32A32_SFLOAT SAMPLES 4
IMAGE my_fb0 DIM_2D WIDTH 64 HEIGHT 64 FORMAT R32G32B32A32_SFLOAT
IMAGE my_fb1 DIM_2D WIDTH 64 HEIGHT 64 FORMAT R32G32B32A32_SFLOAT
IMAGE my_fb2 DIM_2D WIDTH 64 HEIGHT 64 FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  FRAMEBUFFER_SIZE 64 64
  BIND BUFFER my_fb_ms0 AS color LOCATION 0
  BIND BUFFER my_fb_ms1 AS color LOCATION 1
  BIND BUFFER my_fb_ms2 AS color LOCATION 2
  BIND BUFFER my_fb0 AS resolve
  BIND BUFFER my_fb1 AS resolve
  BIND BUFFER my_fb2 AS resolve
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& pipelines = script->GetPipelines();
  ASSERT_EQ(1U, pipelines.size());

  const auto* pipeline = pipelines[0].get();
  const auto& resolve_targets = pipeline->GetResolveTargets();
  ASSERT_EQ(3U, resolve_targets.size());

  for (const auto& buf_info : resolve_targets) {
    ASSERT_TRUE(buf_info.buffer != nullptr);
    EXPECT_EQ(64u * 64u, buf_info.buffer->ElementCount());
    EXPECT_EQ(64u * 64u * 4u, buf_info.buffer->ValueCount());
    EXPECT_EQ(64u * 64u * 4u * sizeof(float),
              buf_info.buffer->GetSizeInBytes());
  }
}

}  // namespace amberscript
}  // namespace amber
