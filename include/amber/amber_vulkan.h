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

#ifndef AMBER_AMBER_VULKAN_H_
#define AMBER_AMBER_VULKAN_H_

#include <string>
#include <vector>

#include "amber/amber.h"
#include "vulkan/vulkan.h"

namespace amber {

/// Configuration for the Vulkan Engine.
struct VulkanEngineConfig : public EngineConfig {
  /// The VkPhysicalDevice to be used for the tests.
  VkPhysicalDevice physical_device = VK_NULL_HANDLE;

  /// Physical device features available for |physical_device|.
  VkPhysicalDeviceFeatures available_features = {};

  /// Physical device extensions available for |physical_device|.
  std::vector<std::string> available_extensions;

  /// The given queue family index to be used for the tests.
  uint32_t queue_family_index;

  /// The VkQueue to be used for the tests.
  VkQueue queue;

  /// The VkDevice to be used for the tests.
  VkDevice device = VK_NULL_HANDLE;
};

/// Returns required physical device features from |recipes|.
VkPhysicalDeviceFeatures GetRequiredVulkanFeatures(
    const std::vector<const Recipe*>& recipes);

/// Returns required physical device extensions from |recipes|.
std::vector<std::string> GetRequiredVulkanExtensions(
    const std::vector<const Recipe*>& recipes);

}  // namespace amber

#endif  // AMBER_AMBER_VULKAN_H_
