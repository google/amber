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

#include <limits>
#include <utility>

#include "amber/amber_vulkan.h"
#include "amber/result.h"

namespace amber {
namespace vulkan {

class Device;

}  // namespace vulkan
}  // namespace amber

namespace sample {

// Proof of concept implementation showing how to provide and use
// EngineConfig within sample amber program. This class creates
// Vulkan instance and device.
class ConfigHelper {
 public:
  ConfigHelper();
  ~ConfigHelper();

  // Create Vulkan instance and device and return them as
  // amber::VulkanEngineConfig.
  std::pair<amber::Result, amber::VulkanEngineConfig> CreateVulkanConfig();

  // Destroy Vulkan instance and device.
  void Shutdown();

 private:
  // Create Vulkan instance.
  amber::Result CreateInstance();

  // Choose Vulkan physical device.
  amber::Result ChoosePhysicalDevice();

  // Create Vulkan logical device.
  amber::Result CreateDevice();

  VkInstance instance_ = VK_NULL_HANDLE;
  VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
  uint32_t queue_family_index_ = std::numeric_limits<uint32_t>::max();
  VkDevice device_ = VK_NULL_HANDLE;
};

}  // namespace sample

#endif  // SAMPLES_CONFIG_HELPER_H_
