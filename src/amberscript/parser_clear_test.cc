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

TEST_F(AmberScriptParserTest, Clear) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
END

CLEAR my_pipeline)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& commands = script->GetCommands();
  ASSERT_EQ(1U, commands.size());

  auto* cmd = commands[0].get();
  ASSERT_TRUE(cmd->IsClear());
}

TEST_F(AmberScriptParserTest, ClearMissingPipeline) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
END

CLEAR)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("12: missing pipeline name for CLEAR command", r.Error());
}

TEST_F(AmberScriptParserTest, ClearInvalidPipeline) {
  std::string in = R"(CLEAR other_pipeline)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: unknown pipeline for CLEAR command: other_pipeline", r.Error());
}

TEST_F(AmberScriptParserTest, ClearComputePipeline) {
  std::string in = R"(
SHADER compute my_shader GLSL
void main() {
  gl_FragColor = vec3(2, 3, 4);
}
END

PIPELINE compute my_pipeline
  ATTACH my_shader
END

CLEAR my_pipeline)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  ASSERT_EQ("12: CLEAR command requires graphics pipeline", r.Error());
}

TEST_F(AmberScriptParserTest, ClearExtraParams) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
END

CLEAR my_pipeline EXTRA)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("12: extra parameters after CLEAR command", r.Error());
}

}  // namespace amberscript
}  // namespace amber
