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

#include <memory>
#include <vector>

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
  void AddAmberSampler(amber::Sampler* sampler) {
    amber_samplers_.push_back(sampler);
  }
  uint32_t GetDescriptorCount() override {
    return static_cast<uint32_t>(amber_samplers_.size());
  }
  SamplerDescriptor* AsSamplerDescriptor() override { return this; }

 private:
  std::vector<amber::Sampler*> amber_samplers_;
  std::vector<std::unique_ptr<amber::vulkan::Sampler>> vulkan_samplers_;
};

}  // namespace vulkan
}  // namespace amber

#endif  // SRC_VULKAN_SAMPLER_DESCRIPTOR_H_
