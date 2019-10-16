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

TEST_F(AmberScriptParserTest, Struct) {
  std::string in = R"(
STRUCT my_struct
  uint8 first
  uint32 second
  vec3<float> third
  mat2x4<float> forth
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  auto type = script->GetType("my_struct");
  ASSERT_TRUE(type != nullptr);
  ASSERT_TRUE(type->IsStruct());

  auto s = type->AsStruct();
  EXPECT_FALSE(s->HasStride());

  const auto& m = s->Members();
  ASSERT_EQ(4U, m.size());
  for (size_t i = 0; i < 4; ++i) {
    ASSERT_TRUE(m[i].type->IsNumber()) << i;
    EXPECT_FALSE(m[i].HasOffset());
    EXPECT_FALSE(m[i].HasArrayStride());
    EXPECT_FALSE(m[i].HasMatrixStride());
  }

  EXPECT_TRUE(type::Type::IsUint8(m[0].type->AsNumber()->GetFormatMode(),
                                  m[0].type->AsNumber()->NumBits()));
  EXPECT_TRUE(type::Type::IsUint32(m[1].type->AsNumber()->GetFormatMode(),
                                   m[1].type->AsNumber()->NumBits()));

  EXPECT_TRUE(m[2].type->IsVec());
  EXPECT_EQ(3U, m[2].type->RowCount());
  EXPECT_TRUE(type::Type::IsFloat32(m[2].type->AsNumber()->GetFormatMode(),
                                    m[2].type->AsNumber()->NumBits()));

  EXPECT_TRUE(m[3].type->IsMatrix());
  EXPECT_EQ(4U, m[3].type->RowCount());
  EXPECT_EQ(2U, m[3].type->ColumnCount());
  EXPECT_TRUE(type::Type::IsFloat32(m[3].type->AsNumber()->GetFormatMode(),
                                    m[3].type->AsNumber()->NumBits()));
}

TEST_F(AmberScriptParserTest, StructWithDuplicateName) {
  std::string in = R"(
STRUCT my_struct
  uint8 first
END

STRUCT my_struct
  float second
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("6: duplicate type name provided", r.Error());
}

TEST_F(AmberScriptParserTest, StructWithStride) {
  std::string in = R"(
STRUCT my_struct STRIDE 20
  uint8 first
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  auto type = script->GetType("my_struct");
  ASSERT_TRUE(type != nullptr);
  ASSERT_TRUE(type->IsStruct());

  auto s = type->AsStruct();
  EXPECT_TRUE(s->HasStride());
  EXPECT_EQ(20U, s->StrideInBytes());
}

TEST_F(AmberScriptParserTest, StructMissingName) {
  std::string in = R"(
STRUCT
  uint8 first
END)";

  Parser parser;
  Result r = parser.Parse(in);
  EXPECT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: invalid STRUCT name provided", r.Error());
}

TEST_F(AmberScriptParserTest, StructMissingNameWithStride) {
  std::string in = R"(
STRUCT STRIDE 20
  uint8 first
END)";

  Parser parser;
  Result r = parser.Parse(in);
  EXPECT_FALSE(r.IsSuccess());
  EXPECT_EQ("2: missing STRUCT name", r.Error());
}

TEST_F(AmberScriptParserTest, StructInvalidName) {
  std::string in = R"(
STRUCT 1234 STRIDE 20
  uint8 first
END)";

  Parser parser;
  Result r = parser.Parse(in);
  EXPECT_FALSE(r.IsSuccess());
  EXPECT_EQ("2: invalid STRUCT name provided", r.Error());
}

TEST_F(AmberScriptParserTest, StructMissingStrideValue) {
  std::string in = R"(
STRUCT foo STRIDE
  uint8 first
END)";

  Parser parser;
  Result r = parser.Parse(in);
  EXPECT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: missing value for STRIDE", r.Error());
}

TEST_F(AmberScriptParserTest, StructInvalidStrideValue) {
  std::string in = R"(
STRUCT foo STRIDE abc
  uint8 first
END)";

  Parser parser;
  Result r = parser.Parse(in);
  EXPECT_FALSE(r.IsSuccess());
  EXPECT_EQ("2: invalid value for STRIDE", r.Error());
}

TEST_F(AmberScriptParserTest, StructMissingEND) {
  std::string in = R"(
STRUCT foo
  uint8 first
)";

  Parser parser;
  Result r = parser.Parse(in);
  EXPECT_FALSE(r.IsSuccess());
  EXPECT_EQ("4: invalid type for STRUCT member", r.Error());
}

