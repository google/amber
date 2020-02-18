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

#ifndef SRC_VULKAN_SAMPLER_H_
#define SRC_VULKAN_SAMPLER_H_

#include "src/sampler.h"
#include "src/vulkan/device.h"

namespace amber {
namespace vulkan {

class Sampler {
 public:
  explicit Sampler(Device* device);
  ~Sampler();

  Result CreateSampler(amber::Sampler* sampler);
  VkSampler GetVkSampler() { return sampler_; }

 private:
  VkSampler sampler_ = VK_NULL_HANDLE;
  Device* device_;
};

}  // namespace vulkan
}  // namespace amber

#endif  // SRC_VULKAN_SAMPLER_H_
