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

TEST_F(AmberScriptParserTest, StencilAllValues) {
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

  STENCIL front
    TEST on
    FAIL_OP increment_and_clamp
    PASS_OP invert
    DEPTH_FAIL_OP keep
    COMPARE_OP equal
    COMPARE_MASK 1
    WRITE_MASK 2
    REFERENCE 3
  END
  STENCIL back
    TEST on
    FAIL_OP zero
    PASS_OP increment_and_wrap
    DEPTH_FAIL_OP replace
    COMPARE_OP greater
    COMPARE_MASK 4
    WRITE_MASK 5
    REFERENCE 6
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

  ASSERT_TRUE(pipeline->GetPipelineData()->GetEnableStencilTest());
  ASSERT_EQ(StencilOp::kIncrementAndClamp,
            pipeline->GetPipelineData()->GetFrontFailOp());
  ASSERT_EQ(StencilOp::kZero, pipeline->GetPipelineData()->GetBackFailOp());
  ASSERT_EQ(StencilOp::kInvert, pipeline->GetPipelineData()->GetFrontPassOp());
  ASSERT_EQ(StencilOp::kIncrementAndWrap,
            pipeline->GetPipelineData()->GetBackPassOp());
  ASSERT_EQ(StencilOp::kKeep,
            pipeline->GetPipelineData()->GetFrontDepthFailOp());
  ASSERT_EQ(StencilOp::kReplace,
            pipeline->GetPipelineData()->GetBackDepthFailOp());
  ASSERT_EQ(CompareOp::kEqual,
            pipeline->GetPipelineData()->GetFrontCompareOp());
  ASSERT_EQ(CompareOp::kGreater,
            pipeline->GetPipelineData()->GetBackCompareOp());

  ASSERT_EQ(1u, pipeline->GetPipelineData()->GetFrontCompareMask());
  ASSERT_EQ(4u, pipeline->GetPipelineData()->GetBackCompareMask());
  ASSERT_EQ(2u, pipeline->GetPipelineData()->GetFrontWriteMask());
  ASSERT_EQ(5u, pipeline->GetPipelineData()->GetBackWriteMask());
  ASSERT_EQ(3u, pipeline->GetPipelineData()->GetFrontReference());
  ASSERT_EQ(6u, pipeline->GetPipelineData()->GetBackReference());
}

TEST_F(AmberScriptParserTest, StencilMissingFace) {
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

  STENCIL
    TEST on
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("16: STENCIL missing face", r.Error());
}

TEST_F(AmberScriptParserTest, StencilInvalidFaceValue) {
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

  STENCIL foo
    TEST on
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("15: STENCIL invalid face: foo", r.Error());
}

TEST_F(AmberScriptParserTest, StencilTestMissingValue) {
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

  STENCIL front
    TEST
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("17: STENCIL invalid value for TEST", r.Error());
}

TEST_F(AmberScriptParserTest, StencilTestInvalidValue) {
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

  STENCIL front
    TEST foo
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("16: STENCIL invalid value for TEST: foo", r.Error());
}

TEST_F(AmberScriptParserTest, StencilFailMissingValue) {
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

  STENCIL front
    TEST on
    FAIL_OP
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("18: STENCIL invalid value for FAIL_OP", r.Error());
}

TEST_F(AmberScriptParserTest, StencilFailInvalidValue) {
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

  STENCIL front
    TEST on
    FAIL_OP foo
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("17: STENCIL invalid value for FAIL_OP: foo", r.Error());
}

TEST_F(AmberScriptParserTest, StencilPassMissingValue) {
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

  STENCIL front
    TEST on
    PASS_OP
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("18: STENCIL invalid value for PASS_OP", r.Error());
}

TEST_F(AmberScriptParserTest, StencilPassInvalidValue) {
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

  STENCIL front
    TEST on
    PASS_OP foo
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("17: STENCIL invalid value for PASS_OP: foo", r.Error());
}

TEST_F(AmberScriptParserTest, StencilDepthFailMissingValue) {
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

  STENCIL front
    TEST on
    DEPTH_FAIL_OP
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("18: STENCIL invalid value for DEPTH_FAIL_OP", r.Error());
}

TEST_F(AmberScriptParserTest, StencilDepthFailInvalidValue) {
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

  STENCIL front
    TEST on
    DEPTH_FAIL_OP foo
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("17: STENCIL invalid value for DEPTH_FAIL_OP: foo", r.Error());
}

TEST_F(AmberScriptParserTest, StencilCompareMissingValue) {
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

  STENCIL front
    TEST on
    COMPARE_OP
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("18: STENCIL invalid value for COMPARE_OP", r.Error());
}

TEST_F(AmberScriptParserTest, StencilCompareInvalidValue) {
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

  STENCIL front
    TEST on
    COMPARE_OP foo
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("17: STENCIL invalid value for COMPARE_OP: foo", r.Error());
}

TEST_F(AmberScriptParserTest, StencilCompareMaskMissingValue) {
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

  STENCIL front
    TEST on
    COMPARE_MASK
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("18: STENCIL invalid value for COMPARE_MASK", r.Error());
}

TEST_F(AmberScriptParserTest, StencilCompareMaskInvalidValue) {
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

  STENCIL front
    TEST on
    COMPARE_MASK foo
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("17: STENCIL invalid value for COMPARE_MASK", r.Error());
}

TEST_F(AmberScriptParserTest, StencilWriteMaskMissingValue) {
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

  STENCIL front
    TEST on
    WRITE_MASK
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("18: STENCIL invalid value for WRITE_MASK", r.Error());
}

TEST_F(AmberScriptParserTest, StencilWriteMaskInvalidValue) {
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

  STENCIL front
    TEST on
    WRITE_MASK foo
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("17: STENCIL invalid value for WRITE_MASK", r.Error());
}

TEST_F(AmberScriptParserTest, StencilReferenceMissingValue) {
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

  STENCIL front
    TEST on
    REFERENCE
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("18: STENCIL invalid value for REFERENCE", r.Error());
}

TEST_F(AmberScriptParserTest, StencilReferenceInvalidValue) {
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

  STENCIL front
    TEST on
    REFERENCE foo
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("17: STENCIL invalid value for REFERENCE", r.Error());
}

}  // namespace amberscript
}  // namespace amber
