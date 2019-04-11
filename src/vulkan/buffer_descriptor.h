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
#include "src/engine.h"
#include "src/vulkan/transfer_buffer.h"

namespace amber {

class Format;

namespace vulkan {

class CommandBuffer;
class Device;

enum class DescriptorType : uint8_t {
  kStorageBuffer = 0,
  kUniformBuffer,
};

// Among Vulkan descriptor types, this class handles Storage Buffers
// and Uniform Buffers.
class BufferDescriptor {
 public:
  BufferDescriptor(Buffer* buffer,
                   DescriptorType type,
                   Device* device,
                   uint32_t desc_set,
                   uint32_t binding);
  ~BufferDescriptor();

  uint32_t GetDescriptorSet() const { return descriptor_set_; }
  uint32_t GetBinding() const { return binding_; }

  VkDescriptorType GetVkDescriptorType() const;

  bool IsStorageBuffer() const {
    return type_ == DescriptorType::kStorageBuffer;
  }
  bool IsUniformBuffer() const {
    return type_ == DescriptorType::kUniformBuffer;
  }

  Result CreateResourceIfNeeded();
  void RecordCopyDataToResourceIfNeeded(CommandBuffer* command);
  Result RecordCopyDataToHost(CommandBuffer* command);
  Result MoveResourceToBufferOutput();
  void UpdateDescriptorSetIfNeeded(VkDescriptorSet descriptor_set);

  Result AddToBuffer(uint32_t offset,
                     uint32_t size_in_bytes,
                     const std::vector<Value>& values);

 private:
  Device* device_ = nullptr;
  Buffer* amber_buffer_ = nullptr;
  std::unique_ptr<TransferBuffer> transfer_buffer_;

  DescriptorType type_ = DescriptorType::kStorageBuffer;

  bool is_descriptor_set_update_needed_ = false;
  uint32_t descriptor_set_ = 0;
  uint32_t binding_ = 0;
};

}  // namespace vulkan
}  // namespace amber

#endif  // SRC_VULKAN_BUFFER_DESCRIPTOR_H_
