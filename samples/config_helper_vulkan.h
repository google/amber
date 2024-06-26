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

  VkInstance vulkan_instance_ = VK_NULL_HANDLE;
  VkDebugReportCallbackEXT vulkan_callback_ = VK_NULL_HANDLE;
  VkPhysicalDevice vulkan_physical_device_ = VK_NULL_HANDLE;
  std::vector<std::string> available_instance_extensions_;
  std::vector<std::string> available_device_extensions_;
  uint32_t vulkan_queue_family_index_ = std::numeric_limits<uint32_t>::max();
  VkQueue vulkan_queue_ = VK_NULL_HANDLE;
  VkDevice vulkan_device_ = VK_NULL_HANDLE;

  bool supports_get_physical_device_properties2_ = false;
  bool supports_variable_pointers_ = false;
  bool supports_shader_float16_int8_ = false;
  bool supports_shader_8bit_storage_ = false;
  bool supports_shader_16bit_storage_ = false;
  bool supports_subgroup_size_control_ = false;
  bool supports_shader_subgroup_extended_types_ = false;
  bool supports_acceleration_structure_ = false;
  bool supports_buffer_device_address_ = false;
  bool supports_ray_tracing_pipeline_ = false;
  VkPhysicalDeviceFeatures available_features_;
  VkPhysicalDeviceFeatures2KHR available_features2_;
  VkPhysicalDeviceVariablePointerFeaturesKHR variable_pointers_feature_;
  VkPhysicalDeviceFloat16Int8FeaturesKHR float16_int8_feature_;
  VkPhysicalDevice8BitStorageFeaturesKHR storage_8bit_feature_;
  VkPhysicalDevice16BitStorageFeaturesKHR storage_16bit_feature_;
  VkPhysicalDeviceSubgroupSizeControlFeaturesEXT subgroup_size_control_feature_;
  VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures
      shader_subgroup_extended_types_feature_;
  VkPhysicalDeviceAccelerationStructureFeaturesKHR
      acceleration_structure_feature_;
  VkPhysicalDeviceBufferDeviceAddressFeatures buffer_device_address_feature_;
  VkPhysicalDeviceRayTracingPipelineFeaturesKHR ray_tracing_pipeline_feature_;
};

}  // namespace sample

#pragma clang diagnostic pop

#endif  // SAMPLES_CONFIG_HELPER_VULKAN_H_
