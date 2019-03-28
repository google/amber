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

#include "src/format.h"

namespace amber {

Format::Format() = default;

Format::Format(const Format&) = default;

Format::~Format() = default;

uint32_t Format::GetByteSize() const {
  switch (type_) {
    case FormatType::kR4G4_UNORM_PACK8:
    case FormatType::kR8_SINT:
    case FormatType::kR8_SNORM:
    case FormatType::kR8_SRGB:
    case FormatType::kR8_SSCALED:
    case FormatType::kR8_UINT:
    case FormatType::kR8_UNORM:
    case FormatType::kR8_USCALED:
    case FormatType::kS8_UINT:
      return 1U;
    case FormatType::kB4G4R4A4_UNORM_PACK16:
    case FormatType::kB5G5R5A1_UNORM_PACK16:
    case FormatType::kB5G6R5_UNORM_PACK16:
    case FormatType::kA1R5G5B5_UNORM_PACK16:
    case FormatType::kD16_UNORM:
    case FormatType::kR16_SFLOAT:
    case FormatType::kR16_SINT:
    case FormatType::kR16_SNORM:
    case FormatType::kR16_SSCALED:
    case FormatType::kR16_UINT:
    case FormatType::kR16_UNORM:
    case FormatType::kR16_USCALED:
    case FormatType::kR4G4B4A4_UNORM_PACK16:
    case FormatType::kR5G5B5A1_UNORM_PACK16:
    case FormatType::kR5G6B5_UNORM_PACK16:
    case FormatType::kR8G8_SINT:
    case FormatType::kR8G8_SNORM:
    case FormatType::kR8G8_SRGB:
    case FormatType::kR8G8_SSCALED:
    case FormatType::kR8G8_UINT:
    case FormatType::kR8G8_UNORM:
    case FormatType::kR8G8_USCALED:
      return 2U;
    case FormatType::kB8G8R8_SINT:
    case FormatType::kB8G8R8_SNORM:
    case FormatType::kB8G8R8_SRGB:
    case FormatType::kB8G8R8_SSCALED:
    case FormatType::kB8G8R8_UINT:
    case FormatType::kB8G8R8_UNORM:
    case FormatType::kB8G8R8_USCALED:
    case FormatType::kD16_UNORM_S8_UINT:
    case FormatType::kR8G8B8_SINT:
    case FormatType::kR8G8B8_SNORM:
    case FormatType::kR8G8B8_SRGB:
    case FormatType::kR8G8B8_SSCALED:
    case FormatType::kR8G8B8_UINT:
    case FormatType::kR8G8B8_UNORM:
    case FormatType::kR8G8B8_USCALED:
      return 3U;
    case FormatType::kA2B10G10R10_SINT_PACK32:
    case FormatType::kA2B10G10R10_SNORM_PACK32:
    case FormatType::kA2B10G10R10_SSCALED_PACK32:
    case FormatType::kA2B10G10R10_UINT_PACK32:
    case FormatType::kA2B10G10R10_UNORM_PACK32:
    case FormatType::kA2B10G10R10_USCALED_PACK32:
    case FormatType::kA2R10G10B10_SINT_PACK32:
    case FormatType::kA2R10G10B10_SNORM_PACK32:
    case FormatType::kA2R10G10B10_SSCALED_PACK32:
    case FormatType::kA2R10G10B10_UINT_PACK32:
    case FormatType::kA2R10G10B10_UNORM_PACK32:
    case FormatType::kA2R10G10B10_USCALED_PACK32:
    case FormatType::kA8B8G8R8_SINT_PACK32:
    case FormatType::kA8B8G8R8_SNORM_PACK32:
    case FormatType::kA8B8G8R8_SRGB_PACK32:
    case FormatType::kA8B8G8R8_SSCALED_PACK32:
    case FormatType::kA8B8G8R8_UINT_PACK32:
    case FormatType::kA8B8G8R8_UNORM_PACK32:
    case FormatType::kA8B8G8R8_USCALED_PACK32:
    case FormatType::kB10G11R11_UFLOAT_PACK32:
    case FormatType::kB8G8R8A8_SINT:
    case FormatType::kB8G8R8A8_SNORM:
    case FormatType::kB8G8R8A8_SRGB:
    case FormatType::kB8G8R8A8_SSCALED:
    case FormatType::kB8G8R8A8_UINT:
    case FormatType::kB8G8R8A8_UNORM:
    case FormatType::kB8G8R8A8_USCALED:
    case FormatType::kD24_UNORM_S8_UINT:
    case FormatType::kD32_SFLOAT:
    case FormatType::kR16G16_SFLOAT:
    case FormatType::kR16G16_SINT:
    case FormatType::kR16G16_SNORM:
    case FormatType::kR16G16_SSCALED:
    case FormatType::kR16G16_UINT:
    case FormatType::kR16G16_UNORM:
    case FormatType::kR16G16_USCALED:
    case FormatType::kR32_SFLOAT:
    case FormatType::kR32_SINT:
    case FormatType::kR32_UINT:
    case FormatType::kX8_D24_UNORM_PACK32:
    case FormatType::kR8G8B8A8_SINT:
    case FormatType::kR8G8B8A8_SNORM:
    case FormatType::kR8G8B8A8_SRGB:
    case FormatType::kR8G8B8A8_SSCALED:
    case FormatType::kR8G8B8A8_UINT:
    case FormatType::kR8G8B8A8_UNORM:
    case FormatType::kR8G8B8A8_USCALED:
      return 4U;
    case FormatType::kD32_SFLOAT_S8_UINT:
      return 5U;
    case FormatType::kR16G16B16_SFLOAT:
    case FormatType::kR16G16B16_SINT:
    case FormatType::kR16G16B16_SNORM:
    case FormatType::kR16G16B16_SSCALED:
    case FormatType::kR16G16B16_UINT:
    case FormatType::kR16G16B16_UNORM:
    case FormatType::kR16G16B16_USCALED:
      return 6U;
    case FormatType::kR16G16B16A16_SFLOAT:
    case FormatType::kR16G16B16A16_SINT:
    case FormatType::kR16G16B16A16_SNORM:
    case FormatType::kR16G16B16A16_SSCALED:
    case FormatType::kR16G16B16A16_UINT:
    case FormatType::kR16G16B16A16_UNORM:
    case FormatType::kR16G16B16A16_USCALED:
    case FormatType::kR32G32_SFLOAT:
    case FormatType::kR32G32_SINT:
    case FormatType::kR32G32_UINT:
    case FormatType::kR64_SFLOAT:
    case FormatType::kR64_SINT:
    case FormatType::kR64_UINT:
      return 8U;
    case FormatType::kR32G32B32_SFLOAT:
    case FormatType::kR32G32B32_SINT:
    case FormatType::kR32G32B32_UINT:
      return 12U;
    case FormatType::kR32G32B32A32_SFLOAT:
    case FormatType::kR32G32B32A32_SINT:
    case FormatType::kR32G32B32A32_UINT:
    case FormatType::kR64G64_SFLOAT:
    case FormatType::kR64G64_SINT:
    case FormatType::kR64G64_UINT:
      return 16U;
    case FormatType::kR64G64B64_SFLOAT:
    case FormatType::kR64G64B64_SINT:
    case FormatType::kR64G64B64_UINT:
      return 24U;
    case FormatType::kR64G64B64A64_SFLOAT:
    case FormatType::kR64G64B64A64_SINT:
    case FormatType::kR64G64B64A64_UINT:
      return 32U;
    default:
      break;
  }
  return 0;
}

}  // namespace amber
