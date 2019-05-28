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

#include "src/buffer.h"

#include <utility>

#include "gtest/gtest.h"
#include "src/format_parser.h"

namespace amber {

using BufferTest = testing::Test;

TEST_F(BufferTest, EmptyByDefault) {
  Buffer b(BufferType::kColor);
  EXPECT_EQ(static_cast<size_t>(0U), b.ElementCount());
  EXPECT_EQ(static_cast<size_t>(0U), b.ValueCount());
  EXPECT_EQ(static_cast<size_t>(0U), b.GetSizeInBytes());
}

TEST_F(BufferTest, Size) {
  FormatParser fp;
  Buffer b(BufferType::kColor);
  b.SetFormat(fp.Parse("R16_SINT"));
  b.SetElementCount(10);
  EXPECT_EQ(10, b.ElementCount());
  EXPECT_EQ(10, b.ValueCount());
  EXPECT_EQ(10 * sizeof(int16_t), b.GetSizeInBytes());
}

TEST_F(BufferTest, SizeFromData) {
  std::vector<Value> values;
  values.resize(5);

  FormatParser fp;
  Buffer b(BufferType::kColor);
  b.SetFormat(fp.Parse("R32_SFLOAT"));
  b.SetData(std::move(values));

  EXPECT_EQ(5, b.ElementCount());
  EXPECT_EQ(5, b.ValueCount());
  EXPECT_EQ(5 * sizeof(float), b.GetSizeInBytes());
}

TEST_F(BufferTest, SizeFromDataDoesNotOverrideSize) {
  std::vector<Value> values;
  values.resize(5);

  FormatParser fp;
  Buffer b(BufferType::kColor);
  b.SetFormat(fp.Parse("R32_SFLOAT"));
  b.SetElementCount(20);
  b.SetData(std::move(values));

  EXPECT_EQ(20, b.ElementCount());
  EXPECT_EQ(20, b.ValueCount());
  EXPECT_EQ(20 * sizeof(float), b.GetSizeInBytes());
}

TEST_F(BufferTest, SizeMatrix) {
  FormatParser fp;
  auto fmt = fp.Parse("R16G16_SINT");
  fmt->SetColumnCount(3);

  Buffer b(BufferType::kColor);
  b.SetFormat(std::move(fmt));
  b.SetElementCount(10);

  EXPECT_EQ(10, b.ElementCount());
  EXPECT_EQ(60, b.ValueCount());
  EXPECT_EQ(60 * sizeof(int16_t), b.GetSizeInBytes());
}

TEST_F(BufferTest, SizeMatrixPadded) {
  FormatParser fp;
  auto fmt = fp.Parse("R32G32B32_SINT");
  fmt->SetColumnCount(3);

  Buffer b(BufferType::kColor);
  b.SetFormat(std::move(fmt));
  b.SetValueCount(9);

  EXPECT_EQ(1U, b.ElementCount());
  EXPECT_EQ(12U, b.ValueCount());
  EXPECT_EQ(12U * sizeof(int32_t), b.GetSizeInBytes());
}

}  // namespace amber
