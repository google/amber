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

#ifndef SRC_VULKAN_DEVICE_H_
#define SRC_VULKAN_DEVICE_H_

#include <memory>
#include <string>
#include <vector>

#include "amber/result.h"
#include "src/feature.h"
#include "vulkan/vulkan.h"

namespace amber {

namespace vulkan {

class Device {
 public:
  Device();
  explicit Device(VkDevice device);
  ~Device();

  Result Initialize(const std::vector<Feature>& required_features,
                    const std::vector<std::string>& required_extensions);
  void Shutdown();

  VkDevice GetDevice() const { return device_; }
  uint32_t GetQueueFamilyIndex() const { return queue_family_index_; }
  VkQueue GetQueue() const { return queue_; }
  const VkPhysicalDeviceMemoryProperties& GetPhysicalMemoryProperties() const {
    return physical_memory_properties_;
  }

 private:
  Result CreateInstance();
  Result CreateDebugReportCallback();

  // Get a physical device by checking if the physical device has a proper
  // queue family, required features, and required extensions.
  Result ChoosePhysicalDevice(
      const std::vector<Feature>& required_features,
      const std::vector<std::string>& required_extensions);

  // Return true if |physical_device| has a queue family that supports both
  // graphics and compute or only a compute pipeline. If the proper queue
  // family exists, |queue_family_index_| and |queue_family_flags_| will have
  // the queue family index and flags, respectively. Return false if the proper
  // queue family does not exist.
  bool ChooseQueueFamilyIndex(const VkPhysicalDevice& physical_device);

  // Create a logical device with enabled features |required_features|
  // and enabled extensions|required_extensions|.
  Result CreateDevice(const std::vector<Feature>& required_features,
                      const std::vector<std::string>& required_extensions);

  VkInstance instance_ = VK_NULL_HANDLE;
  VkDebugReportCallbackEXT callback_ = VK_NULL_HANDLE;
  VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
  VkPhysicalDeviceMemoryProperties physical_memory_properties_ = {};
  VkDevice device_ = VK_NULL_HANDLE;
  VkQueueFlags queue_family_flags_ = 0;
  uint32_t queue_family_index_ = 0;

  VkQueue queue_ = VK_NULL_HANDLE;

  bool destroy_device_ = true;
};

}  // namespace vulkan

}  // namespace amber

#endif  // SRC_VULKAN_DEVICE_H_
