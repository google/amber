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

#include "src/descriptor_set_and_binding_parser.h"

#include "gtest/gtest.h"

namespace amber {

using DescriptorSetAndBindingParserTest = testing::Test;

TEST_F(DescriptorSetAndBindingParserTest, CommaAndBinding) {
  DescriptorSetAndBindingParser parser;
  Result r = parser.Parse(":1234");
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  EXPECT_EQ(0, parser.GetDescriptorSet());
  EXPECT_EQ(1234, parser.GetBinding());
}

TEST_F(DescriptorSetAndBindingParserTest, Binding) {
  DescriptorSetAndBindingParser parser;
  Result r = parser.Parse("1234");
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  EXPECT_EQ(0, parser.GetDescriptorSet());
  EXPECT_EQ(1234, parser.GetBinding());
}

TEST_F(DescriptorSetAndBindingParserTest, DescSetAndBinding) {
  DescriptorSetAndBindingParser parser;
  Result r = parser.Parse("1234:5678");
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  EXPECT_EQ(1234, parser.GetDescriptorSet());
  EXPECT_EQ(5678, parser.GetBinding());
}

TEST_F(DescriptorSetAndBindingParserTest, EmptyBufferId) {
  DescriptorSetAndBindingParser parser;
  Result r = parser.Parse("");
  EXPECT_EQ("Invalid buffer id: ", r.Error());
}

TEST_F(DescriptorSetAndBindingParserTest, InvalidCharacter) {
  DescriptorSetAndBindingParser parser;
  Result r = parser.Parse("abcd");
  EXPECT_EQ("Invalid buffer id: abcd", r.Error());
}

TEST_F(DescriptorSetAndBindingParserTest, InvalidCharacterBetweenTwoNumbers) {
  DescriptorSetAndBindingParser parser;
  Result r = parser.Parse("1234a5678");
  EXPECT_EQ("Invalid buffer id: 1234a5678", r.Error());
}

TEST_F(DescriptorSetAndBindingParserTest, InvalidCharacterAfterComma) {
  DescriptorSetAndBindingParser parser;
  Result r = parser.Parse("1234:a5678");
  EXPECT_EQ(
      "Binding for a buffer must be non-negative integer, but you gave: a5678",
      r.Error());
}

TEST_F(DescriptorSetAndBindingParserTest, NegativeDescSet) {
  DescriptorSetAndBindingParser parser;
  Result r = parser.Parse("-1234:5678");
  EXPECT_EQ(
      "Descriptor set and binding for a buffer must be non-negative integer, "
      "but you gave: -1234",
      r.Error());
}

TEST_F(DescriptorSetAndBindingParserTest, NegativeBindingAfterComma) {
  DescriptorSetAndBindingParser parser;
  Result r = parser.Parse(":-1234");
  EXPECT_EQ(
      "Binding for a buffer must be non-negative integer, but you gave: -1234",
      r.Error());
}

TEST_F(DescriptorSetAndBindingParserTest, NegativeBinding) {
  DescriptorSetAndBindingParser parser;
  Result r = parser.Parse("-1234");
  EXPECT_EQ(
      "Descriptor set and binding for a buffer must be non-negative integer, "
      "but you gave: -1234",
      r.Error());
}

TEST_F(DescriptorSetAndBindingParserTest, DescSetAndNegativeBinding) {
  DescriptorSetAndBindingParser parser;
  Result r = parser.Parse("1234:-5678");
  EXPECT_EQ(
      "Binding for a buffer must be non-negative integer, but you gave: -5678",
      r.Error());
}

}  // namespace amber
