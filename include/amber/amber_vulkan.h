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

#include <limits>
#include <string>
#include <vector>

#include "amber/amber.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"
#include "vulkan/vulkan.h"
#pragma clang diagnostic pop

namespace amber {

/// Configuration for the Vulkan Engine.
struct VulkanEngineConfig : public EngineConfig {
  /// The VkPhysicalDevice to use.
  VkPhysicalDevice physical_device = VK_NULL_HANDLE;

  /// Physical device features available for |physical_device|.
  VkPhysicalDeviceFeatures available_features = {};

  /// Physical device extensions available for |physical_device|.
  std::vector<std::string> available_extensions;

  /// The given queue family index to use.
  uint32_t queue_family_index = std::numeric_limits<uint32_t>::max();

  /// The VkDevice to use.
  VkDevice device = VK_NULL_HANDLE;

  /// The VkQueue to use.
  VkQueue queue = VK_NULL_HANDLE;
};

}  // namespace amber

#endif  // AMBER_AMBER_VULKAN_H_
