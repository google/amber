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

#include "src/vulkan/sampler.h"

namespace amber {
namespace vulkan {
namespace {

VkSamplerAddressMode GetVkAddressMode(AddressMode mode) {
  switch (mode) {
    case AddressMode::kRepeat:
      return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    case AddressMode::kMirroredRepeat:
      return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    case AddressMode::kClampToEdge:
      return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    case AddressMode::kClampToBorder:
      return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    default:
      assert(mode == AddressMode::kMirrorClampToEdge);
      return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
  }
}

VkBorderColor GetVkBorderColor(BorderColor color) {
  switch (color) {
    case BorderColor::kFloatTransparentBlack:
      return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    case BorderColor::kIntTransparentBlack:
      return VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
    case BorderColor::kFloatOpaqueBlack:
      return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
    case BorderColor::kIntOpaqueBlack:
      return VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    case BorderColor::kFloatOpaqueWhite:
      return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    default:
      assert(color == BorderColor::kIntOpaqueWhite);
      return VK_BORDER_COLOR_INT_OPAQUE_WHITE;
  }
}

VkCompareOp ToVkCompareOp(CompareOp op) {
  switch (op) {
    case CompareOp::kNever:
      return VK_COMPARE_OP_NEVER;
    case CompareOp::kLess:
      return VK_COMPARE_OP_LESS;
    case CompareOp::kEqual:
      return VK_COMPARE_OP_EQUAL;
    case CompareOp::kLessOrEqual:
      return VK_COMPARE_OP_LESS_OR_EQUAL;
    case CompareOp::kGreater:
      return VK_COMPARE_OP_GREATER;
    case CompareOp::kNotEqual:
      return VK_COMPARE_OP_NOT_EQUAL;
    case CompareOp::kGreaterOrEqual:
      return VK_COMPARE_OP_GREATER_OR_EQUAL;
    case CompareOp::kAlways:
      return VK_COMPARE_OP_ALWAYS;
    case CompareOp::kUnknown:
      break;
  }
  assert(false && "Vulkan::Unknown CompareOp");
  return VK_COMPARE_OP_NEVER;
}

}  // namespace

Sampler::Sampler(Device* device) : device_(device) {}

Result Sampler::CreateSampler(amber::Sampler* sampler) {
  VkSamplerCreateInfo sampler_info = VkSamplerCreateInfo();
  sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  sampler_info.magFilter = sampler->GetMagFilter() == FilterType::kLinear
                               ? VK_FILTER_LINEAR
                               : VK_FILTER_NEAREST;
  sampler_info.minFilter = sampler->GetMinFilter() == FilterType::kLinear
                               ? VK_FILTER_LINEAR
                               : VK_FILTER_NEAREST;
  sampler_info.mipmapMode = sampler->GetMipmapMode() == FilterType::kLinear
                                ? VK_SAMPLER_MIPMAP_MODE_LINEAR
                                : VK_SAMPLER_MIPMAP_MODE_NEAREST;
  sampler_info.addressModeU = GetVkAddressMode(sampler->GetAddressModeU());
  sampler_info.addressModeV = GetVkAddressMode(sampler->GetAddressModeV());
  sampler_info.addressModeW = GetVkAddressMode(sampler->GetAddressModeW());
  sampler_info.borderColor = GetVkBorderColor(sampler->GetBorderColor());
  sampler_info.minLod = sampler->GetMinLOD();
  sampler_info.maxLod = sampler->GetMaxLOD();
  sampler_info.unnormalizedCoordinates =
      (sampler->GetNormalizedCoords() ? VK_FALSE : VK_TRUE);
  sampler_info.compareEnable =
      (sampler->GetCompareEnable() ? VK_TRUE : VK_FALSE);
  sampler_info.compareOp = ToVkCompareOp(sampler->GetCompareOp());

  if (device_->GetPtrs()->vkCreateSampler(device_->GetVkDevice(), &sampler_info,
                                          nullptr, &sampler_) != VK_SUCCESS) {
    return Result("Vulkan::Calling vkCreateSampler Fail");
  }

  return {};
}

Sampler::~Sampler() {
  if (sampler_ != VK_NULL_HANDLE) {
    device_->GetPtrs()->vkDestroySampler(device_->GetVkDevice(), sampler_,
                                         nullptr);
  }
}

}  // namespace vulkan
}  // namespace amber
