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
#include <unordered_map>
#include <utility>
#include <vector>

#include "amber/result.h"
#include "amber/value.h"
#include "amber/vulkan_header.h"
#include "src/buffer.h"
#include "src/engine.h"
#include "src/vulkan/buffer_backed_descriptor.h"
#include "src/vulkan/pipeline.h"
#include "src/vulkan/transfer_buffer.h"

namespace amber {
namespace vulkan {

class CommandBuffer;
class Device;

/// Stores descriptor set and binding information for storage and uniform
/// buffers.
class BufferDescriptor : public BufferBackedDescriptor {
 public:
  BufferDescriptor(Buffer* buffer,
                   DescriptorType type,
                   Device* device,
                   uint32_t desc_set,
                   uint32_t binding,
                   vulkan::Pipeline* pipeline);
  ~BufferDescriptor() override;

  void UpdateDescriptorSetIfNeeded(VkDescriptorSet descriptor_set) override;
  Result CreateResourceIfNeeded() override;
  std::vector<uint32_t> GetDynamicOffsets() override {
    return dynamic_offsets_;
  }
  void AddDynamicOffset(uint32_t offset) { dynamic_offsets_.push_back(offset); }
  std::vector<uint64_t> GetDescriptorOffsets() override {
    return descriptor_offsets_;
  }
  void AddDescriptorOffset(uint64_t descriptor_offset) {
    descriptor_offsets_.push_back(descriptor_offset);
  }
  std::vector<uint64_t> GetDescriptorRanges() override {
    return descriptor_ranges_;
  }
  void AddDescriptorRange(uint64_t descriptor_range) {
    descriptor_ranges_.push_back(descriptor_range);
  }

  BufferDescriptor* AsBufferDescriptor() override { return this; }

 private:
  std::vector<uint32_t> dynamic_offsets_;
  std::vector<VkDeviceSize> descriptor_offsets_;
  std::vector<VkDeviceSize> descriptor_ranges_;
};

}  // namespace vulkan
}  // namespace amber

#endif  // SRC_VULKAN_BUFFER_DESCRIPTOR_H_
