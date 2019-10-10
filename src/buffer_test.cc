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
#include "src/type_parser.h"

namespace amber {

using BufferTest = testing::Test;

TEST_F(BufferTest, EmptyByDefault) {
  Buffer b(BufferType::kColor);
  EXPECT_EQ(static_cast<size_t>(0U), b.ElementCount());
  EXPECT_EQ(static_cast<size_t>(0U), b.ValueCount());
  EXPECT_EQ(static_cast<size_t>(0U), b.GetSizeInBytes());
}

TEST_F(BufferTest, Size) {
  TypeParser parser;
  auto type = parser.Parse("R16_SINT");
  Format fmt(type.get());

  Buffer b(BufferType::kColor);
  b.SetFormat(&fmt);
  b.SetElementCount(10);
  EXPECT_EQ(10, b.ElementCount());
  EXPECT_EQ(10, b.ValueCount());
  EXPECT_EQ(10 * sizeof(int16_t), b.GetSizeInBytes());
}

TEST_F(BufferTest, SizeFromData) {
  std::vector<Value> values;
  values.resize(5);

  TypeParser parser;
  auto type = parser.Parse("R32_SFLOAT");
  Format fmt(type.get());

  Buffer b(BufferType::kColor);
  b.SetFormat(&fmt);
  b.SetData(std::move(values));

  EXPECT_EQ(5, b.ElementCount());
  EXPECT_EQ(5, b.ValueCount());
  EXPECT_EQ(5 * sizeof(float), b.GetSizeInBytes());
}

TEST_F(BufferTest, SizeFromDataDoesNotOverrideSize) {
  std::vector<Value> values;
  values.resize(5);

  TypeParser parser;
  auto type = parser.Parse("R32_SFLOAT");
  Format fmt(type.get());

  Buffer b(BufferType::kColor);
  b.SetFormat(&fmt);
  b.SetElementCount(20);
  b.SetData(std::move(values));

  EXPECT_EQ(20, b.ElementCount());
  EXPECT_EQ(20, b.ValueCount());
  EXPECT_EQ(20 * sizeof(float), b.GetSizeInBytes());
}

TEST_F(BufferTest, SizeMatrixStd430) {
  TypeParser parser;
  auto type = parser.Parse("R16G16_SINT");
  type->SetColumnCount(3);
  Format fmt(type.get());

  Buffer b(BufferType::kColor);
  b.SetFormat(&fmt);
  b.SetElementCount(10);

  EXPECT_EQ(10, b.ElementCount());
  EXPECT_EQ(60, b.ValueCount());
  EXPECT_EQ(60 * sizeof(int16_t), b.GetSizeInBytes());
}

TEST_F(BufferTest, SizeMatrixStd140) {
  TypeParser parser;
  auto type = parser.Parse("R16G16_SINT");
  type->SetColumnCount(3);
  Format fmt(type.get());
  fmt.SetLayout(Format::Layout::kStd140);

  Buffer b(BufferType::kColor);
  b.SetFormat(&fmt);
  b.SetElementCount(10);

  EXPECT_EQ(10, b.ElementCount());
  EXPECT_EQ(10 * 2 * 3, b.ValueCount());
  EXPECT_EQ(120 * sizeof(int16_t), b.GetSizeInBytes());
}

TEST_F(BufferTest, SizeMatrixPaddedStd430) {
  TypeParser parser;
  auto type = parser.Parse("R32G32B32_SINT");
  type->SetColumnCount(3);
  Format fmt(type.get());

  Buffer b(BufferType::kColor);
  b.SetFormat(&fmt);
  b.SetValueCount(9);

  EXPECT_EQ(1U, b.ElementCount());
  EXPECT_EQ(9U, b.ValueCount());
  EXPECT_EQ(12U * sizeof(int32_t), b.GetSizeInBytes());
}

}  // namespace amber
