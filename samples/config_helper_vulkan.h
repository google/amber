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

#ifndef SAMPLES_CONFIG_HELPER_VULKAN_H_
#define SAMPLES_CONFIG_HELPER_VULKAN_H_

#include <vulkan/vulkan.h>

#include <limits>
#include <memory>
#include <string>
#include <vector>

#include "amber/amber.h"
#include "amber/amber_vulkan.h"
#include "samples/config_helper.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"

namespace sample {

/// Child class of ConfigHelperImpl for Vulkan.
class ConfigHelperVulkan : public ConfigHelperImpl {
 public:
  ConfigHelperVulkan();
  ~ConfigHelperVulkan() override;

  /// Create Vulkan instance and device and return them as
  /// amber::VulkanEngineConfig. Required Vulkan device features and
  /// extensions are given in |required_features| and
  /// |required_extensions|, respectively.
  amber::Result CreateConfig(
      uint32_t engine_major,
      uint32_t engine_minor,
      int32_t selected_device,
      const std::vector<std::string>& required_features,
      const std::vector<std::string>& required_instance_extensions,
      const std::vector<std::string>& required_device_extensions,
      bool disable_validation_layer,
      bool enable_pipeline_runtime_layer,
      bool show_version_info,
      std::unique_ptr<amber::EngineConfig>* config) override;

 private:
  /// Create Vulkan instance.
  amber::Result CreateVulkanInstance(
      uint32_t engine_major,
      uint32_t engine_minor,
      std::vector<std::string> required_instance_extensions,
      bool disable_validation_layer,
      bool enable_pipeline_runtime_layer);

  /// Create |vulkan_callback_| that reports validation layer errors
  /// via debugCallback() function in config_helper_vulkan.cc.
  amber::Result CreateDebugReportCallback();

  /// Check if |physical_device| supports both
  /// |required_features| and |required_extensions|.
  amber::Result CheckVulkanPhysicalDeviceRequirements(
      const VkPhysicalDevice physical_device,
      const std::vector<std::string>& required_features,
      const std::vector<std::string>& required_extensions);

  /// Choose Vulkan physical device that supports both
  /// |required_features| and |required_extensions|.
  amber::Result ChooseVulkanPhysicalDevice(
      const std::vector<std::string>& required_features,
      const std::vector<std::string>& required_extensions,
      const int32_t selected_device);

  /// Create Vulkan logical device that enables both
  /// |required_features| and |required_extensions|.
  amber::Result CreateVulkanDevice(
      const std::vector<std::string>& required_features,
      const std::vector<std::string>& required_extensions);

  /// Sets up the device creation to use VkPhysicalDeviceFeatures.
  amber::Result CreateDeviceWithFeatures1(
      const std::vector<std::string>& required_features,
      const std::vector<std::string>& required_extensions,
      VkDeviceCreateInfo* info);
  /// Sets up the device creation to use VkPhysicalDeviceFeatures2KHR.
  amber::Result CreateDeviceWithFeatures2(
      const std::vector<std::string>& required_features,
      const std::vector<std::string>& required_extensions,
      VkDeviceCreateInfo* info);

  /// Creates the physical device given the device |info|.
  amber::Result DoCreateDevice(VkDeviceCreateInfo* info);

  /// Writes information related to the vulkan instance to stdout.
  void DumpPhysicalDeviceInfo();

  struct {
    VkInstance instance = VK_NULL_HANDLE;
    VkDebugReportCallbackEXT debug_cb = VK_NULL_HANDLE;
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    std::vector<std::string> available_instance_extensions;
    std::vector<std::string> available_device_extensions;
    uint32_t queue_family_index = std::numeric_limits<uint32_t>::max();
    VkQueue queue = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
  } vk_;

  struct {
    bool get_physical_device_properties2 = false;
    bool variable_pointers = false;
    bool shader_float16_int8 = false;
    bool shader_8bit_storage = false;
    bool shader_16bit_storage = false;
    bool subgroup_size_control = false;
    bool depth_clamp_zero_one = false;
    bool shader_subgroup_extended_types = false;
    bool acceleration_structure = false;
    bool buffer_device_address = false;
    bool ray_tracing_pipeline = false;
    bool descriptor_indexing = false;
    bool deferred_host_operations = false;
    bool spirv_1_4 = false;
    bool shader_float_controls = false;
  } supports_;

  struct {
    VkPhysicalDeviceFeatures device{};
    VkPhysicalDeviceFeatures2KHR features2{};
    VkPhysicalDeviceVariablePointerFeaturesKHR variable_pointers{};
    VkPhysicalDeviceFloat16Int8FeaturesKHR float16_int8{};
    VkPhysicalDevice8BitStorageFeaturesKHR storage_8bit{};
    VkPhysicalDevice16BitStorageFeaturesKHR storage_16bit{};
    VkPhysicalDeviceSubgroupSizeControlFeaturesEXT subgroup_size_control{};
    VkPhysicalDeviceDepthClampZeroOneFeaturesEXT depth_clamp_zero_one{};
    VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures
        shader_subgroup_extended_types{};
    VkPhysicalDeviceAccelerationStructureFeaturesKHR acceleration_structure{};
    VkPhysicalDeviceBufferDeviceAddressFeatures buffer_device_address{};
    VkPhysicalDeviceRayTracingPipelineFeaturesKHR ray_tracing_pipeline{};
    VkPhysicalDeviceDescriptorIndexingFeatures descriptor_indexing{};
  } features_;
};

}  // namespace sample

#pragma clang diagnostic pop

#endif  // SAMPLES_CONFIG_HELPER_VULKAN_H_
