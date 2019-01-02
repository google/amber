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

#include "samples/config_helper.h"

#include <set>
#include <string>
#include <vector>

#include "src/vulkan/device.h"
#include "vulkan/vulkan.h"

namespace sample {
namespace {

VkPhysicalDeviceFeatures kRequiredFeatures = {
    VK_FALSE, /* robustBufferAccess */
    VK_FALSE, /* fullDrawIndexUint32 */
    VK_FALSE, /* imageCubeArray */
    VK_FALSE, /* independentBlend */
    VK_FALSE, /* geometryShader */
    VK_FALSE, /* tessellationShader */
    VK_FALSE, /* sampleRateShading */
    VK_FALSE, /* dualSrcBlend */
    VK_FALSE, /* logicOp */
    VK_FALSE, /* multiDrawIndirect */
    VK_FALSE, /* drawIndirectFirstInstance */
    VK_FALSE, /* depthClamp */
    VK_FALSE, /* depthBiasClamp */
    VK_FALSE, /* fillModeNonSolid */
    VK_FALSE, /* depthBounds */
    VK_FALSE, /* wideLines */
    VK_FALSE, /* largePoints */
    VK_FALSE, /* alphaToOne */
    VK_FALSE, /* multiViewport */
    VK_FALSE, /* samplerAnisotropy */
    VK_FALSE, /* textureCompressionETC2 */
    VK_FALSE, /* textureCompressionASTC_LDR */
    VK_FALSE, /* textureCompressionBC */
    VK_FALSE, /* occlusionQueryPrecise */
    VK_FALSE, /* pipelineStatisticsQuery */
    VK_TRUE,  /* vertexPipelineStoresAndAtomics */
    VK_FALSE, /* fragmentStoresAndAtomics */
    VK_FALSE, /* shaderTessellationAndGeometryPointSize */
    VK_FALSE, /* shaderImageGatherExtended */
    VK_FALSE, /* shaderStorageImageExtendedFormats */
    VK_FALSE, /* shaderStorageImageMultisample */
    VK_FALSE, /* shaderStorageImageReadWithoutFormat */
    VK_FALSE, /* shaderStorageImageWriteWithoutFormat */
    VK_FALSE, /* shaderUniformBufferArrayDynamicIndexing */
    VK_FALSE, /* shaderSampledImageArrayDynamicIndexing */
    VK_FALSE, /* shaderStorageBufferArrayDynamicIndexing */
    VK_FALSE, /* shaderStorageImageArrayDynamicIndexing */
    VK_FALSE, /* shaderClipDistance */
    VK_FALSE, /* shaderCullDistance */
    VK_FALSE, /* shaderFloat64 */
    VK_FALSE, /* shaderInt64 */
    VK_FALSE, /* shaderInt16 */
    VK_FALSE, /* shaderResourceResidency */
    VK_FALSE, /* shaderResourceMinLod */
    VK_FALSE, /* sparseBinding */
    VK_FALSE, /* sparseResidencyBuffer */
    VK_FALSE, /* sparseResidencyImage2D */
    VK_FALSE, /* sparseResidencyImage3D */
    VK_FALSE, /* sparseResidency2Samples */
    VK_FALSE, /* sparseResidency4Samples */
    VK_FALSE, /* sparseResidency8Samples */
    VK_FALSE, /* sparseResidency16Samples */
    VK_FALSE, /* sparseResidencyAliased */
    VK_FALSE, /* variableMultisampleRate */
    VK_FALSE, /* inheritedQueries */
};

const char* kRequiredExtensions[] = {
    "VK_KHR_storage_buffer_storage_class",
    "VK_KHR_variable_pointers",
};

const size_t kNumberOfRequiredExtensions =
    sizeof(kRequiredExtensions) / sizeof(kRequiredExtensions[0]);

// Check if |physical_device| supports all required features given
// in |kRequiredFeatures|.
bool AreAllRequiredFeaturesSupported(const VkPhysicalDevice& physical_device) {
  VkPhysicalDeviceFeatures available_features = {};
  vkGetPhysicalDeviceFeatures(physical_device, &available_features);

  if (available_features.robustBufferAccess == VK_FALSE &&
      kRequiredFeatures.robustBufferAccess == VK_TRUE) {
    return false;
  }
  if (available_features.fullDrawIndexUint32 == VK_FALSE &&
      kRequiredFeatures.fullDrawIndexUint32 == VK_TRUE) {
    return false;
  }
  if (available_features.imageCubeArray == VK_FALSE &&
      kRequiredFeatures.imageCubeArray == VK_TRUE) {
    return false;
  }
  if (available_features.independentBlend == VK_FALSE &&
      kRequiredFeatures.independentBlend == VK_TRUE) {
    return false;
  }
  if (available_features.geometryShader == VK_FALSE &&
      kRequiredFeatures.geometryShader == VK_TRUE) {
    return false;
  }
  if (available_features.tessellationShader == VK_FALSE &&
      kRequiredFeatures.tessellationShader == VK_TRUE) {
    return false;
  }
  if (available_features.sampleRateShading == VK_FALSE &&
      kRequiredFeatures.sampleRateShading == VK_TRUE) {
    return false;
  }
  if (available_features.dualSrcBlend == VK_FALSE &&
      kRequiredFeatures.dualSrcBlend == VK_TRUE) {
    return false;
  }
  if (available_features.logicOp == VK_FALSE &&
      kRequiredFeatures.logicOp == VK_TRUE) {
    return false;
  }
  if (available_features.multiDrawIndirect == VK_FALSE &&
      kRequiredFeatures.multiDrawIndirect == VK_TRUE) {
    return false;
  }
  if (available_features.drawIndirectFirstInstance == VK_FALSE &&
      kRequiredFeatures.drawIndirectFirstInstance == VK_TRUE) {
    return false;
  }
  if (available_features.depthClamp == VK_FALSE &&
      kRequiredFeatures.depthClamp == VK_TRUE) {
    return false;
  }
  if (available_features.depthBiasClamp == VK_FALSE &&
      kRequiredFeatures.depthBiasClamp == VK_TRUE) {
    return false;
  }
  if (available_features.fillModeNonSolid == VK_FALSE &&
      kRequiredFeatures.fillModeNonSolid == VK_TRUE) {
    return false;
  }
  if (available_features.depthBounds == VK_FALSE &&
      kRequiredFeatures.depthBounds == VK_TRUE) {
    return false;
  }
  if (available_features.wideLines == VK_FALSE &&
      kRequiredFeatures.wideLines == VK_TRUE) {
    return false;
  }
  if (available_features.largePoints == VK_FALSE &&
      kRequiredFeatures.largePoints == VK_TRUE) {
    return false;
  }
  if (available_features.alphaToOne == VK_FALSE &&
      kRequiredFeatures.alphaToOne == VK_TRUE) {
    return false;
  }
  if (available_features.multiViewport == VK_FALSE &&
      kRequiredFeatures.multiViewport == VK_TRUE) {
    return false;
  }
  if (available_features.samplerAnisotropy == VK_FALSE &&
      kRequiredFeatures.samplerAnisotropy == VK_TRUE) {
    return false;
  }
  if (available_features.textureCompressionETC2 == VK_FALSE &&
      kRequiredFeatures.textureCompressionETC2 == VK_TRUE) {
    return false;
  }
  if (available_features.textureCompressionASTC_LDR == VK_FALSE &&
      kRequiredFeatures.textureCompressionASTC_LDR == VK_TRUE) {
    return false;
  }
  if (available_features.textureCompressionBC == VK_FALSE &&
      kRequiredFeatures.textureCompressionBC == VK_TRUE) {
    return false;
  }
  if (available_features.occlusionQueryPrecise == VK_FALSE &&
      kRequiredFeatures.occlusionQueryPrecise == VK_TRUE) {
    return false;
  }
  if (available_features.pipelineStatisticsQuery == VK_FALSE &&
      kRequiredFeatures.pipelineStatisticsQuery == VK_TRUE) {
    return false;
  }
  if (available_features.vertexPipelineStoresAndAtomics == VK_FALSE &&
      kRequiredFeatures.vertexPipelineStoresAndAtomics == VK_TRUE) {
    return false;
  }
  if (available_features.fragmentStoresAndAtomics == VK_FALSE &&
      kRequiredFeatures.fragmentStoresAndAtomics == VK_TRUE) {
    return false;
  }
  if (available_features.shaderTessellationAndGeometryPointSize == VK_FALSE &&
      kRequiredFeatures.shaderTessellationAndGeometryPointSize == VK_TRUE) {
    return false;
  }
  if (available_features.shaderImageGatherExtended == VK_FALSE &&
      kRequiredFeatures.shaderImageGatherExtended == VK_TRUE) {
    return false;
  }
  if (available_features.shaderStorageImageExtendedFormats == VK_FALSE &&
      kRequiredFeatures.shaderStorageImageExtendedFormats == VK_TRUE) {
    return false;
  }
  if (available_features.shaderStorageImageMultisample == VK_FALSE &&
      kRequiredFeatures.shaderStorageImageMultisample == VK_TRUE) {
    return false;
  }
  if (available_features.shaderStorageImageReadWithoutFormat == VK_FALSE &&
      kRequiredFeatures.shaderStorageImageReadWithoutFormat == VK_TRUE) {
    return false;
  }
  if (available_features.shaderStorageImageWriteWithoutFormat == VK_FALSE &&
      kRequiredFeatures.shaderStorageImageWriteWithoutFormat == VK_TRUE) {
    return false;
  }
  if (available_features.shaderUniformBufferArrayDynamicIndexing == VK_FALSE &&
      kRequiredFeatures.shaderUniformBufferArrayDynamicIndexing == VK_TRUE) {
    return false;
  }
  if (available_features.shaderSampledImageArrayDynamicIndexing == VK_FALSE &&
      kRequiredFeatures.shaderSampledImageArrayDynamicIndexing == VK_TRUE) {
    return false;
  }
  if (available_features.shaderStorageBufferArrayDynamicIndexing == VK_FALSE &&
      kRequiredFeatures.shaderStorageBufferArrayDynamicIndexing == VK_TRUE) {
    return false;
  }
  if (available_features.shaderStorageImageArrayDynamicIndexing == VK_FALSE &&
      kRequiredFeatures.shaderStorageImageArrayDynamicIndexing == VK_TRUE) {
    return false;
  }
  if (available_features.shaderClipDistance == VK_FALSE &&
      kRequiredFeatures.shaderClipDistance == VK_TRUE) {
    return false;
  }
  if (available_features.shaderCullDistance == VK_FALSE &&
      kRequiredFeatures.shaderCullDistance == VK_TRUE) {
    return false;
  }
  if (available_features.shaderFloat64 == VK_FALSE &&
      kRequiredFeatures.shaderFloat64 == VK_TRUE) {
    return false;
  }
  if (available_features.shaderInt64 == VK_FALSE &&
      kRequiredFeatures.shaderInt64 == VK_TRUE) {
    return false;
  }
  if (available_features.shaderInt16 == VK_FALSE &&
      kRequiredFeatures.shaderInt16 == VK_TRUE) {
    return false;
  }
  if (available_features.shaderResourceResidency == VK_FALSE &&
      kRequiredFeatures.shaderResourceResidency == VK_TRUE) {
    return false;
  }
  if (available_features.shaderResourceMinLod == VK_FALSE &&
      kRequiredFeatures.shaderResourceMinLod == VK_TRUE) {
    return false;
  }
  if (available_features.sparseBinding == VK_FALSE &&
      kRequiredFeatures.sparseBinding == VK_TRUE) {
    return false;
  }
  if (available_features.sparseResidencyBuffer == VK_FALSE &&
      kRequiredFeatures.sparseResidencyBuffer == VK_TRUE) {
    return false;
  }
  if (available_features.sparseResidencyImage2D == VK_FALSE &&
      kRequiredFeatures.sparseResidencyImage2D == VK_TRUE) {
    return false;
  }
  if (available_features.sparseResidencyImage3D == VK_FALSE &&
      kRequiredFeatures.sparseResidencyImage3D == VK_TRUE) {
    return false;
  }
  if (available_features.sparseResidency2Samples == VK_FALSE &&
      kRequiredFeatures.sparseResidency2Samples == VK_TRUE) {
    return false;
  }
  if (available_features.sparseResidency4Samples == VK_FALSE &&
      kRequiredFeatures.sparseResidency4Samples == VK_TRUE) {
    return false;
  }
  if (available_features.sparseResidency8Samples == VK_FALSE &&
      kRequiredFeatures.sparseResidency8Samples == VK_TRUE) {
    return false;
  }
  if (available_features.sparseResidency16Samples == VK_FALSE &&
      kRequiredFeatures.sparseResidency16Samples == VK_TRUE) {
    return false;
  }
  if (available_features.sparseResidencyAliased == VK_FALSE &&
      kRequiredFeatures.sparseResidencyAliased == VK_TRUE) {
    return false;
  }
  if (available_features.variableMultisampleRate == VK_FALSE &&
      kRequiredFeatures.variableMultisampleRate == VK_TRUE) {
    return false;
  }
  if (available_features.inheritedQueries == VK_FALSE &&
      kRequiredFeatures.inheritedQueries == VK_TRUE) {
    return false;
  }

  return true;
}

// Check if |physical_device| supports all required extensions given
// in |kRequiredExtensions|.
bool AreAllExtensionsSupported(const VkPhysicalDevice& physical_device) {
  uint32_t available_extension_count = 0;
  std::vector<VkExtensionProperties> available_extension_properties;

  if (vkEnumerateDeviceExtensionProperties(physical_device, nullptr,
                                           &available_extension_count,
                                           nullptr) != VK_SUCCESS) {
    return false;
  }

  if (available_extension_count == 0)
    return false;

  available_extension_properties.resize(available_extension_count);
  if (vkEnumerateDeviceExtensionProperties(
          physical_device, nullptr, &available_extension_count,
          available_extension_properties.data()) != VK_SUCCESS) {
    return false;
  }

  std::set<std::string> required_extension_set(
      kRequiredExtensions, kRequiredExtensions + kNumberOfRequiredExtensions);
  for (const auto& property : available_extension_properties) {
    required_extension_set.erase(property.extensionName);
  }

  return required_extension_set.empty();
}

// Check if |physical_device| supports both compute and graphics
// pipelines.
uint32_t ChooseQueueFamilyIndex(const VkPhysicalDevice& physical_device) {
  uint32_t count;
  std::vector<VkQueueFamilyProperties> properties;

  vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &count, nullptr);
  properties.resize(count);
  vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &count,
                                           properties.data());

