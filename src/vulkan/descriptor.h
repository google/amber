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

#include "amber/result.h"
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

  bool IsDataAlreadySent() { return is_data_already_sent_; }

  virtual Result UpdateDescriptorSet(VkDescriptorSet descriptor_set) = 0;
  virtual void SendDataToDeviceIfNeeded(VkCommandBuffer command) = 0;
  virtual Result SendDataToHostIfNeeded(VkCommandBuffer command) = 0;
  virtual void GetResourceInfo(ResourceInfo* info) = 0;
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

  void SetDataSent() { is_data_already_sent_ = true; }

  uint32_t descriptor_set_ = 0;
  uint32_t binding_ = 0;

 private:
  VkWriteDescriptorSet GetWriteDescriptorSet(
      VkDescriptorSet descriptor_set,
      VkDescriptorType descriptor_type) const;

  DescriptorType type_ = DescriptorType::kSampledImage;
  bool is_data_already_sent_ = false;
  VkDevice device_ = VK_NULL_HANDLE;
};

}  // namespace vulkan
}  // namespace amber

#endif  // SRC_VULKAN_DESCRIPTOR_H_
