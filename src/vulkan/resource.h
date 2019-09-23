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

#ifndef SRC_VULKAN_RESOURCE_H_
#define SRC_VULKAN_RESOURCE_H_

#include <memory>
#include <vector>

#include "amber/result.h"
#include "amber/value.h"
#include "amber/vulkan_header.h"

namespace amber {
namespace vulkan {

class CommandBuffer;
class Device;

// Class for Vulkan resources. Its children are Vulkan Buffer, Vulkan Image,
// and a class for push constant.
class Resource {
 public:
  virtual ~Resource();

  /// Records a command on |command_buffer| to copy the buffer contents from the
  /// host to the device.
  virtual void CopyToDevice(CommandBuffer* command_buffer) = 0;
  /// Records a command on |command_buffer| to copy the buffer contents from the
  /// device to the host.
  virtual void CopyToHost(CommandBuffer* command_buffer) = 0;

  void* HostAccessibleMemoryPtr() const { return memory_ptr_; }

  uint32_t GetSizeInBytes() const { return size_in_bytes_; }

 protected:
  Resource(Device* device, uint32_t size);
  Result CreateVkBuffer(VkBuffer* buffer, VkBufferUsageFlags usage);

  Result AllocateAndBindMemoryToVkBuffer(VkBuffer buffer,
                                         VkDeviceMemory* memory,
                                         VkMemoryPropertyFlags flags,
                                         bool force_flags,
                                         uint32_t* memory_type_index);

  Result MapMemory(VkDeviceMemory memory);
  void UnMapMemory(VkDeviceMemory memory);
  void SetMemoryPtr(void* ptr) { memory_ptr_ = ptr; }

  /// Records a memory barrier on |command_buffer|, to ensure prior writes to
  /// this buffer have completed and are available to subsequent commands.
  void MemoryBarrier(CommandBuffer* command_buffer);

  /// Returns a memory index for the given Vulkan device, for a memory type
  /// which has the given |flags| set. If no memory is found with the given
  /// |flags| set then the first non-zero memory index is returned. If
  /// |require_flags_found| is true then if no memory is found with the given
  /// |flags| then the maximum uint32_t value is returned instead of the
  /// first non-zero memory index.
  uint32_t ChooseMemory(uint32_t memory_type_bits,
                        VkMemoryPropertyFlags flags,
                        bool require_flags_found);
  Result AllocateMemory(VkDeviceMemory* memory,
                        VkDeviceSize size,
                        uint32_t memory_type_index);

  Device* device_ = nullptr;

 private:
  uint32_t size_in_bytes_ = 0;
  void* memory_ptr_ = nullptr;
};

}  // namespace vulkan
}  // namespace amber

#endif  // SRC_VULKAN_RESOURCE_H_
