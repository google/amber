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

#ifndef SRC_VULKAN_BUFFER_DESCRIPTOR_H_
#define SRC_VULKAN_BUFFER_DESCRIPTOR_H_

#include <memory>
#include <vector>

#include "amber/result.h"
#include "amber/value.h"
#include "amber/vulkan_header.h"
#include "src/buffer.h"
#include "src/datum_type.h"
#include "src/engine.h"
#include "src/vulkan/descriptor.h"
#include "src/vulkan/transfer_buffer.h"

namespace amber {
namespace vulkan {

class CommandBuffer;
class Device;

// Among Vulkan descriptor types, this class handles Storage Buffers
// and Uniform Buffers.
class BufferDescriptor : public Descriptor {
 public:
  BufferDescriptor(Buffer* buffer,
                   DescriptorType type,
                   Device* device,
                   uint32_t desc_set,
                   uint32_t binding);
  ~BufferDescriptor() override;

  // Descriptor
  Result CreateResourceIfNeeded(
      const VkPhysicalDeviceMemoryProperties& properties) override;
  Result RecordCopyDataToResourceIfNeeded(CommandBuffer* command) override;
  Result RecordCopyDataToHost(CommandBuffer* command) override;
  Result MoveResourceToBufferOutput() override;
  Result UpdateDescriptorSetIfNeeded(VkDescriptorSet descriptor_set) override;
  void Shutdown() override;

  Result AddToBuffer(DataType type,
                     uint32_t offset,
                     uint32_t size_in_bytes,
                     const std::vector<Value>& values);

 private:
  VkBufferUsageFlagBits GetVkBufferUsage() const {
    return IsStorageBuffer() ? VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
                             : VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
  }

  VkDescriptorType GetVkDescriptorType() const {
    return IsStorageBuffer() ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
                             : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  }

  Buffer* amber_buffer_ = nullptr;
  std::unique_ptr<TransferBuffer> transfer_buffer_;
};

}  // namespace vulkan
}  // namespace amber

#endif  // SRC_VULKAN_BUFFER_DESCRIPTOR_H_
