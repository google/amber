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

TEST_F(AmberScriptParserTest, ImageNameMissing1) {
  std::string in = R"(
IMAGE
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: invalid IMAGE name provided", r.Error());
}

TEST_F(AmberScriptParserTest, ImageNameMissing2) {
  std::string in = R"(
IMAGE DATA_TYPE
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("2: missing IMAGE name", r.Error());
}

TEST_F(AmberScriptParserTest, ImageNameMissing3) {
  std::string in = R"(
IMAGE FORMAT
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("2: missing IMAGE name", r.Error());
}

TEST_F(AmberScriptParserTest, ImageNameInvalid) {
  std::string in = R"(
IMAGE 1
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("2: invalid IMAGE name provided", r.Error());
}

TEST_F(AmberScriptParserTest, ImageDataTypeInvalid) {
  std::string in = R"(
IMAGE image DATA_TYPE blah
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("2: invalid data type 'blah' provided", r.Error());
}

TEST_F(AmberScriptParserTest, ImageFormatInvalid) {
  std::string in = R"(
IMAGE image FORMAT blah
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("2: invalid IMAGE FORMAT", r.Error());
}

TEST_F(AmberScriptParserTest, ImageMipLevelsInvalid) {
  std::string in = R"(
IMAGE image FORMAT R32G32B32A32_SFLOAT MIP_LEVELS mips
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("2: invalid value for MIP_LEVELS", r.Error());
}

TEST_F(AmberScriptParserTest, ImageMissingDataTypeCommand) {
  std::string in = R"(
IMAGE image OTHER
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("2: unknown IMAGE command provided: OTHER", r.Error());
}

TEST_F(AmberScriptParserTest, ImageDimensionalityInvalid) {
  std::string in = R"(
IMAGE image DATA_TYPE uint32 DIM_WRONG
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("2: unknown IMAGE command provided: DIM_WRONG", r.Error());
}

TEST_F(AmberScriptParserTest, ImageDimensionalityInvalid2) {
  std::string in = R"(
IMAGE image DATA_TYPE uint32 4
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("2: expected IMAGE WIDTH", r.Error());
}

TEST_F(AmberScriptParserTest, ImageWidthMissing) {
  std::string in = R"(
IMAGE image DATA_TYPE uint32 DIM_3D HEIGHT 2 DEPTH 2 FILL 0
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("2: expected IMAGE WIDTH", r.Error());
}

TEST_F(AmberScriptParserTest, ImageHeightMissing) {
  std::string in = R"(
IMAGE image DATA_TYPE uint32 DIM_3D WIDTH 2 DEPTH 2 FILL 0
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("2: expected IMAGE HEIGHT", r.Error());
}

TEST_F(AmberScriptParserTest, ImageDepthMissing) {
  std::string in = R"(
IMAGE image DATA_TYPE uint32 DIM_3D WIDTH 2 HEIGHT 2 FILL 0
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("2: expected IMAGE DEPTH", r.Error());
}

TEST_F(AmberScriptParserTest, ImageWidthMissingNumber) {
  std::string in = R"(
IMAGE image DATA_TYPE uint32 DIM_3D WIDTH HEIGHT 2 DEPTH 2 FILL 0
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("2: expected positive IMAGE WIDTH", r.Error());
}

TEST_F(AmberScriptParserTest, ImageHeightMissingNumber) {
  std::string in = R"(
IMAGE image DATA_TYPE uint32 DIM_3D WIDTH 2 HEIGHT DEPTH 2 FILL 0
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("2: expected positive IMAGE HEIGHT", r.Error());
}

TEST_F(AmberScriptParserTest, ImageDepthMissingNumber) {
  std::string in = R"(
IMAGE image DATA_TYPE uint32 DIM_3D WIDTH 2 HEIGHT 2 DEPTH FILL 0
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("2: expected positive IMAGE DEPTH", r.Error());
}

TEST_F(AmberScriptParserTest, Image1D) {
  std::string in = R"(
IMAGE image DATA_TYPE uint32 DIM_1D WIDTH 4
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess());
  auto script = parser.GetScript();
  const auto& buffers = script->GetBuffers();
  ASSERT_EQ(1U, buffers.size());

  ASSERT_TRUE(buffers[0] != nullptr);
  EXPECT_EQ("image", buffers[0]->GetName());

  auto* buffer = buffers[0].get();
  EXPECT_TRUE(buffer->GetFormat()->IsUint32());
  EXPECT_EQ(ImageDimension::k1D, buffer->GetImageDimension());
  EXPECT_EQ(4u, buffer->GetWidth());
  EXPECT_EQ(1u, buffer->GetHeight());
  EXPECT_EQ(1u, buffer->GetDepth());
  EXPECT_EQ(4u, buffer->ElementCount());
}

