// Copyright 2018 The Amber Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "src/tokenizer.h"

#include <cmath>
#include <limits>

#include "gtest/gtest.h"

namespace amber {

using TokenizerTest = testing::Test;

TEST_F(TokenizerTest, ProcessEmpty) {
  Tokenizer t("");
  auto next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  EXPECT_TRUE(next->IsEOS());
}

TEST_F(TokenizerTest, ProcessString) {
  Tokenizer t("TestString");
  auto next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  EXPECT_TRUE(next->IsString());
  EXPECT_EQ("TestString", next->AsString());

  next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  EXPECT_TRUE(next->IsEOS());
}

TEST_F(TokenizerTest, ProcessInt) {
  Tokenizer t("123");
  auto next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  EXPECT_TRUE(next->IsInteger());
  EXPECT_EQ(123U, next->AsUint32());

  next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  EXPECT_TRUE(next->IsEOS());
}

TEST_F(TokenizerTest, ProcessNegative) {
  Tokenizer t("-123");
  auto next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  EXPECT_TRUE(next->IsInteger());
  EXPECT_EQ(-123, next->AsInt32());

  next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  EXPECT_TRUE(next->IsEOS());
}

TEST_F(TokenizerTest, ProcessDouble) {
  Tokenizer t("123.456");
  auto next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  EXPECT_TRUE(next->IsDouble());
  EXPECT_EQ(123.456f, next->AsFloat());

  next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  EXPECT_TRUE(next->IsEOS());
}

namespace {

void TestNaN(const std::string& nan_str) {
  Tokenizer t(nan_str);
  auto next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  EXPECT_TRUE(next->IsDouble());
  EXPECT_TRUE(std::isnan(next->AsDouble()));

  next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  EXPECT_TRUE(next->IsEOS());
}

}  // namespace

TEST_F(TokenizerTest, ProcessNaN) {
  TestNaN("nan");
  TestNaN("naN");
  TestNaN("nAn");
  TestNaN("nAN");
  TestNaN("Nan");
  TestNaN("NaN");
  TestNaN("NAn");
  TestNaN("NAN");
}

TEST_F(TokenizerTest, ProcessNegativeDouble) {
  Tokenizer t("-123.456");
  auto next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  EXPECT_TRUE(next->IsDouble());
  EXPECT_EQ(-123.456f, next->AsFloat());

  next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  EXPECT_TRUE(next->IsEOS());
}

TEST_F(TokenizerTest, ProcessDoubleStartWithDot) {
  Tokenizer t(".123456");
  auto next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  EXPECT_TRUE(next->IsDouble());
  EXPECT_EQ(.123456f, next->AsFloat());

  next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  EXPECT_TRUE(next->IsEOS());
}

TEST_F(TokenizerTest, ProcessStringWithNumberInName) {
  Tokenizer t("BufferAccess32");
  auto next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  EXPECT_TRUE(next->IsString());
  EXPECT_EQ("BufferAccess32", next->AsString());

  next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  EXPECT_TRUE(next->IsEOS());
}

TEST_F(TokenizerTest, ProcessMultiStatement) {
  Tokenizer t("TestValue 123.456");
  auto next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  EXPECT_TRUE(next->IsString());
  EXPECT_EQ("TestValue", next->AsString());

  next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  EXPECT_TRUE(next->IsDouble());
  EXPECT_EQ(123.456f, next->AsFloat());

  next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  EXPECT_TRUE(next->IsEOS());
}

TEST_F(TokenizerTest, ProcessMultiLineStatement) {
  Tokenizer t("TestValue 123.456\nAnotherValue\n\nThirdValue 456");
  auto next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  EXPECT_TRUE(next->IsString());
  EXPECT_EQ("TestValue", next->AsString());
  EXPECT_EQ(1U, t.GetCurrentLine());

  next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  EXPECT_TRUE(next->IsDouble());
  EXPECT_EQ(123.456f, next->AsFloat());
  EXPECT_EQ(1U, t.GetCurrentLine());

  next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  EXPECT_TRUE(next->IsEOL());

  next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  EXPECT_TRUE(next->IsString());
  EXPECT_EQ("AnotherValue", next->AsString());
  EXPECT_EQ(2U, t.GetCurrentLine());

  next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  EXPECT_TRUE(next->IsEOL());

  next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  EXPECT_TRUE(next->IsEOL());

  next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  EXPECT_TRUE(next->IsString());
  EXPECT_EQ("ThirdValue", next->AsString());
  EXPECT_EQ(4U, t.GetCurrentLine());

  next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  EXPECT_TRUE(next->IsInteger());
  EXPECT_EQ(456U, next->AsUint16());
  EXPECT_EQ(4U, t.GetCurrentLine());

  next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  EXPECT_TRUE(next->IsEOS());
}

TEST_F(TokenizerTest, ProcessComments) {
  Tokenizer t(R"(# Initial comment string
TestValue 123.456
    AnotherValue   # Space before, comment after

ThirdValue 456)");
  auto next = t.NextToken();
  // The comment injects a blank line into the output
  // so we can handle full line comment and end of line comment the same.
  ASSERT_TRUE(next != nullptr);
  EXPECT_TRUE(next->IsEOL());

  next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  EXPECT_TRUE(next->IsString());
  EXPECT_EQ("TestValue", next->AsString());

  next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  EXPECT_TRUE(next->IsDouble());
  EXPECT_EQ(123.456f, next->AsFloat());

  next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  EXPECT_TRUE(next->IsEOL());

  next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  EXPECT_TRUE(next->IsString());
  EXPECT_EQ("AnotherValue", next->AsString());

  next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  EXPECT_TRUE(next->IsEOL());

  next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  EXPECT_TRUE(next->IsEOL());

  next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  EXPECT_TRUE(next->IsString());
  EXPECT_EQ("ThirdValue", next->AsString());

  next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  EXPECT_TRUE(next->IsInteger());
  EXPECT_EQ(456U, next->AsUint16());

  next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  EXPECT_TRUE(next->IsEOS());
}

TEST_F(TokenizerTest, HexValue) {
  Tokenizer t("0xff00f0ff");
  auto next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  EXPECT_TRUE(next->IsHex());
  EXPECT_EQ(0xff00f0ff, next->AsHex());

  next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  EXPECT_TRUE(next->IsEOS());
}

TEST_F(TokenizerTest, HexValueAfterWhiteSpace) {
  Tokenizer t("     \t  \t   0xff00f0ff");
  auto next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  EXPECT_TRUE(next->IsHex());
  EXPECT_EQ(0xff00f0ff, next->AsHex());

  next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  EXPECT_TRUE(next->IsEOS());
}

TEST_F(TokenizerTest, StringStartingWithNum) {
  Tokenizer t("1/ABC");
  auto next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  EXPECT_TRUE(next->IsInteger());
  EXPECT_EQ(1U, next->AsUint32());

  next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  EXPECT_TRUE(next->IsString());
  EXPECT_EQ("/ABC", next->AsString());
}

TEST_F(TokenizerTest, BracketsAndCommas) {
  Tokenizer t("(1.0, 2, abc)");
  auto next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  EXPECT_TRUE(next->IsOpenBracket());

  next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  EXPECT_TRUE(next->IsDouble());
  EXPECT_FLOAT_EQ(1.0, next->AsFloat());

  next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  EXPECT_TRUE(next->IsComma());

  next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  EXPECT_TRUE(next->IsInteger());
  EXPECT_EQ(2U, next->AsUint32());

  next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  EXPECT_TRUE(next->IsComma());

  next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  EXPECT_TRUE(next->IsString());
  EXPECT_EQ("abc", next->AsString());

  next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  EXPECT_TRUE(next->IsCloseBracket());

  next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  EXPECT_TRUE(next->IsEOS());
}

TEST_F(TokenizerTest, TokenToDoubleFromDouble) {
  Tokenizer t("-1.234");
  auto next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  ASSERT_TRUE(next->IsDouble());

  Result r = next->ConvertToDouble();
  ASSERT_TRUE(r.IsSuccess());
  EXPECT_FLOAT_EQ(-1.234f, next->AsFloat());
}

TEST_F(TokenizerTest, TokenToDoubleFromInt) {
  Tokenizer t("-1");
  auto next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  ASSERT_TRUE(next->IsInteger());

  Result r = next->ConvertToDouble();
  ASSERT_TRUE(r.IsSuccess());
  EXPECT_FLOAT_EQ(-1.0f, next->AsFloat());
}

TEST_F(TokenizerTest, DashToken) {
  Tokenizer t("-");
  auto next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  ASSERT_TRUE(next->IsString());
  EXPECT_EQ("-", next->AsString());
}

TEST_F(TokenizerTest, ParseUint64Max) {
  Tokenizer t(std::to_string(std::numeric_limits<uint64_t>::max()));
  auto next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  ASSERT_TRUE(next->IsInteger());
  EXPECT_EQ(std::numeric_limits<uint64_t>::max(), next->AsUint64());
}

TEST_F(TokenizerTest, ParseInt64Min) {
  Tokenizer t(std::to_string(std::numeric_limits<int64_t>::min()));
  auto next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  ASSERT_TRUE(next->IsInteger());
  EXPECT_EQ(std::numeric_limits<int64_t>::min(), next->AsInt64());
}

TEST_F(TokenizerTest, TokenToDoubleFromUint64Max) {
  Tokenizer t(std::to_string(std::numeric_limits<uint64_t>::max()));
  auto next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  ASSERT_TRUE(next->IsInteger());

  Result r = next->ConvertToDouble();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("uint64_t value too big to fit in double", r.Error());
}

TEST_F(TokenizerTest, TokenToDoubleFromInt64Min) {
  Tokenizer t(std::to_string(std::numeric_limits<int64_t>::min()));
  auto next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  ASSERT_TRUE(next->IsInteger());

  Result r = next->ConvertToDouble();
  ASSERT_TRUE(r.IsSuccess());
  EXPECT_DOUBLE_EQ(static_cast<double>(std::numeric_limits<int64_t>::min()),
                   next->AsDouble());
}

TEST_F(TokenizerTest, TokenToDoubleFromInt64Max) {
  Tokenizer t(std::to_string(std::numeric_limits<int64_t>::max()));
  auto next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  ASSERT_TRUE(next->IsInteger());

  Result r = next->ConvertToDouble();
  ASSERT_TRUE(r.IsSuccess());
  EXPECT_DOUBLE_EQ(static_cast<double>(std::numeric_limits<int64_t>::max()),
                   next->AsDouble());
}

TEST_F(TokenizerTest, TokenToDoubleFromString) {
  Tokenizer t("INVALID");
  auto next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  ASSERT_TRUE(next->IsString());

  Result r = next->ConvertToDouble();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("Invalid conversion to double", r.Error());
}

TEST_F(TokenizerTest, TokenToDoubleFromHex) {
  Tokenizer t("0xff00f0ff");
  auto next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  ASSERT_TRUE(next->IsHex());

  Result r = next->ConvertToDouble();
  ASSERT_TRUE(r.IsSuccess());
  EXPECT_FLOAT_EQ(static_cast<float>(0xff00f0ff), next->AsFloat());
}

TEST_F(TokenizerTest, TokenToDoubleFromEOS) {
  Tokenizer t("");
  auto next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  ASSERT_TRUE(next->IsEOS());

  Result r = next->ConvertToDouble();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("Invalid conversion to double", r.Error());
}

TEST_F(TokenizerTest, TokenToDoubleFromEOL) {
  Tokenizer t("-1\n-2");
  auto next = t.NextToken();
  next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  ASSERT_TRUE(next->IsEOL());

  Result r = next->ConvertToDouble();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("Invalid conversion to double", r.Error());
}

TEST_F(TokenizerTest, Continuations) {
  Tokenizer t("1 \\\n2");
  auto next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  ASSERT_TRUE(next->IsInteger());
  EXPECT_EQ(1, next->AsInt32());
  EXPECT_EQ(1, t.GetCurrentLine());

  next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  ASSERT_TRUE(next->IsInteger());
  EXPECT_EQ(2, next->AsInt32());
  EXPECT_EQ(2, t.GetCurrentLine());

  next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  EXPECT_TRUE(next->IsEOS());
}

TEST_F(TokenizerTest, ContinuationAtEndOfString) {
  Tokenizer t("1 \\");
  auto next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  ASSERT_TRUE(next->IsInteger());
  EXPECT_EQ(1, next->AsInt32());

  next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  ASSERT_TRUE(next->IsString());
  EXPECT_EQ("\\", next->AsString());

  next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  EXPECT_TRUE(next->IsEOS());
}

TEST_F(TokenizerTest, ContinuationTokenAtOfLine) {
  Tokenizer t("1 \\2");
  auto next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  ASSERT_TRUE(next->IsInteger());
  EXPECT_EQ(1, next->AsInt32());

  next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  ASSERT_TRUE(next->IsString());
  EXPECT_EQ("\\2", next->AsString());

  next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  EXPECT_TRUE(next->IsEOS());
}

TEST_F(TokenizerTest, ContinuationTokenInMiddleOfLine) {
  Tokenizer t("1 \\ 2");
  auto next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  ASSERT_TRUE(next->IsInteger());
  EXPECT_EQ(1, next->AsInt32());

  next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  ASSERT_TRUE(next->IsString());
  EXPECT_EQ("\\", next->AsString());

  next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  ASSERT_TRUE(next->IsInteger());
  EXPECT_EQ(2U, next->AsInt32());

  next = t.NextToken();
  ASSERT_TRUE(next != nullptr);
  EXPECT_TRUE(next->IsEOS());
}

TEST_F(TokenizerTest, ExtractToNext) {
  Tokenizer t("this\nis\na\ntest\nEND");

  auto next = t.NextToken();
  EXPECT_TRUE(next->IsString());
  EXPECT_EQ("this", next->AsString());

  std::string s = t.ExtractToNext("END");
  ASSERT_EQ("\nis\na\ntest\n", s);

  next = t.NextToken();
  EXPECT_TRUE(next->IsString());
  EXPECT_EQ("END", next->AsString());
  EXPECT_EQ(5U, t.GetCurrentLine());

  next = t.NextToken();
  EXPECT_TRUE(next->IsEOS());
}

TEST_F(TokenizerTest, ExtractToNextMissingNext) {
  Tokenizer t("this\nis\na\ntest\n");

  auto next = t.NextToken();
  EXPECT_TRUE(next->IsString());
  EXPECT_EQ("this", next->AsString());

  std::string s = t.ExtractToNext("END");
  ASSERT_EQ("\nis\na\ntest\n", s);

  next = t.NextToken();
  EXPECT_TRUE(next->IsEOS());
  EXPECT_EQ(5U, t.GetCurrentLine());
}

TEST_F(TokenizerTest, ExtractToNextCurrentIsNext) {
  Tokenizer t("END");
  std::string s = t.ExtractToNext("END");
  ASSERT_EQ("", s);

  auto next = t.NextToken();
  EXPECT_TRUE(next->IsString());
  EXPECT_EQ("END", next->AsString());

  next = t.NextToken();
  EXPECT_TRUE(next->IsEOS());
}

}  // namespace amber
