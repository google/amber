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

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "amber/amber.h"
#include "amber/result.h"
#include "amber/vulkan_header.h"
#include "src/buffer.h"
#include "src/format.h"

namespace amber {
namespace vulkan {

struct VulkanPtrs {
#include "vk-wrappers.h"  // NOLINT(build/include)
};

/// Wrapper around a Vulkan Device object.
class Device {
 public:
  Device(VkInstance instance,
         VkPhysicalDevice physical_device,
         uint32_t queue_family_index,
         VkDevice device,
         VkQueue queue);
  ~Device();

  Result Initialize(PFN_vkGetInstanceProcAddr getInstanceProcAddr,
                    Delegate* delegate,
                    const std::vector<std::string>& required_features,
                    const std::vector<std::string>& required_extensions,
                    const VkPhysicalDeviceFeatures& available_features,
                    const VkPhysicalDeviceFeatures2KHR& available_features2,
                    const std::vector<std::string>& available_extensions);

  /// Returns true if |format| and the |buffer|s buffer type combination is
  /// supported by the physical device.
  bool IsFormatSupportedByPhysicalDevice(const Format& format, Buffer* buffer);

  VkDevice GetVkDevice() const { return device_; }
  VkQueue GetVkQueue() const { return queue_; }
  VkFormat GetVkFormat(const Format& format) const;

  uint32_t GetQueueFamilyIndex() const { return queue_family_index_; }
  uint32_t GetMaxPushConstants() const;

  /// Returns true if the given |descriptor_set| is within the bounds of
  /// this device.
  bool IsDescriptorSetInBounds(uint32_t descriptor_set) const;

  /// Returns true if the memory at |memory_type_index| has |flags| set.
  bool HasMemoryFlags(uint32_t memory_type_index,
                      const VkMemoryPropertyFlags flags) const;
  /// Returns true if the memory at |memory_type_index| is host accessible.
  bool IsMemoryHostAccessible(uint32_t memory_type_index) const;
  /// Returns true if the memory at |memory_type_index| is host corherent.
  bool IsMemoryHostCoherent(uint32_t memory_type_index) const;

  /// Returns the pointers to the Vulkan API methods.
  const VulkanPtrs* GetPtrs() const { return &ptrs_; }

 private:
  Result LoadVulkanPointers(PFN_vkGetInstanceProcAddr, Delegate* delegate);

  VkInstance instance_ = VK_NULL_HANDLE;
  VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
  VkPhysicalDeviceProperties physical_device_properties_;
  VkPhysicalDeviceMemoryProperties physical_memory_properties_;
  VkDevice device_ = VK_NULL_HANDLE;
  VkQueue queue_ = VK_NULL_HANDLE;
  uint32_t queue_family_index_ = 0;

  VulkanPtrs ptrs_;
};

}  // namespace vulkan
}  // namespace amber

#endif  // SRC_VULKAN_DEVICE_H_
