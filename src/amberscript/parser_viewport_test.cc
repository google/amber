// Copyright 2021 The Amber Authors.
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

TEST_F(AmberScriptParserTest, NoViewport) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT
BUFFER my_ds FORMAT D32_SFLOAT_S8_UINT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
  BIND BUFFER my_fb AS color LOCATION 0
  BIND BUFFER my_ds AS depth_stencil
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& pipelines = script->GetPipelines();
  ASSERT_EQ(1U, pipelines.size());

  auto* pipeline = pipelines[0].get();
  ASSERT_FALSE(pipeline->GetPipelineData()->HasViewportData());
}

TEST_F(AmberScriptParserTest, ViewportNoDepth) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT
BUFFER my_ds FORMAT D32_SFLOAT_S8_UINT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
  BIND BUFFER my_fb AS color LOCATION 0
  BIND BUFFER my_ds AS depth_stencil

  VIEWPORT 5.0 7.0 SIZE 10.0 12.0
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& pipelines = script->GetPipelines();
  ASSERT_EQ(1U, pipelines.size());

  auto* pipeline = pipelines[0].get();
  ASSERT_TRUE(pipeline->GetPipelineData()->HasViewportData());
  ASSERT_FLOAT_EQ(5.0f, pipeline->GetPipelineData()->GetViewport().x);
  ASSERT_FLOAT_EQ(7.0f, pipeline->GetPipelineData()->GetViewport().y);
  ASSERT_FLOAT_EQ(10.0f, pipeline->GetPipelineData()->GetViewport().w);
  ASSERT_FLOAT_EQ(12.0f, pipeline->GetPipelineData()->GetViewport().h);
  ASSERT_FLOAT_EQ(0.0f, pipeline->GetPipelineData()->GetViewport().mind);
  ASSERT_FLOAT_EQ(1.0f, pipeline->GetPipelineData()->GetViewport().maxd);
}

TEST_F(AmberScriptParserTest, ViewportMinDepth) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT
BUFFER my_ds FORMAT D32_SFLOAT_S8_UINT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
  BIND BUFFER my_fb AS color LOCATION 0
  BIND BUFFER my_ds AS depth_stencil

  VIEWPORT 12.2 9.7 SIZE 0.5 106.1 MIN_DEPTH 0.3
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& pipelines = script->GetPipelines();
  ASSERT_EQ(1U, pipelines.size());

  auto* pipeline = pipelines[0].get();
  ASSERT_TRUE(pipeline->GetPipelineData()->HasViewportData());
  ASSERT_FLOAT_EQ(12.2f, pipeline->GetPipelineData()->GetViewport().x);
  ASSERT_FLOAT_EQ(9.7f, pipeline->GetPipelineData()->GetViewport().y);
  ASSERT_FLOAT_EQ(0.5f, pipeline->GetPipelineData()->GetViewport().w);
  ASSERT_FLOAT_EQ(106.1f, pipeline->GetPipelineData()->GetViewport().h);
  ASSERT_FLOAT_EQ(0.3f, pipeline->GetPipelineData()->GetViewport().mind);
  ASSERT_FLOAT_EQ(1.0f, pipeline->GetPipelineData()->GetViewport().maxd);
}

TEST_F(AmberScriptParserTest, ViewportMaxDepth) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT
BUFFER my_ds FORMAT D32_SFLOAT_S8_UINT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
  BIND BUFFER my_fb AS color LOCATION 0
  BIND BUFFER my_ds AS depth_stencil

  VIEWPORT 12.2 9.7 SIZE 0.5 106.1 MAX_DEPTH 0.456
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& pipelines = script->GetPipelines();
  ASSERT_EQ(1U, pipelines.size());

  auto* pipeline = pipelines[0].get();
  ASSERT_TRUE(pipeline->GetPipelineData()->HasViewportData());
  ASSERT_FLOAT_EQ(12.2f, pipeline->GetPipelineData()->GetViewport().x);
  ASSERT_FLOAT_EQ(9.7f, pipeline->GetPipelineData()->GetViewport().y);
  ASSERT_FLOAT_EQ(0.5f, pipeline->GetPipelineData()->GetViewport().w);
  ASSERT_FLOAT_EQ(106.1f, pipeline->GetPipelineData()->GetViewport().h);
  ASSERT_FLOAT_EQ(0.0f, pipeline->GetPipelineData()->GetViewport().mind);
  ASSERT_FLOAT_EQ(0.456f, pipeline->GetPipelineData()->GetViewport().maxd);
}

TEST_F(AmberScriptParserTest, ViewportAllValues) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT
BUFFER my_ds FORMAT D32_SFLOAT_S8_UINT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
  BIND BUFFER my_fb AS color LOCATION 0
  BIND BUFFER my_ds AS depth_stencil

  VIEWPORT -0.6 5.2 SIZE 13.8 9.4 MIN_DEPTH 0.5 MAX_DEPTH 0.6
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& pipelines = script->GetPipelines();
  ASSERT_EQ(1U, pipelines.size());

  auto* pipeline = pipelines[0].get();
  ASSERT_TRUE(pipeline->GetPipelineData()->HasViewportData());
  ASSERT_FLOAT_EQ(-0.6f, pipeline->GetPipelineData()->GetViewport().x);
  ASSERT_FLOAT_EQ(5.2f, pipeline->GetPipelineData()->GetViewport().y);
  ASSERT_FLOAT_EQ(13.8f, pipeline->GetPipelineData()->GetViewport().w);
  ASSERT_FLOAT_EQ(9.4f, pipeline->GetPipelineData()->GetViewport().h);
  ASSERT_FLOAT_EQ(0.5f, pipeline->GetPipelineData()->GetViewport().mind);
  ASSERT_FLOAT_EQ(0.6f, pipeline->GetPipelineData()->GetViewport().maxd);
}

