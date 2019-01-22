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

#include "src/vulkan/format_data.h"

namespace amber {
namespace vulkan {

VkFormat ToVkFormat(FormatType type) {
  VkFormat ret = VK_FORMAT_UNDEFINED;
  switch (type) {
    case FormatType::kUnknown:
      ret = VK_FORMAT_UNDEFINED;
      break;
    case FormatType::kA1R5G5B5_UNORM_PACK16:
      ret = VK_FORMAT_A1R5G5B5_UNORM_PACK16;
      break;
    case FormatType::kA2B10G10R10_SINT_PACK32:
      ret = VK_FORMAT_A2B10G10R10_SINT_PACK32;
      break;
    case FormatType::kA2B10G10R10_SNORM_PACK32:
      ret = VK_FORMAT_A2B10G10R10_SNORM_PACK32;
      break;
    case FormatType::kA2B10G10R10_SSCALED_PACK32:
      ret = VK_FORMAT_A2B10G10R10_SSCALED_PACK32;
      break;
    case FormatType::kA2B10G10R10_UINT_PACK32:
      ret = VK_FORMAT_A2B10G10R10_UINT_PACK32;
      break;
    case FormatType::kA2B10G10R10_UNORM_PACK32:
      ret = VK_FORMAT_A2B10G10R10_UNORM_PACK32;
      break;
    case FormatType::kA2B10G10R10_USCALED_PACK32:
      ret = VK_FORMAT_A2B10G10R10_USCALED_PACK32;
      break;
    case FormatType::kA2R10G10B10_SINT_PACK32:
      ret = VK_FORMAT_A2R10G10B10_SINT_PACK32;
      break;
    case FormatType::kA2R10G10B10_SNORM_PACK32:
      ret = VK_FORMAT_A2R10G10B10_SNORM_PACK32;
      break;
    case FormatType::kA2R10G10B10_SSCALED_PACK32:
      ret = VK_FORMAT_A2R10G10B10_SSCALED_PACK32;
      break;
    case FormatType::kA2R10G10B10_UINT_PACK32:
      ret = VK_FORMAT_A2R10G10B10_UINT_PACK32;
      break;
    case FormatType::kA2R10G10B10_UNORM_PACK32:
      ret = VK_FORMAT_A2R10G10B10_UNORM_PACK32;
      break;
    case FormatType::kA2R10G10B10_USCALED_PACK32:
      ret = VK_FORMAT_A2R10G10B10_USCALED_PACK32;
      break;
    case FormatType::kA8B8G8R8_SINT_PACK32:
      ret = VK_FORMAT_A8B8G8R8_SINT_PACK32;
      break;
    case FormatType::kA8B8G8R8_SNORM_PACK32:
      ret = VK_FORMAT_A8B8G8R8_SNORM_PACK32;
      break;
    case FormatType::kA8B8G8R8_SRGB_PACK32:
      ret = VK_FORMAT_A8B8G8R8_SRGB_PACK32;
      break;
    case FormatType::kA8B8G8R8_SSCALED_PACK32:
      ret = VK_FORMAT_A8B8G8R8_SSCALED_PACK32;
      break;
    case FormatType::kA8B8G8R8_UINT_PACK32:
      ret = VK_FORMAT_A8B8G8R8_UINT_PACK32;
      break;
    case FormatType::kA8B8G8R8_UNORM_PACK32:
      ret = VK_FORMAT_A8B8G8R8_UNORM_PACK32;
      break;
    case FormatType::kA8B8G8R8_USCALED_PACK32:
      ret = VK_FORMAT_A8B8G8R8_USCALED_PACK32;
      break;
    case FormatType::kB10G11R11_UFLOAT_PACK32:
      ret = VK_FORMAT_B10G11R11_UFLOAT_PACK32;
      break;
    case FormatType::kB4G4R4A4_UNORM_PACK16:
      ret = VK_FORMAT_B4G4R4A4_UNORM_PACK16;
      break;
    case FormatType::kB5G5R5A1_UNORM_PACK16:
      ret = VK_FORMAT_B5G5R5A1_UNORM_PACK16;
      break;
    case FormatType::kB5G6R5_UNORM_PACK16:
      ret = VK_FORMAT_B5G6R5_UNORM_PACK16;
      break;
    case FormatType::kB8G8R8A8_SINT:
      ret = VK_FORMAT_B8G8R8A8_SINT;
      break;
    case FormatType::kB8G8R8A8_SNORM:
      ret = VK_FORMAT_B8G8R8A8_SNORM;
      break;
    case FormatType::kB8G8R8A8_SRGB:
      ret = VK_FORMAT_B8G8R8A8_SRGB;
      break;
    case FormatType::kB8G8R8A8_SSCALED:
      ret = VK_FORMAT_B8G8R8A8_SSCALED;
      break;
    case FormatType::kB8G8R8A8_UINT:
      ret = VK_FORMAT_B8G8R8A8_UINT;
      break;
    case FormatType::kB8G8R8A8_UNORM:
      ret = VK_FORMAT_B8G8R8A8_UNORM;
      break;
    case FormatType::kB8G8R8A8_USCALED:
      ret = VK_FORMAT_B8G8R8A8_USCALED;
      break;
    case FormatType::kB8G8R8_SINT:
      ret = VK_FORMAT_B8G8R8_SINT;
      break;
    case FormatType::kB8G8R8_SNORM:
      ret = VK_FORMAT_B8G8R8_SNORM;
      break;
    case FormatType::kB8G8R8_SRGB:
      ret = VK_FORMAT_B8G8R8_SRGB;
      break;
    case FormatType::kB8G8R8_SSCALED:
      ret = VK_FORMAT_B8G8R8_SSCALED;
      break;
    case FormatType::kB8G8R8_UINT:
      ret = VK_FORMAT_B8G8R8_UINT;
      break;
    case FormatType::kB8G8R8_UNORM:
      ret = VK_FORMAT_B8G8R8_UNORM;
      break;
    case FormatType::kB8G8R8_USCALED:
      ret = VK_FORMAT_B8G8R8_USCALED;
      break;
    case FormatType::kD16_UNORM:
      ret = VK_FORMAT_D16_UNORM;
      break;
    case FormatType::kD16_UNORM_S8_UINT:
      ret = VK_FORMAT_D16_UNORM_S8_UINT;
      break;
    case FormatType::kD24_UNORM_S8_UINT:
      ret = VK_FORMAT_D24_UNORM_S8_UINT;
      break;
    case FormatType::kD32_SFLOAT:
      ret = VK_FORMAT_D32_SFLOAT;
      break;
    case FormatType::kD32_SFLOAT_S8_UINT:
      ret = VK_FORMAT_D32_SFLOAT_S8_UINT;
      break;
    case FormatType::kR16G16B16A16_SFLOAT:
      ret = VK_FORMAT_R16G16B16A16_SFLOAT;
      break;
    case FormatType::kR16G16B16A16_SINT:
      ret = VK_FORMAT_R16G16B16A16_SINT;
      break;
    case FormatType::kR16G16B16A16_SNORM:
      ret = VK_FORMAT_R16G16B16A16_SNORM;
      break;
    case FormatType::kR16G16B16A16_SSCALED:
      ret = VK_FORMAT_R16G16B16A16_SSCALED;
      break;
    case FormatType::kR16G16B16A16_UINT:
      ret = VK_FORMAT_R16G16B16A16_UINT;
      break;
    case FormatType::kR16G16B16A16_UNORM:
      ret = VK_FORMAT_R16G16B16A16_UNORM;
      break;
    case FormatType::kR16G16B16A16_USCALED:
      ret = VK_FORMAT_R16G16B16A16_USCALED;
      break;
    case FormatType::kR16G16B16_SFLOAT:
      ret = VK_FORMAT_R16G16B16_SFLOAT;
      break;
    case FormatType::kR16G16B16_SINT:
      ret = VK_FORMAT_R16G16B16_SINT;
      break;
    case FormatType::kR16G16B16_SNORM:
      ret = VK_FORMAT_R16G16B16_SNORM;
      break;
    case FormatType::kR16G16B16_SSCALED:
      ret = VK_FORMAT_R16G16B16_SSCALED;
      break;
    case FormatType::kR16G16B16_UINT:
      ret = VK_FORMAT_R16G16B16_UINT;
      break;
    case FormatType::kR16G16B16_UNORM:
      ret = VK_FORMAT_R16G16B16_UNORM;
      break;
    case FormatType::kR16G16B16_USCALED:
      ret = VK_FORMAT_R16G16B16_USCALED;
      break;
    case FormatType::kR16G16_SFLOAT:
      ret = VK_FORMAT_R16G16_SFLOAT;
      break;
    case FormatType::kR16G16_SINT:
      ret = VK_FORMAT_R16G16_SINT;
      break;
    case FormatType::kR16G16_SNORM:
      ret = VK_FORMAT_R16G16_SNORM;
      break;
    case FormatType::kR16G16_SSCALED:
      ret = VK_FORMAT_R16G16_SSCALED;
      break;
    case FormatType::kR16G16_UINT:
      ret = VK_FORMAT_R16G16_UINT;
      break;
    case FormatType::kR16G16_UNORM:
      ret = VK_FORMAT_R16G16_UNORM;
      break;
    case FormatType::kR16G16_USCALED:
      ret = VK_FORMAT_R16G16_USCALED;
      break;
    case FormatType::kR16_SFLOAT:
      ret = VK_FORMAT_R16_SFLOAT;
      break;
    case FormatType::kR16_SINT:
      ret = VK_FORMAT_R16_SINT;
      break;
    case FormatType::kR16_SNORM:
      ret = VK_FORMAT_R16_SNORM;
      break;
    case FormatType::kR16_SSCALED:
      ret = VK_FORMAT_R16_SSCALED;
      break;
    case FormatType::kR16_UINT:
      ret = VK_FORMAT_R16_UINT;
      break;
    case FormatType::kR16_UNORM:
      ret = VK_FORMAT_R16_UNORM;
      break;
    case FormatType::kR16_USCALED:
      ret = VK_FORMAT_R16_USCALED;
      break;
    case FormatType::kR32G32B32A32_SFLOAT:
      ret = VK_FORMAT_R32G32B32A32_SFLOAT;
      break;
    case FormatType::kR32G32B32A32_SINT:
      ret = VK_FORMAT_R32G32B32A32_SINT;
      break;
    case FormatType::kR32G32B32A32_UINT:
      ret = VK_FORMAT_R32G32B32A32_UINT;
      break;
    case FormatType::kR32G32B32_SFLOAT:
      ret = VK_FORMAT_R32G32B32_SFLOAT;
      break;
    case FormatType::kR32G32B32_SINT:
      ret = VK_FORMAT_R32G32B32_SINT;
      break;
    case FormatType::kR32G32B32_UINT:
      ret = VK_FORMAT_R32G32B32_UINT;
      break;
    case FormatType::kR32G32_SFLOAT:
      ret = VK_FORMAT_R32G32_SFLOAT;
      break;
    case FormatType::kR32G32_SINT:
      ret = VK_FORMAT_R32G32_SINT;
      break;
    case FormatType::kR32G32_UINT:
      ret = VK_FORMAT_R32G32_UINT;
      break;
    case FormatType::kR32_SFLOAT:
      ret = VK_FORMAT_R32_SFLOAT;
      break;
    case FormatType::kR32_SINT:
      ret = VK_FORMAT_R32_SINT;
      break;
    case FormatType::kR32_UINT:
      ret = VK_FORMAT_R32_UINT;
      break;
    case FormatType::kR4G4B4A4_UNORM_PACK16:
      ret = VK_FORMAT_R4G4B4A4_UNORM_PACK16;
      break;
    case FormatType::kR4G4_UNORM_PACK8:
      ret = VK_FORMAT_R4G4_UNORM_PACK8;
      break;
    case FormatType::kR5G5B5A1_UNORM_PACK16:
      ret = VK_FORMAT_R5G5B5A1_UNORM_PACK16;
      break;
    case FormatType::kR5G6B5_UNORM_PACK16:
      ret = VK_FORMAT_R5G6B5_UNORM_PACK16;
      break;
    case FormatType::kR64G64B64A64_SFLOAT:
      ret = VK_FORMAT_R64G64B64A64_SFLOAT;
      break;
    case FormatType::kR64G64B64A64_SINT:
      ret = VK_FORMAT_R64G64B64A64_SINT;
      break;
    case FormatType::kR64G64B64A64_UINT:
      ret = VK_FORMAT_R64G64B64A64_UINT;
      break;
    case FormatType::kR64G64B64_SFLOAT:
      ret = VK_FORMAT_R64G64B64_SFLOAT;
      break;
    case FormatType::kR64G64B64_SINT:
      ret = VK_FORMAT_R64G64B64_SINT;
      break;
    case FormatType::kR64G64B64_UINT:
      ret = VK_FORMAT_R64G64B64_UINT;
      break;
    case FormatType::kR64G64_SFLOAT:
      ret = VK_FORMAT_R64G64_SFLOAT;
      break;
    case FormatType::kR64G64_SINT:
      ret = VK_FORMAT_R64G64_SINT;
      break;
    case FormatType::kR64G64_UINT:
      ret = VK_FORMAT_R64G64_UINT;
      break;
    case FormatType::kR64_SFLOAT:
      ret = VK_FORMAT_R64_SFLOAT;
      break;
    case FormatType::kR64_SINT:
      ret = VK_FORMAT_R64_SINT;
      break;
    case FormatType::kR64_UINT:
      ret = VK_FORMAT_R64_UINT;
      break;
    case FormatType::kR8G8B8A8_SINT:
      ret = VK_FORMAT_R8G8B8A8_SINT;
      break;
    case FormatType::kR8G8B8A8_SNORM:
      ret = VK_FORMAT_R8G8B8A8_SNORM;
      break;
    case FormatType::kR8G8B8A8_SRGB:
      ret = VK_FORMAT_R8G8B8A8_SRGB;
      break;
    case FormatType::kR8G8B8A8_SSCALED:
      ret = VK_FORMAT_R8G8B8A8_SSCALED;
      break;
    case FormatType::kR8G8B8A8_UINT:
      ret = VK_FORMAT_R8G8B8A8_UINT;
      break;
    case FormatType::kR8G8B8A8_UNORM:
      ret = VK_FORMAT_R8G8B8A8_UNORM;
      break;
    case FormatType::kR8G8B8A8_USCALED:
      ret = VK_FORMAT_R8G8B8A8_USCALED;
      break;
    case FormatType::kR8G8B8_SINT:
      ret = VK_FORMAT_R8G8B8_SINT;
      break;
    case FormatType::kR8G8B8_SNORM:
      ret = VK_FORMAT_R8G8B8_SNORM;
      break;
    case FormatType::kR8G8B8_SRGB:
      ret = VK_FORMAT_R8G8B8_SRGB;
      break;
    case FormatType::kR8G8B8_SSCALED:
      ret = VK_FORMAT_R8G8B8_SSCALED;
      break;
    case FormatType::kR8G8B8_UINT:
      ret = VK_FORMAT_R8G8B8_UINT;
      break;
    case FormatType::kR8G8B8_UNORM:
      ret = VK_FORMAT_R8G8B8_UNORM;
      break;
    case FormatType::kR8G8B8_USCALED:
      ret = VK_FORMAT_R8G8B8_USCALED;
      break;
    case FormatType::kR8G8_SINT:
      ret = VK_FORMAT_R8G8_SINT;
      break;
    case FormatType::kR8G8_SNORM:
      ret = VK_FORMAT_R8G8_SNORM;
      break;
    case FormatType::kR8G8_SRGB:
      ret = VK_FORMAT_R8G8_SRGB;
      break;
    case FormatType::kR8G8_SSCALED:
      ret = VK_FORMAT_R8G8_SSCALED;
      break;
    case FormatType::kR8G8_UINT:
      ret = VK_FORMAT_R8G8_UINT;
      break;
    case FormatType::kR8G8_UNORM:
      ret = VK_FORMAT_R8G8_UNORM;
      break;
    case FormatType::kR8G8_USCALED:
      ret = VK_FORMAT_R8G8_USCALED;
      break;
    case FormatType::kR8_SINT:
      ret = VK_FORMAT_R8_SINT;
      break;
    case FormatType::kR8_SNORM:
      ret = VK_FORMAT_R8_SNORM;
      break;
    case FormatType::kR8_SRGB:
      ret = VK_FORMAT_R8_SRGB;
      break;
    case FormatType::kR8_SSCALED:
      ret = VK_FORMAT_R8_SSCALED;
      break;
    case FormatType::kR8_UINT:
      ret = VK_FORMAT_R8_UINT;
      break;
    case FormatType::kR8_UNORM:
      ret = VK_FORMAT_R8_UNORM;
      break;
    case FormatType::kR8_USCALED:
      ret = VK_FORMAT_R8_USCALED;
      break;
    case FormatType::kS8_UINT:
      ret = VK_FORMAT_S8_UINT;
      break;
    case FormatType::kX8_D24_UNORM_PACK32:
      ret = VK_FORMAT_X8_D24_UNORM_PACK32;
      break;
  }
  return ret;
}

uint32_t VkFormatToByteSize(VkFormat format) {
  switch (format) {
    case VK_FORMAT_R4G4_UNORM_PACK8:
    case VK_FORMAT_R8_SINT:
    case VK_FORMAT_R8_SNORM:
    case VK_FORMAT_R8_SRGB:
    case VK_FORMAT_R8_SSCALED:
    case VK_FORMAT_R8_UINT:
    case VK_FORMAT_R8_UNORM:
    case VK_FORMAT_R8_USCALED:
    case VK_FORMAT_S8_UINT:
      return 1U;
    case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
    case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
    case VK_FORMAT_B5G6R5_UNORM_PACK16:
    case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
    case VK_FORMAT_D16_UNORM:
    case VK_FORMAT_R16_SFLOAT:
    case VK_FORMAT_R16_SINT:
    case VK_FORMAT_R16_SNORM:
    case VK_FORMAT_R16_SSCALED:
    case VK_FORMAT_R16_UINT:
    case VK_FORMAT_R16_UNORM:
    case VK_FORMAT_R16_USCALED:
    case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
    case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
    case VK_FORMAT_R5G6B5_UNORM_PACK16:
    case VK_FORMAT_R8G8_SINT:
    case VK_FORMAT_R8G8_SNORM:
    case VK_FORMAT_R8G8_SRGB:
    case VK_FORMAT_R8G8_SSCALED:
    case VK_FORMAT_R8G8_UINT:
    case VK_FORMAT_R8G8_UNORM:
    case VK_FORMAT_R8G8_USCALED:
      return 2U;
    case VK_FORMAT_B8G8R8_SINT:
    case VK_FORMAT_B8G8R8_SNORM:
    case VK_FORMAT_B8G8R8_SRGB:
    case VK_FORMAT_B8G8R8_SSCALED:
    case VK_FORMAT_B8G8R8_UINT:
    case VK_FORMAT_B8G8R8_UNORM:
    case VK_FORMAT_B8G8R8_USCALED:
    case VK_FORMAT_D16_UNORM_S8_UINT:
    case VK_FORMAT_R8G8B8_SINT:
    case VK_FORMAT_R8G8B8_SNORM:
    case VK_FORMAT_R8G8B8_SRGB:
    case VK_FORMAT_R8G8B8_SSCALED:
    case VK_FORMAT_R8G8B8_UINT:
    case VK_FORMAT_R8G8B8_UNORM:
    case VK_FORMAT_R8G8B8_USCALED:
      return 3U;
    case VK_FORMAT_A2B10G10R10_SINT_PACK32:
    case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
    case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
    case VK_FORMAT_A2B10G10R10_UINT_PACK32:
    case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
    case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
    case VK_FORMAT_A2R10G10B10_SINT_PACK32:
    case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
    case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
    case VK_FORMAT_A2R10G10B10_UINT_PACK32:
    case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
    case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
    case VK_FORMAT_A8B8G8R8_SINT_PACK32:
    case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
    case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
    case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
    case VK_FORMAT_A8B8G8R8_UINT_PACK32:
    case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
    case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
    case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
    case VK_FORMAT_B8G8R8A8_SINT:
    case VK_FORMAT_B8G8R8A8_SNORM:
    case VK_FORMAT_B8G8R8A8_SRGB:
    case VK_FORMAT_B8G8R8A8_SSCALED:
    case VK_FORMAT_B8G8R8A8_UINT:
    case VK_FORMAT_B8G8R8A8_UNORM:
    case VK_FORMAT_B8G8R8A8_USCALED:
    case VK_FORMAT_D24_UNORM_S8_UINT:
    case VK_FORMAT_D32_SFLOAT:
    case VK_FORMAT_R16G16_SFLOAT:
    case VK_FORMAT_R16G16_SINT:
    case VK_FORMAT_R16G16_SNORM:
    case VK_FORMAT_R16G16_SSCALED:
    case VK_FORMAT_R16G16_UINT:
    case VK_FORMAT_R16G16_UNORM:
    case VK_FORMAT_R16G16_USCALED:
    case VK_FORMAT_R32_SFLOAT:
    case VK_FORMAT_R32_SINT:
    case VK_FORMAT_R32_UINT:
    case VK_FORMAT_X8_D24_UNORM_PACK32:
    case VK_FORMAT_R8G8B8A8_SINT:
    case VK_FORMAT_R8G8B8A8_SNORM:
    case VK_FORMAT_R8G8B8A8_SRGB:
    case VK_FORMAT_R8G8B8A8_SSCALED:
    case VK_FORMAT_R8G8B8A8_UINT:
    case VK_FORMAT_R8G8B8A8_UNORM:
    case VK_FORMAT_R8G8B8A8_USCALED:
      return 4U;
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
      return 5U;
    case VK_FORMAT_R16G16B16_SFLOAT:
    case VK_FORMAT_R16G16B16_SINT:
    case VK_FORMAT_R16G16B16_SNORM:
    case VK_FORMAT_R16G16B16_SSCALED:
    case VK_FORMAT_R16G16B16_UINT:
    case VK_FORMAT_R16G16B16_UNORM:
    case VK_FORMAT_R16G16B16_USCALED:
      return 6U;
    case VK_FORMAT_R16G16B16A16_SFLOAT:
    case VK_FORMAT_R16G16B16A16_SINT:
    case VK_FORMAT_R16G16B16A16_SNORM:
    case VK_FORMAT_R16G16B16A16_SSCALED:
    case VK_FORMAT_R16G16B16A16_UINT:
    case VK_FORMAT_R16G16B16A16_UNORM:
    case VK_FORMAT_R16G16B16A16_USCALED:
    case VK_FORMAT_R32G32_SFLOAT:
    case VK_FORMAT_R32G32_SINT:
    case VK_FORMAT_R32G32_UINT:
    case VK_FORMAT_R64_SFLOAT:
    case VK_FORMAT_R64_SINT:
    case VK_FORMAT_R64_UINT:
      return 8U;
    case VK_FORMAT_R32G32B32_SFLOAT:
    case VK_FORMAT_R32G32B32_SINT:
    case VK_FORMAT_R32G32B32_UINT:
      return 12U;
    case VK_FORMAT_R32G32B32A32_SFLOAT:
    case VK_FORMAT_R32G32B32A32_SINT:
    case VK_FORMAT_R32G32B32A32_UINT:
    case VK_FORMAT_R64G64_SFLOAT:
    case VK_FORMAT_R64G64_SINT:
    case VK_FORMAT_R64G64_UINT:
      return 16U;
    case VK_FORMAT_R64G64B64_SFLOAT:
    case VK_FORMAT_R64G64B64_SINT:
    case VK_FORMAT_R64G64B64_UINT:
      return 24U;
    case VK_FORMAT_R64G64B64A64_SFLOAT:
    case VK_FORMAT_R64G64B64A64_SINT:
    case VK_FORMAT_R64G64B64A64_UINT:
      return 32U;
    default:
      break;
  }
  return 0;
}

bool VkFormatHasStencilComponent(VkFormat format) {
  return format == VK_FORMAT_D24_UNORM_S8_UINT ||
         format == VK_FORMAT_D16_UNORM_S8_UINT ||
         format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_S8_UINT;
}

}  // namespace vulkan
}  // namespace amber
