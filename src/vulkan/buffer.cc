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

#include "src/vulkan/buffer.h"

#include "src/vulkan/command_buffer.h"
#include "src/vulkan/device.h"

namespace amber {
namespace vulkan {
namespace {

bool IsMemoryHostAccessible(const VkPhysicalDeviceMemoryProperties& props,
                            uint32_t memory_type_index) {
  return (props.memoryTypes[memory_type_index].propertyFlags &
          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) ==
         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
}

bool IsMemoryHostCoherent(const VkPhysicalDeviceMemoryProperties& props,
                          uint32_t memory_type_index) {
  return (props.memoryTypes[memory_type_index].propertyFlags &
          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) ==
         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
}

}  // namespace

Buffer::Buffer(Device* device,
               size_t size_in_bytes,
               const VkPhysicalDeviceMemoryProperties& properties)
    : Resource(device, size_in_bytes, properties) {}

Buffer::~Buffer() = default;

Result Buffer::Initialize(const VkBufferUsageFlags usage) {
  Result r = CreateVkBuffer(&buffer_, usage);
  if (!r.IsSuccess())
    return r;

  uint32_t memory_type_index = 0;
  r = AllocateAndBindMemoryToVkBuffer(buffer_, &memory_,
                                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                      true, &memory_type_index);
  if (!r.IsSuccess())
    return r;

  if (!IsMemoryHostAccessible(GetMemoryProperties(), memory_type_index) ||
      !IsMemoryHostCoherent(GetMemoryProperties(), memory_type_index)) {
    return Result(
        "Vulkan: Buffer::Initialize() Buffer is not host accessible or not "
        "host coherent.");
  }

  return MapMemory(memory_);
}

Result Buffer::CreateVkBufferView(VkFormat format) {
  VkBufferViewCreateInfo buffer_view_info = VkBufferViewCreateInfo();
  buffer_view_info.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
  buffer_view_info.buffer = buffer_;
  buffer_view_info.format = format;
  buffer_view_info.offset = 0;
  buffer_view_info.range = VK_WHOLE_SIZE;
  if (device_->GetPtrs()->vkCreateBufferView(device_->GetDevice(),
                                             &buffer_view_info, nullptr,
                                             &view_) != VK_SUCCESS) {
    return Result("Vulkan::Calling vkCreateBufferView Fail");
  }

  return {};
}

Result Buffer::CopyToDevice(CommandBuffer* command) {
  // This is redundant because this buffer is always host visible
  // and coherent and vkQueueSubmit will make writes from host
  // avaliable (See chapter 6.9. "Host Write Ordering Guarantees" in
  // Vulkan spec), but we prefer to keep it to simplify our own code.
  MemoryBarrier(command);
  return {};
}

Result Buffer::CopyToHost(CommandBuffer* command) {
  MemoryBarrier(command);
  return {};
}

void Buffer::CopyFromBuffer(CommandBuffer* command, const Buffer& src) {
  VkBufferCopy region = VkBufferCopy();
  region.srcOffset = 0;
  region.dstOffset = 0;
  region.size = src.GetSizeInBytes();

  device_->GetPtrs()->vkCmdCopyBuffer(command->GetCommandBuffer(), src.buffer_,
                                      buffer_, 1, &region);
  MemoryBarrier(command);
}

void Buffer::Shutdown() {
  if (view_ != VK_NULL_HANDLE) {
    device_->GetPtrs()->vkDestroyBufferView(device_->GetDevice(), view_,
                                            nullptr);
  }

  if (memory_ != VK_NULL_HANDLE) {
    UnMapMemory(memory_);
    device_->GetPtrs()->vkFreeMemory(device_->GetDevice(), memory_, nullptr);
  }

  if (buffer_ != VK_NULL_HANDLE)
    device_->GetPtrs()->vkDestroyBuffer(device_->GetDevice(), buffer_, nullptr);

  Resource::Shutdown();
}

}  // namespace vulkan
}  // namespace amber
