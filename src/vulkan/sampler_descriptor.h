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

#ifndef SRC_VULKAN_SAMPLER_DESCRIPTOR_H_
#define SRC_VULKAN_SAMPLER_DESCRIPTOR_H_

#include "src/vulkan/descriptor.h"
#include "src/vulkan/sampler.h"
#include "src/vulkan/transfer_image.h"

namespace amber {
namespace vulkan {

class SamplerDescriptor : public Descriptor {
 public:
  SamplerDescriptor(amber::Sampler* sampler,
                    DescriptorType type,
                    Device* device,
                    uint32_t desc_set,
                    uint32_t binding);
  ~SamplerDescriptor() override;

  void UpdateDescriptorSetIfNeeded(VkDescriptorSet descriptor_set) override;
  Result CreateResourceIfNeeded() override;

 private:
  amber::Sampler* amber_sampler_;
  amber::vulkan::Sampler vulkan_sampler_;
};

}  // namespace vulkan
}  // namespace amber

#endif  // SRC_VULKAN_SAMPLER_DESCRIPTOR_H_
