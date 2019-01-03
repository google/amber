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
#include <memory>
#include <utility>

#include "amber/amber.h"

#if AMBER_ENGINE_VULKAN
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"
#include "amber/amber_vulkan.h"
#pragma clang diagnostic pop
#endif  // AMBER_ENGINE_VULKAN

namespace sample {

// Proof of concept implementation showing how to provide and use
// EngineConfig within sample amber program. This class creates
// Vulkan instance and device.
class ConfigHelper {
 public:
  ConfigHelper();
  ~ConfigHelper();

  // Create instance and device and return them as amber::EngineConfig.
  std::unique_ptr<amber::EngineConfig> CreateConfig(amber::EngineType engine);

  // Destroy Vulkan instance and device.
  void Shutdown();

 private:
#if AMBER_ENGINE_VULKAN
  // Create Vulkan instance and device and return them as
  // amber::VulkanEngineConfig.
  std::unique_ptr<amber::VulkanEngineConfig> CreateVulkanConfig();

  // Create Vulkan instance.
  void CreateVulkanInstance();

  // Choose Vulkan physical device.
  void ChooseVulkanPhysicalDevice();

  // Create Vulkan logical device.
  void CreateVulkanDevice();

  VkInstance vulkan_instance_ = VK_NULL_HANDLE;
  VkPhysicalDevice vulkan_physical_device_ = VK_NULL_HANDLE;
  uint32_t vulkan_queue_family_index_ = std::numeric_limits<uint32_t>::max();
  VkDevice vulkan_device_ = VK_NULL_HANDLE;
#endif  // AMBER_ENGINE_VULKAN
};

}  // namespace sample

#endif  // SAMPLES_CONFIG_HELPER_H_
