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

#include <algorithm>
#include <cassert>
#include <cstring>
#include <utility>
#include <vector>

#include "src/engine.h"
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
    : device_(device),
      amber_buffer_(buffer),
      type_(type),
      descriptor_set_(desc_set),
      binding_(binding) {}

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

void BufferDescriptor::RecordCopyDataToResourceIfNeeded(
    CommandBuffer* command) {
  if (!transfer_buffer_)
    return;

  if (amber_buffer_ && !amber_buffer_->ValuePtr()->empty()) {
    transfer_buffer_->UpdateMemoryWithRawData(*amber_buffer_->ValuePtr());
    amber_buffer_->ValuePtr()->clear();
  }

  transfer_buffer_->CopyToDevice(command);
}

Result BufferDescriptor::RecordCopyDataToHost(CommandBuffer* command) {
  if (!transfer_buffer_) {
    return Result(
        "Vulkan: BufferDescriptor::RecordCopyDataToHost() no transfer buffer");
  }

  transfer_buffer_->CopyToHost(command);
  return {};
}

Result BufferDescriptor::MoveResourceToBufferOutput() {
  if (!transfer_buffer_) {
    return Result(
        "Vulkan: BufferDescriptor::MoveResourceToBufferOutput() no transfer"
        " buffer");
  }

  // Only need to copy the buffer back if we have an attached amber buffer to
  // write too.
  if (amber_buffer_) {
    void* resource_memory_ptr = transfer_buffer_->HostAccessibleMemoryPtr();
    if (!resource_memory_ptr) {
      return Result(
          "Vulkan: BufferDescriptor::MoveResourceToBufferOutput() "
          "no host accessible memory pointer");
    }

    if (!amber_buffer_->ValuePtr()->empty()) {
      return Result(
          "Vulkan: BufferDescriptor::MoveResourceToBufferOutput() "
          "output buffer is not empty");
    }

    auto size_in_bytes = transfer_buffer_->GetSizeInBytes();
    amber_buffer_->SetElementCount(size_in_bytes /
                                   amber_buffer_->GetFormat()->SizeInBytes());
    amber_buffer_->ValuePtr()->resize(size_in_bytes);
    std::memcpy(amber_buffer_->ValuePtr()->data(), resource_memory_ptr,
                size_in_bytes);
  }

  transfer_buffer_ = nullptr;
  return {};
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
  write.descriptorType = IsStorageBuffer() ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
                                           : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  write.pBufferInfo = &buffer_info;

  device_->GetPtrs()->vkUpdateDescriptorSets(device_->GetVkDevice(), 1, &write,
                                             0, nullptr);
  is_descriptor_set_update_needed_ = false;
}

Result BufferDescriptor::SetSizeInElements(uint32_t element_count) {
  if (!amber_buffer_)
    return Result("missing amber_buffer for SetSizeInElements call");

  amber_buffer_->SetSizeInElements(element_count);
  return {};
}

Result BufferDescriptor::AddToBuffer(const std::vector<Value>& values,
                                     uint32_t offset) {
  if (!amber_buffer_)
    return Result("missing amber_buffer for AddToBuffer call");

  return amber_buffer_->SetDataWithOffset(values, offset);
}

VkDescriptorType BufferDescriptor::GetVkDescriptorType() const {
  return IsStorageBuffer() ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
                           : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
}

}  // namespace vulkan
}  // namespace amber
