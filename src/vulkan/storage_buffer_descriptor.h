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

#ifndef SRC_VULKAN_STORAGE_BUFFER_DESCRIPTOR_H_
#define SRC_VULKAN_STORAGE_BUFFER_DESCRIPTOR_H_

#include <memory>
#include <vector>

#include "amber/result.h"
#include "src/datum_type.h"
#include "src/value.h"
#include "src/vulkan/buffer.h"
#include "src/vulkan/descriptor.h"
#include "vulkan/vulkan.h"

namespace amber {
namespace vulkan {

class StorageBufferDescriptor : public Descriptor {
 public:
  StorageBufferDescriptor(VkDevice device,
                          uint32_t desc_set,
                          uint32_t binding,
                          size_t size,
                          const VkPhysicalDeviceMemoryProperties& properties);
  ~StorageBufferDescriptor();

  Result Initialize(DataType type, const std::vector<Value>& values);

  // Descriptor
  void SendDataToGPUIfNeeded(VkCommandBuffer command) override;
  void UpdateDescriptorSet(VkDescriptorSet descriptor_set) override;
  void Shutdown() override;

 private:
  std::unique_ptr<Buffer> buffer_;
};

}  // namespace vulkan
}  // namespace amber

#endif  // SRC_VULKAN_STORAGE_BUFFER_DESCRIPTOR_H_
