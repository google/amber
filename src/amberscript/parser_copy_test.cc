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

TEST_F(AmberScriptParserTest, Copy) {
  std::string in = R"(
BUFFER from FORMAT R32G32B32A32_SFLOAT
BUFFER dest FORMAT R32G32B32A32_SFLOAT
COPY from TO dest)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& commands = script->GetCommands();
  ASSERT_EQ(1U, commands.size());

  auto* cmd = commands[0].get();
  ASSERT_TRUE(cmd->IsCopy());
}

TEST_F(AmberScriptParserTest, CopyUndeclaredOriginBuffer) {
  std::string in = R"(
COPY from)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("2: COPY origin buffer was not declared", r.Error());
}

TEST_F(AmberScriptParserTest, CopyInvalidOriginBufferName) {
  std::string in = R"(
COPY 123)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("2: invalid buffer name after COPY", r.Error());
}

TEST_F(AmberScriptParserTest, CopyUndeclaredDestinationBuffer) {
  std::string in = R"(
BUFFER from FORMAT R32G32B32A32_SFLOAT
COPY from TO dest)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: COPY destination buffer was not declared", r.Error());
}

TEST_F(AmberScriptParserTest, CopyMissingOriginBuffer) {
  std::string in = R"(
COPY)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("2: missing buffer name after COPY", r.Error());
}

TEST_F(AmberScriptParserTest, CopyMissingDestinationBuffer) {
  std::string in = R"(
BUFFER from FORMAT R32G32B32A32_SFLOAT
COPY from TO)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: missing buffer name after TO", r.Error());
}

TEST_F(AmberScriptParserTest, CopyToSameBuffer) {
  std::string in = R"(
BUFFER from FORMAT R32G32B32A32_SFLOAT
COPY from TO from)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: COPY origin and destination buffers are identical", r.Error());
}

TEST_F(AmberScriptParserTest, CopyMissingToKeyword) {
  std::string in = R"(
BUFFER from FORMAT R32G32B32A32_SFLOAT
BUFFER dest FORMAT R32G32B32A32_SFLOAT
COPY from dest)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("4: expected 'TO' after COPY and buffer name", r.Error());
}

}  // namespace amberscript
}  // namespace amber