TEST_F(AmberScriptParserTest, StructExtraParams) {
  std::string in = R"(
STRUCT foo STRIDE 20 BAR
  uint8 first
END)";

  Parser parser;
  Result r = parser.Parse(in);
  EXPECT_FALSE(r.IsSuccess());
  EXPECT_EQ("2: extra token BAR after STRUCT header", r.Error());
}

TEST_F(AmberScriptParserTest, StructMemberTypeInvalid) {
  std::string in = R"(
STRUCT foo
  123 first
END)";

  Parser parser;
  Result r = parser.Parse(in);
  EXPECT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: invalid type for STRUCT member", r.Error());
}

TEST_F(AmberScriptParserTest, StructMemberTypeUnknown) {
  std::string in = R"(
STRUCT foo
  uint99 first
END)";

  Parser parser;
  Result r = parser.Parse(in);
  EXPECT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: unknown type 'uint99' for STRUCT member", r.Error());
}

TEST_F(AmberScriptParserTest, StructMemberNameMissing) {
  std::string in = R"(
STRUCT foo
  uint8
END)";

  Parser parser;
  Result r = parser.Parse(in);
  EXPECT_FALSE(r.IsSuccess());
  EXPECT_EQ("4: missing name for STRUCT member", r.Error());
}

TEST_F(AmberScriptParserTest, StructMemberNameInvalid) {
  std::string in = R"(
STRUCT foo
  uint8 123
END)";

  Parser parser;
  Result r = parser.Parse(in);
  EXPECT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: invalid name for STRUCT member", r.Error());
}

TEST_F(AmberScriptParserTest, StructMemberNameDuplicate) {
  std::string in = R"(
STRUCT foo
  uint8 name
  uint8 name
END)";

  Parser parser;
  Result r = parser.Parse(in);
  EXPECT_FALSE(r.IsSuccess());
  EXPECT_EQ("4: duplicate name for STRUCT member", r.Error());
}

TEST_F(AmberScriptParserTest, StructWithEmbeddedStruct) {
  std::string in = R"(
STRUCT sub_struct
  uint8 first
END

STRUCT my_struct
  float second
  sub_struct third
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  auto type = script->GetType("my_struct");
  ASSERT_TRUE(type != nullptr);
  ASSERT_TRUE(type->IsStruct());

  auto s = type->AsStruct();
  EXPECT_FALSE(s->HasStride());

  const auto& m = s->Members();
  ASSERT_EQ(2U, m.size());

  EXPECT_TRUE(m[0].type->IsNumber());
  EXPECT_TRUE(m[1].type->IsStruct());
}

TEST_F(AmberScriptParserTest, StructDisallowsRecursiveInclusion) {
  std::string in = R"(
STRUCT my_struct
  float second
  my_struct third
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("4: recursive types are not allowed", r.Error());
}

TEST_F(AmberScriptParserTest, StructMemberWithOffset) {
  std::string in = R"(
STRUCT my_struct
  uint8 first OFFSET 20
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  auto type = script->GetType("my_struct");
  ASSERT_TRUE(type != nullptr);
  ASSERT_TRUE(type->IsStruct());

  auto s = type->AsStruct();
  EXPECT_FALSE(s->HasStride());

  const auto& m = s->Members();
  ASSERT_EQ(1U, m.size());
  EXPECT_TRUE(m[0].HasOffset());
  EXPECT_FALSE(m[0].HasArrayStride());
  EXPECT_FALSE(m[0].HasMatrixStride());
  EXPECT_EQ(20, m[0].offset_in_bytes);
}

TEST_F(AmberScriptParserTest, StructMemberOffsetMissingValue) {
  std::string in = R"(
STRUCT my_struct
  uint8 first OFFSET
END)";

  Parser parser;
  Result r = parser.Parse(in);
  EXPECT_FALSE(r.IsSuccess());
  EXPECT_EQ("4: missing value for STRUCT member OFFSET", r.Error());
}

TEST_F(AmberScriptParserTest, StructMemberOffsetInvalidValue) {
  std::string in = R"(
STRUCT my_struct
  uint8 first OFFSET abcd
END)";

  Parser parser;
  Result r = parser.Parse(in);
  EXPECT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: invalid value for STRUCT member OFFSET", r.Error());
}

