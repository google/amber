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

#ifndef SRC_VULKAN_IMAGE_DESCRIPTOR_H_
#define SRC_VULKAN_IMAGE_DESCRIPTOR_H_

#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

#include "src/vulkan/buffer_backed_descriptor.h"
#include "src/vulkan/pipeline.h"
#include "src/vulkan/sampler.h"
#include "src/vulkan/transfer_image.h"

namespace amber {
namespace vulkan {

class ImageDescriptor : public BufferBackedDescriptor {
 public:
  ImageDescriptor(Buffer* buffer,
                  DescriptorType type,
                  Device* device,
                  uint32_t base_mip_level,
                  uint32_t desc_set,
                  uint32_t binding,
                  Pipeline* pipeline);
  ~ImageDescriptor() override;

  void UpdateDescriptorSetIfNeeded(VkDescriptorSet descriptor_set) override;
  Result CreateResourceIfNeeded() override;
  void SetAmberSampler(amber::Sampler* sampler) { amber_sampler_ = sampler; }

  ImageDescriptor* AsImageDescriptor() override { return this; }

 private:
  uint32_t base_mip_level_ = 0;
  amber::Sampler* amber_sampler_ = nullptr;
  amber::vulkan::Sampler vulkan_sampler_;
};

}  // namespace vulkan
}  // namespace amber

#endif  // SRC_VULKAN_IMAGE_DESCRIPTOR_H_
