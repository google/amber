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

TEST_F(AmberScriptParserTest, ClearColor) {
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

CLEAR_COLOR my_pipeline 255 128 64 32)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& commands = script->GetCommands();
  ASSERT_EQ(1U, commands.size());

  auto* cmd = commands[0].get();
  ASSERT_TRUE(cmd->IsClearColor());

  auto* clr = cmd->AsClearColor();
  EXPECT_FLOAT_EQ(255.f / 255.f, clr->GetR());
  EXPECT_FLOAT_EQ(128.f / 255.f, clr->GetG());
  EXPECT_FLOAT_EQ(64.f / 255.f, clr->GetB());
  EXPECT_FLOAT_EQ(32.f / 255.f, clr->GetA());
}

TEST_F(AmberScriptParserTest, ClearColorWithComputePipeline) {
  std::string in = R"(
SHADER compute my_shader GLSL
# shader
END

PIPELINE compute my_pipeline
  ATTACH my_shader
END

CLEAR_COLOR my_pipeline 255 128 64 32)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("10: CLEAR_COLOR command requires graphics pipeline", r.Error());
}

TEST_F(AmberScriptParserTest, ClearColorMissingPipeline) {
  std::string in = "CLEAR_COLOR 255 255 255 255";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: missing pipeline name for CLEAR_COLOR command", r.Error());
}

TEST_F(AmberScriptParserTest, ClearColorInvalidPipeline) {
  std::string in = "CLEAR_COLOR unknown_pipeline 255 255 255 255";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());

  EXPECT_EQ("1: unknown pipeline for CLEAR_COLOR command: unknown_pipeline",
            r.Error());
}

struct ClearColorTestData {
  std::string data;
  std::string error;
};
using AmberScriptParserClearColorTest =
    testing::TestWithParam<ClearColorTestData>;
TEST_P(AmberScriptParserClearColorTest, InvalidParams) {
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

CLEAR_COLOR my_pipeline )" +
                   test_data.data;

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess()) << test_data.data;
  EXPECT_EQ(std::string("13: ") + test_data.error, r.Error()) << test_data.data;
}

INSTANTIATE_TEST_SUITE_P(
    AmberScriptParserClearColorTests,
    AmberScriptParserClearColorTest,
    testing::Values(
        ClearColorTestData{"", "missing R value for CLEAR_COLOR command"},
        ClearColorTestData{"255", "missing G value for CLEAR_COLOR command"},
        ClearColorTestData{"255 255",
                           "missing B value for CLEAR_COLOR command"},
        ClearColorTestData{"255 255 255",
                           "missing A value for CLEAR_COLOR command"},
        ClearColorTestData{"INVALID 255 255 255",
                           "invalid R value for CLEAR_COLOR command: INVALID"},
        ClearColorTestData{"255 INVALID 255 255",
                           "invalid G value for CLEAR_COLOR command: INVALID"},
        ClearColorTestData{"255 255 INVALID 255",
                           "invalid B value for CLEAR_COLOR command: INVALID"},
        ClearColorTestData{"255 255 255 INVALID",
                           "invalid A value for CLEAR_COLOR command: INVALID"},
        ClearColorTestData{"255 255 255 255 EXTRA",
                           "extra parameters after CLEAR_COLOR command"},
        ClearColorTestData{"-1 255 255 255",
                           "invalid R value for CLEAR_COLOR command: -1"},
        ClearColorTestData{"5.2 255 255 255",
                           "invalid R value for CLEAR_COLOR command: 5.2"},
        ClearColorTestData{"256 255 255 255",
                           "invalid R value for CLEAR_COLOR command: 256"},
        ClearColorTestData{"255 -1 255 255",
                           "invalid G value for CLEAR_COLOR command: -1"},
        ClearColorTestData{"255 5.2 255 255",
                           "invalid G value for CLEAR_COLOR command: 5.2"},
        ClearColorTestData{"255 256 255 255",
                           "invalid G value for CLEAR_COLOR command: 256"},
        ClearColorTestData{"255 255 -1 255",
                           "invalid B value for CLEAR_COLOR command: -1"},
        ClearColorTestData{"255 255 5.2 255",
                           "invalid B value for CLEAR_COLOR command: 5.2"},
        ClearColorTestData{"255 255 256 255",
                           "invalid B value for CLEAR_COLOR command: 256"},
        ClearColorTestData{"255 255 255 -1",
                           "invalid A value for CLEAR_COLOR command: -1"},
        ClearColorTestData{"255 255 255 5.2",
                           "invalid A value for CLEAR_COLOR command: 5.2"},
        ClearColorTestData{"255 255 255 256",
                           "invalid A value for CLEAR_COLOR "
                           "command: 256"}));  // NOLINT(whitespace/parens)

}  // namespace amberscript
}  // namespace amber
