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

#include "src/vkscript/datum_type_parser.h"

#include "gtest/gtest.h"

namespace amber {
namespace vkscript {

using DatumTypeParserTest = testing::Test;

TEST_F(DatumTypeParserTest, EmptyType) {
  DatumTypeParser tp;
  Result r = tp.Parse("");
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("Invalid type provided: ", r.Error());
}

TEST_F(DatumTypeParserTest, InvalidType) {
  DatumTypeParser tp;
  Result r = tp.Parse("INVALID");
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("Invalid type provided: INVALID", r.Error());
}

struct DatumTypeData {
  const char* name;
  DataType type;
  uint32_t column_count;
  uint32_t row_count;
};
using DatumTypeDataTest = testing::TestWithParam<DatumTypeData>;

TEST_P(DatumTypeDataTest, Parser) {
  const auto& test_data = GetParam();

  DatumTypeParser tp;
  Result r = tp.Parse(test_data.name);
  const auto& t = tp.GetType();
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  EXPECT_EQ(test_data.type, t.GetType());
  EXPECT_EQ(test_data.column_count, t.ColumnCount());
  EXPECT_EQ(test_data.row_count, t.RowCount());
}

INSTANTIATE_TEST_CASE_P(
    DatumTypeParserTest1,
    DatumTypeDataTest,
    testing::Values(DatumTypeData{"int", DataType::kInt32, 1, 1},
                    DatumTypeData{"uint", DataType::kUint32, 1, 1},
                    DatumTypeData{"int8_t", DataType::kInt8, 1, 1},
                    DatumTypeData{"uint8_t", DataType::kUint8, 1, 1},
                    DatumTypeData{"int16_t", DataType::kInt16, 1, 1},
                    DatumTypeData{"uint16_t", DataType::kUint16, 1, 1},
                    DatumTypeData{"int64_t", DataType::kInt64, 1, 1},
                    DatumTypeData{"uint64_t", DataType::kUint64, 1, 1},
                    DatumTypeData{"float", DataType::kFloat, 1, 1},
                    DatumTypeData{"double", DataType::kDouble, 1, 1},
                    DatumTypeData{"vec2", DataType::kFloat, 1, 2},
                    DatumTypeData{"vec3", DataType::kFloat, 1, 3},
                    DatumTypeData{"vec4", DataType::kFloat, 1, 4},
                    DatumTypeData{"dvec2", DataType::kDouble, 1, 2},
                    DatumTypeData{"dvec3", DataType::kDouble, 1, 3},
                    DatumTypeData{"dvec4", DataType::kDouble, 1, 4},
                    DatumTypeData{"ivec2", DataType::kInt32, 1, 2},
                    DatumTypeData{"ivec3", DataType::kInt32, 1, 3},
                    DatumTypeData{"ivec4", DataType::kInt32, 1, 4},
                    DatumTypeData{"uvec2", DataType::kUint32, 1, 2},
                    DatumTypeData{"uvec3", DataType::kUint32, 1, 3},
                    DatumTypeData{"uvec4", DataType::kUint32, 1, 4},
                    DatumTypeData{"i8vec2", DataType::kInt8, 1, 2},
                    DatumTypeData{"i8vec3", DataType::kInt8, 1, 3},
                    DatumTypeData{"i8vec4", DataType::kInt8, 1, 4},
                    DatumTypeData{"u8vec2", DataType::kUint8, 1, 2},
                    DatumTypeData{"u8vec3", DataType::kUint8, 1, 3},
                    DatumTypeData{"u8vec4", DataType::kUint8, 1, 4},
                    DatumTypeData{"i16vec2", DataType::kInt16, 1,
                                  2}), );  // NOLINT(whitespace/parens)

INSTANTIATE_TEST_CASE_P(
    DatumTypeParserTest2,
    DatumTypeDataTest,
    testing::Values(DatumTypeData{"i16vec3", DataType::kInt16, 1, 3},
                    DatumTypeData{"i16vec4", DataType::kInt16, 1, 4},
                    DatumTypeData{"u16vec2", DataType::kUint16, 1, 2},
                    DatumTypeData{"u16vec3", DataType::kUint16, 1, 3},
                    DatumTypeData{"u16vec4", DataType::kUint16, 1, 4},
                    DatumTypeData{"i64vec2", DataType::kInt64, 1, 2},
                    DatumTypeData{"i64vec3", DataType::kInt64, 1, 3},
                    DatumTypeData{"i64vec4", DataType::kInt64, 1, 4},
                    DatumTypeData{"u64vec2", DataType::kUint64, 1, 2},
                    DatumTypeData{"u64vec3", DataType::kUint64, 1, 3},
                    DatumTypeData{"u64vec4", DataType::kUint64, 1, 4},
                    DatumTypeData{"mat2", DataType::kFloat, 2, 2},
                    DatumTypeData{"mat2x2", DataType::kFloat, 2, 2},
                    DatumTypeData{"mat2x3", DataType::kFloat, 2, 3},
                    DatumTypeData{"mat2x4", DataType::kFloat, 2, 4},
                    DatumTypeData{"mat3", DataType::kFloat, 3, 3},
                    DatumTypeData{"mat3x2", DataType::kFloat, 3, 2},
                    DatumTypeData{"mat3x3", DataType::kFloat, 3, 3},
                    DatumTypeData{"mat3x4", DataType::kFloat, 3, 4},
                    DatumTypeData{"mat4", DataType::kFloat, 4, 4},
                    DatumTypeData{"mat4x2", DataType::kFloat, 4, 2},
                    DatumTypeData{"mat4x3", DataType::kFloat, 4, 3},
                    DatumTypeData{"mat4x4", DataType::kFloat, 4, 4},
                    DatumTypeData{"dmat2", DataType::kDouble, 2, 2},
                    DatumTypeData{"dmat2x2", DataType::kDouble, 2, 2},
                    DatumTypeData{"dmat2x3", DataType::kDouble, 2, 3},
                    DatumTypeData{"dmat2x4", DataType::kDouble, 2, 4},
                    DatumTypeData{"dmat3", DataType::kDouble, 3, 3},
                    DatumTypeData{"dmat3x2", DataType::kDouble, 3, 2},
                    DatumTypeData{"dmat3x3", DataType::kDouble, 3, 3},
                    DatumTypeData{"dmat3x4", DataType::kDouble, 3, 4},
                    DatumTypeData{"dmat4", DataType::kDouble, 4, 4},
                    DatumTypeData{"dmat4x2", DataType::kDouble, 4, 2},
                    DatumTypeData{"dmat4x3", DataType::kDouble, 4, 3},
                    DatumTypeData{"dmat4x4", DataType::kDouble, 4,
                                  4}), );  // NOLINT(whitespace/parens)

}  // namespace vkscript
}  // namespace amber
