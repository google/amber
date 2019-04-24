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

#include "src/vulkan/resource.h"

#include <limits>

#include "src/make_unique.h"
#include "src/vulkan/command_buffer.h"
#include "src/vulkan/device.h"

namespace amber {
namespace vulkan {
namespace {

VkMemoryBarrier kMemoryBarrierForAll = {
    VK_STRUCTURE_TYPE_MEMORY_BARRIER, nullptr,
    VK_ACCESS_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_INDEX_READ_BIT |
        VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT | VK_ACCESS_UNIFORM_READ_BIT |
        VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT |
        VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
        VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT |
        VK_ACCESS_HOST_READ_BIT | VK_ACCESS_HOST_WRITE_BIT,
    VK_ACCESS_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_INDEX_READ_BIT |
        VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT | VK_ACCESS_UNIFORM_READ_BIT |
        VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT |
        VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
        VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT |
        VK_ACCESS_HOST_READ_BIT | VK_ACCESS_HOST_WRITE_BIT};

}  // namespace

Resource::Resource(Device* device, uint32_t size_in_bytes)
    : device_(device), size_in_bytes_(size_in_bytes) {}

Resource::~Resource() = default;

Result Resource::CreateVkBuffer(VkBuffer* buffer, VkBufferUsageFlags usage) {
  if (!buffer)
    return Result("Vulkan::Given VkBuffer pointer is nullptr");

  VkBufferCreateInfo buffer_info = VkBufferCreateInfo();
  buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  buffer_info.size = size_in_bytes_;
  buffer_info.usage = usage;

  if (device_->GetPtrs()->vkCreateBuffer(device_->GetVkDevice(), &buffer_info,
                                         nullptr, buffer) != VK_SUCCESS) {
    return Result("Vulkan::Calling vkCreateBuffer Fail");
  }

  return {};
}

uint32_t Resource::ChooseMemory(uint32_t memory_type_bits,
                                VkMemoryPropertyFlags flags,
                                bool require_flags_found) {
  // Based on Vulkan spec about VkMemoryRequirements, N th bit of
  // |memory_type_bits| is 1 where N can be the proper memory type index.
  // This code is looking for the first non-zero bit whose memory type
  // VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT property. If not exists,
  // it returns the first non-zero bit.
  uint32_t first_non_zero = std::numeric_limits<uint32_t>::max();
  uint32_t memory_type_index = 0;
  while (memory_type_bits) {
    if (memory_type_bits % 2) {
      if (first_non_zero == std::numeric_limits<uint32_t>::max())
        first_non_zero = memory_type_index;

      if (device_->HasMemoryFlags(memory_type_index, flags))
        return memory_type_index;
    }

    ++memory_type_index;
    memory_type_bits >>= 1;
  }

  if (require_flags_found)
    return std::numeric_limits<uint32_t>::max();

  return first_non_zero;
}

Result Resource::AllocateAndBindMemoryToVkBuffer(VkBuffer buffer,
                                                 VkDeviceMemory* memory,
                                                 VkMemoryPropertyFlags flags,
                                                 bool require_flags_found,
                                                 uint32_t* memory_type_index) {
  if (memory_type_index == nullptr) {
    return Result(
        "Vulkan: Resource::AllocateAndBindMemoryToVkBuffer memory_type_index "
        "is nullptr");
  }

  *memory_type_index = 0;

  if (buffer == VK_NULL_HANDLE)
    return Result("Vulkan::Given VkBuffer is VK_NULL_HANDLE");
  if (memory == nullptr)
    return Result("Vulkan::Given VkDeviceMemory pointer is nullptr");

  VkMemoryRequirements requirement;
  device_->GetPtrs()->vkGetBufferMemoryRequirements(device_->GetVkDevice(),
                                                    buffer, &requirement);

  *memory_type_index =
      ChooseMemory(requirement.memoryTypeBits, flags, require_flags_found);
  if (*memory_type_index == std::numeric_limits<uint32_t>::max())
    return Result("Vulkan::Find Proper Memory Fail");

  Result r = AllocateMemory(memory, requirement.size, *memory_type_index);
  if (!r.IsSuccess())
    return r;

  if (device_->GetPtrs()->vkBindBufferMemory(device_->GetVkDevice(), buffer,
                                             *memory, 0) != VK_SUCCESS) {
    return Result("Vulkan::Calling vkBindBufferMemory Fail");
  }

  return {};
}

Result Resource::AllocateMemory(VkDeviceMemory* memory,
                                VkDeviceSize size,
                                uint32_t memory_type_index) {
  VkMemoryAllocateInfo alloc_info = VkMemoryAllocateInfo();
  alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  alloc_info.allocationSize = size;
  alloc_info.memoryTypeIndex = memory_type_index;
  if (device_->GetPtrs()->vkAllocateMemory(device_->GetVkDevice(), &alloc_info,
                                           nullptr, memory) != VK_SUCCESS) {
    return Result("Vulkan::Calling vkAllocateMemory Fail");
  }

  return {};
}

Result Resource::MapMemory(VkDeviceMemory memory) {
  if (device_->GetPtrs()->vkMapMemory(device_->GetVkDevice(), memory, 0,
                                      VK_WHOLE_SIZE, 0,
                                      &memory_ptr_) != VK_SUCCESS) {
    return Result("Vulkan::Calling vkMapMemory Fail");
  }

  return {};
}

void Resource::UnMapMemory(VkDeviceMemory memory) {
  device_->GetPtrs()->vkUnmapMemory(device_->GetVkDevice(), memory);
}

void Resource::MemoryBarrier(CommandBuffer* command_buffer) {
  // TODO(jaebaek): Current memory barrier is natively implemented.
  // Update it with the following access flags:
  // (r = read, w = write)
  //
  //                                 Host           Device
  // VertexBuffer                  host w         vertex r
  //                           transfer w       transfer r
  //
  // IndexBuffer                   host w          index r
  //                           transfer w       transfer r
  //
  // FrameBuffer                   host r          color w
  //                                       depth/stencil w
  //                           transfer r       transfer w
  //
  // ReadWrite Descriptors       host r/w       shader r/w
  //                         transfer r/w     transfer r/w
  //
  // ReadOnly Descriptors          host w         shader r
  //                           transfer w       transfer r
  device_->GetPtrs()->vkCmdPipelineBarrier(
      command_buffer->GetVkCommandBuffer(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
      VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 1, &kMemoryBarrierForAll, 0,
      nullptr, 0, nullptr);
}

}  // namespace vulkan
}  // namespace amber
