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

#include "samples/config_helper_vulkan.h"

#include <algorithm>
#include <cassert>
#include <set>

#include "src/make_unique.h"

namespace sample {
namespace {

// Convert required features given as a string array to
// VkPhysicalDeviceFeatures.
VkPhysicalDeviceFeatures NamesToVulkanFeatures(
    const std::vector<std::string>& required_feature_names) {
  VkPhysicalDeviceFeatures required_vulkan_features = {};
  for (const auto& name : required_feature_names) {
    if (name == "robustBufferAccess") {
      required_vulkan_features.robustBufferAccess = VK_TRUE;
      continue;
    }
    if (name == "fullDrawIndexUint32") {
      required_vulkan_features.fullDrawIndexUint32 = VK_TRUE;
      continue;
    }
    if (name == "imageCubeArray") {
      required_vulkan_features.imageCubeArray = VK_TRUE;
      continue;
    }
    if (name == "independentBlend") {
      required_vulkan_features.independentBlend = VK_TRUE;
      continue;
    }
    if (name == "geometryShader") {
      required_vulkan_features.geometryShader = VK_TRUE;
      continue;
    }
    if (name == "tessellationShader") {
      required_vulkan_features.tessellationShader = VK_TRUE;
      continue;
    }
    if (name == "sampleRateShading") {
      required_vulkan_features.sampleRateShading = VK_TRUE;
      continue;
    }
    if (name == "dualSrcBlend") {
      required_vulkan_features.dualSrcBlend = VK_TRUE;
      continue;
    }
    if (name == "logicOp") {
      required_vulkan_features.logicOp = VK_TRUE;
      continue;
    }
    if (name == "multiDrawIndirect") {
      required_vulkan_features.multiDrawIndirect = VK_TRUE;
      continue;
    }
    if (name == "drawIndirectFirstInstance") {
      required_vulkan_features.drawIndirectFirstInstance = VK_TRUE;
      continue;
    }
    if (name == "depthClamp") {
      required_vulkan_features.depthClamp = VK_TRUE;
      continue;
    }
    if (name == "depthBiasClamp") {
      required_vulkan_features.depthBiasClamp = VK_TRUE;
      continue;
    }
    if (name == "fillModeNonSolid") {
      required_vulkan_features.fillModeNonSolid = VK_TRUE;
      continue;
    }
    if (name == "depthBounds") {
      required_vulkan_features.depthBounds = VK_TRUE;
      continue;
    }
    if (name == "wideLines") {
      required_vulkan_features.wideLines = VK_TRUE;
      continue;
    }
    if (name == "largePoints") {
      required_vulkan_features.largePoints = VK_TRUE;
      continue;
    }
    if (name == "alphaToOne") {
      required_vulkan_features.alphaToOne = VK_TRUE;
      continue;
    }
    if (name == "multiViewport") {
      required_vulkan_features.multiViewport = VK_TRUE;
      continue;
    }
    if (name == "samplerAnisotropy") {
      required_vulkan_features.samplerAnisotropy = VK_TRUE;
      continue;
    }
    if (name == "textureCompressionETC2") {
      required_vulkan_features.textureCompressionETC2 = VK_TRUE;
      continue;
    }
    if (name == "textureCompressionASTC_LDR") {
      required_vulkan_features.textureCompressionASTC_LDR = VK_TRUE;
      continue;
    }
    if (name == "textureCompressionBC") {
      required_vulkan_features.textureCompressionBC = VK_TRUE;
      continue;
    }
    if (name == "occlusionQueryPrecise") {
      required_vulkan_features.occlusionQueryPrecise = VK_TRUE;
      continue;
    }
    if (name == "pipelineStatisticsQuery") {
      required_vulkan_features.pipelineStatisticsQuery = VK_TRUE;
      continue;
    }
    if (name == "vertexPipelineStoresAndAtomics") {
      required_vulkan_features.vertexPipelineStoresAndAtomics = VK_TRUE;
      continue;
    }
    if (name == "fragmentStoresAndAtomics") {
      required_vulkan_features.fragmentStoresAndAtomics = VK_TRUE;
      continue;
    }
    if (name == "shaderTessellationAndGeometryPointSize") {
      required_vulkan_features.shaderTessellationAndGeometryPointSize = VK_TRUE;
      continue;
    }
    if (name == "shaderImageGatherExtended") {
      required_vulkan_features.shaderImageGatherExtended = VK_TRUE;
      continue;
    }
    if (name == "shaderStorageImageExtendedFormats") {
      required_vulkan_features.shaderStorageImageExtendedFormats = VK_TRUE;
      continue;
    }
    if (name == "shaderStorageImageMultisample") {
      required_vulkan_features.shaderStorageImageMultisample = VK_TRUE;
      continue;
    }
    if (name == "shaderStorageImageReadWithoutFormat") {
      required_vulkan_features.shaderStorageImageReadWithoutFormat = VK_TRUE;
      continue;
    }
    if (name == "shaderStorageImageWriteWithoutFormat") {
      required_vulkan_features.shaderStorageImageWriteWithoutFormat = VK_TRUE;
      continue;
    }
    if (name == "shaderUniformBufferArrayDynamicIndexing") {
      required_vulkan_features.shaderUniformBufferArrayDynamicIndexing =
          VK_TRUE;
      continue;
    }
    if (name == "shaderSampledImageArrayDynamicIndexing") {
      required_vulkan_features.shaderSampledImageArrayDynamicIndexing = VK_TRUE;
      continue;
    }
    if (name == "shaderStorageBufferArrayDynamicIndexing") {
      required_vulkan_features.shaderStorageBufferArrayDynamicIndexing =
          VK_TRUE;
      continue;
    }
    if (name == "shaderStorageImageArrayDynamicIndexing") {
      required_vulkan_features.shaderStorageImageArrayDynamicIndexing = VK_TRUE;
      continue;
    }
    if (name == "shaderClipDistance") {
      required_vulkan_features.shaderClipDistance = VK_TRUE;
      continue;
    }
    if (name == "shaderCullDistance") {
      required_vulkan_features.shaderCullDistance = VK_TRUE;
      continue;
    }
    if (name == "shaderFloat64") {
      required_vulkan_features.shaderFloat64 = VK_TRUE;
      continue;
    }
    if (name == "shaderInt64") {
      required_vulkan_features.shaderInt64 = VK_TRUE;
      continue;
    }
    if (name == "shaderInt16") {
      required_vulkan_features.shaderInt16 = VK_TRUE;
      continue;
    }
    if (name == "shaderResourceResidency") {
      required_vulkan_features.shaderResourceResidency = VK_TRUE;
      continue;
    }
    if (name == "shaderResourceMinLod") {
      required_vulkan_features.shaderResourceMinLod = VK_TRUE;
      continue;
    }
    if (name == "sparseBinding") {
      required_vulkan_features.sparseBinding = VK_TRUE;
      continue;
    }
    if (name == "sparseResidencyBuffer") {
      required_vulkan_features.sparseResidencyBuffer = VK_TRUE;
      continue;
    }
    if (name == "sparseResidencyImage2D") {
      required_vulkan_features.sparseResidencyImage2D = VK_TRUE;
      continue;
    }
    if (name == "sparseResidencyImage3D") {
      required_vulkan_features.sparseResidencyImage3D = VK_TRUE;
      continue;
    }
    if (name == "sparseResidency2Samples") {
      required_vulkan_features.sparseResidency2Samples = VK_TRUE;
      continue;
    }
    if (name == "sparseResidency4Samples") {
      required_vulkan_features.sparseResidency4Samples = VK_TRUE;
      continue;
    }
    if (name == "sparseResidency8Samples") {
      required_vulkan_features.sparseResidency8Samples = VK_TRUE;
      continue;
    }
    if (name == "sparseResidency16Samples") {
      required_vulkan_features.sparseResidency16Samples = VK_TRUE;
      continue;
    }
    if (name == "sparseResidencyAliased") {
      required_vulkan_features.sparseResidencyAliased = VK_TRUE;
      continue;
    }
    if (name == "variableMultisampleRate") {
      required_vulkan_features.variableMultisampleRate = VK_TRUE;
      continue;
    }
    if (name == "inheritedQueries") {
      required_vulkan_features.inheritedQueries = VK_TRUE;
      continue;
    }

    assert(false && "Sample: Unknown Vulkan feature");
  }
  return required_vulkan_features;
}

// Check if |physical_device| supports all required features given
// in |required_features|.
bool AreAllRequiredFeaturesSupported(
    const VkPhysicalDeviceFeatures& available_features,
    const VkPhysicalDeviceFeatures& required_features) {
  if (available_features.robustBufferAccess == VK_FALSE &&
      required_features.robustBufferAccess == VK_TRUE) {
    return false;
  }
  if (available_features.fullDrawIndexUint32 == VK_FALSE &&
      required_features.fullDrawIndexUint32 == VK_TRUE) {
    return false;
  }
  if (available_features.imageCubeArray == VK_FALSE &&
      required_features.imageCubeArray == VK_TRUE) {
    return false;
  }
  if (available_features.independentBlend == VK_FALSE &&
      required_features.independentBlend == VK_TRUE) {
    return false;
  }
  if (available_features.geometryShader == VK_FALSE &&
      required_features.geometryShader == VK_TRUE) {
    return false;
  }
  if (available_features.tessellationShader == VK_FALSE &&
      required_features.tessellationShader == VK_TRUE) {
    return false;
  }
  if (available_features.sampleRateShading == VK_FALSE &&
      required_features.sampleRateShading == VK_TRUE) {
    return false;
  }
  if (available_features.dualSrcBlend == VK_FALSE &&
      required_features.dualSrcBlend == VK_TRUE) {
    return false;
  }
  if (available_features.logicOp == VK_FALSE &&
      required_features.logicOp == VK_TRUE) {
    return false;
  }
  if (available_features.multiDrawIndirect == VK_FALSE &&
      required_features.multiDrawIndirect == VK_TRUE) {
    return false;
  }
  if (available_features.drawIndirectFirstInstance == VK_FALSE &&
      required_features.drawIndirectFirstInstance == VK_TRUE) {
    return false;
  }
  if (available_features.depthClamp == VK_FALSE &&
      required_features.depthClamp == VK_TRUE) {
    return false;
  }
  if (available_features.depthBiasClamp == VK_FALSE &&
      required_features.depthBiasClamp == VK_TRUE) {
    return false;
  }
  if (available_features.fillModeNonSolid == VK_FALSE &&
      required_features.fillModeNonSolid == VK_TRUE) {
    return false;
  }
  if (available_features.depthBounds == VK_FALSE &&
      required_features.depthBounds == VK_TRUE) {
    return false;
  }
  if (available_features.wideLines == VK_FALSE &&
      required_features.wideLines == VK_TRUE) {
    return false;
  }
  if (available_features.largePoints == VK_FALSE &&
      required_features.largePoints == VK_TRUE) {
    return false;
  }
  if (available_features.alphaToOne == VK_FALSE &&
      required_features.alphaToOne == VK_TRUE) {
    return false;
  }
  if (available_features.multiViewport == VK_FALSE &&
      required_features.multiViewport == VK_TRUE) {
    return false;
  }
  if (available_features.samplerAnisotropy == VK_FALSE &&
      required_features.samplerAnisotropy == VK_TRUE) {
    return false;
  }
  if (available_features.textureCompressionETC2 == VK_FALSE &&
      required_features.textureCompressionETC2 == VK_TRUE) {
    return false;
  }
  if (available_features.textureCompressionASTC_LDR == VK_FALSE &&
      required_features.textureCompressionASTC_LDR == VK_TRUE) {
    return false;
  }
  if (available_features.textureCompressionBC == VK_FALSE &&
      required_features.textureCompressionBC == VK_TRUE) {
    return false;
  }
  if (available_features.occlusionQueryPrecise == VK_FALSE &&
      required_features.occlusionQueryPrecise == VK_TRUE) {
    return false;
  }
  if (available_features.pipelineStatisticsQuery == VK_FALSE &&
      required_features.pipelineStatisticsQuery == VK_TRUE) {
    return false;
  }
  if (available_features.vertexPipelineStoresAndAtomics == VK_FALSE &&
      required_features.vertexPipelineStoresAndAtomics == VK_TRUE) {
    return false;
  }
  if (available_features.fragmentStoresAndAtomics == VK_FALSE &&
      required_features.fragmentStoresAndAtomics == VK_TRUE) {
    return false;
  }
  if (available_features.shaderTessellationAndGeometryPointSize == VK_FALSE &&
      required_features.shaderTessellationAndGeometryPointSize == VK_TRUE) {
    return false;
  }
  if (available_features.shaderImageGatherExtended == VK_FALSE &&
      required_features.shaderImageGatherExtended == VK_TRUE) {
    return false;
  }
  if (available_features.shaderStorageImageExtendedFormats == VK_FALSE &&
      required_features.shaderStorageImageExtendedFormats == VK_TRUE) {
    return false;
  }
  if (available_features.shaderStorageImageMultisample == VK_FALSE &&
      required_features.shaderStorageImageMultisample == VK_TRUE) {
    return false;
  }
  if (available_features.shaderStorageImageReadWithoutFormat == VK_FALSE &&
      required_features.shaderStorageImageReadWithoutFormat == VK_TRUE) {
    return false;
  }
  if (available_features.shaderStorageImageWriteWithoutFormat == VK_FALSE &&
      required_features.shaderStorageImageWriteWithoutFormat == VK_TRUE) {
    return false;
  }
  if (available_features.shaderUniformBufferArrayDynamicIndexing == VK_FALSE &&
      required_features.shaderUniformBufferArrayDynamicIndexing == VK_TRUE) {
    return false;
  }
  if (available_features.shaderSampledImageArrayDynamicIndexing == VK_FALSE &&
      required_features.shaderSampledImageArrayDynamicIndexing == VK_TRUE) {
    return false;
  }
  if (available_features.shaderStorageBufferArrayDynamicIndexing == VK_FALSE &&
      required_features.shaderStorageBufferArrayDynamicIndexing == VK_TRUE) {
    return false;
  }
  if (available_features.shaderStorageImageArrayDynamicIndexing == VK_FALSE &&
      required_features.shaderStorageImageArrayDynamicIndexing == VK_TRUE) {
    return false;
  }
  if (available_features.shaderClipDistance == VK_FALSE &&
      required_features.shaderClipDistance == VK_TRUE) {
    return false;
  }
  if (available_features.shaderCullDistance == VK_FALSE &&
      required_features.shaderCullDistance == VK_TRUE) {
    return false;
  }
  if (available_features.shaderFloat64 == VK_FALSE &&
      required_features.shaderFloat64 == VK_TRUE) {
    return false;
  }
  if (available_features.shaderInt64 == VK_FALSE &&
      required_features.shaderInt64 == VK_TRUE) {
    return false;
  }
  if (available_features.shaderInt16 == VK_FALSE &&
      required_features.shaderInt16 == VK_TRUE) {
    return false;
  }
  if (available_features.shaderResourceResidency == VK_FALSE &&
      required_features.shaderResourceResidency == VK_TRUE) {
    return false;
  }
  if (available_features.shaderResourceMinLod == VK_FALSE &&
      required_features.shaderResourceMinLod == VK_TRUE) {
    return false;
  }
  if (available_features.sparseBinding == VK_FALSE &&
      required_features.sparseBinding == VK_TRUE) {
    return false;
  }
  if (available_features.sparseResidencyBuffer == VK_FALSE &&
      required_features.sparseResidencyBuffer == VK_TRUE) {
    return false;
  }
  if (available_features.sparseResidencyImage2D == VK_FALSE &&
      required_features.sparseResidencyImage2D == VK_TRUE) {
    return false;
  }
  if (available_features.sparseResidencyImage3D == VK_FALSE &&
      required_features.sparseResidencyImage3D == VK_TRUE) {
    return false;
  }
  if (available_features.sparseResidency2Samples == VK_FALSE &&
      required_features.sparseResidency2Samples == VK_TRUE) {
    return false;
  }
  if (available_features.sparseResidency4Samples == VK_FALSE &&
      required_features.sparseResidency4Samples == VK_TRUE) {
    return false;
  }
  if (available_features.sparseResidency8Samples == VK_FALSE &&
      required_features.sparseResidency8Samples == VK_TRUE) {
    return false;
  }
  if (available_features.sparseResidency16Samples == VK_FALSE &&
      required_features.sparseResidency16Samples == VK_TRUE) {
    return false;
  }
  if (available_features.sparseResidencyAliased == VK_FALSE &&
      required_features.sparseResidencyAliased == VK_TRUE) {
    return false;
  }
  if (available_features.variableMultisampleRate == VK_FALSE &&
      required_features.variableMultisampleRate == VK_TRUE) {
    return false;
  }
  if (available_features.inheritedQueries == VK_FALSE &&
      required_features.inheritedQueries == VK_TRUE) {
    return false;
  }
  return true;
}

// Get all available extensions of |physical_device|.
std::vector<std::string> GetAvailableExtensions(
    const VkPhysicalDevice& physical_device) {
  std::vector<std::string> available_extensions;
  uint32_t available_extension_count = 0;
  std::vector<VkExtensionProperties> available_extension_properties;

  if (vkEnumerateDeviceExtensionProperties(physical_device, nullptr,
                                           &available_extension_count,
                                           nullptr) != VK_SUCCESS) {
    return available_extensions;
  }

  if (available_extension_count == 0)
    return available_extensions;

  available_extension_properties.resize(available_extension_count);
  if (vkEnumerateDeviceExtensionProperties(
          physical_device, nullptr, &available_extension_count,
          available_extension_properties.data()) != VK_SUCCESS) {
    return available_extensions;
  }

  for (const auto& property : available_extension_properties)
    available_extensions.push_back(property.extensionName);

  return available_extensions;
}

// Check if |physical_device| supports all required extensions given
// in |required_extensions|.
bool AreAllExtensionsSupported(
    const std::vector<std::string>& available_extensions,
    const std::vector<std::string>& required_extensions) {
  if (required_extensions.empty())
    return true;

  std::set<std::string> required_extension_set(required_extensions.begin(),
                                               required_extensions.end());
  for (const auto& extension : available_extensions) {
    required_extension_set.erase(extension);
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

ConfigHelperVulkan::ConfigHelperVulkan() = default;

ConfigHelperVulkan::~ConfigHelperVulkan() = default;

void ConfigHelperVulkan::CreateVulkanInstance() {
  VkApplicationInfo app_info = {};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.apiVersion = VK_MAKE_VERSION(1, 0, 0);

  VkInstanceCreateInfo instance_info = {};
  instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instance_info.pApplicationInfo = &app_info;

  assert(vkCreateInstance(&instance_info, nullptr, &vulkan_instance_) ==
         VK_SUCCESS);
}

void ConfigHelperVulkan::ChooseVulkanPhysicalDevice(
    const VkPhysicalDeviceFeatures& required_features,
    const std::vector<std::string>& required_extensions) {
  uint32_t count;
  std::vector<VkPhysicalDevice> physical_devices;

  assert(vkEnumeratePhysicalDevices(vulkan_instance_, &count, nullptr) ==
         VK_SUCCESS);
  physical_devices.resize(count);
  assert(vkEnumeratePhysicalDevices(vulkan_instance_, &count,
                                    physical_devices.data()) == VK_SUCCESS);

  for (uint32_t i = 0; i < count; ++i) {
    vkGetPhysicalDeviceFeatures(physical_devices[i], &available_features_);
    if (!AreAllRequiredFeaturesSupported(available_features_,
                                         required_features)) {
      continue;
    }

    available_extensions_ = GetAvailableExtensions(physical_devices[i]);
    if (!AreAllExtensionsSupported(available_extensions_,
                                   required_extensions)) {
      continue;
    }

    vulkan_queue_family_index_ = ChooseQueueFamilyIndex(physical_devices[i]);
    if (vulkan_queue_family_index_ != std::numeric_limits<uint32_t>::max()) {
      vulkan_physical_device_ = physical_devices[i];
      return;
    }
  }

  assert(false && "Vulkan::No physical device supports Vulkan");
}

void ConfigHelperVulkan::CreateVulkanDevice(
    const VkPhysicalDeviceFeatures& required_features,
    const std::vector<std::string>& required_extensions) {
  VkDeviceQueueCreateInfo queue_info;
  const float priorities[] = {1.0f};

  queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queue_info.queueFamilyIndex = vulkan_queue_family_index_;
  queue_info.queueCount = 1;
  queue_info.pQueuePriorities = priorities;

  VkDeviceCreateInfo info = {};
  info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  info.pQueueCreateInfos = &queue_info;
  info.queueCreateInfoCount = 1;
  info.pEnabledFeatures = &required_features;

  std::vector<const char*> required_extensions_in_char;
  std::transform(
      required_extensions.begin(), required_extensions.end(),
      std::back_inserter(required_extensions_in_char),
      [](const std::string& ext) -> const char* { return ext.c_str(); });
  info.enabledExtensionCount =
      static_cast<uint32_t>(required_extensions_in_char.size());
  info.ppEnabledExtensionNames = required_extensions_in_char.data();

  assert(vkCreateDevice(vulkan_physical_device_, &info, nullptr,
                        &vulkan_device_) == VK_SUCCESS);
}

std::unique_ptr<amber::EngineConfig> ConfigHelperVulkan::CreateConfig(
    const std::vector<std::string>& required_features,
    const std::vector<std::string>& required_extensions) {
  std::unique_ptr<amber::VulkanEngineConfig> config =
      amber::MakeUnique<amber::VulkanEngineConfig>();

  auto required_vulkan_features = NamesToVulkanFeatures(required_features);

  CreateVulkanInstance();
  ChooseVulkanPhysicalDevice(required_vulkan_features, required_extensions);
  CreateVulkanDevice(required_vulkan_features, required_extensions);
  vkGetDeviceQueue(vulkan_device_, vulkan_queue_family_index_, 0,
                   &vulkan_queue_);

  config->physical_device = vulkan_physical_device_;
  config->available_features = available_features_;
  config->available_extensions = available_extensions_;
  config->queue_family_index = vulkan_queue_family_index_;
  config->queue = vulkan_queue_;
  config->device = vulkan_device_;
  return config;
}

void ConfigHelperVulkan::Shutdown() {
  if (vulkan_device_ != VK_NULL_HANDLE)
    vkDestroyDevice(vulkan_device_, nullptr);

  if (vulkan_instance_ != VK_NULL_HANDLE)
    vkDestroyInstance(vulkan_instance_, nullptr);
}

}  // namespace sample
