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

#include "src/vulkan/descriptor.h"

#include <cassert>

namespace amber {
namespace vulkan {

VkDescriptorType ToVkDescriptorType(DescriptorType type) {
  switch (type) {
    case DescriptorType::kStorageImage:
      return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    case DescriptorType::kSampler:
      return VK_DESCRIPTOR_TYPE_SAMPLER;
    case DescriptorType::kSampledImage:
      return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    case DescriptorType::kCombinedImageSampler:
      return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    case DescriptorType::kUniformTexelBuffer:
      return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
    case DescriptorType::kStorageTexelBuffer:
      return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
    case DescriptorType::kStorageBuffer:
      return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    case DescriptorType::kUniformBuffer:
      return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    case DescriptorType::kDynamicUniformBuffer:
      return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    case DescriptorType::kDynamicStorageBuffer:
      return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
    case DescriptorType::kInputAttachment:
      return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
  }

  assert(false && "Unknown resource type");
  return VK_DESCRIPTOR_TYPE_SAMPLER;
}

Descriptor::Descriptor(DescriptorType type,
                       VkDevice device,
                       uint32_t desc_set,
                       uint32_t binding)
    : descriptor_set_(desc_set),
      binding_(binding),
      type_(type),
      device_(device) {}

Descriptor::~Descriptor() = default;

VkWriteDescriptorSet Descriptor::GetWriteDescriptorSet(
    VkDescriptorSet descriptor_set,
    VkDescriptorType descriptor_type) const {
  VkWriteDescriptorSet write = {};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.dstSet = descriptor_set;
  write.dstBinding = binding_;
  write.dstArrayElement = 0;
  write.descriptorCount = 1;
  write.descriptorType = descriptor_type;
  return write;
}

Result Descriptor::UpdateDescriptorSetForBuffer(
    VkDescriptorSet descriptor_set,
    VkDescriptorType descriptor_type,
    const VkDescriptorBufferInfo& buffer_info) {
  VkWriteDescriptorSet write =
      GetWriteDescriptorSet(descriptor_set, descriptor_type);
  switch (descriptor_type) {
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
      write.pBufferInfo = &buffer_info;
      break;
    default:
      return Result(
          "Vulkan::UpdateDescriptorSetForBuffer not descriptor based on "
          "buffer");
  }

  UpdateVkDescriptorSet(write);
  return {};
}

Result Descriptor::UpdateDescriptorSetForImage(
    VkDescriptorSet descriptor_set,
    VkDescriptorType descriptor_type,
    const VkDescriptorImageInfo& image_info) {
  VkWriteDescriptorSet write =
      GetWriteDescriptorSet(descriptor_set, descriptor_type);
  switch (descriptor_type) {
    case VK_DESCRIPTOR_TYPE_SAMPLER:
    case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
    case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
    case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
    case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
      write.pImageInfo = &image_info;
      break;
    default:
      return Result(
          "Vulkan::UpdateDescriptorSetForImage not descriptor based on image");
  }

  UpdateVkDescriptorSet(write);
  return {};
}

Result Descriptor::UpdateDescriptorSetForBufferView(
    VkDescriptorSet descriptor_set,
    VkDescriptorType descriptor_type,
    const VkBufferView& texel_view) {
  VkWriteDescriptorSet write =
      GetWriteDescriptorSet(descriptor_set, descriptor_type);
  switch (descriptor_type) {
    case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
    case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
      write.pTexelBufferView = &texel_view;
      break;
    default:
      return Result(
          "Vulkan::UpdateDescriptorSetForImage not descriptor based on buffer "
          "view");
  }

  UpdateVkDescriptorSet(write);
  return {};
}

void Descriptor::UpdateVkDescriptorSet(const VkWriteDescriptorSet& write) {
  vkUpdateDescriptorSets(device_, 1, &write, 0, nullptr);
  is_descriptor_set_update_needed_ = false;
}

void Descriptor::AddToBufferDataQueue(DataType type,
                                      uint32_t offset,
                                      size_t size_in_bytes,
                                      const std::vector<Value>& values) {
  buffer_data_queue_.push_back(
      {offset, size_in_bytes, type, values, std::vector<uint8_t>()});
}

}  // namespace vulkan
}  // namespace amber
