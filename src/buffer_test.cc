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

#include <limits>
#include <utility>

#include "gtest/gtest.h"
#include "src/float16_helper.h"
#include "src/type_parser.h"

namespace amber {

using BufferTest = testing::Test;

TEST_F(BufferTest, EmptyByDefault) {
  Buffer b;
  EXPECT_EQ(static_cast<size_t>(0U), b.ElementCount());
  EXPECT_EQ(static_cast<size_t>(0U), b.ValueCount());
  EXPECT_EQ(static_cast<size_t>(0U), b.GetSizeInBytes());
}

TEST_F(BufferTest, Size) {
  TypeParser parser;
  auto type = parser.Parse("R16_SINT");
  Format fmt(type.get());

  Buffer b;
  b.SetFormat(&fmt);
  b.SetElementCount(10);
  EXPECT_EQ(10u, b.ElementCount());
  EXPECT_EQ(10u, b.ValueCount());
  EXPECT_EQ(10u * sizeof(int16_t), b.GetSizeInBytes());
}

TEST_F(BufferTest, SizeFromData) {
  std::vector<Value> values;
  values.resize(5);

  TypeParser parser;
  auto type = parser.Parse("R32_SFLOAT");
  Format fmt(type.get());

  Buffer b;
  b.SetFormat(&fmt);
  b.SetData(std::move(values));

  EXPECT_EQ(5u, b.ElementCount());
  EXPECT_EQ(5u, b.ValueCount());
  EXPECT_EQ(5u * sizeof(float), b.GetSizeInBytes());
}

TEST_F(BufferTest, SizeFromDataDoesNotOverrideSize) {
  std::vector<Value> values;
  values.resize(5);

  TypeParser parser;
  auto type = parser.Parse("R32_SFLOAT");
  Format fmt(type.get());

  Buffer b;
  b.SetFormat(&fmt);
  b.SetElementCount(20);
  b.SetData(std::move(values));

  EXPECT_EQ(20u, b.ElementCount());
  EXPECT_EQ(20u, b.ValueCount());
  EXPECT_EQ(20u * sizeof(float), b.GetSizeInBytes());
}

TEST_F(BufferTest, SizeMatrixStd430) {
  TypeParser parser;
  auto type = parser.Parse("R16G16_SINT");
  type->SetColumnCount(3);
  Format fmt(type.get());

  Buffer b;
  b.SetFormat(&fmt);
  b.SetElementCount(10);

  EXPECT_EQ(10u, b.ElementCount());
  EXPECT_EQ(60u, b.ValueCount());
  EXPECT_EQ(60u * sizeof(int16_t), b.GetSizeInBytes());
}

TEST_F(BufferTest, SizeMatrixStd140) {
  TypeParser parser;
  auto type = parser.Parse("R16G16_SINT");
  type->SetColumnCount(3);
  Format fmt(type.get());
  fmt.SetLayout(Format::Layout::kStd140);

  Buffer b;
  b.SetFormat(&fmt);
  b.SetElementCount(10);

  EXPECT_EQ(10u, b.ElementCount());
  EXPECT_EQ(10u * 2u * 3u, b.ValueCount());
  EXPECT_EQ(120u * sizeof(int16_t), b.GetSizeInBytes());
}

TEST_F(BufferTest, SizeMatrixPaddedStd430) {
  TypeParser parser;
  auto type = parser.Parse("R32G32B32_SINT");
  type->SetColumnCount(3);
  Format fmt(type.get());

  Buffer b;
  b.SetFormat(&fmt);
  b.SetValueCount(9);

  EXPECT_EQ(1U, b.ElementCount());
  EXPECT_EQ(9U, b.ValueCount());
  EXPECT_EQ(12U * sizeof(int32_t), b.GetSizeInBytes());
}

// Creates 10 RGBA pixel values, with the blue channels ranging from 0 to 255,
// and checks that the bin for each blue channel value contains 1, as expected.
TEST_F(BufferTest, GetHistogramForChannelGradient) {
  TypeParser parser;
  auto type = parser.Parse("R8G8B8A8_UINT");
  Format fmt(type.get());

  // Creates 10 RBGA pixel values with the blue channels ranging from 0 to 255.
  // Every value gets multiplied by 25 to create a gradient
  std::vector<Value> values(40);
  for (uint32_t i = 0; i < values.size(); i += 4)
    values[i + 2].SetIntValue(i / 4 * 25);

  Buffer b;
  b.SetFormat(&fmt);
  b.SetData(values);

  std::vector<uint64_t> bins = b.GetHistogramForChannel(2, 256);
  for (uint32_t i = 0; i < values.size(); i += 4)
    EXPECT_EQ(1u, bins[i / 4 * 25]);
}

// Creates 10 RGBA pixel values, with all channels being 0, and checks that all
// channels have a count of 10 (all pixels) in the 0 bin.
TEST_F(BufferTest, GetHistogramForChannelAllBlack) {
  TypeParser parser;
  auto type = parser.Parse("R8G8B8A8_UINT");
  Format fmt(type.get());

  std::vector<Value> values(40);
  for (uint32_t i = 0; i < values.size(); i++)
    values[i].SetIntValue(0);

  Buffer b;
  b.SetFormat(&fmt);
  b.SetData(values);

  for (uint8_t i = 0; i < 4; i++) {
    std::vector<uint64_t> bins = b.GetHistogramForChannel(i, 256);
    for (uint32_t y = 0; y < values.size(); y++)
      EXPECT_EQ(10u, bins[0]);
  }
}

// Creates 10 RGBA pixel values, with all channels being the maximum value of 8
// bit uint, and checks that all channels have a count of 10 (all pixels) in the
// 255 (max uint8_t) bin.
TEST_F(BufferTest, GetHistogramForChannelAllWhite) {
  TypeParser parser;
  auto type = parser.Parse("R8G8B8A8_UINT");
  Format fmt(type.get());

  std::vector<Value> values(40);
  for (uint32_t i = 0; i < values.size(); i++)
    values[i].SetIntValue(std::numeric_limits<uint8_t>::max());

  Buffer b;
  b.SetFormat(&fmt);
  b.SetData(values);

  for (uint8_t i = 0; i < 4; i++) {
    std::vector<uint64_t> bins = b.GetHistogramForChannel(i, 256);
    for (uint32_t y = 0; y < values.size(); y++)
      EXPECT_EQ(10u, bins[255]);
  }
}

// Creates two sets of equal pixel values, except for one pixel that has +50 in
// its red channel. Compares the histograms to see if they are equal with a low
// threshold, which we expect to fail.
TEST_F(BufferTest, CompareHistogramEMDToleranceFalse) {
  TypeParser parser;
  auto type = parser.Parse("R8G8B8A8_UINT");
  Format fmt(type.get());

  // Every value gets multiplied by 25 to create a gradient
  std::vector<Value> values1(40);
  for (uint32_t i = 0; i < values1.size(); i += 4)
    values1[i].SetIntValue(i / 4 * 25);

  std::vector<Value> values2 = values1;
  values2[4].SetIntValue(values2[4].AsUint8() + 50);

  Buffer b1;
  b1.SetFormat(&fmt);
  b1.SetData(values1);

  Buffer b2;
  b2.SetFormat(&fmt);
  b2.SetData(values2);

  EXPECT_FALSE(b1.CompareHistogramEMD(&b2, 0.001f).IsSuccess());
}

// Creates two sets of equal pixel values, except for one pixel that has +50 in
// its red channel. Compares the histograms to see if they are equal with a high
// threshold, which we expect to succeed.
TEST_F(BufferTest, CompareHistogramEMDToleranceTrue) {
  TypeParser parser;
  auto type = parser.Parse("R8G8B8A8_UINT");
  Format fmt(type.get());

  // Every value gets multiplied by 25 to create a gradient
  std::vector<Value> values1(40);
  for (uint32_t i = 0; i < values1.size(); i += 4)
    values1[i].SetIntValue(i / 4 * 25);

  std::vector<Value> values2 = values1;
  values2[4].SetIntValue(values2[4].AsUint8() + 50);

  Buffer b1;
  b1.SetFormat(&fmt);
  b1.SetData(values1);

  Buffer b2;
  b2.SetFormat(&fmt);
  b2.SetData(values2);

  EXPECT_TRUE(b1.CompareHistogramEMD(&b2, 0.02f).IsSuccess());
}

// Creates two identical sets of RGBA pixel values and checks that the
// histograms are equal.
TEST_F(BufferTest, CompareHistogramEMDToleranceAllBlack) {
  TypeParser parser;
  auto type = parser.Parse("R8G8B8A8_UINT");
  Format fmt(type.get());

  std::vector<Value> values1(40);
  for (uint32_t i = 0; i < values1.size(); i++)
    values1[i].SetIntValue(0);

  std::vector<Value> values2 = values1;

  Buffer b1;
  b1.SetFormat(&fmt);
  b1.SetData(values1);

  Buffer b2;
  b2.SetFormat(&fmt);
  b2.SetData(values2);

  EXPECT_TRUE(b1.CompareHistogramEMD(&b2, 0.0f).IsSuccess());
}

// Creates two identical sets of RGBA pixel values and checks that the
// histograms are equal.
TEST_F(BufferTest, CompareHistogramEMDToleranceAllWhite) {
  TypeParser parser;
  auto type = parser.Parse("R8G8B8A8_UINT");
  Format fmt(type.get());

  std::vector<Value> values1(40);
  for (uint32_t i = 0; i < values1.size(); i++)
    values1[i].SetIntValue(std::numeric_limits<uint8_t>().max());

  std::vector<Value> values2 = values1;

  Buffer b1;
  b1.SetFormat(&fmt);
  b1.SetData(values1);

  Buffer b2;
  b2.SetFormat(&fmt);
  b2.SetData(values2);

  EXPECT_TRUE(b1.CompareHistogramEMD(&b2, 0.0f).IsSuccess());
}

TEST_F(BufferTest, SetFloat16) {
  std::vector<Value> values;
  values.resize(2);
  values[0].SetDoubleValue(2.8);
  values[1].SetDoubleValue(1234.567);

  TypeParser parser;
  auto type = parser.Parse("R16_SFLOAT");

  Format fmt(type.get());
  Buffer b;
  b.SetFormat(&fmt);
  b.SetData(std::move(values));

  EXPECT_EQ(2u, b.ElementCount());
  EXPECT_EQ(2u, b.ValueCount());
  EXPECT_EQ(4u, b.GetSizeInBytes());

  auto v = b.GetValues<uint16_t>();
  EXPECT_EQ(float16::FloatToHexFloat16(2.8f), v[0]);
  EXPECT_EQ(float16::FloatToHexFloat16(1234.567f), v[1]);
}

}  // namespace amber
