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

#include "src/type_parser.h"

#include "gtest/gtest.h"
#include "src/format.h"

namespace amber {

using TypeParserTest = testing::Test;

TEST_F(TypeParserTest, Formats) {
  struct {
    const char* name;
    FormatType type;
    uint8_t pack_size;
    uint8_t component_count;
    struct {
      FormatComponentType type;
      FormatMode mode;
      uint8_t num_bits;
    } components[4];
  } formats[] = {
      {"A1R5G5B5_UNORM_PACK16",
       FormatType::kA1R5G5B5_UNORM_PACK16,
       16U,
       4U,
       {
           {FormatComponentType::kA, FormatMode::kUNorm, 1},
           {FormatComponentType::kR, FormatMode::kUNorm, 5},
           {FormatComponentType::kG, FormatMode::kUNorm, 5},
           {FormatComponentType::kB, FormatMode::kUNorm, 5},
       }},
      {"A2B10G10R10_SINT_PACK32",
       FormatType::kA2B10G10R10_SINT_PACK32,
       32U,
       4U,
       {
           {FormatComponentType::kA, FormatMode::kSInt, 2},
           {FormatComponentType::kB, FormatMode::kSInt, 10},
           {FormatComponentType::kG, FormatMode::kSInt, 10},
           {FormatComponentType::kR, FormatMode::kSInt, 10},
       }},
      {"A2B10G10R10_SNORM_PACK32",
       FormatType::kA2B10G10R10_SNORM_PACK32,
       32U,
       4U,
       {
           {FormatComponentType::kA, FormatMode::kSNorm, 2},
           {FormatComponentType::kB, FormatMode::kSNorm, 10},
           {FormatComponentType::kG, FormatMode::kSNorm, 10},
           {FormatComponentType::kR, FormatMode::kSNorm, 10},
       }},
      {"A2B10G10R10_SSCALED_PACK32",
       FormatType::kA2B10G10R10_SSCALED_PACK32,
       32U,
       4U,
       {
           {FormatComponentType::kA, FormatMode::kSScaled, 2},
           {FormatComponentType::kB, FormatMode::kSScaled, 10},
           {FormatComponentType::kG, FormatMode::kSScaled, 10},
           {FormatComponentType::kR, FormatMode::kSScaled, 10},
       }},
      {"A2B10G10R10_UINT_PACK32",
       FormatType::kA2B10G10R10_UINT_PACK32,
       32U,
       4U,
       {
           {FormatComponentType::kA, FormatMode::kUInt, 2},
           {FormatComponentType::kB, FormatMode::kUInt, 10},
           {FormatComponentType::kG, FormatMode::kUInt, 10},
           {FormatComponentType::kR, FormatMode::kUInt, 10},
       }},
      {"A2B10G10R10_UNORM_PACK32",
       FormatType::kA2B10G10R10_UNORM_PACK32,
       32U,
       4U,
       {
           {FormatComponentType::kA, FormatMode::kUNorm, 2},
           {FormatComponentType::kB, FormatMode::kUNorm, 10},
           {FormatComponentType::kG, FormatMode::kUNorm, 10},
           {FormatComponentType::kR, FormatMode::kUNorm, 10},
       }},
      {"A2B10G10R10_USCALED_PACK32",
       FormatType::kA2B10G10R10_USCALED_PACK32,
       32U,
       4U,
       {
           {FormatComponentType::kA, FormatMode::kUScaled, 2},
           {FormatComponentType::kB, FormatMode::kUScaled, 10},
           {FormatComponentType::kG, FormatMode::kUScaled, 10},
           {FormatComponentType::kR, FormatMode::kUScaled, 10},
       }},
      {"A2R10G10B10_SINT_PACK32",
       FormatType::kA2R10G10B10_SINT_PACK32,
       32U,
       4U,
       {
           {FormatComponentType::kA, FormatMode::kSInt, 2},
           {FormatComponentType::kR, FormatMode::kSInt, 10},
           {FormatComponentType::kG, FormatMode::kSInt, 10},
           {FormatComponentType::kB, FormatMode::kSInt, 10},
       }},
      {"A2R10G10B10_SNORM_PACK32",
       FormatType::kA2R10G10B10_SNORM_PACK32,
       32U,
       4U,
       {
           {FormatComponentType::kA, FormatMode::kSNorm, 2},
           {FormatComponentType::kR, FormatMode::kSNorm, 10},
           {FormatComponentType::kG, FormatMode::kSNorm, 10},
           {FormatComponentType::kB, FormatMode::kSNorm, 10},
       }},
      {"A2R10G10B10_SSCALED_PACK32",
       FormatType::kA2R10G10B10_SSCALED_PACK32,
       32U,
       4U,
       {
           {FormatComponentType::kA, FormatMode::kSScaled, 2},
           {FormatComponentType::kR, FormatMode::kSScaled, 10},
           {FormatComponentType::kG, FormatMode::kSScaled, 10},
           {FormatComponentType::kB, FormatMode::kSScaled, 10},
       }},
      {"A2R10G10B10_UINT_PACK32",
       FormatType::kA2R10G10B10_UINT_PACK32,
       32U,
       4U,
       {
           {FormatComponentType::kA, FormatMode::kUInt, 2},
           {FormatComponentType::kR, FormatMode::kUInt, 10},
           {FormatComponentType::kG, FormatMode::kUInt, 10},
           {FormatComponentType::kB, FormatMode::kUInt, 10},
       }},
      {"A2R10G10B10_UNORM_PACK32",
       FormatType::kA2R10G10B10_UNORM_PACK32,
       32U,
       4U,
       {
           {FormatComponentType::kA, FormatMode::kUNorm, 2},
           {FormatComponentType::kR, FormatMode::kUNorm, 10},
           {FormatComponentType::kG, FormatMode::kUNorm, 10},
           {FormatComponentType::kB, FormatMode::kUNorm, 10},
       }},
      {"A2R10G10B10_USCALED_PACK32",
       FormatType::kA2R10G10B10_USCALED_PACK32,
       32U,
       4U,
       {
           {FormatComponentType::kA, FormatMode::kUScaled, 2},
           {FormatComponentType::kR, FormatMode::kUScaled, 10},
           {FormatComponentType::kG, FormatMode::kUScaled, 10},
           {FormatComponentType::kB, FormatMode::kUScaled, 10},
       }},
      {"A8B8G8R8_SINT_PACK32",
       FormatType::kA8B8G8R8_SINT_PACK32,
       32U,
       4U,
       {
           {FormatComponentType::kA, FormatMode::kSInt, 8},
           {FormatComponentType::kB, FormatMode::kSInt, 8},
           {FormatComponentType::kG, FormatMode::kSInt, 8},
           {FormatComponentType::kR, FormatMode::kSInt, 8},
       }},
      {"A8B8G8R8_SNORM_PACK32",
       FormatType::kA8B8G8R8_SNORM_PACK32,
       32U,
       4U,
       {
           {FormatComponentType::kA, FormatMode::kSNorm, 8},
           {FormatComponentType::kB, FormatMode::kSNorm, 8},
           {FormatComponentType::kG, FormatMode::kSNorm, 8},
           {FormatComponentType::kR, FormatMode::kSNorm, 8},
       }},
      {"A8B8G8R8_SRGB_PACK32",
       FormatType::kA8B8G8R8_SRGB_PACK32,
       32U,
       4U,
       {
           {FormatComponentType::kA, FormatMode::kSRGB, 8},
           {FormatComponentType::kB, FormatMode::kSRGB, 8},
           {FormatComponentType::kG, FormatMode::kSRGB, 8},
           {FormatComponentType::kR, FormatMode::kSRGB, 8},
       }},
      {"A8B8G8R8_SSCALED_PACK32",
       FormatType::kA8B8G8R8_SSCALED_PACK32,
       32U,
       4U,
       {
           {FormatComponentType::kA, FormatMode::kSScaled, 8},
           {FormatComponentType::kB, FormatMode::kSScaled, 8},
           {FormatComponentType::kG, FormatMode::kSScaled, 8},
           {FormatComponentType::kR, FormatMode::kSScaled, 8},
       }},
      {"A8B8G8R8_UINT_PACK32",
       FormatType::kA8B8G8R8_UINT_PACK32,
       32U,
       4U,
       {
           {FormatComponentType::kA, FormatMode::kUInt, 8},
           {FormatComponentType::kB, FormatMode::kUInt, 8},
           {FormatComponentType::kG, FormatMode::kUInt, 8},
           {FormatComponentType::kR, FormatMode::kUInt, 8},
       }},
      {"A8B8G8R8_UNORM_PACK32",
       FormatType::kA8B8G8R8_UNORM_PACK32,
       32U,
       4U,
       {
           {FormatComponentType::kA, FormatMode::kUNorm, 8},
           {FormatComponentType::kB, FormatMode::kUNorm, 8},
           {FormatComponentType::kG, FormatMode::kUNorm, 8},
           {FormatComponentType::kR, FormatMode::kUNorm, 8},
       }},
      {"A8B8G8R8_USCALED_PACK32",
       FormatType::kA8B8G8R8_USCALED_PACK32,
       32U,
       4U,
       {
           {FormatComponentType::kA, FormatMode::kUScaled, 8},
           {FormatComponentType::kB, FormatMode::kUScaled, 8},
           {FormatComponentType::kG, FormatMode::kUScaled, 8},
           {FormatComponentType::kR, FormatMode::kUScaled, 8},
       }},
      {"B10G11R11_UFLOAT_PACK32",
       FormatType::kB10G11R11_UFLOAT_PACK32,
       32U,
       3U,
       {
           {FormatComponentType::kB, FormatMode::kUFloat, 10},
           {FormatComponentType::kG, FormatMode::kUFloat, 11},
           {FormatComponentType::kR, FormatMode::kUFloat, 11},
       }},
      {"B4G4R4A4_UNORM_PACK16",
       FormatType::kB4G4R4A4_UNORM_PACK16,
       16U,
       4U,
       {
           {FormatComponentType::kB, FormatMode::kUNorm, 4},
           {FormatComponentType::kG, FormatMode::kUNorm, 4},
           {FormatComponentType::kR, FormatMode::kUNorm, 4},
           {FormatComponentType::kA, FormatMode::kUNorm, 4},
       }},
      {"B5G5R5A1_UNORM_PACK16",
       FormatType::kB5G5R5A1_UNORM_PACK16,
       16U,
       4U,
       {
           {FormatComponentType::kB, FormatMode::kUNorm, 5},
           {FormatComponentType::kG, FormatMode::kUNorm, 5},
           {FormatComponentType::kR, FormatMode::kUNorm, 5},
           {FormatComponentType::kA, FormatMode::kUNorm, 1},
       }},
      {"B5G6R5_UNORM_PACK16",
       FormatType::kB5G6R5_UNORM_PACK16,
       16U,
       3U,
       {
           {FormatComponentType::kB, FormatMode::kUNorm, 5},
           {FormatComponentType::kG, FormatMode::kUNorm, 6},
           {FormatComponentType::kR, FormatMode::kUNorm, 5},
       }},
      {"B8G8R8A8_SINT",
       FormatType::kB8G8R8A8_SINT,
       0U,
       4U,
       {
           {FormatComponentType::kB, FormatMode::kSInt, 8},
           {FormatComponentType::kG, FormatMode::kSInt, 8},
           {FormatComponentType::kR, FormatMode::kSInt, 8},
           {FormatComponentType::kA, FormatMode::kSInt, 8},
       }},
      {"B8G8R8A8_SNORM",
       FormatType::kB8G8R8A8_SNORM,
       0U,
       4U,
       {
           {FormatComponentType::kB, FormatMode::kSNorm, 8},
           {FormatComponentType::kG, FormatMode::kSNorm, 8},
           {FormatComponentType::kR, FormatMode::kSNorm, 8},
           {FormatComponentType::kA, FormatMode::kSNorm, 8},
       }},
      {"B8G8R8A8_SRGB",
       FormatType::kB8G8R8A8_SRGB,
       0U,
       4U,
       {
           {FormatComponentType::kB, FormatMode::kSRGB, 8},
           {FormatComponentType::kG, FormatMode::kSRGB, 8},
           {FormatComponentType::kR, FormatMode::kSRGB, 8},
           {FormatComponentType::kA, FormatMode::kSRGB, 8},
       }},
      {"B8G8R8A8_SSCALED",
       FormatType::kB8G8R8A8_SSCALED,
       0U,
       4U,
       {
           {FormatComponentType::kB, FormatMode::kSScaled, 8},
           {FormatComponentType::kG, FormatMode::kSScaled, 8},
           {FormatComponentType::kR, FormatMode::kSScaled, 8},
           {FormatComponentType::kA, FormatMode::kSScaled, 8},
       }},
      {"B8G8R8A8_UINT",
       FormatType::kB8G8R8A8_UINT,
       0U,
       4U,
       {
           {FormatComponentType::kB, FormatMode::kUInt, 8},
           {FormatComponentType::kG, FormatMode::kUInt, 8},
           {FormatComponentType::kR, FormatMode::kUInt, 8},
           {FormatComponentType::kA, FormatMode::kUInt, 8},
       }},
      {"B8G8R8A8_UNORM",
       FormatType::kB8G8R8A8_UNORM,
       0U,
       4U,
       {
           {FormatComponentType::kB, FormatMode::kUNorm, 8},
           {FormatComponentType::kG, FormatMode::kUNorm, 8},
           {FormatComponentType::kR, FormatMode::kUNorm, 8},
           {FormatComponentType::kA, FormatMode::kUNorm, 8},
       }},
      {"B8G8R8A8_USCALED",
       FormatType::kB8G8R8A8_USCALED,
       0U,
       4U,
       {
           {FormatComponentType::kB, FormatMode::kUScaled, 8},
           {FormatComponentType::kG, FormatMode::kUScaled, 8},
           {FormatComponentType::kR, FormatMode::kUScaled, 8},
           {FormatComponentType::kA, FormatMode::kUScaled, 8},
       }},
      {"B8G8R8_SINT",
       FormatType::kB8G8R8_SINT,
       0U,
       3U,
       {
           {FormatComponentType::kB, FormatMode::kSInt, 8},
           {FormatComponentType::kG, FormatMode::kSInt, 8},
           {FormatComponentType::kR, FormatMode::kSInt, 8},
       }},
      {"B8G8R8_SNORM",
       FormatType::kB8G8R8_SNORM,
       0U,
       3U,
       {
           {FormatComponentType::kB, FormatMode::kSNorm, 8},
           {FormatComponentType::kG, FormatMode::kSNorm, 8},
           {FormatComponentType::kR, FormatMode::kSNorm, 8},
       }},
      {"B8G8R8_SRGB",
       FormatType::kB8G8R8_SRGB,
       0U,
       3U,
       {
           {FormatComponentType::kB, FormatMode::kSRGB, 8},
           {FormatComponentType::kG, FormatMode::kSRGB, 8},
           {FormatComponentType::kR, FormatMode::kSRGB, 8},
       }},
      {"B8G8R8_SSCALED",
       FormatType::kB8G8R8_SSCALED,
       0U,
       3U,
       {
           {FormatComponentType::kB, FormatMode::kSScaled, 8},
           {FormatComponentType::kG, FormatMode::kSScaled, 8},
           {FormatComponentType::kR, FormatMode::kSScaled, 8},
       }},
      {"B8G8R8_UINT",
       FormatType::kB8G8R8_UINT,
       0U,
       3U,
       {
           {FormatComponentType::kB, FormatMode::kUInt, 8},
           {FormatComponentType::kG, FormatMode::kUInt, 8},
           {FormatComponentType::kR, FormatMode::kUInt, 8},
       }},
      {"B8G8R8_UNORM",
       FormatType::kB8G8R8_UNORM,
       0U,
       3U,
       {
           {FormatComponentType::kB, FormatMode::kUNorm, 8},
           {FormatComponentType::kG, FormatMode::kUNorm, 8},
           {FormatComponentType::kR, FormatMode::kUNorm, 8},
       }},
      {"B8G8R8_USCALED",
       FormatType::kB8G8R8_USCALED,
       0U,
       3U,
       {
           {FormatComponentType::kB, FormatMode::kUScaled, 8},
           {FormatComponentType::kG, FormatMode::kUScaled, 8},
           {FormatComponentType::kR, FormatMode::kUScaled, 8},
       }},
      {"D16_UNORM",
       FormatType::kD16_UNORM,
       0U,
       1U,
       {
           {FormatComponentType::kD, FormatMode::kUNorm, 16},
       }},
      {"D16_UNORM_S8_UINT",
       FormatType::kD16_UNORM_S8_UINT,
       0U,
       2U,
       {
           {FormatComponentType::kD, FormatMode::kUNorm, 16},
           {FormatComponentType::kS, FormatMode::kUInt, 8},
       }},
      {"D24_UNORM_S8_UINT",
       FormatType::kD24_UNORM_S8_UINT,
       0U,
       2U,
       {
           {FormatComponentType::kD, FormatMode::kUNorm, 24},
           {FormatComponentType::kS, FormatMode::kUInt, 8},
       }},
      {"D32_SFLOAT",
       FormatType::kD32_SFLOAT,
       0U,
       1U,
       {
           {FormatComponentType::kD, FormatMode::kSFloat, 32},
       }},
      {"D32_SFLOAT_S8_UINT",
       FormatType::kD32_SFLOAT_S8_UINT,
       0U,
       2U,
       {
           {FormatComponentType::kD, FormatMode::kSFloat, 32},
           {FormatComponentType::kS, FormatMode::kUInt, 8},
       }},
      {"R16G16B16A16_SFLOAT",
       FormatType::kR16G16B16A16_SFLOAT,
       0U,
       4U,
       {
           {FormatComponentType::kR, FormatMode::kSFloat, 16},
           {FormatComponentType::kG, FormatMode::kSFloat, 16},
           {FormatComponentType::kB, FormatMode::kSFloat, 16},
           {FormatComponentType::kA, FormatMode::kSFloat, 16},
       }},
      {"R16G16B16A16_SINT",
       FormatType::kR16G16B16A16_SINT,
       0U,
       4U,
       {
           {FormatComponentType::kR, FormatMode::kSInt, 16},
           {FormatComponentType::kG, FormatMode::kSInt, 16},
           {FormatComponentType::kB, FormatMode::kSInt, 16},
           {FormatComponentType::kA, FormatMode::kSInt, 16},
       }},
      {"R16G16B16A16_SNORM",
       FormatType::kR16G16B16A16_SNORM,
       0U,
       4U,
       {
           {FormatComponentType::kR, FormatMode::kSNorm, 16},
           {FormatComponentType::kG, FormatMode::kSNorm, 16},
           {FormatComponentType::kB, FormatMode::kSNorm, 16},
           {FormatComponentType::kA, FormatMode::kSNorm, 16},
       }},
      {"R16G16B16A16_SSCALED",
       FormatType::kR16G16B16A16_SSCALED,
       0U,
       4U,
       {
           {FormatComponentType::kR, FormatMode::kSScaled, 16},
           {FormatComponentType::kG, FormatMode::kSScaled, 16},
           {FormatComponentType::kB, FormatMode::kSScaled, 16},
           {FormatComponentType::kA, FormatMode::kSScaled, 16},
       }},
      {"R16G16B16A16_UINT",
       FormatType::kR16G16B16A16_UINT,
       0U,
       4U,
       {
           {FormatComponentType::kR, FormatMode::kUInt, 16},
           {FormatComponentType::kG, FormatMode::kUInt, 16},
           {FormatComponentType::kB, FormatMode::kUInt, 16},
           {FormatComponentType::kA, FormatMode::kUInt, 16},
       }},
      {"R16G16B16A16_UNORM",
       FormatType::kR16G16B16A16_UNORM,
       0U,
       4U,
       {
           {FormatComponentType::kR, FormatMode::kUNorm, 16},
           {FormatComponentType::kG, FormatMode::kUNorm, 16},
           {FormatComponentType::kB, FormatMode::kUNorm, 16},
           {FormatComponentType::kA, FormatMode::kUNorm, 16},
       }},
      {"R16G16B16A16_USCALED",
       FormatType::kR16G16B16A16_USCALED,
       0U,
       4U,
       {
           {FormatComponentType::kR, FormatMode::kUScaled, 16},
           {FormatComponentType::kG, FormatMode::kUScaled, 16},
           {FormatComponentType::kB, FormatMode::kUScaled, 16},
           {FormatComponentType::kA, FormatMode::kUScaled, 16},
       }},
      {"R16G16B16_SFLOAT",
       FormatType::kR16G16B16_SFLOAT,
       0U,
       3U,
       {
           {FormatComponentType::kR, FormatMode::kSFloat, 16},
           {FormatComponentType::kG, FormatMode::kSFloat, 16},
           {FormatComponentType::kB, FormatMode::kSFloat, 16},
       }},
      {"R16G16B16_SINT",
       FormatType::kR16G16B16_SINT,
       0U,
       3U,
       {
           {FormatComponentType::kR, FormatMode::kSInt, 16},
           {FormatComponentType::kG, FormatMode::kSInt, 16},
           {FormatComponentType::kB, FormatMode::kSInt, 16},
       }},
      {"R16G16B16_SNORM",
       FormatType::kR16G16B16_SNORM,
       0U,
       3U,
       {
           {FormatComponentType::kR, FormatMode::kSNorm, 16},
           {FormatComponentType::kG, FormatMode::kSNorm, 16},
           {FormatComponentType::kB, FormatMode::kSNorm, 16},
       }},
      {"R16G16B16_SSCALED",
       FormatType::kR16G16B16_SSCALED,
       0U,
       3U,
       {
           {FormatComponentType::kR, FormatMode::kSScaled, 16},
           {FormatComponentType::kG, FormatMode::kSScaled, 16},
           {FormatComponentType::kB, FormatMode::kSScaled, 16},
       }},
      {"R16G16B16_UINT",
       FormatType::kR16G16B16_UINT,
       0U,
       3U,
       {
           {FormatComponentType::kR, FormatMode::kUInt, 16},
           {FormatComponentType::kG, FormatMode::kUInt, 16},
           {FormatComponentType::kB, FormatMode::kUInt, 16},
       }},
      {"R16G16B16_UNORM",
       FormatType::kR16G16B16_UNORM,
       0U,
       3U,
       {
           {FormatComponentType::kR, FormatMode::kUNorm, 16},
           {FormatComponentType::kG, FormatMode::kUNorm, 16},
           {FormatComponentType::kB, FormatMode::kUNorm, 16},
       }},
      {"R16G16B16_USCALED",
       FormatType::kR16G16B16_USCALED,
       0U,
       3U,
       {
           {FormatComponentType::kR, FormatMode::kUScaled, 16},
           {FormatComponentType::kG, FormatMode::kUScaled, 16},
           {FormatComponentType::kB, FormatMode::kUScaled, 16},
       }},
      {"R16G16_SFLOAT",
       FormatType::kR16G16_SFLOAT,
       0U,
       2U,
       {
           {FormatComponentType::kR, FormatMode::kSFloat, 16},
           {FormatComponentType::kG, FormatMode::kSFloat, 16},
       }},
      {"R16G16_SINT",
       FormatType::kR16G16_SINT,
       0U,
       2U,
       {
           {FormatComponentType::kR, FormatMode::kSInt, 16},
           {FormatComponentType::kG, FormatMode::kSInt, 16},
       }},
      {"R16G16_SNORM",
       FormatType::kR16G16_SNORM,
       0U,
       2U,
       {
           {FormatComponentType::kR, FormatMode::kSNorm, 16},
           {FormatComponentType::kG, FormatMode::kSNorm, 16},
       }},
      {"R16G16_SSCALED",
       FormatType::kR16G16_SSCALED,
       0U,
       2U,
       {
           {FormatComponentType::kR, FormatMode::kSScaled, 16},
           {FormatComponentType::kG, FormatMode::kSScaled, 16},
       }},
      {"R16G16_UINT",
       FormatType::kR16G16_UINT,
       0U,
       2U,
       {
           {FormatComponentType::kR, FormatMode::kUInt, 16},
           {FormatComponentType::kG, FormatMode::kUInt, 16},
       }},
      {"R16G16_UNORM",
       FormatType::kR16G16_UNORM,
       0U,
       2U,
       {
           {FormatComponentType::kR, FormatMode::kUNorm, 16},
           {FormatComponentType::kG, FormatMode::kUNorm, 16},
       }},
      {"R16G16_USCALED",
       FormatType::kR16G16_USCALED,
       0U,
       2U,
       {
           {FormatComponentType::kR, FormatMode::kUScaled, 16},
           {FormatComponentType::kG, FormatMode::kUScaled, 16},
       }},
      {"R16_SFLOAT",
       FormatType::kR16_SFLOAT,
       0U,
       1U,
       {
           {FormatComponentType::kR, FormatMode::kSFloat, 16},
       }},
      {"R16_SINT",
       FormatType::kR16_SINT,
       0U,
       1U,
       {
           {FormatComponentType::kR, FormatMode::kSInt, 16},
       }},
      {"R16_SNORM",
       FormatType::kR16_SNORM,
       0U,
       1U,
       {
           {FormatComponentType::kR, FormatMode::kSNorm, 16},
       }},
      {"R16_SSCALED",
       FormatType::kR16_SSCALED,
       0U,
       1U,
       {
           {FormatComponentType::kR, FormatMode::kSScaled, 16},
       }},
      {"R16_UINT",
       FormatType::kR16_UINT,
       0U,
       1U,
       {
           {FormatComponentType::kR, FormatMode::kUInt, 16},
       }},
      {"R16_UNORM",
       FormatType::kR16_UNORM,
       0U,
       1U,
       {
           {FormatComponentType::kR, FormatMode::kUNorm, 16},
       }},
      {"R16_USCALED",
       FormatType::kR16_USCALED,
       0U,
       1U,
       {
           {FormatComponentType::kR, FormatMode::kUScaled, 16},
       }},
      {"R32G32B32A32_SFLOAT",
       FormatType::kR32G32B32A32_SFLOAT,
       0U,
       4U,
       {
           {FormatComponentType::kR, FormatMode::kSFloat, 32},
           {FormatComponentType::kG, FormatMode::kSFloat, 32},
           {FormatComponentType::kB, FormatMode::kSFloat, 32},
           {FormatComponentType::kA, FormatMode::kSFloat, 32},
       }},
      {"R32G32B32A32_SINT",
       FormatType::kR32G32B32A32_SINT,
       0U,
       4U,
       {
           {FormatComponentType::kR, FormatMode::kSInt, 32},
           {FormatComponentType::kG, FormatMode::kSInt, 32},
           {FormatComponentType::kB, FormatMode::kSInt, 32},
           {FormatComponentType::kA, FormatMode::kSInt, 32},
       }},
      {"R32G32B32A32_UINT",
       FormatType::kR32G32B32A32_UINT,
       0U,
       4U,
       {
           {FormatComponentType::kR, FormatMode::kUInt, 32},
           {FormatComponentType::kG, FormatMode::kUInt, 32},
           {FormatComponentType::kB, FormatMode::kUInt, 32},
           {FormatComponentType::kA, FormatMode::kUInt, 32},
       }},
      {"R32G32B32_SFLOAT",
       FormatType::kR32G32B32_SFLOAT,
       0U,
       3U,
       {
           {FormatComponentType::kR, FormatMode::kSFloat, 32},
           {FormatComponentType::kG, FormatMode::kSFloat, 32},
           {FormatComponentType::kB, FormatMode::kSFloat, 32},
       }},
      {"R32G32B32_SINT",
       FormatType::kR32G32B32_SINT,
       0U,
       3U,
       {
           {FormatComponentType::kR, FormatMode::kSInt, 32},
           {FormatComponentType::kG, FormatMode::kSInt, 32},
           {FormatComponentType::kB, FormatMode::kSInt, 32},
       }},
      {"R32G32B32_UINT",
       FormatType::kR32G32B32_UINT,
       0U,
       3U,
       {
           {FormatComponentType::kR, FormatMode::kUInt, 32},
           {FormatComponentType::kG, FormatMode::kUInt, 32},
           {FormatComponentType::kB, FormatMode::kUInt, 32},
       }},
      {"R32G32_SFLOAT",
       FormatType::kR32G32_SFLOAT,
       0U,
       2U,
       {
           {FormatComponentType::kR, FormatMode::kSFloat, 32},
           {FormatComponentType::kG, FormatMode::kSFloat, 32},
       }},
      {"R32G32_SINT",
       FormatType::kR32G32_SINT,
       0U,
       2U,
       {
           {FormatComponentType::kR, FormatMode::kSInt, 32},
           {FormatComponentType::kG, FormatMode::kSInt, 32},
       }},
      {"R32G32_UINT",
       FormatType::kR32G32_UINT,
       0U,
       2U,
       {
           {FormatComponentType::kR, FormatMode::kUInt, 32},
           {FormatComponentType::kG, FormatMode::kUInt, 32},
       }},
      {"R32_SFLOAT",
       FormatType::kR32_SFLOAT,
       0U,
       1U,
       {
           {FormatComponentType::kR, FormatMode::kSFloat, 32},
       }},
      {"R32_SINT",
       FormatType::kR32_SINT,
       0U,
       1U,
       {
           {FormatComponentType::kR, FormatMode::kSInt, 32},
       }},
      {"R32_UINT",
       FormatType::kR32_UINT,
       0U,
       1U,
       {
           {FormatComponentType::kR, FormatMode::kUInt, 32},
       }},
      {"R4G4B4A4_UNORM_PACK16",
       FormatType::kR4G4B4A4_UNORM_PACK16,
       16U,
       4U,
       {
           {FormatComponentType::kR, FormatMode::kUNorm, 4},
           {FormatComponentType::kG, FormatMode::kUNorm, 4},
           {FormatComponentType::kB, FormatMode::kUNorm, 4},
           {FormatComponentType::kA, FormatMode::kUNorm, 4},
       }},
      {"R4G4_UNORM_PACK8",
       FormatType::kR4G4_UNORM_PACK8,
       8U,
       2U,
       {
           {FormatComponentType::kR, FormatMode::kUNorm, 4},
           {FormatComponentType::kG, FormatMode::kUNorm, 4},
       }},
      {"R5G5B5A1_UNORM_PACK16",
       FormatType::kR5G5B5A1_UNORM_PACK16,
       16U,
       4U,
       {
           {FormatComponentType::kR, FormatMode::kUNorm, 5},
           {FormatComponentType::kG, FormatMode::kUNorm, 5},
           {FormatComponentType::kB, FormatMode::kUNorm, 5},
           {FormatComponentType::kA, FormatMode::kUNorm, 1},
       }},
      {"R5G6B5_UNORM_PACK16",
       FormatType::kR5G6B5_UNORM_PACK16,
       16U,
       3U,
       {
           {FormatComponentType::kR, FormatMode::kUNorm, 5},
           {FormatComponentType::kG, FormatMode::kUNorm, 6},
           {FormatComponentType::kB, FormatMode::kUNorm, 5},
       }},
      {"R64G64B64A64_SFLOAT",
       FormatType::kR64G64B64A64_SFLOAT,
       0U,
       4U,
       {
           {FormatComponentType::kR, FormatMode::kSFloat, 64},
           {FormatComponentType::kG, FormatMode::kSFloat, 64},
           {FormatComponentType::kB, FormatMode::kSFloat, 64},
           {FormatComponentType::kA, FormatMode::kSFloat, 64},
       }},
      {"R64G64B64A64_SINT",
       FormatType::kR64G64B64A64_SINT,
       0U,
       4U,
       {
           {FormatComponentType::kR, FormatMode::kSInt, 64},
           {FormatComponentType::kG, FormatMode::kSInt, 64},
           {FormatComponentType::kB, FormatMode::kSInt, 64},
           {FormatComponentType::kA, FormatMode::kSInt, 64},
       }},
      {"R64G64B64A64_UINT",
       FormatType::kR64G64B64A64_UINT,
       0U,
       4U,
       {
           {FormatComponentType::kR, FormatMode::kUInt, 64},
           {FormatComponentType::kG, FormatMode::kUInt, 64},
           {FormatComponentType::kB, FormatMode::kUInt, 64},
           {FormatComponentType::kA, FormatMode::kUInt, 64},
       }},
      {"R64G64B64_SFLOAT",
       FormatType::kR64G64B64_SFLOAT,
       0U,
       3U,
       {
           {FormatComponentType::kR, FormatMode::kSFloat, 64},
           {FormatComponentType::kG, FormatMode::kSFloat, 64},
           {FormatComponentType::kB, FormatMode::kSFloat, 64},
       }},
      {"R64G64B64_SINT",
       FormatType::kR64G64B64_SINT,
       0U,
       3U,
       {
           {FormatComponentType::kR, FormatMode::kSInt, 64},
           {FormatComponentType::kG, FormatMode::kSInt, 64},
           {FormatComponentType::kB, FormatMode::kSInt, 64},
       }},
      {"R64G64B64_UINT",
       FormatType::kR64G64B64_UINT,
       0U,
       3U,
       {
           {FormatComponentType::kR, FormatMode::kUInt, 64},
           {FormatComponentType::kG, FormatMode::kUInt, 64},
           {FormatComponentType::kB, FormatMode::kUInt, 64},
       }},
      {"R64G64_SFLOAT",
       FormatType::kR64G64_SFLOAT,
       0U,
       2U,
       {
           {FormatComponentType::kR, FormatMode::kSFloat, 64},
           {FormatComponentType::kG, FormatMode::kSFloat, 64},
       }},
      {"R64G64_SINT",
       FormatType::kR64G64_SINT,
       0U,
       2U,
       {
           {FormatComponentType::kR, FormatMode::kSInt, 64},
           {FormatComponentType::kG, FormatMode::kSInt, 64},
       }},
      {"R64G64_UINT",
       FormatType::kR64G64_UINT,
       0U,
       2U,
       {
           {FormatComponentType::kR, FormatMode::kUInt, 64},
           {FormatComponentType::kG, FormatMode::kUInt, 64},
       }},
      {"R64_SFLOAT",
       FormatType::kR64_SFLOAT,
       0U,
       1U,
       {
           {FormatComponentType::kR, FormatMode::kSFloat, 64},
       }},
      {"R64_SINT",
       FormatType::kR64_SINT,
       0U,
       1U,
       {
           {FormatComponentType::kR, FormatMode::kSInt, 64},
       }},
      {"R64_UINT",
       FormatType::kR64_UINT,
       0U,
       1U,
       {
           {FormatComponentType::kR, FormatMode::kUInt, 64},
       }},
      {"R8G8B8A8_SINT",
       FormatType::kR8G8B8A8_SINT,
       0U,
       4U,
       {
           {FormatComponentType::kR, FormatMode::kSInt, 8},
           {FormatComponentType::kG, FormatMode::kSInt, 8},
           {FormatComponentType::kB, FormatMode::kSInt, 8},
           {FormatComponentType::kA, FormatMode::kSInt, 8},
       }},
      {"R8G8B8A8_SNORM",
       FormatType::kR8G8B8A8_SNORM,
       0U,
       4U,
       {
           {FormatComponentType::kR, FormatMode::kSNorm, 8},
           {FormatComponentType::kG, FormatMode::kSNorm, 8},
           {FormatComponentType::kB, FormatMode::kSNorm, 8},
           {FormatComponentType::kA, FormatMode::kSNorm, 8},
       }},
      {"R8G8B8A8_SRGB",
       FormatType::kR8G8B8A8_SRGB,
       0U,
       4U,
       {
           {FormatComponentType::kR, FormatMode::kSRGB, 8},
           {FormatComponentType::kG, FormatMode::kSRGB, 8},
           {FormatComponentType::kB, FormatMode::kSRGB, 8},
           {FormatComponentType::kA, FormatMode::kSRGB, 8},
       }},
      {"R8G8B8A8_SSCALED",
       FormatType::kR8G8B8A8_SSCALED,
       0U,
       4U,
       {
           {FormatComponentType::kR, FormatMode::kSScaled, 8},
           {FormatComponentType::kG, FormatMode::kSScaled, 8},
           {FormatComponentType::kB, FormatMode::kSScaled, 8},
           {FormatComponentType::kA, FormatMode::kSScaled, 8},
       }},
      {"R8G8B8A8_UINT",
       FormatType::kR8G8B8A8_UINT,
       0U,
       4U,
       {
           {FormatComponentType::kR, FormatMode::kUInt, 8},
           {FormatComponentType::kG, FormatMode::kUInt, 8},
           {FormatComponentType::kB, FormatMode::kUInt, 8},
           {FormatComponentType::kA, FormatMode::kUInt, 8},
       }},
      {"R8G8B8A8_UNORM",
       FormatType::kR8G8B8A8_UNORM,
       0U,
       4U,
       {
           {FormatComponentType::kR, FormatMode::kUNorm, 8},
           {FormatComponentType::kG, FormatMode::kUNorm, 8},
           {FormatComponentType::kB, FormatMode::kUNorm, 8},
           {FormatComponentType::kA, FormatMode::kUNorm, 8},
       }},
      {"R8G8B8A8_USCALED",
       FormatType::kR8G8B8A8_USCALED,
       0U,
       4U,
       {
           {FormatComponentType::kR, FormatMode::kUScaled, 8},
           {FormatComponentType::kG, FormatMode::kUScaled, 8},
           {FormatComponentType::kB, FormatMode::kUScaled, 8},
           {FormatComponentType::kA, FormatMode::kUScaled, 8},
       }},
      {"R8G8B8_SINT",
       FormatType::kR8G8B8_SINT,
       0U,
       3U,
       {
           {FormatComponentType::kR, FormatMode::kSInt, 8},
           {FormatComponentType::kG, FormatMode::kSInt, 8},
           {FormatComponentType::kB, FormatMode::kSInt, 8},
       }},
      {"R8G8B8_SNORM",
       FormatType::kR8G8B8_SNORM,
       0U,
       3U,
       {
           {FormatComponentType::kR, FormatMode::kSNorm, 8},
           {FormatComponentType::kG, FormatMode::kSNorm, 8},
           {FormatComponentType::kB, FormatMode::kSNorm, 8},
       }},
      {"R8G8B8_SRGB",
       FormatType::kR8G8B8_SRGB,
       0U,
       3U,
       {
           {FormatComponentType::kR, FormatMode::kSRGB, 8},
           {FormatComponentType::kG, FormatMode::kSRGB, 8},
           {FormatComponentType::kB, FormatMode::kSRGB, 8},
       }},
      {"R8G8B8_SSCALED",
       FormatType::kR8G8B8_SSCALED,
       0U,
       3U,
       {
           {FormatComponentType::kR, FormatMode::kSScaled, 8},
           {FormatComponentType::kG, FormatMode::kSScaled, 8},
           {FormatComponentType::kB, FormatMode::kSScaled, 8},
       }},
      {"R8G8B8_UINT",
       FormatType::kR8G8B8_UINT,
       0U,
       3U,
       {
           {FormatComponentType::kR, FormatMode::kUInt, 8},
           {FormatComponentType::kG, FormatMode::kUInt, 8},
           {FormatComponentType::kB, FormatMode::kUInt, 8},
       }},
      {"R8G8B8_UNORM",
       FormatType::kR8G8B8_UNORM,
       0U,
       3U,
       {
           {FormatComponentType::kR, FormatMode::kUNorm, 8},
           {FormatComponentType::kG, FormatMode::kUNorm, 8},
           {FormatComponentType::kB, FormatMode::kUNorm, 8},
       }},
      {"R8G8B8_USCALED",
       FormatType::kR8G8B8_USCALED,
       0U,
       3U,
       {
           {FormatComponentType::kR, FormatMode::kUScaled, 8},
           {FormatComponentType::kG, FormatMode::kUScaled, 8},
           {FormatComponentType::kB, FormatMode::kUScaled, 8},
       }},
      {"R8G8_SINT",
       FormatType::kR8G8_SINT,
       0U,
       2U,
       {
           {FormatComponentType::kR, FormatMode::kSInt, 8},
           {FormatComponentType::kG, FormatMode::kSInt, 8},
       }},
      {"R8G8_SNORM",
       FormatType::kR8G8_SNORM,
       0U,
       2U,
       {
           {FormatComponentType::kR, FormatMode::kSNorm, 8},
           {FormatComponentType::kG, FormatMode::kSNorm, 8},
       }},
      {"R8G8_SRGB",
       FormatType::kR8G8_SRGB,
       0U,
       2U,
       {
           {FormatComponentType::kR, FormatMode::kSRGB, 8},
           {FormatComponentType::kG, FormatMode::kSRGB, 8},
       }},
      {"R8G8_SSCALED",
       FormatType::kR8G8_SSCALED,
       0U,
       2U,
       {
           {FormatComponentType::kR, FormatMode::kSScaled, 8},
           {FormatComponentType::kG, FormatMode::kSScaled, 8},
       }},
      {"R8G8_UINT",
       FormatType::kR8G8_UINT,
       0U,
       2U,
       {
           {FormatComponentType::kR, FormatMode::kUInt, 8},
           {FormatComponentType::kG, FormatMode::kUInt, 8},
       }},
      {"R8G8_UNORM",
       FormatType::kR8G8_UNORM,
       0U,
       2U,
       {
           {FormatComponentType::kR, FormatMode::kUNorm, 8},
           {FormatComponentType::kG, FormatMode::kUNorm, 8},
       }},
      {"R8G8_USCALED",
       FormatType::kR8G8_USCALED,
       0U,
       2U,
       {
           {FormatComponentType::kR, FormatMode::kUScaled, 8},
           {FormatComponentType::kG, FormatMode::kUScaled, 8},
       }},
      {"R8_SINT",
       FormatType::kR8_SINT,
       0U,
       1U,
       {
           {FormatComponentType::kR, FormatMode::kSInt, 8},
       }},
      {"R8_SNORM",
       FormatType::kR8_SNORM,
       0U,
       1U,
       {
           {FormatComponentType::kR, FormatMode::kSNorm, 8},
       }},
      {"R8_SRGB",
       FormatType::kR8_SRGB,
       0U,
       1U,
       {
           {FormatComponentType::kR, FormatMode::kSRGB, 8},
       }},
      {"R8_SSCALED",
       FormatType::kR8_SSCALED,
       0U,
       1U,
       {
           {FormatComponentType::kR, FormatMode::kSScaled, 8},
       }},
      {"R8_UINT",
       FormatType::kR8_UINT,
       0U,
       1U,
       {
           {FormatComponentType::kR, FormatMode::kUInt, 8},
       }},
      {"R8_UNORM",
       FormatType::kR8_UNORM,
       0U,
       1U,
       {
           {FormatComponentType::kR, FormatMode::kUNorm, 8},
       }},
      {"R8_USCALED",
       FormatType::kR8_USCALED,
       0U,
       1U,
       {
           {FormatComponentType::kR, FormatMode::kUScaled, 8},
       }},
      {"S8_UINT",
       FormatType::kS8_UINT,
       0U,
       1U,
       {
           {FormatComponentType::kS, FormatMode::kUInt, 8},
       }},
      {"X8_D24_UNORM_PACK32",
       FormatType::kX8_D24_UNORM_PACK32,
       32U,
       2U,
       {
           {FormatComponentType::kX, FormatMode::kUNorm, 8},
           {FormatComponentType::kD, FormatMode::kUNorm, 24},
       }},
  };

  for (const auto& data : formats) {
    TypeParser parser;
    auto type = parser.Parse(data.name);

    ASSERT_TRUE(type != nullptr) << data.name;

    Format fmt(type.get());
    EXPECT_EQ(data.type, fmt.GetFormatType()) << data.name;
    if (data.pack_size > 0) {
      ASSERT_TRUE(fmt.GetType()->IsList());
      ASSERT_TRUE(fmt.GetType()->AsList()->IsPacked());
      EXPECT_EQ(data.pack_size, fmt.GetType()->AsList()->PackSizeInBits());

      const auto& members = fmt.GetType()->AsList()->Members();
      for (size_t i = 0; i < data.component_count; ++i) {
        EXPECT_EQ(data.components[i].type, members[i].name) << data.name;
        EXPECT_EQ(data.components[i].mode, members[i].mode) << data.name;
        EXPECT_EQ(data.components[i].num_bits, members[i].num_bits)
            << data.name;
      }
    } else {
      auto& segs = fmt.GetSegments();
      ASSERT_TRUE(data.component_count <= segs.size()) << data.name;

      for (size_t i = 0; i < data.component_count; ++i) {
        EXPECT_EQ(data.components[i].mode, segs[i].GetFormatMode());
        EXPECT_EQ(data.components[i].num_bits, segs[i].GetNumBits())
            << data.name;
      }

      if (data.component_count < segs.size()) {
        // Only one padding added
        EXPECT_EQ(1, segs.size() - data.component_count);
        EXPECT_TRUE(segs.back().IsPadding());
      }
    }
  }
}  // NOLINT(readability/fn_size)

TEST_F(TypeParserTest, InvalidFormat) {
  TypeParser parser;
  auto type = parser.Parse("BLAH_BLAH_BLAH");
  EXPECT_TRUE(type == nullptr);
}

TEST_F(TypeParserTest, EmptyFormat) {
  TypeParser parser;
  auto type = parser.Parse("");
  EXPECT_TRUE(type == nullptr);
}

TEST_F(TypeParserTest, GlslString) {
  TypeParser parser;
  auto type = parser.Parse("float/vec3");
  ASSERT_TRUE(type != nullptr);

  Format fmt(type.get());
  EXPECT_EQ(FormatType::kR32G32B32_SFLOAT, fmt.GetFormatType());

  auto& segs = fmt.GetSegments();
  ASSERT_EQ(4U, segs.size());

  for (size_t i = 0; i < 3; ++i) {
    EXPECT_TRUE(
        type::Type::IsFloat32(segs[i].GetFormatMode(), segs[i].GetNumBits()));
  }

  EXPECT_TRUE(segs[3].IsPadding());
}

TEST_F(TypeParserTest, GlslStrings) {
  struct {
    const char* name;
    FormatType type;
  } strs[] = {
      {"float/vec4", FormatType::kR32G32B32A32_SFLOAT},
      {"float/ivec3", FormatType::kR32G32B32_SFLOAT},
      {"float/dvec2", FormatType::kR32G32_SFLOAT},
      {"float/uvec2", FormatType::kR32G32_SFLOAT},
      {"float/float", FormatType::kR32_SFLOAT},
      {"double/double", FormatType::kR64_SFLOAT},
      {"half/float", FormatType::kR16_SFLOAT},
      {"byte/int", FormatType::kR8_SINT},
      {"ubyte/uint", FormatType::kR8_UINT},
      {"short/int", FormatType::kR16_SINT},
      {"ushort/uint", FormatType::kR16_UINT},
      {"int/int", FormatType::kR32_SINT},
      {"uint/uint", FormatType::kR32_UINT},
  };

  for (const auto& str : strs) {
    TypeParser parser;
    auto type = parser.Parse(str.name);
    ASSERT_FALSE(type == nullptr);

    Format fmt(type.get());
    EXPECT_EQ(str.type, fmt.GetFormatType()) << str.name;
  }
}

TEST_F(TypeParserTest, GlslStringInvalid) {
  struct {
    const char* name;
  } strs[] = {
      {"flot/vec3"},
      {"float/vec1"},
      {"float/vec22"},
      {"float/dvec0"},
  };

  for (const auto& str : strs) {
    TypeParser parser;
    auto fmt = parser.Parse(str.name);
    EXPECT_TRUE(fmt == nullptr);
  }
}

}  // namespace amber