  for (uint32_t i = 0; i < count; ++i) {
    if (properties[i].queueFlags &
        (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT)) {
      return i;
    }
  }

  return std::numeric_limits<uint32_t>::max();
}

}  // namespace

ConfigHelper::ConfigHelper() = default;

ConfigHelper::~ConfigHelper() = default;

amber::Result ConfigHelper::CreateInstance() {
  if (instance_ != VK_NULL_HANDLE)
    return amber::Result("Sample: Vulkan instance already exists");

  VkApplicationInfo app_info = {};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.apiVersion = VK_MAKE_VERSION(1, 0, 0);

  VkInstanceCreateInfo instance_info = {};
  instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instance_info.pApplicationInfo = &app_info;

  if (vkCreateInstance(&instance_info, nullptr, &instance_) != VK_SUCCESS)
    return amber::Result("Sample: vkCreateInstance Fail");

  return {};
}

amber::Result ConfigHelper::ChoosePhysicalDevice() {
  uint32_t count;
  std::vector<VkPhysicalDevice> physical_devices;

  if (vkEnumeratePhysicalDevices(instance_, &count, nullptr) != VK_SUCCESS)
    return amber::Result("Sample: vkEnumeratePhysicalDevices Fail");
  physical_devices.resize(count);
  if (vkEnumeratePhysicalDevices(instance_, &count, physical_devices.data()) !=
      VK_SUCCESS) {
    return amber::Result("Sample: vkEnumeratePhysicalDevices Fail");
  }

  for (uint32_t i = 0; i < count; ++i) {
    if (!AreAllRequiredFeaturesSupported(physical_devices[i])) {
      continue;
    }

    if (!AreAllExtensionsSupported(physical_devices[i])) {
      continue;
    }

    queue_family_index_ = ChooseQueueFamilyIndex(physical_devices[i]);
    if (queue_family_index_ != std::numeric_limits<uint32_t>::max()) {
      physical_device_ = physical_devices[i];
      return {};
    }
  }

  return amber::Result("Vulkan::No physical device supports Vulkan");
}

