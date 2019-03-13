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

#include "src/vulkan/device.h"

namespace amber {
namespace vulkan {

VkDescriptorType ToVkDescriptorType(DescriptorType type) {
  switch (type) {
    case DescriptorType::kStorageBuffer:
      return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    case DescriptorType::kUniformBuffer:
      return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  }

  assert(false && "Unknown resource type");
  return VK_DESCRIPTOR_TYPE_SAMPLER;
}

Descriptor::Descriptor(DescriptorType type,
                       Device* device,
                       uint32_t desc_set,
                       uint32_t binding)
    : descriptor_set_(desc_set),
      binding_(binding),
      device_(device),
      type_(type) {}

Descriptor::~Descriptor() = default;

VkWriteDescriptorSet Descriptor::GetWriteDescriptorSet(
    VkDescriptorSet descriptor_set,
    VkDescriptorType descriptor_type) const {
  VkWriteDescriptorSet write = VkWriteDescriptorSet();
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

void Descriptor::UpdateVkDescriptorSet(const VkWriteDescriptorSet& write) {
  device_->GetPtrs()->vkUpdateDescriptorSets(device_->GetDevice(), 1, &write, 0,
                                             nullptr);
  is_descriptor_set_update_needed_ = false;
}

}  // namespace vulkan
}  // namespace amber
