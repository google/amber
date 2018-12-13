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

namespace amber {
namespace vulkan {

Buffer::Buffer(VkDevice device,
               size_t size_in_bytes,
               const VkPhysicalDeviceMemoryProperties& properties)
    : Resource(device, size_in_bytes, properties) {}

Buffer::~Buffer() = default;

Result Buffer::Initialize(const VkBufferUsageFlags usage) {
  Result r = CreateVkBuffer(&buffer_, usage);
  if (!r.IsSuccess())
    return r;

  AllocateResult allocate_result = AllocateAndBindMemoryToVkBuffer(
      buffer_, &memory_, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, false);
  if (!allocate_result.r.IsSuccess())
    return allocate_result.r;

  if (IsMemoryHostAccessible(allocate_result.memory_type_index)) {
    is_buffer_host_accessible_ = true;
    is_buffer_host_coherent_ =
        IsMemoryHostCoherent(allocate_result.memory_type_index);
    return MapMemory(memory_);
  }

  is_buffer_host_accessible_ = false;
  return Resource::Initialize();
}

Result Buffer::CreateVkBufferView(VkFormat format) {
  VkBufferViewCreateInfo buffer_view_info = {};
  buffer_view_info.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
  buffer_view_info.buffer = buffer_;
  buffer_view_info.format = format;
  buffer_view_info.offset = 0;
  buffer_view_info.range = VK_WHOLE_SIZE;
  if (vkCreateBufferView(GetDevice(), &buffer_view_info, nullptr, &view_) !=
      VK_SUCCESS) {
    return Result("Vulkan::Calling vkCreateBufferView Fail");
  }

  return {};
}

Result Buffer::CopyToDevice(VkCommandBuffer command) {
  if (is_buffer_host_accessible_)
    return FlushMemoryIfNeeded();

  VkBufferCopy region = {};
  region.srcOffset = 0;
  region.dstOffset = 0;
  region.size = GetSizeInBytes();

  vkCmdCopyBuffer(command, GetHostAccessibleBuffer(), buffer_, 1, &region);
  MemoryBarrier(command);
  return {};
}

Result Buffer::CopyToHost(VkCommandBuffer command) {
  if (is_buffer_host_accessible_)
    return InvalidateMemoryIfNeeded();

  VkBufferCopy region = {};
  region.srcOffset = 0;
  region.dstOffset = 0;
  region.size = GetSizeInBytes();

  vkCmdCopyBuffer(command, buffer_, GetHostAccessibleBuffer(), 1, &region);
  MemoryBarrier(command);
  return {};
}

void Buffer::CopyFromBuffer(VkCommandBuffer command, const Buffer& src) {
  VkBufferCopy region = {};
  region.srcOffset = 0;
  region.dstOffset = 0;
  region.size = src.GetSizeInBytes();

  vkCmdCopyBuffer(command, src.buffer_, buffer_, 1, &region);
  MemoryBarrier(command);
}

void Buffer::Shutdown() {
  // TODO(jaebaek): Doublecheck what happens if |view_| is VK_NULL_HANDLE on
  //                Android and Windows.
  if (view_ != VK_NULL_HANDLE) {
    vkDestroyBufferView(GetDevice(), view_, nullptr);
    view_ = VK_NULL_HANDLE;
  }

  if (is_buffer_host_accessible_)
    UnMapMemory(memory_);

  vkDestroyBuffer(GetDevice(), buffer_, nullptr);
  vkFreeMemory(GetDevice(), memory_, nullptr);
  buffer_ = VK_NULL_HANDLE;
  memory_ = VK_NULL_HANDLE;

  if (!is_buffer_host_accessible_)
    Resource::Shutdown();
}

Result Buffer::InvalidateMemoryIfNeeded() {
  if (is_buffer_host_coherent_)
    return {};

  VkMappedMemoryRange range = {};
  range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
  range.memory = GetHostAccessMemory();
  range.offset = 0;
  range.size = VK_WHOLE_SIZE;
  if (vkInvalidateMappedMemoryRanges(GetDevice(), 1, &range) != VK_SUCCESS)
    return Result("Vulkan: vkInvalidateMappedMemoryRanges fail");

  return {};
}

Result Buffer::FlushMemoryIfNeeded() {
  if (is_buffer_host_coherent_)
    return {};

  VkMappedMemoryRange range = {};
  range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
  range.memory = GetHostAccessMemory();
  range.offset = 0;
  range.size = VK_WHOLE_SIZE;
  if (vkFlushMappedMemoryRanges(GetDevice(), 1, &range) != VK_SUCCESS)
    return Result("Vulkan: vkFlushMappedMemoryRanges fail");

  return {};
}

}  // namespace vulkan
}  // namespace amber
