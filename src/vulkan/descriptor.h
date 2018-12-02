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

#ifndef SRC_VULKAN_DESCRIPTOR_H_
#define SRC_VULKAN_DESCRIPTOR_H_

#include <memory>
#include <vector>

#include "amber/result.h"
#include "src/datum_type.h"
#include "src/engine.h"
#include "vulkan/vulkan.h"

namespace amber {
namespace vulkan {

enum class DescriptorType : uint8_t {
  kStorageImage = 0,
  kSampler,
  kSampledImage,
  kCombinedImageSampler,
  kUniformTexelBuffer,
  kStorageTexelBuffer,
  kStorageBuffer,
  kUniformBuffer,
  kDynamicUniformBuffer,
  kDynamicStorageBuffer,
  kInputAttachment,
};

VkDescriptorType ToVkDescriptorType(DescriptorType type);

struct PushDataInfo {
  DataType type;
  uint32_t offset;
  size_t size_in_bytes;
  std::vector<Value> values;

  // TODO(jaebaek): Add info for image
};

class Descriptor {
 public:
  Descriptor(DescriptorType type,
             VkDevice device,
             uint32_t desc_set,
             uint32_t binding);

  virtual ~Descriptor();

  uint32_t GetDescriptorSet() const { return descriptor_set_; }
  uint32_t GetBinding() const { return binding_; }
  bool operator<(const Descriptor& r) {
    if (descriptor_set_ == r.descriptor_set_)
      return binding_ < r.binding_;
    return descriptor_set_ < r.descriptor_set_;
  }

  DescriptorType GetType() const { return type_; }

  bool IsStorageImage() const { return type_ == DescriptorType::kStorageImage; }
  bool IsSampler() const { return type_ == DescriptorType::kSampler; }
  bool IsSampledImage() const { return type_ == DescriptorType::kSampledImage; }
  bool IsCombinedImageSampler() const {
    return type_ == DescriptorType::kCombinedImageSampler;
  }
  bool IsUniformTexelBuffer() const {
    return type_ == DescriptorType::kUniformTexelBuffer;
  }
  bool IsStorageTexelBuffer() const {
    return type_ == DescriptorType::kStorageTexelBuffer;
  }
  bool IsStorageBuffer() const {
    return type_ == DescriptorType::kStorageBuffer;
  }
  bool IsUniformBuffer() const {
    return type_ == DescriptorType::kUniformBuffer;
  }
  bool IsDynamicUniformBuffer() const {
    return type_ == DescriptorType::kDynamicUniformBuffer;
  }
  bool IsDynamicStorageBuffer() const {
    return type_ == DescriptorType::kDynamicStorageBuffer;
  }

  bool HasDataNotSent() { return !push_data_info_.empty(); }

  void PushData(DataType type,
                uint32_t offset,
                size_t size_in_bytes,
                const std::vector<Value>& values);

  // Call vkUpdateDescriptorSets() to update the backing resource
  // for this descriptor only when the backing resource was newly
  // created or changed. The child class i.e., each descriptor type
  // must use UpdateDescriptorSetForBuffer(), UpdateDescriptorSetForImage(),
  // UpdateDescriptorSetForBufferView() correctly according to its
  // backing resource type.
  virtual Result UpdateDescriptorSetIfNeeded(VkDescriptorSet) = 0;

  // Create new vulkan resource if needed i.e., if it was not created
  // yet or if we need bigger one. In addition, record commands
  // for copying the existing resource data to the new one if it
  // recreated it and send the updated data. Note that it only
  // records those commands and the actual submission of the command
  // must be done later.
  virtual Result UpdateResourceIfNeeded(
      VkCommandBuffer command,
      const VkPhysicalDeviceMemoryProperties& properties) = 0;

  // Only record the copy command for sending the bound resource
  // data to the host accessible memory. The actual submission of
  // the command must be done later.
  virtual Result SendDataToHostIfNeeded(VkCommandBuffer command) = 0;
  virtual ResourceInfo GetResourceInfo() = 0;
  virtual void Shutdown() = 0;

 protected:
  Result UpdateDescriptorSetForBuffer(
      VkDescriptorSet descriptor_set,
      VkDescriptorType descriptor_type,
      const VkDescriptorBufferInfo& buffer_info);
  Result UpdateDescriptorSetForImage(VkDescriptorSet descriptor_set,
                                     VkDescriptorType descriptor_type,
                                     const VkDescriptorImageInfo& image_info);
  Result UpdateDescriptorSetForBufferView(VkDescriptorSet descriptor_set,
                                          VkDescriptorType descriptor_type,
                                          const VkBufferView& texel_view);

  VkDevice GetDevice() const { return device_; }

  const std::vector<PushDataInfo>& GetPushDataInfo() const {
    return push_data_info_;
  }

  void ClearPushDataInfo() { push_data_info_.clear(); }

  void SetUpdateDescriptorSetNeeded() {
    is_descriptor_set_update_needed_ = true;
  }
  bool IsDescriptorSetUpdateNeeded() {
    return is_descriptor_set_update_needed_;
  }

  uint32_t descriptor_set_ = 0;
  uint32_t binding_ = 0;

 private:
  VkWriteDescriptorSet GetWriteDescriptorSet(
      VkDescriptorSet descriptor_set,
      VkDescriptorType descriptor_type) const;

  DescriptorType type_ = DescriptorType::kSampledImage;
  VkDevice device_ = VK_NULL_HANDLE;
  std::vector<PushDataInfo> push_data_info_;
  bool is_descriptor_set_update_needed_ = false;
};

}  // namespace vulkan
}  // namespace amber

#endif  // SRC_VULKAN_DESCRIPTOR_H_
