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

  EXPECT_EQ(12U, fmt->SizeInBytes());
  EXPECT_EQ(12U, fmt->SizeInBytesPerRow());
}

TEST_F(FormatTest, SizeInBytesMatrix) {
  FormatParser fp;
  auto fmt = fp.Parse("R32G32B32_SFLOAT");
  ASSERT_TRUE(fmt != nullptr);
  fmt->SetColumnCount(3);

  EXPECT_EQ(36U, fmt->SizeInBytes());
  EXPECT_EQ(12U, fmt->SizeInBytesPerRow());
}

TEST_F(FormatTest, RowCount) {
  FormatParser fp;
  auto fmt = fp.Parse("R32G32B32_SFLOAT");
  ASSERT_TRUE(fmt != nullptr);

  EXPECT_EQ(3U, fmt->RowCount());
}

}  // namespace amber