TEST_F(AmberScriptParserTest, ViewportIntegers) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT
BUFFER my_ds FORMAT D32_SFLOAT_S8_UINT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
  BIND BUFFER my_fb AS color LOCATION 0
  BIND BUFFER my_ds AS depth_stencil

  VIEWPORT -2 7 SIZE 15 20 MIN_DEPTH 1 MAX_DEPTH 2
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& pipelines = script->GetPipelines();
  ASSERT_EQ(1U, pipelines.size());

  auto* pipeline = pipelines[0].get();
  ASSERT_TRUE(pipeline->GetPipelineData()->HasViewportData());
  ASSERT_FLOAT_EQ(-2.0f, pipeline->GetPipelineData()->GetViewport().x);
  ASSERT_FLOAT_EQ(7.0f, pipeline->GetPipelineData()->GetViewport().y);
  ASSERT_FLOAT_EQ(15.0f, pipeline->GetPipelineData()->GetViewport().w);
  ASSERT_FLOAT_EQ(20.0f, pipeline->GetPipelineData()->GetViewport().h);
  ASSERT_FLOAT_EQ(1.0f, pipeline->GetPipelineData()->GetViewport().mind);
  ASSERT_FLOAT_EQ(2.0f, pipeline->GetPipelineData()->GetViewport().maxd);
}

TEST_F(AmberScriptParserTest, ViewportMixedIntegers) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT
BUFFER my_ds FORMAT D32_SFLOAT_S8_UINT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
  BIND BUFFER my_fb AS color LOCATION 0
  BIND BUFFER my_ds AS depth_stencil

  VIEWPORT -2 13.1 SIZE 15.9 20
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& pipelines = script->GetPipelines();
  ASSERT_EQ(1U, pipelines.size());

  auto* pipeline = pipelines[0].get();
  ASSERT_TRUE(pipeline->GetPipelineData()->HasViewportData());
  ASSERT_FLOAT_EQ(-2.0f, pipeline->GetPipelineData()->GetViewport().x);
  ASSERT_FLOAT_EQ(13.1f, pipeline->GetPipelineData()->GetViewport().y);
  ASSERT_FLOAT_EQ(15.9f, pipeline->GetPipelineData()->GetViewport().w);
  ASSERT_FLOAT_EQ(20.0f, pipeline->GetPipelineData()->GetViewport().h);
  ASSERT_FLOAT_EQ(0.0f, pipeline->GetPipelineData()->GetViewport().mind);
  ASSERT_FLOAT_EQ(1.0f, pipeline->GetPipelineData()->GetViewport().maxd);
}

TEST_F(AmberScriptParserTest, ViewportInvalidMissingSize) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT
BUFFER my_ds FORMAT D32_SFLOAT_S8_UINT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
  BIND BUFFER my_fb AS color LOCATION 0
  BIND BUFFER my_ds AS depth_stencil

  VIEWPORT 0.0 2.0 12.0 24.0
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("15: missing SIZE for VIEWPORT command", r.Error());
}

TEST_F(AmberScriptParserTest, ViewportInvalidSizeNotOptional) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT
BUFFER my_ds FORMAT D32_SFLOAT_S8_UINT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
  BIND BUFFER my_fb AS color LOCATION 0
  BIND BUFFER my_ds AS depth_stencil

  VIEWPORT 0.0 2.0
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("16: missing SIZE for VIEWPORT command", r.Error());
}

TEST_F(AmberScriptParserTest, ViewportInvalidMissingOffset) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT
BUFFER my_ds FORMAT D32_SFLOAT_S8_UINT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
  BIND BUFFER my_fb AS color LOCATION 0
  BIND BUFFER my_ds AS depth_stencil

  VIEWPORT 0.0 SIZE 12.0 24.0
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("15: invalid offset for VIEWPORT command", r.Error());
}

TEST_F(AmberScriptParserTest, ViewportInvalidMissingSizeValue) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT
BUFFER my_ds FORMAT D32_SFLOAT_S8_UINT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
  BIND BUFFER my_fb AS color LOCATION 0
  BIND BUFFER my_ds AS depth_stencil

  VIEWPORT 0.0 2.0 SIZE 12.0
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("16: missing size for VIEWPORT command", r.Error());
}

TEST_F(AmberScriptParserTest, ViewportInvalidMissingDepthValue) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT
BUFFER my_ds FORMAT D32_SFLOAT_S8_UINT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
  BIND BUFFER my_fb AS color LOCATION 0
  BIND BUFFER my_ds AS depth_stencil

  VIEWPORT 0.0 2.0 SIZE 12.0 24.0 MIN_DEPTH MAX_DEPTH 1.0
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("15: invalid min_depth for VIEWPORT command", r.Error());
}

}  // namespace amberscript
}  // namespace amber
