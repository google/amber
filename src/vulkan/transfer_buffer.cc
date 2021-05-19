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

#include "src/vulkan/transfer_buffer.h"

#include "src/vulkan/command_buffer.h"
#include "src/vulkan/device.h"

namespace amber {
namespace vulkan {

TransferBuffer::TransferBuffer(Device* device,
                               uint32_t size_in_bytes,
                               Format* format)
    : Resource(device, size_in_bytes) {
  if (format)
    format_ = device->GetVkFormat(*format);
}

TransferBuffer::~TransferBuffer() {
  if (device_) {
    device_->GetPtrs()->vkDestroyBufferView(device_->GetVkDevice(), view_,
                                            nullptr);

    if (memory_ != VK_NULL_HANDLE) {
      UnMapMemory(memory_);
      device_->GetPtrs()->vkFreeMemory(device_->GetVkDevice(), memory_,
                                       nullptr);
    }

    device_->GetPtrs()->vkDestroyBuffer(device_->GetVkDevice(), buffer_,
                                        nullptr);
  }
}

Result TransferBuffer::Initialize() {
  if (buffer_) {
    return Result(
        "Vulkan: TransferBuffer::Initialize() transfer buffer already "
        "initialized.");
  }

  Result r = CreateVkBuffer(&buffer_, usage_flags_);
  if (!r.IsSuccess())
    return r;

  uint32_t memory_type_index = 0;
  r = AllocateAndBindMemoryToVkBuffer(buffer_, &memory_,
                                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                      true, &memory_type_index);
  if (!r.IsSuccess())
    return r;

  // Create buffer view
  if (usage_flags_ & (VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT |
                      VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT)) {
    VkBufferViewCreateInfo buffer_view_info = VkBufferViewCreateInfo();
    buffer_view_info.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
    buffer_view_info.buffer = buffer_;
    buffer_view_info.format = format_;
    buffer_view_info.offset = 0;
    buffer_view_info.range = VK_WHOLE_SIZE;

    if (device_->GetPtrs()->vkCreateBufferView(device_->GetVkDevice(),
                                               &buffer_view_info, nullptr,
                                               &view_) != VK_SUCCESS) {
      return Result("Vulkan::Calling vkCreateBufferView Fail");
    }
  }

  if (!device_->IsMemoryHostAccessible(memory_type_index) ||
      !device_->IsMemoryHostCoherent(memory_type_index)) {
    return Result(
        "Vulkan: TransferBuffer::Initialize() buffer is not host accessible or"
        " not host coherent.");
  }

  return MapMemory(memory_);
}

void TransferBuffer::CopyToDevice(CommandBuffer* command_buffer) {
  // This is redundant because this buffer is always host visible
  // and coherent and vkQueueSubmit will make writes from host
  // available (See chapter 6.9. "Host Write Ordering Guarantees" in
  // Vulkan spec), but we prefer to keep it to simplify our own code.
  MemoryBarrier(command_buffer);
}

void TransferBuffer::CopyToHost(CommandBuffer* command_buffer) {
  MemoryBarrier(command_buffer);
}

}  // namespace vulkan
}  // namespace amber