amber::Result ConfigHelper::CreateDevice() {
  if (device_ != VK_NULL_HANDLE)
    return amber::Result("Sample: Vulkan device already exists");

  VkDeviceQueueCreateInfo queue_info;
  const float priorities[] = {1.0f};

  queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queue_info.queueFamilyIndex = queue_family_index_;
  queue_info.queueCount = 1;
  queue_info.pQueuePriorities = priorities;

  VkDeviceCreateInfo info = {};
  info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  info.pQueueCreateInfos = &queue_info;
  info.queueCreateInfoCount = 1;
  info.pEnabledFeatures = &kRequiredFeatures;
  info.enabledExtensionCount =
      static_cast<uint32_t>(kNumberOfRequiredExtensions);
  info.ppEnabledExtensionNames = kRequiredExtensions;

  if (vkCreateDevice(physical_device_, &info, nullptr, &device_) != VK_SUCCESS)
    return amber::Result("Vulkan::Calling vkCreateDevice Fail");

  return {};
}

std::pair<amber::Result, amber::VulkanEngineConfig>
ConfigHelper::CreateVulkanConfig() {
  amber::VulkanEngineConfig config = {};

  amber::Result r = CreateInstance();
  if (!r.IsSuccess())
    return std::make_pair(r, config);

  r = ChoosePhysicalDevice();
  if (!r.IsSuccess())
    return std::make_pair(r, config);

  r = CreateDevice();
  if (!r.IsSuccess())
    return std::make_pair(r, config);

  config.instance = instance_;
  config.physical_device = physical_device_;
  config.device = device_;
  return std::make_pair(r, config);
}

void ConfigHelper::Shutdown() {
  if (device_ != VK_NULL_HANDLE)
    vkDestroyDevice(device_, nullptr);

  if (instance_ != VK_NULL_HANDLE)
    vkDestroyInstance(instance_, nullptr);
}

}  // namespace sample
