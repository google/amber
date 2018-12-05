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

#include "src/vulkan/device.h"

#include <memory>
#include <vector>

#include "src/make_unique.h"

namespace amber {
namespace vulkan {
namespace {

bool AreAllRequiredFeaturesSupported(
    const VkPhysicalDeviceFeatures& available_features,
    const VkPhysicalDeviceFeatures& required_features) {
  if (required_features.robustBufferAccess == VK_TRUE &&
      available_features.robustBufferAccess == VK_FALSE)
    return false;
  if (required_features.fullDrawIndexUint32 == VK_TRUE &&
      available_features.fullDrawIndexUint32 == VK_FALSE)
    return false;
  if (required_features.imageCubeArray == VK_TRUE &&
      available_features.imageCubeArray == VK_FALSE)
    return false;
  if (required_features.independentBlend == VK_TRUE &&
      available_features.independentBlend == VK_FALSE)
    return false;
  if (required_features.geometryShader == VK_TRUE &&
      available_features.geometryShader == VK_FALSE)
    return false;
  if (required_features.tessellationShader == VK_TRUE &&
      available_features.tessellationShader == VK_FALSE)
    return false;
  if (required_features.sampleRateShading == VK_TRUE &&
      available_features.sampleRateShading == VK_FALSE)
    return false;
  if (required_features.dualSrcBlend == VK_TRUE &&
      available_features.dualSrcBlend == VK_FALSE)
    return false;
  if (required_features.logicOp == VK_TRUE &&
      available_features.logicOp == VK_FALSE)
    return false;
  if (required_features.multiDrawIndirect == VK_TRUE &&
      available_features.multiDrawIndirect == VK_FALSE)
    return false;
  if (required_features.drawIndirectFirstInstance == VK_TRUE &&
      available_features.drawIndirectFirstInstance == VK_FALSE)
    return false;
  if (required_features.depthClamp == VK_TRUE &&
      available_features.depthClamp == VK_FALSE)
    return false;
  if (required_features.depthBiasClamp == VK_TRUE &&
      available_features.depthBiasClamp == VK_FALSE)
    return false;
  if (required_features.fillModeNonSolid == VK_TRUE &&
      available_features.fillModeNonSolid == VK_FALSE)
    return false;
  if (required_features.depthBounds == VK_TRUE &&
      available_features.depthBounds == VK_FALSE)
    return false;
  if (required_features.wideLines == VK_TRUE &&
      available_features.wideLines == VK_FALSE)
    return false;
  if (required_features.largePoints == VK_TRUE &&
      available_features.largePoints == VK_FALSE)
    return false;
  if (required_features.alphaToOne == VK_TRUE &&
      available_features.alphaToOne == VK_FALSE)
    return false;
  if (required_features.multiViewport == VK_TRUE &&
      available_features.multiViewport == VK_FALSE)
    return false;
  if (required_features.samplerAnisotropy == VK_TRUE &&
      available_features.samplerAnisotropy == VK_FALSE)
    return false;
  if (required_features.textureCompressionETC2 == VK_TRUE &&
      available_features.textureCompressionETC2 == VK_FALSE)
    return false;
  if (required_features.textureCompressionASTC_LDR == VK_TRUE &&
      available_features.textureCompressionASTC_LDR == VK_FALSE)
    return false;
  if (required_features.textureCompressionBC == VK_TRUE &&
      available_features.textureCompressionBC == VK_FALSE)
    return false;
  if (required_features.occlusionQueryPrecise == VK_TRUE &&
      available_features.occlusionQueryPrecise == VK_FALSE)
    return false;
  if (required_features.pipelineStatisticsQuery == VK_TRUE &&
      available_features.pipelineStatisticsQuery == VK_FALSE)
    return false;
  if (required_features.vertexPipelineStoresAndAtomics == VK_TRUE &&
      available_features.vertexPipelineStoresAndAtomics == VK_FALSE)
    return false;
  if (required_features.fragmentStoresAndAtomics == VK_TRUE &&
      available_features.fragmentStoresAndAtomics == VK_FALSE)
    return false;
  if (required_features.shaderTessellationAndGeometryPointSize == VK_TRUE &&
      available_features.shaderTessellationAndGeometryPointSize == VK_FALSE)
    return false;
  if (required_features.shaderImageGatherExtended == VK_TRUE &&
      available_features.shaderImageGatherExtended == VK_FALSE)
    return false;
  if (required_features.shaderStorageImageExtendedFormats == VK_TRUE &&
      available_features.shaderStorageImageExtendedFormats == VK_FALSE)
    return false;
  if (required_features.shaderStorageImageMultisample == VK_TRUE &&
      available_features.shaderStorageImageMultisample == VK_FALSE)
    return false;
  if (required_features.shaderStorageImageReadWithoutFormat == VK_TRUE &&
      available_features.shaderStorageImageReadWithoutFormat == VK_FALSE)
    return false;
  if (required_features.shaderStorageImageWriteWithoutFormat == VK_TRUE &&
      available_features.shaderStorageImageWriteWithoutFormat == VK_FALSE)
    return false;
  if (required_features.shaderUniformBufferArrayDynamicIndexing == VK_TRUE &&
      available_features.shaderUniformBufferArrayDynamicIndexing == VK_FALSE)
    return false;
  if (required_features.shaderSampledImageArrayDynamicIndexing == VK_TRUE &&
      available_features.shaderSampledImageArrayDynamicIndexing == VK_FALSE)
    return false;
  if (required_features.shaderStorageBufferArrayDynamicIndexing == VK_TRUE &&
      available_features.shaderStorageBufferArrayDynamicIndexing == VK_FALSE)
    return false;
  if (required_features.shaderStorageImageArrayDynamicIndexing == VK_TRUE &&
      available_features.shaderStorageImageArrayDynamicIndexing == VK_FALSE)
    return false;
  if (required_features.shaderClipDistance == VK_TRUE &&
      available_features.shaderClipDistance == VK_FALSE)
    return false;
  if (required_features.shaderCullDistance == VK_TRUE &&
      available_features.shaderCullDistance == VK_FALSE)
    return false;
  if (required_features.shaderFloat64 == VK_TRUE &&
      available_features.shaderFloat64 == VK_FALSE)
    return false;
  if (required_features.shaderInt64 == VK_TRUE &&
      available_features.shaderInt64 == VK_FALSE)
    return false;
  if (required_features.shaderInt16 == VK_TRUE &&
      available_features.shaderInt16 == VK_FALSE)
    return false;
  if (required_features.shaderResourceResidency == VK_TRUE &&
      available_features.shaderResourceResidency == VK_FALSE)
    return false;
  if (required_features.shaderResourceMinLod == VK_TRUE &&
      available_features.shaderResourceMinLod == VK_FALSE)
    return false;
  if (required_features.sparseBinding == VK_TRUE &&
      available_features.sparseBinding == VK_FALSE)
    return false;
  if (required_features.sparseResidencyBuffer == VK_TRUE &&
      available_features.sparseResidencyBuffer == VK_FALSE)
    return false;
  if (required_features.sparseResidencyImage2D == VK_TRUE &&
      available_features.sparseResidencyImage2D == VK_FALSE)
    return false;
  if (required_features.sparseResidencyImage3D == VK_TRUE &&
      available_features.sparseResidencyImage3D == VK_FALSE)
    return false;
  if (required_features.sparseResidency2Samples == VK_TRUE &&
      available_features.sparseResidency2Samples == VK_FALSE)
    return false;
  if (required_features.sparseResidency4Samples == VK_TRUE &&
      available_features.sparseResidency4Samples == VK_FALSE)
    return false;
  if (required_features.sparseResidency8Samples == VK_TRUE &&
      available_features.sparseResidency8Samples == VK_FALSE)
    return false;
  if (required_features.sparseResidency16Samples == VK_TRUE &&
      available_features.sparseResidency16Samples == VK_FALSE)
    return false;
  if (required_features.sparseResidencyAliased == VK_TRUE &&
      available_features.sparseResidencyAliased == VK_FALSE)
    return false;
  if (required_features.variableMultisampleRate == VK_TRUE &&
      available_features.variableMultisampleRate == VK_FALSE)
    return false;
  if (required_features.inheritedQueries == VK_TRUE &&
      available_features.inheritedQueries == VK_FALSE)
    return false;

  return true;
}

}  // namespace

Device::Device() = default;
Device::Device(VkDevice device) : device_(device), destroy_device_(false) {}
Device::~Device() = default;

void Device::Shutdown() {
  if (destroy_device_) {
    vkDestroyDevice(device_, nullptr);
    vkDestroyInstance(instance_, nullptr);
  }
}

Result Device::Initialize(const VkPhysicalDeviceFeatures& required_features) {
  if (device_ == VK_NULL_HANDLE) {
    Result r = CreateInstance();
    if (!r.IsSuccess())
      return r;

    r = ChoosePhysicalDevice(required_features);
    if (!r.IsSuccess())
      return r;

    r = CreateDevice(required_features);
    if (!r.IsSuccess())
      return r;
  }

  if (queue_ == VK_NULL_HANDLE) {
    vkGetDeviceQueue(device_, queue_family_index_, 0, &queue_);
    if (queue_ == VK_NULL_HANDLE)
      return Result("Vulkan::Calling vkGetDeviceQueue Fail");
  }
  return {};
}

bool Device::ChooseQueueFamilyIndex(const VkPhysicalDevice& physical_device) {
  uint32_t count;
  std::vector<VkQueueFamilyProperties> properties;

  vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &count, nullptr);
  properties.resize(count);
  vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &count,
                                           properties.data());

