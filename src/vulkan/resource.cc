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

#include <cstring>
#include <limits>

#include "src/make_unique.h"
#include "src/vulkan/command_buffer.h"
#include "src/vulkan/device.h"
#include "src/vulkan/format_data.h"
#include "src/vulkan/vklog.h"

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

// Fill the contents of |buffer| with |values|.
template <typename T>
void SetValuesForBuffer(void* buffer, const std::vector<Value>& values) {
  T* ptr = static_cast<T*>(buffer);
  for (const auto& v : values) {
    *ptr = v.IsInteger() ? static_cast<T>(v.AsUint64())
                         : static_cast<T>(v.AsDouble());
    ++ptr;
  }
}

}  // namespace

void BufferInput::UpdateBufferWithValues(void* buffer) const {
  uint8_t* ptr = static_cast<uint8_t*>(buffer) + offset;
  switch (type) {
    case DataType::kInt8:
      SetValuesForBuffer<int8_t>(ptr, values);
      break;
    case DataType::kUint8:
      SetValuesForBuffer<uint8_t>(ptr, values);
      break;
    case DataType::kInt16:
      SetValuesForBuffer<int16_t>(ptr, values);
      break;
    case DataType::kUint16:
      SetValuesForBuffer<uint16_t>(ptr, values);
      break;
    case DataType::kInt32:
      SetValuesForBuffer<int32_t>(ptr, values);
      break;
    case DataType::kUint32:
      SetValuesForBuffer<uint32_t>(ptr, values);
      break;
    case DataType::kInt64:
      SetValuesForBuffer<int64_t>(ptr, values);
      break;
    case DataType::kUint64:
      SetValuesForBuffer<uint64_t>(ptr, values);
      break;
    case DataType::kFloat:
      SetValuesForBuffer<float>(ptr, values);
      break;
    case DataType::kDouble:
      SetValuesForBuffer<double>(ptr, values);
      break;
  }
}

Resource::Resource(Device* device,
                   size_t size_in_bytes,
                   const VkPhysicalDeviceMemoryProperties& properties)
    : device_(device),
      size_in_bytes_(size_in_bytes),
      physical_memory_properties_(properties) {}

Resource::~Resource() = default;

Result Resource::UpdateMemoryWithInput(const BufferInput& input) {
  if (static_cast<size_t>(input.offset) >= size_in_bytes_) {
    return Result(
        "Vulkan: Resource::UpdateMemoryWithInput BufferInput offset exceeds "
        "memory size");
  }

  if (input.size_in_bytes >
      (size_in_bytes_ - static_cast<size_t>(input.offset))) {
    return Result(
        "Vulkan: Resource::UpdateMemoryWithInput BufferInput offset + size in "
        "bytes exceeds memory size");
  }

  input.UpdateBufferWithValues(memory_ptr_);
  return {};
}

void Resource::UpdateMemoryWithRawData(const std::vector<uint8_t>& raw_data) {
  size_t effective_size =
      raw_data.size() > size_in_bytes_ ? size_in_bytes_ : raw_data.size();
  std::memcpy(memory_ptr_, raw_data.data(), effective_size);
}

void Resource::Shutdown() {
  if (host_accessible_memory_ != VK_NULL_HANDLE) {
    UnMapMemory(host_accessible_memory_);
    VKLOG(device_->GetPtrs()->vkFreeMemory(device_->GetDevice(),
                                           host_accessible_memory_, nullptr));
  }

  if (host_accessible_buffer_ != VK_NULL_HANDLE) {
    VKLOG(device_->GetPtrs()->vkDestroyBuffer(
        device_->GetDevice(), host_accessible_buffer_, nullptr));
  }
}

Result Resource::Initialize() {
  Result r = CreateVkBuffer(
      &host_accessible_buffer_,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
  if (!r.IsSuccess())
    return r;

  uint32_t memory_type_index = 0;
  r = AllocateAndBindMemoryToVkBuffer(host_accessible_buffer_,
                                      &host_accessible_memory_,
                                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                      true, &memory_type_index);
  if (!r.IsSuccess())
    return r;

  return MapMemory(host_accessible_memory_);
}

Result Resource::CreateVkBuffer(VkBuffer* buffer, VkBufferUsageFlags usage) {
  if (buffer == nullptr)
    return Result("Vulkan::Given VkBuffer pointer is nullptr");

  VkBufferCreateInfo buffer_info = VkBufferCreateInfo();
  buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  buffer_info.size = size_in_bytes_;
  buffer_info.usage = usage;

  if (VKLOG(device_->GetPtrs()->vkCreateBuffer(
          device_->GetDevice(), &buffer_info, nullptr, buffer)) != VK_SUCCESS) {
    return Result("Vulkan::Calling vkCreateBuffer Fail");
  }

  return {};
}

uint32_t Resource::ChooseMemory(uint32_t memory_type_bits,
                                VkMemoryPropertyFlags flags,
                                bool force_flags) {
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

      if ((physical_memory_properties_.memoryTypes[memory_type_index]
               .propertyFlags &
           flags) == flags) {
        return memory_type_index;
      }
    }

    ++memory_type_index;
    memory_type_bits >>= 1;
  }

  if (force_flags)
    return std::numeric_limits<uint32_t>::max();

  return first_non_zero;
}

const VkMemoryRequirements Resource::GetVkBufferMemoryRequirements(
    VkBuffer buffer) const {
  VkMemoryRequirements requirement;
  VKLOG(device_->GetPtrs()->vkGetBufferMemoryRequirements(
      device_->GetDevice(), buffer, &requirement));
  return requirement;
}

Result Resource::AllocateAndBindMemoryToVkBuffer(VkBuffer buffer,
                                                 VkDeviceMemory* memory,
                                                 VkMemoryPropertyFlags flags,
                                                 bool force_flags,
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

  auto requirement = GetVkBufferMemoryRequirements(buffer);

  *memory_type_index =
      ChooseMemory(requirement.memoryTypeBits, flags, force_flags);
  if (*memory_type_index == std::numeric_limits<uint32_t>::max())
    return Result("Vulkan::Find Proper Memory Fail");

  Result r = AllocateMemory(memory, requirement.size, *memory_type_index);
  if (!r.IsSuccess())
    return r;

  if (VKLOG(device_->GetPtrs()->vkBindBufferMemory(device_->GetDevice(), buffer,
                                                   *memory, 0)) != VK_SUCCESS) {
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
  if (VKLOG(device_->GetPtrs()->vkAllocateMemory(
          device_->GetDevice(), &alloc_info, nullptr, memory)) != VK_SUCCESS) {
    return Result("Vulkan::Calling vkAllocateMemory Fail");
  }

  return {};
}

Result Resource::MapMemory(VkDeviceMemory memory) {
  if (VKLOG(device_->GetPtrs()->vkMapMemory(device_->GetDevice(), memory, 0,
                                            VK_WHOLE_SIZE, 0, &memory_ptr_)) !=
      VK_SUCCESS) {
    return Result("Vulkan::Calling vkMapMemory Fail");
  }

  return {};
}

void Resource::UnMapMemory(VkDeviceMemory memory) {
  VKLOG(device_->GetPtrs()->vkUnmapMemory(device_->GetDevice(), memory));
}

void Resource::MemoryBarrier(CommandBuffer* command) {
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
  VKLOG(device_->GetPtrs()->vkCmdPipelineBarrier(
      command->GetCommandBuffer(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
      VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 1, &kMemoryBarrierForAll, 0,
      nullptr, 0, nullptr));
}

}  // namespace vulkan
}  // namespace amber
