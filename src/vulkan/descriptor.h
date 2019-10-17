// Copyright 2019 The Amber Authors.
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

#ifndef SRC_VULKAN_DESCRIPTOR_H_
#define SRC_VULKAN_DESCRIPTOR_H_

#include <memory>
#include <vector>

#include "amber/result.h"
#include "amber/value.h"
#include "amber/vulkan_header.h"
#include "src/buffer.h"
#include "src/engine.h"
#include "src/vulkan/resource.h"

namespace amber {
namespace vulkan {

class CommandBuffer;
class Device;

enum class DescriptorType : uint8_t {
  kStorageBuffer = 0,
  kUniformBuffer,
  kStorageImage,
  kSampledImage,
  kCombinedImageSampler
};

class Descriptor {
 public:
  Descriptor(Buffer* buffer,
             DescriptorType type,
             Device* device,
             uint32_t desc_set,
             uint32_t binding);
  ~Descriptor();

  uint32_t GetDescriptorSet() const { return descriptor_set_; }
  uint32_t GetBinding() const { return binding_; }
  VkDescriptorType GetVkDescriptorType() const;

  virtual Result CreateResourceIfNeeded() = 0;
  virtual void RecordCopyDataToResourceIfNeeded(CommandBuffer* command);
  virtual Result RecordCopyDataToHost(CommandBuffer* command);
  virtual Result MoveResourceToBufferOutput();
  virtual void UpdateDescriptorSetIfNeeded(VkDescriptorSet descriptor_set) = 0;
  virtual Resource* GetResource() = 0;

  bool IsStorageBuffer() const {
    return type_ == DescriptorType::kStorageBuffer;
  }
  bool IsUniformBuffer() const {
    return type_ == DescriptorType::kUniformBuffer;
  }

 protected:
  bool is_descriptor_set_update_needed_ = false;
  Device* device_ = nullptr;
  DescriptorType type_ = DescriptorType::kStorageBuffer;
  uint32_t descriptor_set_ = 0;
  uint32_t binding_ = 0;
  Buffer* amber_buffer_ = nullptr;
};

}  // namespace vulkan
}  // namespace amber

#endif  // SRC_VULKAN_DESCRIPTOR_H_
