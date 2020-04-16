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

TEST_F(AmberScriptParserTest, ClearDepth) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
END

CLEAR_DEPTH my_pipeline 1.5)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& commands = script->GetCommands();
  ASSERT_EQ(1U, commands.size());

  auto* cmd = commands[0].get();
  ASSERT_TRUE(cmd->IsClearDepth());

  auto* clr = cmd->AsClearDepth();
  EXPECT_FLOAT_EQ(1.5, clr->GetValue());
}

TEST_F(AmberScriptParserTest, ClearDepthWithComputePipeline) {
  std::string in = R"(
SHADER compute my_shader GLSL
# shader
END

PIPELINE compute my_pipeline
  ATTACH my_shader
END

CLEAR_DEPTH my_pipeline 0.0)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("10: CLEAR_DEPTH command requires graphics pipeline", r.Error());
}

TEST_F(AmberScriptParserTest, ClearDepthMissingPipeline) {
  std::string in = "CLEAR_DEPTH 0.0";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: missing pipeline name for CLEAR_DEPTH command", r.Error());
}

TEST_F(AmberScriptParserTest, ClearDepthInvalidPipeline) {
  std::string in = "CLEAR_DEPTH unknown_pipeline 0.0";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());

  EXPECT_EQ("1: unknown pipeline for CLEAR_DEPTH command: unknown_pipeline",
            r.Error());
}

struct ClearDepthTestData {
  std::string data;
  std::string error;
};
using AmberScriptParserClearDepthTest =
    testing::TestWithParam<ClearDepthTestData>;
TEST_P(AmberScriptParserClearDepthTest, InvalidParams) {
  auto test_data = GetParam();

  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
END

CLEAR_DEPTH my_pipeline )" +
                   test_data.data;

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess()) << test_data.data;
  EXPECT_EQ(std::string("13: ") + test_data.error, r.Error()) << test_data.data;
}

INSTANTIATE_TEST_SUITE_P(
    AmberScriptParserClearDepthTests,
    AmberScriptParserClearDepthTest,
    testing::Values(
        ClearDepthTestData{"", "missing value for CLEAR_DEPTH command"},
        ClearDepthTestData{"INVALID",
                           "invalid value for CLEAR_DEPTH command: INVALID"},
        ClearDepthTestData{"5", "invalid value for CLEAR_DEPTH command: 5"},
        ClearDepthTestData{"1.0 EXTRA",
                           "extra parameters after CLEAR_DEPTH command: "
                           "EXTRA"}));  // NOLINT(whitespace/parens)

}  // namespace amberscript
}  // namespace amber
