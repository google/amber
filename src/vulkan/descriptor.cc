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

#include "src/vulkan/descriptor.h"

#include <cassert>

namespace amber {
namespace vulkan {

VkDescriptorType ToVkDescriptorType(DescriptorType type) {
  switch (type) {
    case DescriptorType::kStorageImage:
      return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    case DescriptorType::kSampler:
      return VK_DESCRIPTOR_TYPE_SAMPLER;
    case DescriptorType::kSampledImage:
      return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    case DescriptorType::kCombinedImageSampler:
      return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    case DescriptorType::kUniformTexelBuffer:
      return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
    case DescriptorType::kStorageTexelBuffer:
      return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
    case DescriptorType::kStorageBuffer:
      return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    case DescriptorType::kUniformBuffer:
      return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    case DescriptorType::kDynamicUniformBuffer:
      return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    case DescriptorType::kDynamicStorageBuffer:
      return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
    case DescriptorType::kInputAttachment:
      return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
  }

  assert(false && "Unknown resource type");
  return VK_DESCRIPTOR_TYPE_SAMPLER;
}

Descriptor::Descriptor(DescriptorType type, uint32_t desc_set, uint32_t binding)
    : type_(type), descriptor_set_(desc_set), binding_(binding) {}

Descriptor::~Descriptor() = default;

}  // namespace vulkan
}  // namespace amber
