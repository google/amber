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
#include "src/format.h"

namespace amber {
namespace vkscript {
namespace {

bool AllCompsAreType(Format* fmt, FormatMode mode, uint8_t num_bits) {
  for (auto& seg : fmt->GetSegments()) {
    if (seg.IsPadding())
      continue;
    if (seg.GetNumBits() != num_bits)
      return false;
    if (seg.GetFormatMode() != mode)
      return false;
  }

  return true;
}

}  // namespace

using DatumTypeParserTest = testing::Test;

TEST_F(DatumTypeParserTest, EmptyType) {
  DatumTypeParser tp;
  auto type = tp.Parse("");
  ASSERT_TRUE(type == nullptr);
}

TEST_F(DatumTypeParserTest, InvalidType) {
  DatumTypeParser tp;
  auto type = tp.Parse("INVALID");
  ASSERT_TRUE(type == nullptr);
}

struct DatumTypeData {
  const char* name;
  FormatMode type;
  uint8_t num_bits;
  uint32_t column_count;
  uint32_t row_count;
};

using DatumTypeDataTest = testing::TestWithParam<DatumTypeData>;

TEST_P(DatumTypeDataTest, Parser) {
  const auto& test_data = GetParam();

  DatumTypeParser tp;
  auto type = tp.Parse(test_data.name);

  ASSERT_TRUE(type != nullptr);
  Format fmt(type.get());
  EXPECT_TRUE(AllCompsAreType(&fmt, test_data.type, test_data.num_bits));
  EXPECT_EQ(test_data.column_count, type->ColumnCount());
  EXPECT_EQ(test_data.row_count, type->RowCount());
}

INSTANTIATE_TEST_SUITE_P(
    DatumTypeParserTest1,
    DatumTypeDataTest,
    testing::Values(DatumTypeData{"int", FormatMode::kSInt, 32, 1, 1},
                    DatumTypeData{"uint", FormatMode::kUInt, 32, 1, 1},
                    DatumTypeData{"int8_t", FormatMode::kSInt, 8, 1, 1},
                    DatumTypeData{"uint8_t", FormatMode::kUInt, 8, 1, 1},
                    DatumTypeData{"int16_t", FormatMode::kSInt, 16, 1, 1},
                    DatumTypeData{"uint16_t", FormatMode::kUInt, 16, 1, 1},
                    DatumTypeData{"int64_t", FormatMode::kSInt, 64, 1, 1},
                    DatumTypeData{"uint64_t", FormatMode::kUInt, 64, 1, 1},
                    DatumTypeData{"float", FormatMode::kSFloat, 32, 1, 1},
                    DatumTypeData{"double", FormatMode::kSFloat, 64, 1, 1},
                    DatumTypeData{"vec2", FormatMode::kSFloat, 32, 1, 2},
                    DatumTypeData{"vec3", FormatMode::kSFloat, 32, 1, 3},
                    DatumTypeData{"vec4", FormatMode::kSFloat, 32, 1, 4},
                    DatumTypeData{"dvec2", FormatMode::kSFloat, 64, 1, 2},
                    DatumTypeData{"dvec3", FormatMode::kSFloat, 64, 1, 3},
                    DatumTypeData{"dvec4", FormatMode::kSFloat, 64, 1, 4},
                    DatumTypeData{"ivec2", FormatMode::kSInt, 32, 1, 2},
                    DatumTypeData{"ivec3", FormatMode::kSInt, 32, 1, 3},
                    DatumTypeData{"ivec4", FormatMode::kSInt, 32, 1, 4},
                    DatumTypeData{"uvec2", FormatMode::kUInt, 32, 1, 2},
                    DatumTypeData{"uvec3", FormatMode::kUInt, 32, 1, 3},
                    DatumTypeData{"uvec4", FormatMode::kUInt, 32, 1, 4},
                    DatumTypeData{"i8vec2", FormatMode::kSInt, 8, 1, 2},
                    DatumTypeData{"i8vec3", FormatMode::kSInt, 8, 1, 3},
                    DatumTypeData{"i8vec4", FormatMode::kSInt, 8, 1, 4},
                    DatumTypeData{"u8vec2", FormatMode::kUInt, 8, 1, 2},
                    DatumTypeData{"u8vec3", FormatMode::kUInt, 8, 1, 3},
                    DatumTypeData{"u8vec4", FormatMode::kUInt, 8, 1, 4},
                    DatumTypeData{"i16vec2", FormatMode::kSInt, 16, 1,
                                  2}));  // NOLINT(whitespace/parens)

INSTANTIATE_TEST_SUITE_P(
    DatumTypeParserTest2,
    DatumTypeDataTest,
    testing::Values(DatumTypeData{"i16vec3", FormatMode::kSInt, 16, 1, 3},
                    DatumTypeData{"i16vec4", FormatMode::kSInt, 16, 1, 4},
                    DatumTypeData{"u16vec2", FormatMode::kUInt, 16, 1, 2},
                    DatumTypeData{"u16vec3", FormatMode::kUInt, 16, 1, 3},
                    DatumTypeData{"u16vec4", FormatMode::kUInt, 16, 1, 4},
                    DatumTypeData{"i64vec2", FormatMode::kSInt, 64, 1, 2},
                    DatumTypeData{"i64vec3", FormatMode::kSInt, 64, 1, 3},
                    DatumTypeData{"i64vec4", FormatMode::kSInt, 64, 1, 4},
                    DatumTypeData{"u64vec2", FormatMode::kUInt, 64, 1, 2},
                    DatumTypeData{"u64vec3", FormatMode::kUInt, 64, 1, 3},
                    DatumTypeData{"u64vec4", FormatMode::kUInt, 64, 1, 4},
                    DatumTypeData{"mat2", FormatMode::kSFloat, 32, 2, 2},
                    DatumTypeData{"mat2x2", FormatMode::kSFloat, 32, 2, 2},
                    DatumTypeData{"mat2x3", FormatMode::kSFloat, 32, 2, 3},
                    DatumTypeData{"mat2x4", FormatMode::kSFloat, 32, 2, 4},
                    DatumTypeData{"mat3", FormatMode::kSFloat, 32, 3, 3},
                    DatumTypeData{"mat3x2", FormatMode::kSFloat, 32, 3, 2},
                    DatumTypeData{"mat3x3", FormatMode::kSFloat, 32, 3, 3},
                    DatumTypeData{"mat3x4", FormatMode::kSFloat, 32, 3, 4},
                    DatumTypeData{"mat4", FormatMode::kSFloat, 32, 4, 4},
                    DatumTypeData{"mat4x2", FormatMode::kSFloat, 32, 4, 2},
                    DatumTypeData{"mat4x3", FormatMode::kSFloat, 32, 4, 3},
                    DatumTypeData{"mat4x4", FormatMode::kSFloat, 32, 4, 4},
                    DatumTypeData{"dmat2", FormatMode::kSFloat, 64, 2, 2},
                    DatumTypeData{"dmat2x2", FormatMode::kSFloat, 64, 2, 2},
                    DatumTypeData{"dmat2x3", FormatMode::kSFloat, 64, 2, 3},
                    DatumTypeData{"dmat2x4", FormatMode::kSFloat, 64, 2, 4},
                    DatumTypeData{"dmat3", FormatMode::kSFloat, 64, 3, 3},
                    DatumTypeData{"dmat3x2", FormatMode::kSFloat, 64, 3, 2},
                    DatumTypeData{"dmat3x3", FormatMode::kSFloat, 64, 3, 3},
                    DatumTypeData{"dmat3x4", FormatMode::kSFloat, 64, 3, 4},
                    DatumTypeData{"dmat4", FormatMode::kSFloat, 64, 4, 4},
                    DatumTypeData{"dmat4x2", FormatMode::kSFloat, 64, 4, 2},
                    DatumTypeData{"dmat4x3", FormatMode::kSFloat, 64, 4, 3},
                    DatumTypeData{"dmat4x4", FormatMode::kSFloat, 64, 4,
                                  4}));  // NOLINT(whitespace/parens)

struct DatumFormatData {
  std::string name;
  FormatType format_type;
};

using DatumTypeTestFormat = testing::TestWithParam<DatumFormatData>;
TEST_P(DatumTypeTestFormat, ToFormat) {
  auto test_data = GetParam();

  DatumTypeParser tp;
  auto type = tp.Parse(test_data.name);

  ASSERT_TRUE(type != nullptr) << test_data.name;

  Format fmt(type.get());
  ASSERT_EQ(test_data.format_type, fmt.GetFormatType()) << test_data.name;
}

INSTANTIATE_TEST_SUITE_P(
    DatumTypeFormat1,
    DatumTypeTestFormat,
    testing::Values(
        DatumFormatData{"int", FormatType::kR32_SINT},
        DatumFormatData{"uint", FormatType::kR32_UINT},
        DatumFormatData{"int8_t", FormatType::kR8_SINT},
        DatumFormatData{"uint8_t", FormatType::kR8_UINT},
        DatumFormatData{"int16_t", FormatType::kR16_SINT},
        DatumFormatData{"uint16_t", FormatType::kR16_UINT},
        DatumFormatData{"int64_t", FormatType::kR64_SINT},
        DatumFormatData{"uint64_t", FormatType::kR64_UINT},
        DatumFormatData{"float", FormatType::kR32_SFLOAT},
        DatumFormatData{"double", FormatType::kR64_SFLOAT},
        DatumFormatData{"vec2", FormatType::kR32G32_SFLOAT},
        DatumFormatData{"vec3", FormatType::kR32G32B32_SFLOAT},
        DatumFormatData{"vec4", FormatType::kR32G32B32A32_SFLOAT},
        DatumFormatData{"dvec2", FormatType::kR64G64_SFLOAT},
        DatumFormatData{"dvec3", FormatType::kR64G64B64_SFLOAT},
        DatumFormatData{"dvec4", FormatType::kR64G64B64A64_SFLOAT},
        DatumFormatData{"ivec2", FormatType::kR32G32_SINT},
        DatumFormatData{"ivec3", FormatType::kR32G32B32_SINT},
        DatumFormatData{"ivec4", FormatType::kR32G32B32A32_SINT},
        DatumFormatData{"uvec2", FormatType::kR32G32_UINT},
        DatumFormatData{"uvec3", FormatType::kR32G32B32_UINT},
        DatumFormatData{"uvec4", FormatType::kR32G32B32A32_UINT},
        DatumFormatData{"i8vec2", FormatType::kR8G8_SINT},
        DatumFormatData{"i8vec3", FormatType::kR8G8B8_SINT},
        DatumFormatData{"i8vec4", FormatType::kR8G8B8A8_SINT},
        DatumFormatData{"u8vec2", FormatType::kR8G8_UINT},
        DatumFormatData{"u8vec3", FormatType::kR8G8B8_UINT},
        DatumFormatData{"u8vec4", FormatType::kR8G8B8A8_UINT},
        DatumFormatData{
            "i16vec2",
            FormatType::kR16G16_SINT}));  // NOLINT(whitespace/parens)

INSTANTIATE_TEST_SUITE_P(
    DatumTypeFormat2,
    DatumTypeTestFormat,
    testing::Values(DatumFormatData{"i16vec3", FormatType::kR16G16B16_SINT},
                    DatumFormatData{"i16vec4", FormatType::kR16G16B16A16_SINT},
                    DatumFormatData{"u16vec2", FormatType::kR16G16_UINT},
                    DatumFormatData{"u16vec3", FormatType::kR16G16B16_UINT},
                    DatumFormatData{"u16vec4", FormatType::kR16G16B16A16_UINT},
                    DatumFormatData{"i64vec2", FormatType::kR64G64_SINT},
                    DatumFormatData{"i64vec3", FormatType::kR64G64B64_SINT},
                    DatumFormatData{"i64vec4", FormatType::kR64G64B64A64_SINT},
                    DatumFormatData{"u64vec2", FormatType::kR64G64_UINT},
                    DatumFormatData{"u64vec3", FormatType::kR64G64B64_UINT},
                    DatumFormatData{"u64vec4", FormatType::kR64G64B64A64_UINT},
                    DatumFormatData{"mat2", FormatType::kUnknown},
                    DatumFormatData{"mat2x2", FormatType::kUnknown},
                    DatumFormatData{"mat2x3", FormatType::kUnknown},
                    DatumFormatData{"mat2x4", FormatType::kUnknown},
                    DatumFormatData{"mat3", FormatType::kUnknown},
                    DatumFormatData{"mat3x2", FormatType::kUnknown},
                    DatumFormatData{"mat3x3", FormatType::kUnknown},
                    DatumFormatData{"mat3x4", FormatType::kUnknown},
                    DatumFormatData{"mat4", FormatType::kUnknown},
                    DatumFormatData{"mat4x2", FormatType::kUnknown},
                    DatumFormatData{"mat4x3", FormatType::kUnknown},
                    DatumFormatData{"mat4x4", FormatType::kUnknown},
                    DatumFormatData{"dmat2", FormatType::kUnknown},
                    DatumFormatData{"dmat2x2", FormatType::kUnknown},
                    DatumFormatData{"dmat2x3", FormatType::kUnknown},
                    DatumFormatData{"dmat2x4", FormatType::kUnknown},
                    DatumFormatData{"dmat3", FormatType::kUnknown},
                    DatumFormatData{"dmat3x2", FormatType::kUnknown},
                    DatumFormatData{"dmat3x3", FormatType::kUnknown},
                    DatumFormatData{"dmat3x4", FormatType::kUnknown},
                    DatumFormatData{"dmat4", FormatType::kUnknown},
                    DatumFormatData{"dmat4x2", FormatType::kUnknown},
                    DatumFormatData{"dmat4x3", FormatType::kUnknown},
                    DatumFormatData{
                        "dmat4x4",
                        FormatType::kUnknown}));  // NOLINT(whitespace/parens)

}  // namespace vkscript
}  // namespace amber
