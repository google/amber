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

#include "src/vulkan/descriptor.h"

#include <cassert>
#include <cstring>

#include "src/vulkan/command_buffer.h"
#include "src/vulkan/device.h"

namespace amber {
namespace vulkan {

Descriptor::Descriptor(DescriptorType type,
                       Device* device,
                       uint32_t desc_set,
                       uint32_t binding)
    : device_(device),
      type_(type),
      descriptor_set_(desc_set),
      binding_(binding) {}

Descriptor::~Descriptor() = default;

VkDescriptorType Descriptor::GetVkDescriptorType() const {
  switch (type_) {
    case DescriptorType::kStorageBuffer:
      return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    case DescriptorType::kStorageBufferDynamic:
      return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
    case DescriptorType::kUniformBuffer:
      return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    case DescriptorType::kUniformBufferDynamic:
      return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    case DescriptorType::kSampler:
      return VK_DESCRIPTOR_TYPE_SAMPLER;
    case DescriptorType::kStorageImage:
      return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    case DescriptorType::kCombinedImageSampler:
      return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    case DescriptorType::kUniformTexelBuffer:
      return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
    case DescriptorType::kStorageTexelBuffer:
      return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
    default:
      assert(type_ == DescriptorType::kSampledImage);
      return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
  }
}

}  // namespace vulkan
}  // namespace amber
