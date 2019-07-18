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
#include "src/format_parser.h"
#include "src/make_unique.h"

namespace amber {

using FormatTest = testing::Test;

TEST_F(FormatTest, Copy) {
  Format fmt;
  fmt.SetIsStd140();
  fmt.SetColumnCount(1);
  fmt.SetFormatType(FormatType::kR32G32B32_SFLOAT);
  fmt.AddComponent(FormatComponentType::kR, FormatMode::kSFloat, 32);
  fmt.AddComponent(FormatComponentType::kG, FormatMode::kSFloat, 32);
  fmt.AddComponent(FormatComponentType::kB, FormatMode::kSFloat, 32);

  auto copy = MakeUnique<Format>(fmt);
  EXPECT_TRUE(copy->IsFloat());
  EXPECT_EQ(16U, copy->SizeInBytes());
  EXPECT_EQ(3U, copy->GetComponents().size());
  EXPECT_TRUE(copy->IsStd140());
  EXPECT_EQ(FormatType::kR32G32B32_SFLOAT, copy->GetFormatType());

  auto& comp = copy->GetComponents();
  EXPECT_EQ(FormatComponentType::kR, comp[0].type);
  EXPECT_EQ(FormatMode::kSFloat, comp[0].mode);
  EXPECT_EQ(32U, comp[0].num_bits);
  EXPECT_EQ(FormatComponentType::kG, comp[1].type);
  EXPECT_EQ(FormatMode::kSFloat, comp[1].mode);
  EXPECT_EQ(32U, comp[1].num_bits);
  EXPECT_EQ(FormatComponentType::kB, comp[2].type);
  EXPECT_EQ(FormatMode::kSFloat, comp[2].mode);
  EXPECT_EQ(32U, comp[2].num_bits);
}

TEST_F(FormatTest, SizeInBytesVector) {
  FormatParser fp;
  auto fmt = fp.Parse("R32G32B32_SFLOAT");
  ASSERT_TRUE(fmt != nullptr);

  EXPECT_EQ(3U, fmt->InputNeededPerElement());
  EXPECT_EQ(4U, fmt->ValuesPerElement());
  EXPECT_EQ(16U, fmt->SizeInBytes());
  EXPECT_EQ(16U, fmt->SizeInBytesPerRow());
}

TEST_F(FormatTest, SizeInBytesMatrix) {
  FormatParser fp;
  auto fmt = fp.Parse("R32G32B32_SFLOAT");
  ASSERT_TRUE(fmt != nullptr);
  fmt->SetColumnCount(3);

  EXPECT_EQ(9U, fmt->InputNeededPerElement());
  EXPECT_EQ(4U, fmt->ValuesPerRow());
  EXPECT_EQ(12U, fmt->ValuesPerElement());
  EXPECT_EQ(48U, fmt->SizeInBytes());
  EXPECT_EQ(16U, fmt->SizeInBytesPerRow());
}

TEST_F(FormatTest, SizeInBytesMatrixStd140) {
  FormatParser fp;
  auto fmt = fp.Parse("R32G32_SFLOAT");
  ASSERT_TRUE(fmt != nullptr);
  fmt->SetColumnCount(2);
  fmt->SetIsStd140();

  EXPECT_EQ(32U, fmt->SizeInBytes());
  EXPECT_EQ(16U, fmt->SizeInBytesPerRow());
}

TEST_F(FormatTest, RowCount) {
  FormatParser fp;
  auto fmt = fp.Parse("R32G32B32_SFLOAT");
  ASSERT_TRUE(fmt != nullptr);

  EXPECT_EQ(3U, fmt->RowCount());
}

struct StdData {
  const char* name;
  const char* fmt;
  uint32_t column_count;
  bool is_std140;
  uint32_t values_per_element;
  uint32_t size_in_bytes_per_row;
  uint32_t size_in_bytes;
};
using FormatStdTest = testing::TestWithParam<StdData>;
TEST_P(FormatStdTest, Test) {
  auto test_data = GetParam();

  FormatParser fp;
  auto fmt = fp.Parse(test_data.fmt);
  ASSERT_TRUE(fmt != nullptr) << test_data.name;

  fmt->SetColumnCount(test_data.column_count);
  if (test_data.is_std140)
    fmt->SetIsStd140();

  EXPECT_EQ(test_data.values_per_element, fmt->ValuesPerElement())
      << test_data.name;
  EXPECT_EQ(test_data.size_in_bytes, fmt->SizeInBytes()) << test_data.name;
  EXPECT_EQ(test_data.size_in_bytes_per_row, fmt->SizeInBytesPerRow())
      << test_data.name;
}

INSTANTIATE_TEST_SUITE_P(
    FormatStdTestSamples,
    FormatStdTest,
    testing::Values(
        StdData{"mat2x2-std140", "R32G32_SFLOAT", 2, true, 8U, 16U, 32U},
        StdData{"mat2x3-std140", "R32G32B32_SFLOAT", 2, true, 8U, 16U, 32U},
        StdData{"mat2x4-std140", "R32G32B32A32_SFLOAT", 2, true, 8U, 16U, 32U},
        StdData{"mat3x2-std140", "R32G32_SFLOAT", 3, true, 12U, 16U, 48U},
        StdData{"mat3x3-std140", "R32G32B32_SFLOAT", 3, true, 12U, 16U, 48U},
        StdData{"mat3x4-std140", "R32G32B32A32_SFLOAT", 3, true, 12U, 16U, 48U},
        StdData{"mat4x2-std140", "R32G32_SFLOAT", 4, true, 16U, 16U, 64U},
        StdData{"mat4x3-std140", "R32G32B32_SFLOAT", 4, true, 16U, 16U, 64U},
        StdData{"mat4x4-std140", "R32G32B32A32_SFLOAT", 4, true, 16U, 16U, 64U},
        StdData{"mat2x2-std430", "R32G32_SFLOAT", 2, false, 4U, 8U, 16U},
        StdData{"mat2x3-std430", "R32G32B32_SFLOAT", 2, false, 8U, 16U, 32U},
        StdData{"mat2x4-std430", "R32G32B32A32_SFLOAT", 2, false, 8U, 16U, 32U},
        StdData{"mat3x2-std430", "R32G32_SFLOAT", 3, false, 6U, 8U, 24U},
        StdData{"mat3x3-std430", "R32G32B32_SFLOAT", 3, false, 12U, 16U, 48U},
        StdData{"mat3x4-std430", "R32G32B32A32_SFLOAT", 3, false, 12U, 16U,
                48U},
        StdData{"mat4x2-std430", "R32G32_SFLOAT", 4, false, 8U, 8U, 32U},
        StdData{"mat4x3-std430", "R32G32B32_SFLOAT", 4, false, 16U, 16U, 64U},
        StdData{"mat4x4-std430", "R32G32B32A32_SFLOAT", 4, false, 16U, 16U,
                64U},
        StdData{"float-std140", "R32_SFLOAT", 1, true, 1U, 4U, 4U},
        StdData{"float-std430", "R32_SFLOAT", 1, false, 1U, 4U,
                4U}));  // NOLINT(whitespace/parens)

}  // namespace amber
