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

TEST_F(AmberScriptParserTest, Set) {
  std::string in = "SET ENGINE_DATA fence_timeout_ms 125";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  auto& data = script->GetEngineData();

  EXPECT_EQ(125U, data.fence_timeout_ms);
}

TEST_F(AmberScriptParserTest, SetMissingEngineData) {
  std::string in = "SET fence_timeout_ms 125";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: SET missing ENGINE_DATA", r.Error());
}

TEST_F(AmberScriptParserTest, SetMissingVariable) {
  std::string in = "SET ENGINE_DATA";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: SET missing variable to be set", r.Error());
}

TEST_F(AmberScriptParserTest, SetInvalidVariable) {
  std::string in = "SET ENGINE_DATA 1234";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: SET invalid variable to set: 1234", r.Error());
}

TEST_F(AmberScriptParserTest, SetWithUnknownVariable) {
  std::string in = "SET ENGINE_DATA unknown";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: SET unknown variable provided: unknown", r.Error());
}

TEST_F(AmberScriptParserTest, SetFenceTimeoutMissingValue) {
  std::string in = "SET ENGINE_DATA fence_timeout_ms";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: SET missing value for fence_timeout_ms", r.Error());
}

TEST_F(AmberScriptParserTest, SetFenceTimeInvalidValue) {
  std::string in = "SET ENGINE_DATA fence_timeout_ms INVALID";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: SET invalid value for fence_timeout_ms, must be uint32",
            r.Error());
}

TEST_F(AmberScriptParserTest, SetFenceTimeoutExtraParams) {
  std::string in = "SET ENGINE_DATA fence_timeout_ms 100 EXTRA";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: extra parameters after SET command", r.Error());
}

}  // namespace amberscript
}  // namespace amber