TEST_F(AmberScriptParserTest, DISABLED_StructMemberWithArrayStride) {
  std::string in = R"(
STRUCT my_struct
  uint8 first[2] ARRAY_STRIDE 20
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  auto type = script->GetType("my_struct");
  ASSERT_TRUE(type != nullptr);
  ASSERT_TRUE(type->IsStruct());

  auto s = type->AsStruct();
  EXPECT_FALSE(s->HasStride());

  const auto& m = s->Members();
  ASSERT_EQ(1U, m.size());
  EXPECT_FALSE(m[0].HasOffset());
  EXPECT_TRUE(m[0].HasArrayStride());
  EXPECT_FALSE(m[0].HasMatrixStride());
  EXPECT_EQ(20, m[0].array_stride_in_bytes);
}

TEST_F(AmberScriptParserTest, StructMemberArrayStrideMissingValue) {
  std::string in = R"(
STRUCT my_struct
  uint8 first ARRAY_STRIDE
END)";

  Parser parser;
  Result r = parser.Parse(in);
  EXPECT_FALSE(r.IsSuccess());
  EXPECT_EQ("4: missing value for STRUCT member ARRAY_STRIDE", r.Error());
}

TEST_F(AmberScriptParserTest, StructMemberArrayStrideInvalidValue) {
  std::string in = R"(
STRUCT my_struct
  uint8 first ARRAY_STRIDE abcd
END)";

  Parser parser;
  Result r = parser.Parse(in);
  EXPECT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: invalid value for STRUCT member ARRAY_STRIDE", r.Error());
}

TEST_F(AmberScriptParserTest, StrictInvalidTypeWithArrayStride) {
  std::string in = R"(
STRUCT s
  uint32 a ARRAY_STRIDE 10
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: ARRAY_STRIDE only valid on array members", r.Error());
}

TEST_F(AmberScriptParserTest, StructMemberWithMatrixStride) {
  std::string in = R"(
STRUCT my_struct
  mat2x2<float> first MATRIX_STRIDE 20
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  auto type = script->GetType("my_struct");
  ASSERT_TRUE(type != nullptr);
  ASSERT_TRUE(type->IsStruct());

  auto s = type->AsStruct();
  EXPECT_FALSE(s->HasStride());

  const auto& m = s->Members();
  ASSERT_EQ(1U, m.size());
  EXPECT_FALSE(m[0].HasOffset());
  EXPECT_FALSE(m[0].HasArrayStride());
  EXPECT_TRUE(m[0].HasMatrixStride());
  EXPECT_EQ(20, m[0].matrix_stride_in_bytes);
}

TEST_F(AmberScriptParserTest, StructMemberMatrixStrideMissingValue) {
  std::string in = R"(
STRUCT my_struct
  mat2x2<float> first MATRIX_STRIDE
END)";

  Parser parser;
  Result r = parser.Parse(in);
  EXPECT_FALSE(r.IsSuccess());
  EXPECT_EQ("4: missing value for STRUCT member MATRIX_STRIDE", r.Error());
}

TEST_F(AmberScriptParserTest, StructMemberMatrixStrideInvalidValue) {
  std::string in = R"(
STRUCT my_struct
  mat2x2<float> first MATRIX_STRIDE abcd
END)";

  Parser parser;
  Result r = parser.Parse(in);
  EXPECT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: invalid value for STRUCT member MATRIX_STRIDE", r.Error());
}

TEST_F(AmberScriptParserTest, StructInvalidTypeWithMatrixStride) {
  std::string in = R"(
STRUCT s
  uint32 a MATRIX_STRIDE 10
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: MATRIX_STRIDE only valid on matrix members", r.Error());
}

TEST_F(AmberScriptParserTest, StructMemberExtraParam) {
  std::string in = R"(
STRUCT my_struct
  uint8 first 1234
END)";

  Parser parser;
  Result r = parser.Parse(in);
  EXPECT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: extra param for STRUCT member", r.Error());
}

TEST_F(AmberScriptParserTest, StructMemberUnknownParam) {
  std::string in = R"(
STRUCT my_struct
  uint8 first UNKNOWN
END)";

  Parser parser;
  Result r = parser.Parse(in);
  EXPECT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: unknown param 'UNKNOWN' for STRUCT member", r.Error());
}

}  // namespace amberscript
}  // namespace amber
