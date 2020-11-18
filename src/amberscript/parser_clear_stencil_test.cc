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

TEST_F(AmberScriptParserTest, ClearStencil) {
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

CLEAR_STENCIL my_pipeline 15)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& commands = script->GetCommands();
  ASSERT_EQ(1U, commands.size());

  auto* cmd = commands[0].get();
  ASSERT_TRUE(cmd->IsClearStencil());

  auto* clr = cmd->AsClearStencil();
  EXPECT_EQ(15u, clr->GetValue());
}

TEST_F(AmberScriptParserTest, ClearStencilWithComputePipeline) {
  std::string in = R"(
SHADER compute my_shader GLSL
# shader
END

PIPELINE compute my_pipeline
  ATTACH my_shader
END

CLEAR_STENCIL my_pipeline 0.0)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("10: CLEAR_STENCIL command requires graphics pipeline", r.Error());
}

TEST_F(AmberScriptParserTest, ClearStencilMissingPipeline) {
  std::string in = "CLEAR_STENCIL 0";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: missing pipeline name for CLEAR_STENCIL command", r.Error());
}

TEST_F(AmberScriptParserTest, ClearStencilInvalidPipeline) {
  std::string in = "CLEAR_STENCIL unknown_pipeline 0";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());

  EXPECT_EQ("1: unknown pipeline for CLEAR_STENCIL command: unknown_pipeline",
            r.Error());
}

struct ClearStencilTestData {
  std::string data;
  std::string error;
};
using AmberScriptParserClearStencilTest =
    testing::TestWithParam<ClearStencilTestData>;
TEST_P(AmberScriptParserClearStencilTest, InvalidParams) {
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

CLEAR_STENCIL my_pipeline )" +
                   test_data.data;

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess()) << test_data.data;
  EXPECT_EQ(std::string("13: ") + test_data.error, r.Error()) << test_data.data;
}

INSTANTIATE_TEST_SUITE_P(
    AmberScriptParserClearStencilTests,
    AmberScriptParserClearStencilTest,
    testing::Values(
        ClearStencilTestData{"", "missing value for CLEAR_STENCIL command"},
        ClearStencilTestData{
            "INVALID", "invalid value for CLEAR_STENCIL command: INVALID"},
        ClearStencilTestData{"-5",
                             "invalid value for CLEAR_STENCIL command: -5"},
        ClearStencilTestData{"256",
                             "invalid value for CLEAR_STENCIL command: 256"},
        ClearStencilTestData{"10 EXTRA",
                             "extra parameters after CLEAR_STENCIL command: "
                             "EXTRA"}));  // NOLINT(whitespace/parens)

}  // namespace amberscript
}  // namespace amber
