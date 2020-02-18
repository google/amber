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

#include "src/vulkan/sampler_descriptor.h"
#include "src/vulkan/device.h"
#include "src/vulkan/resource.h"

namespace amber {
namespace vulkan {

SamplerDescriptor::SamplerDescriptor(amber::Sampler* sampler,
                                     DescriptorType type,
                                     Device* device,
                                     uint32_t desc_set,
                                     uint32_t binding)
    : Descriptor(type, device, desc_set, binding),
      amber_sampler_(sampler),
      vulkan_sampler_(device) {}

SamplerDescriptor::~SamplerDescriptor() = default;

Result SamplerDescriptor::CreateResourceIfNeeded() {
  Result r = vulkan_sampler_.CreateSampler(amber_sampler_);
  if (!r.IsSuccess())
    return r;

  return {};
}

void SamplerDescriptor::UpdateDescriptorSetIfNeeded(
    VkDescriptorSet descriptor_set) {
  VkDescriptorImageInfo image_info = {vulkan_sampler_.GetVkSampler(),
                                      VK_NULL_HANDLE, VK_IMAGE_LAYOUT_GENERAL};

  VkWriteDescriptorSet write = VkWriteDescriptorSet();
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.dstSet = descriptor_set;
  write.dstBinding = binding_;
  write.dstArrayElement = 0;
  write.descriptorCount = 1;
  write.descriptorType = GetVkDescriptorType();
  write.pImageInfo = &image_info;

  device_->GetPtrs()->vkUpdateDescriptorSets(device_->GetVkDevice(), 1, &write,
                                             0, nullptr);
}

}  // namespace vulkan
}  // namespace amber
