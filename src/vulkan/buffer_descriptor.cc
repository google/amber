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

#include "src/vulkan/buffer_descriptor.h"

#include <vector>

#include "src/make_unique.h"
#include "src/vulkan/command_buffer.h"
#include "src/vulkan/device.h"

namespace amber {
namespace vulkan {

BufferDescriptor::BufferDescriptor(Buffer* buffer,
                                   DescriptorType type,
                                   Device* device,
                                   uint32_t desc_set,
                                   uint32_t binding)
    : Descriptor(buffer, type, device, desc_set, binding) {}

BufferDescriptor::~BufferDescriptor() = default;

Result BufferDescriptor::CreateResourceIfNeeded() {
  if (transfer_buffer_) {
    return Result(
        "Vulkan: BufferDescriptor::CreateResourceIfNeeded() must be called "
        "only when |transfer_buffer| is empty");
  }

  if (amber_buffer_ && amber_buffer_->ValuePtr()->empty())
    return {};

  uint32_t size_in_bytes =
      amber_buffer_ ? static_cast<uint32_t>(amber_buffer_->ValuePtr()->size())
                    : 0;
  transfer_buffer_ = MakeUnique<TransferBuffer>(device_, size_in_bytes);

  Result r = transfer_buffer_->Initialize(
      (IsStorageBuffer() ? VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
                         : VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) |
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
  if (!r.IsSuccess())
    return r;

  is_descriptor_set_update_needed_ = true;
  return {};
}

Result BufferDescriptor::MoveResourceToBufferOutput() {
  Result r = Descriptor::MoveResourceToBufferOutput();
  transfer_buffer_ = nullptr;

  return r;
}

void BufferDescriptor::UpdateDescriptorSetIfNeeded(
    VkDescriptorSet descriptor_set) {
  if (!is_descriptor_set_update_needed_)
    return;

  VkDescriptorBufferInfo buffer_info = VkDescriptorBufferInfo();
  buffer_info.buffer = transfer_buffer_->GetVkBuffer();
  buffer_info.offset = 0;
  buffer_info.range = VK_WHOLE_SIZE;

  VkWriteDescriptorSet write = VkWriteDescriptorSet();
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.dstSet = descriptor_set;
  write.dstBinding = binding_;
  write.dstArrayElement = 0;
  write.descriptorCount = 1;
  write.descriptorType = GetVkDescriptorType();
  write.pBufferInfo = &buffer_info;

  device_->GetPtrs()->vkUpdateDescriptorSets(device_->GetVkDevice(), 1, &write,
                                             0, nullptr);
  is_descriptor_set_update_needed_ = false;
}

}  // namespace vulkan
}  // namespace amber