  for (uint32_t i = 0; i < count; ++i) {
    if (properties[i].queueFlags &
        (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT)) {
      queue_family_flags_ = properties[i].queueFlags;
      queue_family_index_ = i;
      return true;
    }
  }

  for (uint32_t i = 0; i < count; ++i) {
    if (properties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
      queue_family_flags_ = properties[i].queueFlags;
      queue_family_index_ = i;
      return true;
    }
  }

  return false;
}

Result Device::CreateInstance() {
  VkApplicationInfo appInfo = {};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);

  VkInstanceCreateInfo instInfo = {};
  instInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instInfo.pApplicationInfo = &appInfo;
  // TODO(jaebaek): Enable layers, extensions

  if (vkCreateInstance(&instInfo, nullptr, &instance_) != VK_SUCCESS)
    return Result("Vulkan::Calling vkCreateInstance Fail");

  return {};
}

Result Device::ChoosePhysicalDevice(
    const VkPhysicalDeviceFeatures& required_features) {
  uint32_t count;
  std::vector<VkPhysicalDevice> physical_devices;

  if (vkEnumeratePhysicalDevices(instance_, &count, nullptr) != VK_SUCCESS)
    return Result("Vulkan::Calling vkEnumeratePhysicalDevices Fail");
  physical_devices.resize(count);
  if (vkEnumeratePhysicalDevices(instance_, &count, physical_devices.data()) !=
      VK_SUCCESS)
    return Result("Vulkan::Calling vkEnumeratePhysicalDevices Fail");

  for (uint32_t i = 0; i < count; ++i) {
    VkPhysicalDeviceFeatures available_features = {};
    vkGetPhysicalDeviceFeatures(physical_devices[i], &available_features);

    if (!AreAllRequiredFeaturesSupported(available_features, required_features))
      continue;

    if (ChooseQueueFamilyIndex(physical_devices[i])) {
      physical_device_ = physical_devices[i];
      vkGetPhysicalDeviceMemoryProperties(physical_device_,
                                          &physical_memory_properties_);
      return {};
    }
  }

  return Result("Vulkan::No physical device supports Vulkan");
}

Result Device::CreateDevice(const VkPhysicalDeviceFeatures& required_features) {
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
  // TODO(jaebaek): Enable layers, extensions
  info.pEnabledFeatures = &required_features;

  if (vkCreateDevice(physical_device_, &info, nullptr, &device_) != VK_SUCCESS)
    return Result("Vulkan::Calling vkCreateDevice Fail");

  return {};
}

}  // namespace vulkan
}  // namespace amber
