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

TEST_F(AmberScriptParserTest, BlendAllValues) {
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

  BLEND
    SRC_COLOR_FACTOR src_alpha
    DST_COLOR_FACTOR one_minus_src_alpha
    COLOR_OP add
    SRC_ALPHA_FACTOR one
    DST_ALPHA_FACTOR zero
    ALPHA_OP max
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& pipelines = script->GetPipelines();
  ASSERT_EQ(1U, pipelines.size());

  auto* pipeline = pipelines[0].get();

  ASSERT_TRUE(pipeline->GetPipelineData()->GetEnableBlend());
  ASSERT_EQ(BlendFactor::kSrcAlpha,
            pipeline->GetPipelineData()->GetSrcColorBlendFactor());
  ASSERT_EQ(BlendFactor::kOneMinusSrcAlpha,
            pipeline->GetPipelineData()->GetDstColorBlendFactor());
  ASSERT_EQ(BlendOp::kAdd,
            pipeline->GetPipelineData()->GetColorBlendOp());

  ASSERT_EQ(BlendFactor::kOne,
            pipeline->GetPipelineData()->GetSrcAlphaBlendFactor());
  ASSERT_EQ(BlendFactor::kZero,
            pipeline->GetPipelineData()->GetDstAlphaBlendFactor());
  ASSERT_EQ(BlendOp::kMax,
            pipeline->GetPipelineData()->GetAlphaBlendOp());
}

TEST_F(AmberScriptParserTest, BlendDefaultValues) {
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

  BLEND
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& pipelines = script->GetPipelines();
  ASSERT_EQ(1U, pipelines.size());

  auto* pipeline = pipelines[0].get();

  ASSERT_TRUE(pipeline->GetPipelineData()->GetEnableBlend());
  ASSERT_EQ(BlendFactor::kOne,
            pipeline->GetPipelineData()->GetSrcColorBlendFactor());
  ASSERT_EQ(BlendFactor::kZero,
            pipeline->GetPipelineData()->GetDstColorBlendFactor());
  ASSERT_EQ(BlendOp::kAdd,
            pipeline->GetPipelineData()->GetColorBlendOp());

  ASSERT_EQ(BlendFactor::kOne,
            pipeline->GetPipelineData()->GetSrcAlphaBlendFactor());
  ASSERT_EQ(BlendFactor::kZero,
            pipeline->GetPipelineData()->GetDstAlphaBlendFactor());
  ASSERT_EQ(BlendOp::kAdd,
            pipeline->GetPipelineData()->GetAlphaBlendOp());
}

TEST_F(AmberScriptParserTest, BlendInvalidColorFactor) {
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

  BLEND
    SRC_COLOR_FACTOR foo
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("14: BLEND invalid value for SRC_COLOR_FACTOR: foo", r.Error());
}

}  // namespace amberscript
}  // namespace amber
