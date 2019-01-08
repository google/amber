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
#include "src/datum_type.h"
#include "src/engine.h"
#include "src/value.h"
#include "src/vulkan/buffer.h"
#include "src/vulkan/descriptor.h"
#include "vulkan/vulkan.h"

namespace amber {
namespace vulkan {

// Among Vulkan descriptor types, this class handles Storage Buffer
// a.k.a. SSBO and Uniform Buffer a.k.a. UBO.
class BufferDescriptor : public Descriptor {
 public:
  BufferDescriptor(DescriptorType type,
                   VkDevice device,
                   uint32_t desc_set,
                   uint32_t binding);
  ~BufferDescriptor() override;

  // |data| contains information of what parts of |buffer_| must be
  // updated as what values. This method conducts the update.
  void FillBufferWithData(const BufferData& data);

  // Descriptor
  Result CreateOrResizeIfNeeded(
      VkCommandBuffer command,
      const VkPhysicalDeviceMemoryProperties& properties) override;
  void UpdateResourceIfNeeded(VkCommandBuffer command) override;
  Result SendDataToHostIfNeeded(VkCommandBuffer command) override;
  Result UpdateDescriptorSetIfNeeded(VkDescriptorSet descriptor_set) override;
  ResourceInfo GetResourceInfo() override;
  void Shutdown() override;

 private:
  VkBufferUsageFlagBits GetVkBufferUsage() const {
    return GetType() == DescriptorType::kStorageBuffer
               ? VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
               : VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
  }

  VkDescriptorType GetVkDescriptorType() const {
    return GetType() == DescriptorType::kStorageBuffer
               ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
               : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  }

  std::unique_ptr<Buffer> buffer_;
  std::vector<std::unique_ptr<Buffer>> not_destroyed_buffers_;
};

}  // namespace vulkan
}  // namespace amber

#endif  // SRC_VULKAN_BUFFER_DESCRIPTOR_H_
