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

#include "src/amberscript/buffer.h"
#include "gtest/gtest.h"

namespace amber {
namespace amberscript {

using BufferTest = testing::Test;

TEST_F(BufferTest, BufferEmptyByDefault) {
  Buffer b(BufferType::kColor);
  EXPECT_EQ(static_cast<size_t>(0U), b.GetSize());
  EXPECT_EQ(static_cast<size_t>(0U), b.GetSizeInBytes());
}

TEST_F(BufferTest, BufferSize) {
  DatumType type;
  type.SetType(DataType::kInt16);

  Buffer b(BufferType::kColor);
  b.SetDatumType(type);
  b.SetSize(10);
  EXPECT_EQ(10, b.GetSize());
  EXPECT_EQ(2 * 10, b.GetSizeInBytes());
}

TEST_F(BufferTest, BufferSizeMatrix) {
  DatumType type;
  type.SetType(DataType::kInt16);
  type.SetRowCount(2);
  type.SetColumnCount(3);

  Buffer b(BufferType::kColor);
  b.SetDatumType(type);
  b.SetSize(10);
  EXPECT_EQ(10, b.GetSize());
  EXPECT_EQ(2 * 10 * 3 * 2, b.GetSizeInBytes());
}

}  // namespace amberscript
}  // namespace amber
