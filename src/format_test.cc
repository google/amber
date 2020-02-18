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
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "src/format.h"

#include "gtest/gtest.h"
#include "src/make_unique.h"
#include "src/type_parser.h"

namespace amber {

using FormatTest = testing::Test;

TEST_F(FormatTest, DefaultToStd430) {
  auto f32 = type::Number::Float(32);
  Format fmt(f32.get());
  EXPECT_EQ(Format::Layout::kStd430, fmt.GetLayout());
}

TEST_F(FormatTest, SizeInBytesVector_Std430) {
  TypeParser parser;
  auto type = parser.Parse("R32G32B32_SFLOAT");
  ASSERT_TRUE(type != nullptr);

  Format fmt(type.get());
  EXPECT_EQ(3U, fmt.InputNeededPerElement());
  EXPECT_EQ(16U, fmt.SizeInBytes());
}

TEST_F(FormatTest, SizeInBytesMatrix_Std430) {
  TypeParser parser;
  auto type = parser.Parse("R32G32B32_SFLOAT");
  ASSERT_TRUE(type != nullptr);
  type->SetColumnCount(3);

  Format fmt(type.get());
  EXPECT_EQ(9U, fmt.InputNeededPerElement());
  EXPECT_EQ(48U, fmt.SizeInBytes());
}

TEST_F(FormatTest, SizeInBytesMatrix_Std140) {
  TypeParser parser;
  auto type = parser.Parse("R32G32_SFLOAT");
  ASSERT_TRUE(type != nullptr);
  type->SetColumnCount(2);

  Format fmt(type.get());
  fmt.SetLayout(Format::Format::Layout::kStd140);
  EXPECT_EQ(32U, fmt.SizeInBytes());
}

struct StdData {
  const char* name;
  const char* fmt;
  uint32_t column_count;
  bool is_std140;
  uint32_t size_in_bytes;
};
using FormatStdTest = testing::TestWithParam<StdData>;
TEST_P(FormatStdTest, Test) {
  auto test_data = GetParam();

  TypeParser parser;
  auto type = parser.Parse(test_data.fmt);
  ASSERT_TRUE(type != nullptr) << test_data.name;

  type->SetColumnCount(test_data.column_count);

  Format fmt(type.get());
  if (test_data.is_std140)
    fmt.SetLayout(Format::Format::Layout::kStd140);

  EXPECT_EQ(test_data.size_in_bytes, fmt.SizeInBytes()) << test_data.name;
}

INSTANTIATE_TEST_SUITE_P(
    FormatStdTestSamples,
    FormatStdTest,
    testing::Values(
        StdData{"mat2x2-std140", "R32G32_SFLOAT", 2, true, 32U},
        StdData{"mat2x3-std140", "R32G32B32_SFLOAT", 2, true, 32U},
        StdData{"mat2x4-std140", "R32G32B32A32_SFLOAT", 2, true, 32U},
        StdData{"mat3x2-std140", "R32G32_SFLOAT", 3, true, 48U},
        StdData{"mat3x3-std140", "R32G32B32_SFLOAT", 3, true, 48U},
        StdData{"mat3x4-std140", "R32G32B32A32_SFLOAT", 3, true, 48U},
        StdData{"mat4x2-std140", "R32G32_SFLOAT", 4, true, 64U},
        StdData{"mat4x3-std140", "R32G32B32_SFLOAT", 4, true, 64U},
        StdData{"mat4x4-std140", "R32G32B32A32_SFLOAT", 4, true, 64U},
        StdData{"mat2x2-std430", "R32G32_SFLOAT", 2, false, 16U},
        StdData{"mat2x3-std430", "R32G32B32_SFLOAT", 2, false, 32U},
        StdData{"mat2x4-std430", "R32G32B32A32_SFLOAT", 2, false, 32U},
        StdData{"mat3x2-std430", "R32G32_SFLOAT", 3, false, 24U},
        StdData{"mat3x3-std430", "R32G32B32_SFLOAT", 3, false, 48U},
        StdData{"mat3x4-std430", "R32G32B32A32_SFLOAT", 3, false, 48U},
        StdData{"mat4x2-std430", "R32G32_SFLOAT", 4, false, 32U},
        StdData{"mat4x3-std430", "R32G32B32_SFLOAT", 4, false, 64U},
        StdData{"mat4x4-std430", "R32G32B32A32_SFLOAT", 4, false, 64U},
        StdData{"float-std140", "R32_SFLOAT", 1, true, 4U},
        StdData{"float-std430", "R32_SFLOAT", 1, false,
                4U}));  // NOLINT(whitespace/parens)

struct Name {
  const char* name;
};
using FormatNameTest = testing::TestWithParam<Name>;

TEST_P(FormatNameTest, Test) {
  auto test_data = GetParam();

  TypeParser parser;
  auto type = parser.Parse(test_data.name);
  ASSERT_TRUE(type != nullptr) << test_data.name;

  Format fmt(type.get());
  EXPECT_EQ(test_data.name, fmt.GenerateNameForTesting());
}
INSTANTIATE_TEST_SUITE_P(
    FormatNameGenerateTest,
    FormatNameTest,
    testing::Values(Name{"A1R5G5B5_UNORM_PACK16"},
                    Name{"A2B10G10R10_SINT_PACK32"},
                    Name{"A2B10G10R10_SNORM_PACK32"},
                    Name{"A2B10G10R10_SSCALED_PACK32"},
                    Name{"A2B10G10R10_UINT_PACK32"},
                    Name{"A2B10G10R10_UNORM_PACK32"},
                    Name{"A2B10G10R10_USCALED_PACK32"},
                    Name{"A2R10G10B10_SINT_PACK32"},
                    Name{"A2R10G10B10_SNORM_PACK32"},
                    Name{"A2R10G10B10_SSCALED_PACK32"},
                    Name{"A2R10G10B10_UINT_PACK32"},
                    Name{"A2R10G10B10_UNORM_PACK32"},
                    Name{"A2R10G10B10_USCALED_PACK32"},
                    Name{"A8B8G8R8_SINT_PACK32"},
                    Name{"A8B8G8R8_SNORM_PACK32"},
                    Name{"A8B8G8R8_SRGB_PACK32"},
                    Name{"A8B8G8R8_SSCALED_PACK32"},
                    Name{"A8B8G8R8_UINT_PACK32"},
                    Name{"A8B8G8R8_UNORM_PACK32"},
                    Name{"A8B8G8R8_USCALED_PACK32"},
                    Name{"B10G11R11_UFLOAT_PACK32"},
                    Name{"B4G4R4A4_UNORM_PACK16"},
                    Name{"B5G5R5A1_UNORM_PACK16"},
                    Name{"B5G6R5_UNORM_PACK16"},
                    Name{"B8G8R8A8_SINT"},
                    Name{"B8G8R8A8_SNORM"},
                    Name{"B8G8R8A8_SRGB"},
                    Name{"B8G8R8A8_SSCALED"},
                    Name{"B8G8R8A8_UINT"},
                    Name{"B8G8R8A8_UNORM"},
                    Name{"B8G8R8A8_USCALED"},
                    Name{"B8G8R8_SINT"},
                    Name{"B8G8R8_SNORM"},
                    Name{"B8G8R8_SRGB"},
                    Name{"B8G8R8_SSCALED"},
                    Name{"B8G8R8_UINT"},
                    Name{"B8G8R8_UNORM"},
                    Name{"B8G8R8_USCALED"},
                    Name{"D16_UNORM"},
                    Name{"D16_UNORM_S8_UINT"},
                    Name{"D24_UNORM_S8_UINT"},
                    Name{"D32_SFLOAT"},
                    Name{"D32_SFLOAT_S8_UINT"},
                    Name{"R16G16B16A16_SFLOAT"},
                    Name{"R16G16B16A16_SINT"},
                    Name{"R16G16B16A16_SNORM"},
                    Name{"R16G16B16A16_SSCALED"},
                    Name{"R16G16B16A16_UINT"},
                    Name{"R16G16B16A16_UNORM"},
                    Name{"R16G16B16A16_USCALED"},
                    Name{"R16G16B16_SFLOAT"},
                    Name{"R16G16B16_SINT"},
                    Name{"R16G16B16_SNORM"},
                    Name{"R16G16B16_SSCALED"},
                    Name{"R16G16B16_UINT"},
                    Name{"R16G16B16_UNORM"},
                    Name{"R16G16B16_USCALED"},
                    Name{"R16G16_SFLOAT"},
                    Name{"R16G16_SINT"},
                    Name{"R16G16_SNORM"},
                    Name{"R16G16_SSCALED"},
                    Name{"R16G16_UINT"},
                    Name{"R16G16_UNORM"},
                    Name{"R16G16_USCALED"},
                    Name{"R16_SFLOAT"},
                    Name{"R16_SINT"},
                    Name{"R16_SNORM"},
                    Name{"R16_SSCALED"},
                    Name{"R16_UINT"},
                    Name{"R16_UNORM"},
                    Name{"R16_USCALED"},
                    Name{"R32G32B32A32_SFLOAT"},
                    Name{"R32G32B32A32_SINT"},
                    Name{"R32G32B32A32_UINT"},
                    Name{"R32G32B32_SFLOAT"},
                    Name{"R32G32B32_SINT"},
                    Name{"R32G32B32_UINT"},
                    Name{"R32G32_SFLOAT"},
                    Name{"R32G32_SINT"},
                    Name{"R32G32_UINT"},
                    Name{"R32_SFLOAT"},
                    Name{"R32_SINT"},
                    Name{"R32_UINT"},
                    Name{"R4G4B4A4_UNORM_PACK16"},
                    Name{"R4G4_UNORM_PACK8"},
                    Name{"R5G5B5A1_UNORM_PACK16"},
                    Name{"R5G6B5_UNORM_PACK16"},
                    Name{"R64G64B64A64_SFLOAT"},
                    Name{"R64G64B64A64_SINT"},
                    Name{"R64G64B64A64_UINT"},
                    Name{"R64G64B64_SFLOAT"},
                    Name{"R64G64B64_SINT"},
                    Name{"R64G64B64_UINT"},
                    Name{"R64G64_SFLOAT"},
                    Name{"R64G64_SINT"},
                    Name{"R64G64_UINT"},
                    Name{"R64_SFLOAT"},
                    Name{"R64_SINT"},
                    Name{"R64_UINT"},
                    Name{"R8G8B8A8_SINT"},
                    Name{"R8G8B8A8_SNORM"},
                    Name{"R8G8B8A8_SRGB"},
                    Name{"R8G8B8A8_SSCALED"},
                    Name{"R8G8B8A8_UINT"},
                    Name{"R8G8B8A8_UNORM"},
                    Name{"R8G8B8A8_USCALED"},
                    Name{"R8G8B8_SINT"},
                    Name{"R8G8B8_SNORM"},
                    Name{"R8G8B8_SRGB"},
                    Name{"R8G8B8_SSCALED"},
                    Name{"R8G8B8_UINT"},
                    Name{"R8G8B8_UNORM"},
                    Name{"R8G8B8_USCALED"},
                    Name{"R8G8_SINT"},
                    Name{"R8G8_SNORM"},
                    Name{"R8G8_SRGB"},
                    Name{"R8G8_SSCALED"},
                    Name{"R8G8_UINT"},
                    Name{"R8G8_UNORM"},
                    Name{"R8G8_USCALED"},
                    Name{"R8_SINT"},
                    Name{"R8_SNORM"},
                    Name{"R8_SRGB"},
                    Name{"R8_SSCALED"},
                    Name{"R8_UINT"},
                    Name{"R8_UNORM"},
                    Name{"R8_USCALED"},
                    Name{"S8_UINT"},
                    Name{"X8_D24_UNORM_PACK32"}));  // NOLINT(whitespace/parens)

TEST_F(FormatTest, SegmentPackedList_Std430) {
  TypeParser parser;
  auto type = parser.Parse("A8B8G8R8_SINT_PACK32");

  Format fmt(type.get());
  EXPECT_EQ(4, fmt.SizeInBytes());

  const auto& segs = fmt.GetSegments();
  ASSERT_EQ(1U, segs.size());
  // Always packs into a unsigned ...
  EXPECT_EQ(FormatMode::kUInt, segs[0].GetFormatMode());
  EXPECT_EQ(4, segs[0].SizeInBytes());
}

TEST_F(FormatTest, SegmentListR32G32_Std140) {
  TypeParser parser;
  auto type = parser.Parse("R32G32_UINT");

  Format fmt(type.get());
  fmt.SetLayout(Format::Layout::kStd140);
  EXPECT_EQ(8, fmt.SizeInBytes());

  const auto& segs = fmt.GetSegments();
  ASSERT_EQ(2U, segs.size());
  EXPECT_EQ(FormatMode::kUInt, segs[0].GetFormatMode());
  EXPECT_EQ(4, segs[0].SizeInBytes());
  EXPECT_EQ(FormatMode::kUInt, segs[1].GetFormatMode());
  EXPECT_EQ(4, segs[1].SizeInBytes());
}

TEST_F(FormatTest, SegmentListR32G32B32_Std140) {
  TypeParser parser;
  auto type = parser.Parse("R32G32B32_UINT");

  Format fmt(type.get());
  fmt.SetLayout(Format::Layout::kStd140);
  EXPECT_EQ(16, fmt.SizeInBytes());

  const auto& segs = fmt.GetSegments();
  ASSERT_EQ(4U, segs.size());
  EXPECT_EQ(FormatMode::kUInt, segs[0].GetFormatMode());
  EXPECT_EQ(4, segs[0].SizeInBytes());
  EXPECT_EQ(FormatMode::kUInt, segs[1].GetFormatMode());
  EXPECT_EQ(4, segs[1].SizeInBytes());
  EXPECT_EQ(FormatMode::kUInt, segs[2].GetFormatMode());
  EXPECT_EQ(4, segs[2].SizeInBytes());
  EXPECT_TRUE(segs[3].IsPadding());
  EXPECT_EQ(4, segs[3].SizeInBytes());
}

TEST_F(FormatTest, SegmentListR32G32B32_Std430) {
  TypeParser parser;
  auto type = parser.Parse("R32G32B32_UINT");

  Format fmt(type.get());
  fmt.SetLayout(Format::Format::Layout::kStd430);
  EXPECT_EQ(16, fmt.SizeInBytes());

  const auto& segs = fmt.GetSegments();
  ASSERT_EQ(4U, segs.size());
  EXPECT_EQ(FormatMode::kUInt, segs[0].GetFormatMode());
  EXPECT_EQ(4, segs[0].SizeInBytes());
  EXPECT_EQ(FormatMode::kUInt, segs[1].GetFormatMode());
  EXPECT_EQ(4, segs[1].SizeInBytes());
  EXPECT_EQ(FormatMode::kUInt, segs[2].GetFormatMode());
  EXPECT_EQ(4, segs[2].SizeInBytes());
  EXPECT_TRUE(segs[3].IsPadding());
  EXPECT_EQ(4, segs[3].SizeInBytes());
}

TEST_F(FormatTest, SegmentMat2x2_Std140) {
  TypeParser parser;
  auto type = parser.Parse("R32G32_SFLOAT");
  type->SetColumnCount(2);

  Format fmt(type.get());
  fmt.SetLayout(Format::Format::Layout::kStd140);
  EXPECT_EQ(32, fmt.SizeInBytes());

  const auto& segs = fmt.GetSegments();
  ASSERT_EQ(6U, segs.size());
  EXPECT_EQ(FormatMode::kSFloat, segs[0].GetFormatMode());
  EXPECT_EQ(4, segs[0].SizeInBytes());
  EXPECT_EQ(FormatMode::kSFloat, segs[1].GetFormatMode());
  EXPECT_EQ(4, segs[1].SizeInBytes());
  EXPECT_TRUE(segs[2].IsPadding());
  EXPECT_EQ(8, segs[2].SizeInBytes());

  EXPECT_EQ(FormatMode::kSFloat, segs[3].GetFormatMode());
  EXPECT_EQ(4, segs[3].SizeInBytes());
  EXPECT_EQ(FormatMode::kSFloat, segs[4].GetFormatMode());
  EXPECT_EQ(4, segs[4].SizeInBytes());
  EXPECT_TRUE(segs[5].IsPadding());
  EXPECT_EQ(8, segs[5].SizeInBytes());
}

TEST_F(FormatTest, SegmentMat2x2_Std430) {
  TypeParser parser;
  auto type = parser.Parse("R32G32_SFLOAT");
  type->SetColumnCount(2);

  Format fmt(type.get());
  fmt.SetLayout(Format::Layout::kStd430);
  EXPECT_EQ(16, fmt.SizeInBytes());

  const auto& segs = fmt.GetSegments();
  ASSERT_EQ(4U, segs.size());
  EXPECT_EQ(FormatMode::kSFloat, segs[0].GetFormatMode());
  EXPECT_EQ(4, segs[0].SizeInBytes());
  EXPECT_EQ(FormatMode::kSFloat, segs[1].GetFormatMode());
  EXPECT_EQ(4, segs[1].SizeInBytes());

  EXPECT_EQ(FormatMode::kSFloat, segs[2].GetFormatMode());
  EXPECT_EQ(4, segs[2].SizeInBytes());
  EXPECT_EQ(FormatMode::kSFloat, segs[3].GetFormatMode());
  EXPECT_EQ(4, segs[3].SizeInBytes());
}

TEST_F(FormatTest, SegmentMat2x3_Std430) {
  TypeParser parser;
  auto type = parser.Parse("R32G32B32_SFLOAT");
  type->SetColumnCount(2);

  Format fmt(type.get());
  fmt.SetLayout(Format::Layout::kStd430);
  EXPECT_EQ(32, fmt.SizeInBytes());

  const auto& segs = fmt.GetSegments();
  ASSERT_EQ(8U, segs.size());
  EXPECT_EQ(FormatMode::kSFloat, segs[0].GetFormatMode());
  EXPECT_EQ(4, segs[0].SizeInBytes());
  EXPECT_EQ(FormatMode::kSFloat, segs[1].GetFormatMode());
  EXPECT_EQ(4, segs[1].SizeInBytes());
  EXPECT_EQ(FormatMode::kSFloat, segs[2].GetFormatMode());
  EXPECT_EQ(4, segs[2].SizeInBytes());
  EXPECT_TRUE(segs[3].IsPadding());
  EXPECT_EQ(4, segs[3].SizeInBytes());

  EXPECT_EQ(FormatMode::kSFloat, segs[4].GetFormatMode());
  EXPECT_EQ(4, segs[4].SizeInBytes());
  EXPECT_EQ(FormatMode::kSFloat, segs[5].GetFormatMode());
  EXPECT_EQ(4, segs[5].SizeInBytes());
  EXPECT_EQ(FormatMode::kSFloat, segs[6].GetFormatMode());
  EXPECT_EQ(4, segs[6].SizeInBytes());
  EXPECT_TRUE(segs[7].IsPadding());
  EXPECT_EQ(4, segs[7].SizeInBytes());
}

TEST_F(FormatTest, SegmentRuntimeArray_Std140) {
  TypeParser parser;
  auto type = parser.Parse("R32_SFLOAT");
  type->SetIsRuntimeArray();

  Format fmt(type.get());
  fmt.SetLayout(Format::Format::Layout::kStd140);
  EXPECT_EQ(16, fmt.SizeInBytes());

  const auto& segs = fmt.GetSegments();
  ASSERT_EQ(2U, segs.size());
  EXPECT_EQ(FormatMode::kSFloat, segs[0].GetFormatMode());
  EXPECT_EQ(4, segs[0].SizeInBytes());
  EXPECT_TRUE(segs[1].IsPadding());
  EXPECT_EQ(12, segs[1].SizeInBytes());
}

TEST_F(FormatTest, SegmentRuntimeArray_Std430) {
  TypeParser parser;
  auto type = parser.Parse("R32_SFLOAT");
  type->SetIsRuntimeArray();

  Format fmt(type.get());
  fmt.SetLayout(Format::Layout::kStd430);
  EXPECT_EQ(4, fmt.SizeInBytes());

  const auto& segs = fmt.GetSegments();
  ASSERT_EQ(1U, segs.size());
  EXPECT_EQ(FormatMode::kSFloat, segs[0].GetFormatMode());
  EXPECT_EQ(4, segs[0].SizeInBytes());
}

// struct {
//  float x;
//  int32 y;
// }
TEST_F(FormatTest, SegmentStruct_Std140) {
  auto s = MakeUnique<type::Struct>();
  auto f32 = type::Number::Float(32);
  auto u32 = type::Number::Uint(32);
  s->AddMember(f32.get());
  s->AddMember(u32.get());

  Format fmt(s.get());
  fmt.SetLayout(Format::Layout::kStd140);
  EXPECT_EQ(16, fmt.SizeInBytes());

  const auto& segs = fmt.GetSegments();
  ASSERT_EQ(3, segs.size());
  EXPECT_EQ(FormatMode::kSFloat, segs[0].GetFormatMode());
  EXPECT_EQ(4, segs[0].SizeInBytes());
  EXPECT_EQ(FormatMode::kUInt, segs[1].GetFormatMode());
  EXPECT_EQ(4, segs[1].SizeInBytes());
  EXPECT_TRUE(segs[2].IsPadding());
  EXPECT_EQ(8, segs[2].SizeInBytes());
}
TEST_F(FormatTest, SegmentStruct_Std430) {
  auto s = MakeUnique<type::Struct>();
  auto f32 = type::Number::Float(32);
  auto u32 = type::Number::Uint(32);
  s->AddMember(f32.get());
  s->AddMember(u32.get());

  Format fmt(s.get());
  fmt.SetLayout(Format::Layout::kStd430);
  EXPECT_EQ(8, fmt.SizeInBytes());

  const auto& segs = fmt.GetSegments();
  ASSERT_EQ(2U, segs.size());
  EXPECT_EQ(FormatMode::kSFloat, segs[0].GetFormatMode());
  EXPECT_EQ(4, segs[0].SizeInBytes());
  EXPECT_EQ(FormatMode::kUInt, segs[1].GetFormatMode());
  EXPECT_EQ(4, segs[1].SizeInBytes());
}

// struct STRIDE 20 {
//  float x;
//  int32 y;
// }
// Note, the STRIDE is the stride over the entire structure.
TEST_F(FormatTest, SegmentStructWithStride_Std140) {
  auto s = MakeUnique<type::Struct>();
  auto f32 = type::Number::Float(32);
  auto u32 = type::Number::Uint(32);
  s->AddMember(f32.get());
  s->AddMember(u32.get());
  s->SetStrideInBytes(20);

  Format fmt(s.get());
  fmt.SetLayout(Format::Layout::kStd140);
  EXPECT_EQ(20, fmt.SizeInBytes());

  const auto& segs = fmt.GetSegments();
  ASSERT_EQ(3U, segs.size());
  EXPECT_EQ(FormatMode::kSFloat, segs[0].GetFormatMode());
  EXPECT_EQ(4, segs[0].SizeInBytes());
  EXPECT_EQ(FormatMode::kUInt, segs[1].GetFormatMode());
  EXPECT_EQ(4, segs[1].SizeInBytes());
  EXPECT_TRUE(segs[2].IsPadding());
  EXPECT_EQ((20 - sizeof(float) - sizeof(uint32_t)), segs[2].SizeInBytes());
}
TEST_F(FormatTest, SegmentStructWithStride_Std430) {
  auto s = MakeUnique<type::Struct>();
  auto f32 = type::Number::Float(32);
  auto u32 = type::Number::Uint(32);
  s->AddMember(f32.get());
  s->AddMember(u32.get());
  s->SetStrideInBytes(20);

  Format fmt(s.get());
  fmt.SetLayout(Format::Layout::kStd430);
  EXPECT_EQ(20, fmt.SizeInBytes());

  const auto& segs = fmt.GetSegments();
  ASSERT_EQ(3U, segs.size());
  EXPECT_EQ(FormatMode::kSFloat, segs[0].GetFormatMode());
  EXPECT_EQ(32U, segs[0].GetNumBits());
  EXPECT_EQ(FormatMode::kUInt, segs[1].GetFormatMode());
  EXPECT_EQ(32U, segs[1].GetNumBits());
  EXPECT_TRUE(segs[2].IsPadding());
  EXPECT_EQ((20 - sizeof(float) - sizeof(uint32_t)) * 8, segs[2].GetNumBits());
}

// struct {
//  float x OFFSET 4;
//  int32 y;
// }
TEST_F(FormatTest, SegmentStructWithMemberOffset_Std140) {
  auto s = MakeUnique<type::Struct>();
  auto f32 = type::Number::Float(32);
  auto u32 = type::Number::Uint(32);

  auto m = s->AddMember(f32.get());
  m->offset_in_bytes = 4;

  s->AddMember(u32.get());

  Format fmt(s.get());
  fmt.SetLayout(Format::Layout::kStd140);
  EXPECT_EQ(16, fmt.SizeInBytes());

  const auto& segs = fmt.GetSegments();
  ASSERT_EQ(4U, segs.size());
  EXPECT_TRUE(segs[0].IsPadding());
  EXPECT_EQ(4, segs[0].SizeInBytes());
  EXPECT_EQ(FormatMode::kSFloat, segs[1].GetFormatMode());
  EXPECT_EQ(4, segs[1].SizeInBytes());
  EXPECT_EQ(FormatMode::kUInt, segs[2].GetFormatMode());
  EXPECT_EQ(4, segs[2].SizeInBytes());
  EXPECT_TRUE(segs[3].IsPadding());
  EXPECT_EQ(4, segs[3].SizeInBytes());
}
TEST_F(FormatTest, SegmentStructWithMemberOffset_Std430) {
  auto s = MakeUnique<type::Struct>();
  auto f32 = type::Number::Float(32);
  auto u32 = type::Number::Uint(32);

  auto m = s->AddMember(f32.get());
  m->offset_in_bytes = 4;

  s->AddMember(u32.get());

  Format fmt(s.get());
  fmt.SetLayout(Format::Layout::kStd430);
  EXPECT_EQ(12, fmt.SizeInBytes());

  const auto& segs = fmt.GetSegments();
  ASSERT_EQ(3U, segs.size());
  EXPECT_TRUE(segs[0].IsPadding());
  EXPECT_EQ(4, segs[0].SizeInBytes());
  EXPECT_EQ(FormatMode::kSFloat, segs[1].GetFormatMode());
  EXPECT_EQ(4, segs[1].SizeInBytes());
  EXPECT_EQ(FormatMode::kUInt, segs[2].GetFormatMode());
  EXPECT_EQ(4, segs[2].SizeInBytes());
}

// struct {
//   struct {
//     int32 a;
//     float b;
//   } x;
//   float y;
// }
TEST_F(FormatTest, SegmentStructWithStruct_Std140) {
  auto x = MakeUnique<type::Struct>();
  auto f32 = type::Number::Float(32);
  auto i32 = type::Number::Int(32);
  x->AddMember(i32.get());
  x->AddMember(f32.get());

  auto s = MakeUnique<type::Struct>();
  s->AddMember(x.get());
  s->AddMember(f32.get());

  Format fmt(s.get());
  fmt.SetLayout(Format::Layout::kStd140);
  EXPECT_EQ(32, fmt.SizeInBytes());

  const auto& segs = fmt.GetSegments();
  ASSERT_EQ(5U, segs.size());
  EXPECT_EQ(FormatMode::kSInt, segs[0].GetFormatMode());
  EXPECT_EQ(4, segs[0].SizeInBytes());
  EXPECT_EQ(FormatMode::kSFloat, segs[1].GetFormatMode());
  EXPECT_EQ(4, segs[1].SizeInBytes());
  EXPECT_TRUE(segs[2].IsPadding());
  EXPECT_EQ(8, segs[2].SizeInBytes());
  EXPECT_EQ(FormatMode::kSFloat, segs[3].GetFormatMode());
  EXPECT_EQ(4, segs[3].SizeInBytes());
  EXPECT_TRUE(segs[4].IsPadding());
  EXPECT_EQ(12, segs[4].SizeInBytes());
}
TEST_F(FormatTest, SegmentStructWithStruct_Std430) {
  auto x = MakeUnique<type::Struct>();
  auto f32 = type::Number::Float(32);
  auto i32 = type::Number::Int(32);
  x->AddMember(i32.get());
  x->AddMember(f32.get());

  auto s = MakeUnique<type::Struct>();
  s->AddMember(x.get());
  s->AddMember(f32.get());

  Format fmt(s.get());
  fmt.SetLayout(Format::Layout::kStd430);
  EXPECT_EQ(12, fmt.SizeInBytes());

  const auto& segs = fmt.GetSegments();
  ASSERT_EQ(3U, segs.size());
  EXPECT_EQ(FormatMode::kSInt, segs[0].GetFormatMode());
  EXPECT_EQ(4, segs[0].SizeInBytes());
  EXPECT_EQ(FormatMode::kSFloat, segs[1].GetFormatMode());
  EXPECT_EQ(4, segs[1].SizeInBytes());
  EXPECT_EQ(FormatMode::kSFloat, segs[2].GetFormatMode());
  EXPECT_EQ(4, segs[2].SizeInBytes());
}

// struct {
//   int32 w;
//   vec2<float> x;
//   float y;
// }
TEST_F(FormatTest, SegmentStructWithVec2_Std140) {
  auto s = MakeUnique<type::Struct>();
  auto f32 = type::Number::Float(32);
  auto i32 = type::Number::Int(32);
  auto vec2 = type::Number::Float(32);
  vec2->SetRowCount(2);

  s->AddMember(i32.get());
  s->AddMember(vec2.get());
  s->AddMember(f32.get());

  Format fmt(s.get());
  fmt.SetLayout(Format::Layout::kStd140);
  EXPECT_EQ(32, fmt.SizeInBytes());

  const auto& segs = fmt.GetSegments();
  ASSERT_EQ(6U, segs.size());
  /* w */
  EXPECT_EQ(FormatMode::kSInt, segs[0].GetFormatMode());
  EXPECT_EQ(4, segs[0].SizeInBytes());
  /* pad */
  EXPECT_TRUE(segs[1].IsPadding());
  EXPECT_EQ(4, segs[1].SizeInBytes());
  /* vec2 */
  EXPECT_EQ(FormatMode::kSFloat, segs[2].GetFormatMode());
  EXPECT_EQ(4, segs[2].SizeInBytes());
  EXPECT_EQ(FormatMode::kSFloat, segs[3].GetFormatMode());
  EXPECT_EQ(4, segs[3].SizeInBytes());
  /* y */
  EXPECT_EQ(FormatMode::kSFloat, segs[4].GetFormatMode());
  EXPECT_EQ(4, segs[4].SizeInBytes());
  /* pad */
  EXPECT_TRUE(segs[5].IsPadding());
  EXPECT_EQ(12, segs[5].SizeInBytes());
}
TEST_F(FormatTest, SegmentStructWithVec2_Std430) {
  auto s = MakeUnique<type::Struct>();
  auto f32 = type::Number::Float(32);
  auto i32 = type::Number::Int(32);
  auto vec2 = type::Number::Float(32);
  vec2->SetRowCount(2);

  s->AddMember(i32.get());
  s->AddMember(vec2.get());
  s->AddMember(f32.get());

  Format fmt(s.get());
  fmt.SetLayout(Format::Layout::kStd430);
  EXPECT_EQ(24, fmt.SizeInBytes());

  const auto& segs = fmt.GetSegments();
  ASSERT_EQ(6U, segs.size());
  /* w */
  EXPECT_EQ(FormatMode::kSInt, segs[0].GetFormatMode());
  EXPECT_EQ(4, segs[0].SizeInBytes());
  /* pad */
  EXPECT_TRUE(segs[1].IsPadding());
  EXPECT_EQ(4, segs[1].SizeInBytes());
  /* vec2 */
  EXPECT_EQ(FormatMode::kSFloat, segs[2].GetFormatMode());
  EXPECT_EQ(4, segs[2].SizeInBytes());
  EXPECT_EQ(FormatMode::kSFloat, segs[3].GetFormatMode());
  EXPECT_EQ(4, segs[3].SizeInBytes());
  /* y */
  EXPECT_EQ(FormatMode::kSFloat, segs[4].GetFormatMode());
  EXPECT_EQ(4, segs[4].SizeInBytes());
  /* pad */
  EXPECT_TRUE(segs[5].IsPadding());
  EXPECT_EQ(4, segs[5].SizeInBytes());
}

// struct {
//   int32 w;
//   vec3<float> x;
//   float y;
// }
TEST_F(FormatTest, SegmentStructWithFloatPackedToVec_Std140) {
  auto s = MakeUnique<type::Struct>();
  auto f32 = type::Number::Float(32);
  auto i32 = type::Number::Int(32);
  auto vec3 = type::Number::Float(32);
  vec3->SetRowCount(3);

  s->AddMember(i32.get());
  s->AddMember(vec3.get());
  s->AddMember(f32.get());

  Format fmt(s.get());
  fmt.SetLayout(Format::Layout::kStd140);
  EXPECT_EQ(32, fmt.SizeInBytes());

  const auto& segs = fmt.GetSegments();
  ASSERT_EQ(6U, segs.size());
  /* w */
  EXPECT_EQ(FormatMode::kSInt, segs[0].GetFormatMode());
  EXPECT_EQ(4, segs[0].SizeInBytes());
  /* pad */
  EXPECT_TRUE(segs[1].IsPadding());
  EXPECT_EQ(12, segs[1].SizeInBytes());
  /* vec2 */
  EXPECT_EQ(FormatMode::kSFloat, segs[2].GetFormatMode());
  EXPECT_EQ(4, segs[2].SizeInBytes());
  EXPECT_EQ(FormatMode::kSFloat, segs[3].GetFormatMode());
  EXPECT_EQ(4, segs[3].SizeInBytes());
  EXPECT_EQ(FormatMode::kSFloat, segs[4].GetFormatMode());
  EXPECT_EQ(4, segs[4].SizeInBytes());
  /* y */
  EXPECT_EQ(FormatMode::kSFloat, segs[5].GetFormatMode());
  EXPECT_EQ(4, segs[5].SizeInBytes());
}
TEST_F(FormatTest, SegmentStructWithFloatPackedToVec_Std430) {
  auto s = MakeUnique<type::Struct>();
  auto f32 = type::Number::Float(32);
  auto i32 = type::Number::Int(32);
  auto vec3 = type::Number::Float(32);
  vec3->SetRowCount(3);

  s->AddMember(i32.get());
  s->AddMember(vec3.get());
  s->AddMember(f32.get());

  Format fmt(s.get());
  fmt.SetLayout(Format::Layout::kStd430);
  EXPECT_EQ(32, fmt.SizeInBytes());

  const auto& segs = fmt.GetSegments();
  ASSERT_EQ(6U, segs.size());
  /* w */
  EXPECT_EQ(FormatMode::kSInt, segs[0].GetFormatMode());
  EXPECT_EQ(4, segs[0].SizeInBytes());
  /* pad */
  EXPECT_TRUE(segs[1].IsPadding());
  EXPECT_EQ(12, segs[1].SizeInBytes());
  /* vec2 */
  EXPECT_EQ(FormatMode::kSFloat, segs[2].GetFormatMode());
  EXPECT_EQ(4, segs[2].SizeInBytes());
  EXPECT_EQ(FormatMode::kSFloat, segs[3].GetFormatMode());
  EXPECT_EQ(4, segs[3].SizeInBytes());
  EXPECT_EQ(FormatMode::kSFloat, segs[4].GetFormatMode());
  EXPECT_EQ(4, segs[4].SizeInBytes());
  /* y */
  EXPECT_EQ(FormatMode::kSFloat, segs[5].GetFormatMode());
  EXPECT_EQ(4, segs[5].SizeInBytes());
}

// struct {
//   int32 w;
//   vec3<float> x;
//   vec2<float> y;
// }
TEST_F(FormatTest, SegmentStructVec3Vec2_Std140) {
  auto s = MakeUnique<type::Struct>();
  auto i32 = type::Number::Int(32);
  auto vec3 = type::Number::Float(32);
  vec3->SetRowCount(3);
  auto vec2 = type::Number::Float(32);
  vec2->SetRowCount(2);

  s->AddMember(i32.get());
  s->AddMember(vec3.get());
  s->AddMember(vec2.get());

  Format fmt(s.get());
  fmt.SetLayout(Format::Layout::kStd140);
  EXPECT_EQ(48, fmt.SizeInBytes());

  const auto& segs = fmt.GetSegments();
  ASSERT_EQ(9U, segs.size());
  /* w */
  EXPECT_EQ(FormatMode::kSInt, segs[0].GetFormatMode());
  EXPECT_EQ(4, segs[0].SizeInBytes());
  /* pad */
  EXPECT_TRUE(segs[1].IsPadding());
  EXPECT_EQ(12, segs[1].SizeInBytes());
  /* vec3 */
  EXPECT_EQ(FormatMode::kSFloat, segs[2].GetFormatMode());
  EXPECT_EQ(4, segs[2].SizeInBytes());
  EXPECT_EQ(FormatMode::kSFloat, segs[3].GetFormatMode());
  EXPECT_EQ(4, segs[3].SizeInBytes());
  EXPECT_EQ(FormatMode::kSFloat, segs[4].GetFormatMode());
  EXPECT_EQ(4, segs[4].SizeInBytes());
  /* pad */
  EXPECT_TRUE(segs[5].IsPadding());
  EXPECT_EQ(4, segs[5].SizeInBytes());
  /* vec2 */
  EXPECT_EQ(FormatMode::kSFloat, segs[6].GetFormatMode());
  EXPECT_EQ(4, segs[6].SizeInBytes());
  EXPECT_EQ(FormatMode::kSFloat, segs[7].GetFormatMode());
  EXPECT_EQ(4, segs[7].SizeInBytes());
  /* pad */
  EXPECT_TRUE(segs[8].IsPadding());
  EXPECT_EQ(8, segs[8].SizeInBytes());
}
TEST_F(FormatTest, SegmentStructVec3Vec2_Std430) {
  auto s = MakeUnique<type::Struct>();
  auto i32 = type::Number::Int(32);
  auto vec3 = type::Number::Float(32);
  vec3->SetRowCount(3);
  auto vec2 = type::Number::Float(32);
  vec2->SetRowCount(2);

  s->AddMember(i32.get());
  s->AddMember(vec3.get());
  s->AddMember(vec2.get());

  Format fmt(s.get());
  fmt.SetLayout(Format::Layout::kStd430);
  EXPECT_EQ(48, fmt.SizeInBytes());

  const auto& segs = fmt.GetSegments();
  ASSERT_EQ(9U, segs.size());
  /* w */
  EXPECT_EQ(FormatMode::kSInt, segs[0].GetFormatMode());
  EXPECT_EQ(4, segs[0].SizeInBytes());
  /* pad */
  EXPECT_TRUE(segs[1].IsPadding());
  EXPECT_EQ(12, segs[1].SizeInBytes());
  /* vec3 */
  EXPECT_EQ(FormatMode::kSFloat, segs[2].GetFormatMode());
  EXPECT_EQ(4, segs[2].SizeInBytes());
  EXPECT_EQ(FormatMode::kSFloat, segs[3].GetFormatMode());
  EXPECT_EQ(4, segs[3].SizeInBytes());
  EXPECT_EQ(FormatMode::kSFloat, segs[4].GetFormatMode());
  EXPECT_EQ(4, segs[4].SizeInBytes());
  /* pad */
  EXPECT_TRUE(segs[5].IsPadding());
  EXPECT_EQ(4, segs[5].SizeInBytes());
  /* vec2 */
  EXPECT_EQ(FormatMode::kSFloat, segs[6].GetFormatMode());
  EXPECT_EQ(4, segs[6].SizeInBytes());
  EXPECT_EQ(FormatMode::kSFloat, segs[7].GetFormatMode());
  EXPECT_EQ(4, segs[7].SizeInBytes());
  /* pad */
  EXPECT_TRUE(segs[8].IsPadding());
  EXPECT_EQ(8, segs[8].SizeInBytes());
}

// struct {
//   int32 w;
//   mat2x2<float> x;
//   float y;
// }
TEST_F(FormatTest, SegmentStructMat2x2_Std140) {
  auto s = MakeUnique<type::Struct>();
  auto f32 = type::Number::Float(32);
  auto i32 = type::Number::Int(32);
  auto mat2x2 = type::Number::Float(32);
  mat2x2->SetRowCount(2);
  mat2x2->SetColumnCount(2);

  s->AddMember(i32.get());
  s->AddMember(mat2x2.get());
  s->AddMember(f32.get());

  Format fmt(s.get());
  fmt.SetLayout(Format::Layout::kStd140);
  EXPECT_EQ(64, fmt.SizeInBytes());

  const auto& segs = fmt.GetSegments();
  ASSERT_EQ(10U, segs.size());
  /* w */
  EXPECT_EQ(FormatMode::kSInt, segs[0].GetFormatMode());
  EXPECT_EQ(4, segs[0].SizeInBytes());
  /* pad */
  EXPECT_TRUE(segs[1].IsPadding());
  EXPECT_EQ(12, segs[1].SizeInBytes());
  /* column 1 */
  EXPECT_EQ(FormatMode::kSFloat, segs[2].GetFormatMode());
  EXPECT_EQ(4, segs[2].SizeInBytes());
  EXPECT_EQ(FormatMode::kSFloat, segs[3].GetFormatMode());
  EXPECT_EQ(4, segs[3].SizeInBytes());
  EXPECT_TRUE(segs[4].IsPadding());
  EXPECT_EQ(8, segs[4].SizeInBytes());
  /* column 2 */
  EXPECT_EQ(FormatMode::kSFloat, segs[5].GetFormatMode());
  EXPECT_EQ(4, segs[5].SizeInBytes());
  EXPECT_EQ(FormatMode::kSFloat, segs[6].GetFormatMode());
  EXPECT_EQ(4, segs[6].SizeInBytes());
  EXPECT_TRUE(segs[7].IsPadding());
  EXPECT_EQ(8, segs[7].SizeInBytes());
  /* y */
  EXPECT_EQ(FormatMode::kSFloat, segs[8].GetFormatMode());
  EXPECT_EQ(4, segs[8].SizeInBytes());
  /* pad */
  EXPECT_TRUE(segs[9].IsPadding());
  EXPECT_EQ(12, segs[9].SizeInBytes());
}
TEST_F(FormatTest, SegmentStructMat2x2_Std430) {
  auto s = MakeUnique<type::Struct>();
  auto f32 = type::Number::Float(32);
  auto i32 = type::Number::Int(32);
  auto mat2x2 = type::Number::Float(32);
  mat2x2->SetRowCount(2);
  mat2x2->SetColumnCount(2);

  s->AddMember(i32.get());
  s->AddMember(mat2x2.get());
  s->AddMember(f32.get());

  Format fmt(s.get());
  fmt.SetLayout(Format::Layout::kStd430);
  EXPECT_EQ(32, fmt.SizeInBytes());

  const auto& segs = fmt.GetSegments();
  ASSERT_EQ(8U, segs.size());
  /* w */
  EXPECT_EQ(FormatMode::kSInt, segs[0].GetFormatMode());
  EXPECT_EQ(4, segs[0].SizeInBytes());
  /* pad */
  EXPECT_TRUE(segs[1].IsPadding());
  EXPECT_EQ(4, segs[1].SizeInBytes());
  /* column 1 */
  EXPECT_EQ(FormatMode::kSFloat, segs[2].GetFormatMode());
  EXPECT_EQ(4, segs[2].SizeInBytes());
  EXPECT_EQ(FormatMode::kSFloat, segs[3].GetFormatMode());
  EXPECT_EQ(4, segs[3].SizeInBytes());
  /* column 2 */
  EXPECT_EQ(FormatMode::kSFloat, segs[4].GetFormatMode());
  EXPECT_EQ(4, segs[4].SizeInBytes());
  EXPECT_EQ(FormatMode::kSFloat, segs[5].GetFormatMode());
  EXPECT_EQ(4, segs[5].SizeInBytes());
  /* y */
  EXPECT_EQ(FormatMode::kSFloat, segs[6].GetFormatMode());
  EXPECT_EQ(4, segs[6].SizeInBytes());
  /* pad */
  EXPECT_TRUE(segs[7].IsPadding());
  EXPECT_EQ(4, segs[7].SizeInBytes());
}

// struct {
//   int32 w;
//   struct {
//     int32 a;
//     int32 b;
//     float c;
//   } x;
//   float y;
// }
TEST_F(FormatTest, SegmentStructWithStructNoPack_Std140) {
  auto s = MakeUnique<type::Struct>();
  auto f32 = type::Number::Float(32);
  auto i32 = type::Number::Int(32);
  auto x = MakeUnique<type::Struct>();
  x->AddMember(i32.get());
  x->AddMember(i32.get());
  x->AddMember(f32.get());

  s->AddMember(i32.get());
  s->AddMember(x.get());
  s->AddMember(f32.get());

  Format fmt(s.get());
  fmt.SetLayout(Format::Layout::kStd140);
  EXPECT_EQ(48, fmt.SizeInBytes());

  const auto& segs = fmt.GetSegments();
  ASSERT_EQ(8U, segs.size());
  /* w */
  EXPECT_EQ(FormatMode::kSInt, segs[0].GetFormatMode());
  EXPECT_EQ(4, segs[0].SizeInBytes());
  /* pad */
  EXPECT_TRUE(segs[1].IsPadding());
  EXPECT_EQ(12, segs[1].SizeInBytes());
  /* a */
  EXPECT_EQ(FormatMode::kSInt, segs[2].GetFormatMode());
  EXPECT_EQ(4, segs[2].SizeInBytes());
  /* b */
  EXPECT_EQ(FormatMode::kSInt, segs[3].GetFormatMode());
  EXPECT_EQ(4, segs[3].SizeInBytes());
  /* c */
  EXPECT_EQ(FormatMode::kSFloat, segs[4].GetFormatMode());
  EXPECT_EQ(4, segs[4].SizeInBytes());
  /* pad */
  EXPECT_TRUE(segs[5].IsPadding());
  EXPECT_EQ(4, segs[5].SizeInBytes());
  /* y */
  EXPECT_EQ(FormatMode::kSFloat, segs[6].GetFormatMode());
  EXPECT_EQ(4, segs[6].SizeInBytes());
  /* pad */
  EXPECT_TRUE(segs[7].IsPadding());
  EXPECT_EQ(12, segs[7].SizeInBytes());
}
TEST_F(FormatTest, SegmentStructWithStructNoPack_Std430) {
  auto s = MakeUnique<type::Struct>();
  auto f32 = type::Number::Float(32);
  auto i32 = type::Number::Int(32);
  auto x = MakeUnique<type::Struct>();
  x->AddMember(i32.get());
  x->AddMember(i32.get());
  x->AddMember(f32.get());

  s->AddMember(i32.get());
  s->AddMember(x.get());
  s->AddMember(f32.get());

  Format fmt(s.get());
  fmt.SetLayout(Format::Layout::kStd430);
  EXPECT_EQ(20, fmt.SizeInBytes());

  const auto& segs = fmt.GetSegments();
  ASSERT_EQ(5U, segs.size());
  /* w */
  EXPECT_EQ(FormatMode::kSInt, segs[0].GetFormatMode());
  EXPECT_EQ(4, segs[0].SizeInBytes());
  /* a */
  EXPECT_EQ(FormatMode::kSInt, segs[1].GetFormatMode());
  EXPECT_EQ(4, segs[1].SizeInBytes());
  /* b */
  EXPECT_EQ(FormatMode::kSInt, segs[2].GetFormatMode());
  EXPECT_EQ(4, segs[2].SizeInBytes());
  /* c */
  EXPECT_EQ(FormatMode::kSFloat, segs[3].GetFormatMode());
  EXPECT_EQ(4, segs[3].SizeInBytes());
  /* y */
  EXPECT_EQ(FormatMode::kSFloat, segs[4].GetFormatMode());
  EXPECT_EQ(4, segs[4].SizeInBytes());
}

// struct {
//   int32 w;
//   struct {
//     int32 a;
//     int32 b;
//     float c[3];
//   } x;
//   float y;
// }
TEST_F(FormatTest, SegmentStructWithStructArray_Std140) {
  auto s = MakeUnique<type::Struct>();
  auto f32 = type::Number::Float(32);
  auto i32 = type::Number::Int(32);
  auto f32_ary = type::Number::Float(32);
  f32_ary->SetIsSizedArray(3);

  auto x = MakeUnique<type::Struct>();
  x->AddMember(i32.get());
  x->AddMember(i32.get());
  x->AddMember(f32_ary.get());

  s->AddMember(i32.get());
  s->AddMember(x.get());
  s->AddMember(f32.get());

  Format fmt(s.get());
  fmt.SetLayout(Format::Layout::kStd140);
  EXPECT_EQ(96, fmt.SizeInBytes());

  const auto& segs = fmt.GetSegments();
  ASSERT_EQ(13U, segs.size());
  /* w */
  EXPECT_EQ(FormatMode::kSInt, segs[0].GetFormatMode());
  EXPECT_EQ(4, segs[0].SizeInBytes());
  /* pad */
  EXPECT_TRUE(segs[1].IsPadding());
  EXPECT_EQ(12, segs[1].SizeInBytes());
  /* a */
  EXPECT_EQ(FormatMode::kSInt, segs[2].GetFormatMode());
  EXPECT_EQ(4, segs[2].SizeInBytes());
  /* b */
  EXPECT_EQ(FormatMode::kSInt, segs[3].GetFormatMode());
  EXPECT_EQ(4, segs[3].SizeInBytes());
  /* pad */
  EXPECT_TRUE(segs[4].IsPadding());
  EXPECT_EQ(8, segs[4].SizeInBytes());
  /* c[0] */
  EXPECT_EQ(FormatMode::kSFloat, segs[5].GetFormatMode());
  EXPECT_EQ(4, segs[5].SizeInBytes());
  /* pad */
  EXPECT_TRUE(segs[6].IsPadding());
  EXPECT_EQ(12, segs[6].SizeInBytes());
  /* c[1] */
  EXPECT_EQ(FormatMode::kSFloat, segs[7].GetFormatMode());
  EXPECT_EQ(4, segs[7].SizeInBytes());
  /* pad */
  EXPECT_TRUE(segs[8].IsPadding());
  EXPECT_EQ(12, segs[8].SizeInBytes());
  /* c[2] */
  EXPECT_EQ(FormatMode::kSFloat, segs[9].GetFormatMode());
  EXPECT_EQ(4, segs[9].SizeInBytes());
  /* pad */
  EXPECT_TRUE(segs[10].IsPadding());
  EXPECT_EQ(12, segs[10].SizeInBytes());
  /* y */
  EXPECT_EQ(FormatMode::kSFloat, segs[11].GetFormatMode());
  EXPECT_EQ(4, segs[11].SizeInBytes());
  /* pad */
  EXPECT_TRUE(segs[12].IsPadding());
  EXPECT_EQ(12, segs[12].SizeInBytes());
}
TEST_F(FormatTest, SegmentStructWithStructArray_Std430) {
  auto s = MakeUnique<type::Struct>();
  auto f32 = type::Number::Float(32);
  auto i32 = type::Number::Int(32);
  auto f32_ary = type::Number::Float(32);
  f32_ary->SetIsSizedArray(3);
  auto x = MakeUnique<type::Struct>();
  x->AddMember(i32.get());
  x->AddMember(i32.get());
  x->AddMember(f32_ary.get());

  s->AddMember(i32.get());
  s->AddMember(x.get());
  s->AddMember(f32.get());

  Format fmt(s.get());
  fmt.SetLayout(Format::Layout::kStd430);
  EXPECT_EQ(28, fmt.SizeInBytes());

  const auto& segs = fmt.GetSegments();
  ASSERT_EQ(7U, segs.size());
  /* w */
  EXPECT_EQ(FormatMode::kSInt, segs[0].GetFormatMode());
  EXPECT_EQ(4, segs[0].SizeInBytes());
  /* a */
  EXPECT_EQ(FormatMode::kSInt, segs[1].GetFormatMode());
  EXPECT_EQ(4, segs[1].SizeInBytes());
  /* b */
  EXPECT_EQ(FormatMode::kSInt, segs[2].GetFormatMode());
  EXPECT_EQ(4, segs[2].SizeInBytes());
  /* c[0] */
  EXPECT_EQ(FormatMode::kSFloat, segs[3].GetFormatMode());
  EXPECT_EQ(4, segs[3].SizeInBytes());
  /* c[1] */
  EXPECT_EQ(FormatMode::kSFloat, segs[4].GetFormatMode());
  EXPECT_EQ(4, segs[4].SizeInBytes());
  /* c[2] */
  EXPECT_EQ(FormatMode::kSFloat, segs[5].GetFormatMode());
  EXPECT_EQ(4, segs[5].SizeInBytes());
  /* y */
  EXPECT_EQ(FormatMode::kSFloat, segs[6].GetFormatMode());
  EXPECT_EQ(4, segs[6].SizeInBytes());
}

}  // namespace amber
