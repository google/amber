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

#include "src/vulkan/sampler_descriptor.h"
#include "src/vulkan/device.h"
#include "src/vulkan/resource.h"

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

}  // namespace

VkSamplerCreateInfo GetSamplerCreateInfo(Sampler* sampler) {
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
  sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  sampler_info.borderColor = GetVkBorderColor(sampler->GetBorderColor());
  sampler_info.maxLod = 1.0f;

  return sampler_info;
}

SamplerDescriptor::SamplerDescriptor(Sampler* sampler,
                                     DescriptorType type,
                                     Device* device,
                                     uint32_t desc_set,
                                     uint32_t binding)
    : Descriptor(type, device, desc_set, binding),
      amber_sampler_(sampler),
      sampler_(VK_NULL_HANDLE) {}

SamplerDescriptor::~SamplerDescriptor() {
  if (sampler_ != VK_NULL_HANDLE) {
    device_->GetPtrs()->vkDestroySampler(device_->GetVkDevice(), sampler_,
                                         nullptr);
  }
}

Result SamplerDescriptor::CreateResourceIfNeeded() {
  VkSamplerCreateInfo sampler_info = GetSamplerCreateInfo(amber_sampler_);

  if (device_->GetPtrs()->vkCreateSampler(device_->GetVkDevice(), &sampler_info,
                                          nullptr, &sampler_) != VK_SUCCESS) {
    return Result("Vulkan::Calling vkCreateSampler Fail");
  }

  return {};
}

void SamplerDescriptor::UpdateDescriptorSetIfNeeded(
    VkDescriptorSet descriptor_set) {
  VkDescriptorImageInfo image_info = {sampler_, VK_NULL_HANDLE,
                                      VK_IMAGE_LAYOUT_GENERAL};

  VkWriteDescriptorSet write = VkWriteDescriptorSet();
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.dstSet = descriptor_set;
  write.dstBinding = binding_;
  write.dstArrayElement = 0;
  write.descriptorCount = 1;
  write.descriptorType = GetVkDescriptorType();
  write.pImageInfo = &image_info;

  device_->GetPtrs()->vkUpdateDescriptorSets(device_->GetVkDevice(), 1, &write,
                                             0, nullptr);
}

}  // namespace vulkan
}  // namespace amber
