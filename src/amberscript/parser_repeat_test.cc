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

TEST_F(AmberScriptParserTest, Repeat) {
  std::string in = R"(
SHADER compute shader GLSL
# shader
END

PIPELINE compute my_pipeline
  ATTACH shader
END

REPEAT 4
  RUN my_pipeline 1 2 3
  RUN my_pipeline 4 5 6
  RUN my_pipeline 7 8 9
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& commands = script->GetCommands();
  ASSERT_EQ(1U, commands.size());

  auto* cmd = commands[0].get();
  ASSERT_TRUE(cmd->IsRepeat());

  auto* repeat = cmd->AsRepeat();
  EXPECT_EQ(4U, repeat->GetCount());

  const auto& repeat_cmds = repeat->GetCommands();
  ASSERT_EQ(3U, repeat_cmds.size());
  ASSERT_TRUE(repeat_cmds[0]->IsCompute());
  EXPECT_EQ(1U, repeat_cmds[0]->AsCompute()->GetX());
  ASSERT_TRUE(repeat_cmds[1]->IsCompute());
  EXPECT_EQ(4U, repeat_cmds[1]->AsCompute()->GetX());
  ASSERT_TRUE(repeat_cmds[2]->IsCompute());
  EXPECT_EQ(7U, repeat_cmds[2]->AsCompute()->GetX());
}

TEST_F(AmberScriptParserTest, RepeatMissingNum) {
  std::string in = R"(
REPEAT
  RUN my_pipeline 1 1 1
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: missing count parameter for REPEAT command", r.Error());
}

TEST_F(AmberScriptParserTest, RepeatInvalidNum) {
  std::string in = R"(
REPEAT INVALID
  RUN my_pipeline 1 1 1
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("2: invalid count parameter for REPEAT command: INVALID",
            r.Error());
}

TEST_F(AmberScriptParserTest, RepeatFloatNum) {
  std::string in = R"(
REPEAT 3.4
  RUN my_pipeline 1 1 1
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("2: invalid count parameter for REPEAT command: 3.4", r.Error());
}

TEST_F(AmberScriptParserTest, RepeatMissingEnd) {
  std::string in = R"(
SHADER compute shader GLSL
# shader
END

PIPELINE compute my_pipeline
  ATTACH shader
END
REPEAT 3
  RUN my_pipeline 1 1 1
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("11: missing END for REPEAT command", r.Error());
}

TEST_F(AmberScriptParserTest, RepeatExtraParams) {
  std::string in = R"(
REPEAT 3 EXTRA
  RUN my_pipeline 1 1 1
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("2: unknown token: EXTRA", r.Error());
}

TEST_F(AmberScriptParserTest, RepeatNegativeCount) {
  std::string in = R"(
REPEAT -3
  RUN my_pipeline 1 1 1
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("2: count parameter must be > 0 for REPEAT command", r.Error());
}

TEST_F(AmberScriptParserTest, RepeatZeroCount) {
  std::string in = R"(
REPEAT 0
  RUN my_pipeline 1 1 1
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("2: count parameter must be > 0 for REPEAT command", r.Error());
}

}  // namespace amberscript
}  // namespace amber
