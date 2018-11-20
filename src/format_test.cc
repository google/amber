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

#include "gtest/gtest.h"
#include "src/datum_type.h"
#include "src/format.h"

namespace amber {

using FormatTest = testing::Test;

TEST_F(FormatTest, ToDataType) {
  struct {
    const char* name;
    DataType data_type;
  } formats[] = {
      {"A1R5G5B5_UNORM_PACK16", DataType::kUint16},
      {"A2B10G10R10_SINT_PACK32", DataType::kInt32},
      {"A2B10G10R10_SNORM_PACK32", DataType::kInt32},
      {"A2B10G10R10_SSCALED_PACK32", DataType::kInt32},
      {"A2B10G10R10_UINT_PACK32", DataType::kUint32},
      {"A2B10G10R10_UNORM_PACK32", DataType::kUint32},
      {"A2B10G10R10_USCALED_PACK32", DataType::kUint32},
      {"A2R10G10B10_SINT_PACK32", DataType::kInt32},
      {"A2R10G10B10_SNORM_PACK32", DataType::kInt32},
      {"A2R10G10B10_SSCALED_PACK32", DataType::kInt32},
      {"A2R10G10B10_UINT_PACK32", DataType::kUint32},
      {"A2R10G10B10_UNORM_PACK32", DataType::kUint32},
      {"A2R10G10B10_USCALED_PACK32", DataType::kUint32},
      {"A8B8G8R8_SINT_PACK32", DataType::kInt32},
      {"A8B8G8R8_SNORM_PACK32", DataType::kInt32},
      {"A8B8G8R8_SRGB_PACK32", DataType::kInt32},
      {"A8B8G8R8_SSCALED_PACK32", DataType::kInt32},
      {"A8B8G8R8_UINT_PACK32", DataType::kUint32},
      {"A8B8G8R8_UNORM_PACK32", DataType::kUint32},
      {"A8B8G8R8_USCALED_PACK32", DataType::kUint32},
      {"B10G11R11_UFLOAT_PACK32", DataType::kFloat},
      {"B4G4R4A4_UNORM_PACK16", DataType::kUint16},
      {"B5G5R5A1_UNORM_PACK16", DataType::kUint16},
      {"B5G6R5_UNORM_PACK16", DataType::kUint16},
      {"B8G8R8A8_SINT", DataType::kInt8},
      {"B8G8R8A8_SNORM", DataType::kInt8},
      {"B8G8R8A8_SRGB", DataType::kInt8},
      {"B8G8R8A8_SSCALED", DataType::kInt8},
      {"B8G8R8A8_UINT", DataType::kUint8},
      {"B8G8R8A8_UNORM", DataType::kUint8},
      {"B8G8R8A8_USCALED", DataType::kUint8},
      {"B8G8R8_SINT", DataType::kInt8},
      {"B8G8R8_SNORM", DataType::kInt8},
      {"B8G8R8_SRGB", DataType::kInt8},
      {"B8G8R8_SSCALED", DataType::kInt8},
      {"B8G8R8_UINT", DataType::kUint8},
      {"B8G8R8_UNORM", DataType::kUint8},
      {"B8G8R8_USCALED", DataType::kUint8},
      {"D16_UNORM", DataType::kUint16},
      {"D16_UNORM_S8_UINT", DataType::kUint16},
      {"D24_UNORM_S8_UINT", DataType::kUint32},
      {"D32_SFLOAT", DataType::kFloat},
      {"D32_SFLOAT_S8_UINT", DataType::kFloat},
      {"R16G16B16A16_SFLOAT", DataType::kFloat},
      {"R16G16B16A16_SINT", DataType::kInt16},
      {"R16G16B16A16_SNORM", DataType::kInt16},
      {"R16G16B16A16_SSCALED", DataType::kInt16},
      {"R16G16B16A16_UINT", DataType::kUint16},
      {"R16G16B16A16_UNORM", DataType::kUint16},
      {"R16G16B16A16_USCALED", DataType::kUint16},
      {"R16G16B16_SFLOAT", DataType::kFloat},
      {"R16G16B16_SINT", DataType::kInt16},
      {"R16G16B16_SNORM", DataType::kInt16},
      {"R16G16B16_SSCALED", DataType::kInt16},
      {"R16G16B16_UINT", DataType::kUint16},
      {"R16G16B16_UNORM", DataType::kUint16},
      {"R16G16B16_USCALED", DataType::kUint16},
      {"R16G16_SFLOAT", DataType::kFloat},
      {"R16G16_SINT", DataType::kInt16},
      {"R16G16_SNORM", DataType::kInt16},
      {"R16G16_SSCALED", DataType::kInt16},
      {"R16G16_UINT", DataType::kUint16},
      {"R16G16_UNORM", DataType::kUint16},
      {"R16G16_USCALED", DataType::kUint16},
      {"R16_SFLOAT", DataType::kFloat},
      {"R16_SINT", DataType::kInt16},
      {"R16_SNORM", DataType::kInt16},
      {"R16_SSCALED", DataType::kInt16},
      {"R16_UINT", DataType::kUint16},
      {"R16_UNORM", DataType::kUint16},
      {"R16_USCALED", DataType::kUint16},
      {"R32G32B32A32_SFLOAT", DataType::k},
      {"R32G32B32A32_SINT", DataType::k},
      {"R32G32B32A32_UINT", DataType::k},
      {"R32G32B32_SFLOAT", DataType::k},
      {"R32G32B32_SINT", DataType::k},
      {"R32G32B32_UINT", DataType::k},
      {"R32G32_SFLOAT", DataType::k},
      {"R32G32_SINT", DataType::k},
      {"R32G32_UINT", DataType::k},
      {"R32_SFLOAT", DataType::k},
      {"R32_SINT", DataType::k},
      {"R32_UINT", DataType::k},
      {"R4G4B4A4_UNORM_PACK16", DataType::k},
      {"R4G4_UNORM_PACK8", DataType::k},
      {"R5G5B5A1_UNORM_PACK16", DataType::k},
      {"R5G6B5_UNORM_PACK16", DataType::k},
      {"R64G64B64A64_SFLOAT", DataType::k},
      {"R64G64B64A64_SINT", DataType::k},
      {"R64G64B64A64_UINT", DataType::k},
      {"R64G64B64_SFLOAT", DataType::k},
      {"R64G64B64_SINT", DataType::k},
      {"R64G64B64_UINT", DataType::k},
      {"R64G64_SFLOAT", DataType::k},
      {"R64G64_SINT", DataType::k},
      {"R64G64_UINT", DataType::k},
      {"R64_SFLOAT", DataType::k},
      {"R64_SINT", DataType::k},
      {"R64_UINT", DataType::k},
      {"R8G8B8A8_SINT", DataType::k},
      {"R8G8B8A8_SNORM", DataType::k},
      {"R8G8B8A8_SRGB", DataType::k},
      {"R8G8B8A8_SSCALED", DataType::k},
      {"R8G8B8A8_UINT", DataType::k},
      {"R8G8B8A8_UNORM", DataType::k},
      {"R8G8B8A8_USCALED", DataType::k},
      {"R8G8B8_SINT", DataType::k},
      {"R8G8B8_SNORM", DataType::k},
      {"R8G8B8_SRGB", DataType::k},
      {"R8G8B8_SSCALED", DataType::k},
      {"R8G8B8_UINT", DataType::k},
      {"R8G8B8_UNORM", DataType::k},
      {"R8G8B8_USCALED", DataType::k},
      {"R8G8_SINT", DataType::k},
      {"R8G8_SNORM", DataType::k},
      {"R8G8_SRGB", DataType::k},
      {"R8G8_SSCALED", DataType::k},
      {"R8G8_UINT", DataType::k},
      {"R8G8_UNORM", DataType::k},
      {"R8G8_USCALED", DataType::k},
      {"R8_SINT", DataType::k},
      {"R8_SNORM", DataType::k},
      {"R8_SRGB", DataType::k},
      {"R8_SSCALED", DataType::k},
      {"R8_UINT", DataType::k},
      {"R8_UNORM", DataType::k},
      {"R8_USCALED", DataType::k},
      {"S8_UINT", DataType::k},
      {"X8_D24_UNORM_PACK32", DataType::k}
  };

  for (const auto& fmt : formats) {
    FormatParser parser;
    auto format = parser.Parse(fmt.name);

    ASSERT_TRUE(format != nullptr) << fmt.name;
    EXPECT_EQ(fmt.type, format->GetFormatType()) << fmt.name;
    EXPECT_EQ(fmt.pack_size, format->GetPackSize()) << fmt.name;

    auto& comps = format->GetComponents();
    ASSERT_EQ(fmt.component_count, comps.size());

    for (size_t i = 0; i < fmt.component_count; ++i) {
      EXPECT_EQ(fmt.components[i].type, comps[i].type) << fmt.name;
      EXPECT_EQ(fmt.components[i].mode, comps[i].mode) << fmt.name;
      EXPECT_EQ(fmt.components[i].num_bits, comps[i].num_bits) << fmt.name;
    }
  }
}



}  // namespace