TEST_F(AmberScriptParserTest, Image2D) {
  std::string in = R"(
IMAGE image DATA_TYPE uint32 DIM_2D WIDTH 3 HEIGHT 4
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess());
  auto script = parser.GetScript();
  const auto& buffers = script->GetBuffers();
  ASSERT_EQ(1U, buffers.size());

  ASSERT_TRUE(buffers[0] != nullptr);
  EXPECT_EQ("image", buffers[0]->GetName());

  auto* buffer = buffers[0].get();
  EXPECT_TRUE(buffer->GetFormat()->IsUint32());
  EXPECT_EQ(ImageDimension::k2D, buffer->GetImageDimension());
  EXPECT_EQ(3u, buffer->GetWidth());
  EXPECT_EQ(4u, buffer->GetHeight());
  EXPECT_EQ(1u, buffer->GetDepth());
  EXPECT_EQ(12u, buffer->ElementCount());
}

TEST_F(AmberScriptParserTest, Image2DMultiSample) {
  std::string in = R"(
IMAGE image DATA_TYPE uint32 DIM_2D WIDTH 3 HEIGHT 4 SAMPLES 4
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess());
  auto script = parser.GetScript();
  const auto& buffers = script->GetBuffers();
  ASSERT_EQ(1U, buffers.size());

  ASSERT_TRUE(buffers[0] != nullptr);
  EXPECT_EQ("image", buffers[0]->GetName());

  auto* buffer = buffers[0].get();
  EXPECT_EQ(4u, buffer->GetSamples());
}

TEST_F(AmberScriptParserTest, Image2DInvalidSampleValue) {
  std::string in = R"(
IMAGE image DATA_TYPE uint32 DIM_2D WIDTH 3 HEIGHT 4 SAMPLES foo
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("2: expected integer value for SAMPLES", r.Error());
}

TEST_F(AmberScriptParserTest, Image2DInvalidSampleCount) {
  std::string in = R"(
IMAGE image DATA_TYPE uint32 DIM_2D WIDTH 3 HEIGHT 4 SAMPLES 5
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("2: invalid sample count: 5", r.Error());
}

TEST_F(AmberScriptParserTest, Image3D) {
  std::string in = R"(
IMAGE image DATA_TYPE uint32 DIM_3D WIDTH 3 HEIGHT 4 DEPTH 5
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess());
  auto script = parser.GetScript();
  const auto& buffers = script->GetBuffers();
  ASSERT_EQ(1U, buffers.size());

  ASSERT_TRUE(buffers[0] != nullptr);
  EXPECT_EQ("image", buffers[0]->GetName());

  auto* buffer = buffers[0].get();
  EXPECT_TRUE(buffer->GetFormat()->IsUint32());
  EXPECT_EQ(ImageDimension::k3D, buffer->GetImageDimension());
  EXPECT_EQ(3u, buffer->GetWidth());
  EXPECT_EQ(4u, buffer->GetHeight());
  EXPECT_EQ(5u, buffer->GetDepth());
  EXPECT_EQ(60u, buffer->ElementCount());
}

TEST_F(AmberScriptParserTest, ImageWithData) {
  std::string in = R"(
IMAGE image DATA_TYPE float DIM_3D HEIGHT 2 WIDTH 2 DEPTH 2 DATA
 0.11 0.12
 0.21 0.22

 0.31 0.32
 0.41 0.42
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  auto script = parser.GetScript();
  const auto& buffers = script->GetBuffers();
  ASSERT_EQ(1U, buffers.size());

  ASSERT_TRUE(buffers[0] != nullptr);
  EXPECT_EQ("image", buffers[0]->GetName());

  auto* buffer = buffers[0].get();
  EXPECT_TRUE(buffer->GetFormat()->IsFloat32());
  EXPECT_EQ(ImageDimension::k3D, buffer->GetImageDimension());
  EXPECT_EQ(2u, buffer->GetWidth());
  EXPECT_EQ(2u, buffer->GetHeight());
  EXPECT_EQ(2u, buffer->GetDepth());
  EXPECT_EQ(8u, buffer->ElementCount());

  auto* values = buffer->GetValues<float>();
  std::vector<float> result = {0.11f, 0.12f, 0.21f, 0.22f,
                               0.31f, 0.32f, 0.41f, 0.42f};

  EXPECT_EQ((*buffer->ValuePtr()).size(), 8u * sizeof(float));
  for (size_t i = 0; i < result.size(); ++i) {
    EXPECT_FLOAT_EQ(result[i], values[i]);
  }
}

TEST_F(AmberScriptParserTest, ImageDataSizeIncorrect) {
  std::string in = R"(
IMAGE image DATA_TYPE float DIM_3D HEIGHT 2 WIDTH 2 DEPTH 2 DATA
 0.11 0.12
 0.21 0.22
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ(
      "6: Elements provided in data does not match size specified: 8 specified "
      "vs 4 provided",
      r.Error());
}

}  // namespace amberscript
}  // namespace amber
