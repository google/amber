// Copyright 2020 The Amber Authors.
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

TEST_F(AmberScriptParserTest, DepthAllValues) {
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

  DEPTH
    TEST on
    WRITE on
    COMPARE_OP less_or_equal
    CLAMP on
    BOUNDS min 1.5 max 6.7
    BIAS constant 2.1 clamp 3.5 slope 5.5
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& pipelines = script->GetPipelines();
  ASSERT_EQ(1U, pipelines.size());

  auto* pipeline = pipelines[0].get();
  ASSERT_NE(nullptr, pipeline->GetDepthStencilBuffer().buffer);

  ASSERT_TRUE(pipeline->GetPipelineData()->GetEnableDepthTest());
  ASSERT_TRUE(pipeline->GetPipelineData()->GetEnableDepthWrite());
  ASSERT_TRUE(pipeline->GetPipelineData()->GetEnableDepthClamp());
  ASSERT_FLOAT_EQ(1.5f, pipeline->GetPipelineData()->GetMinDepthBounds());
  ASSERT_FLOAT_EQ(6.7f, pipeline->GetPipelineData()->GetMaxDepthBounds());
  ASSERT_FLOAT_EQ(2.1f,
                  pipeline->GetPipelineData()->GetDepthBiasConstantFactor());
  ASSERT_FLOAT_EQ(3.5f, pipeline->GetPipelineData()->GetDepthBiasClamp());
  ASSERT_FLOAT_EQ(5.5f, pipeline->GetPipelineData()->GetDepthBiasSlopeFactor());
}

TEST_F(AmberScriptParserTest, DepthTestMissingValue) {
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

  DEPTH
    TEST
    WRITE on
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("17: invalid value for TEST", r.Error());
}

TEST_F(AmberScriptParserTest, DepthTestInvalidValue) {
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

  DEPTH
    TEST foo
    WRITE on
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("16: invalid value for TEST: foo", r.Error());
}

TEST_F(AmberScriptParserTest, DepthWriteMissingValue) {
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

  DEPTH
    TEST on
    WRITE
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("18: invalid value for WRITE", r.Error());
}

TEST_F(AmberScriptParserTest, DepthWriteInvalidValue) {
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

  DEPTH
    TEST on
    WRITE foo
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("17: invalid value for WRITE: foo", r.Error());
}

TEST_F(AmberScriptParserTest, DepthClampMissingValue) {
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

  DEPTH
    TEST on
    CLAMP
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("18: invalid value for CLAMP", r.Error());
}

TEST_F(AmberScriptParserTest, DepthClampInvalidValue) {
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

  DEPTH
    TEST on
    CLAMP foo
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("17: invalid value for CLAMP: foo", r.Error());
}

TEST_F(AmberScriptParserTest, DepthCompareMissingValue) {
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

  DEPTH
    TEST on
    COMPARE_OP
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("18: invalid value for COMPARE_OP", r.Error());
}

TEST_F(AmberScriptParserTest, DepthCompareInvalidValue) {
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

  DEPTH
    TEST on
    COMPARE_OP foo
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("17: invalid value for COMPARE_OP: foo", r.Error());
}

TEST_F(AmberScriptParserTest, DepthBoundsExpectingMin) {
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

  DEPTH
    TEST on
    BOUNDS
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("18: BOUNDS expecting min", r.Error());
}

TEST_F(AmberScriptParserTest, DepthBoundsMinInvalidValue) {
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

  DEPTH
    TEST on
    BOUNDS min foo
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("17: BOUNDS invalid value for min", r.Error());
}

TEST_F(AmberScriptParserTest, DepthBoundsExpectingMax) {
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

  DEPTH
    TEST on
    BOUNDS min 0.0 foo
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("17: BOUNDS expecting max", r.Error());
}

TEST_F(AmberScriptParserTest, DepthBoundsMaxInvalidValue) {
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

  DEPTH
    TEST on
    BOUNDS min 0.0 max foo
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("17: BOUNDS invalid value for max", r.Error());
}

TEST_F(AmberScriptParserTest, DepthBiasExpectingConstant) {
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

  DEPTH
    TEST on
    BIAS
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("18: BIAS expecting constant", r.Error());
}

TEST_F(AmberScriptParserTest, DepthBiasConstantInvalidValue) {
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

  DEPTH
    TEST on
    BIAS constant foo
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("17: BIAS invalid value for constant", r.Error());
}

TEST_F(AmberScriptParserTest, DepthBiasExpectingClamp) {
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

  DEPTH
    TEST on
    BIAS constant 0.0 foo
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("17: BIAS expecting clamp", r.Error());
}

TEST_F(AmberScriptParserTest, DepthBiasClampInvalidValue) {
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

  DEPTH
    TEST on
    BIAS constant 0.0 clamp foo
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("17: BIAS invalid value for clamp", r.Error());
}

TEST_F(AmberScriptParserTest, DepthBiasExpectingSlope) {
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

  DEPTH
    TEST on
    BIAS constant 0.0 clamp 0.0
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("18: BIAS expecting slope", r.Error());
}

TEST_F(AmberScriptParserTest, DepthBiasSlopeInvalidValue) {
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

  DEPTH
    TEST on
    BIAS constant 0.0 clamp 0.0 slope foo
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("17: BIAS invalid value for slope", r.Error());
}

}  // namespace amberscript
}  // namespace amber
