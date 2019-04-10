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

class Format;

namespace vulkan {

class CommandBuffer;
class Device;

/// Contains information for data to be added to a buffer.
struct BufferInput {
  void UpdateBufferWithValues(void* buffer) const;

  /// Offset into the buffer to write the data.
  uint32_t offset;
  /// Number of bytes this data consumes.
  uint32_t size_in_bytes;
  /// The format of the data.
  Format* format;
  /// The actual data values.
  std::vector<Value> values;
};

/// Class for Vulkan resources.
class Resource {
 public:
  virtual ~Resource();

  virtual void CopyToHost(CommandBuffer* command) = 0;
  virtual void CopyToDevice(CommandBuffer* command) = 0;

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
  void MemoryBarrier(CommandBuffer* command);

  uint32_t ChooseMemory(uint32_t memory_type_bits,
                        VkMemoryPropertyFlags flags,
                        bool force_flags);
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
