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

#ifndef SAMPLES_CONFIG_HELPER_H_
#define SAMPLES_CONFIG_HELPER_H_

#include <memory>
#include <utility>
#include <vector>

#include "amber/amber_vulkan.h"
#include "amber/recipe.h"
#include "amber/result.h"

namespace amber {
namespace vulkan {

class Device;

}  // namespace vulkan
}  // namespace amber

namespace sample {

// Proof of concept implementation showing how to provide and use
// EngineConfig within sample amber program. This class uses amber
// internal components e.g., amber::Script, amber::Feature,
// amber::vulkan::Device to create Vulkan instance and device.
// In practice e.g., Vulkan CTS, Vulkan instance and device will
// be created using its own code not amber's internal components.
class ConfigHelper {
 public:
  ConfigHelper();
  ~ConfigHelper();

  std::pair<amber::Result, amber::VulkanEngineConfig> CreateVulkanConfig(
      const std::vector<amber::Recipe>& recipes);

  void Shutdown();

 private:
  std::unique_ptr<amber::vulkan::Device> device_;
};

}  // namespace sample

#endif  // SAMPLES_CONFIG_HELPER_H_
