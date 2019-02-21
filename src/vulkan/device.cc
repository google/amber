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

#include <cstring>
#include <memory>
#include <set>
#include <vector>

#include "src/make_unique.h"

namespace amber {
namespace vulkan {
namespace {

bool AreAllRequiredFeaturesSupported(
    const VkPhysicalDeviceFeatures& available_features,
    const std::vector<Feature>& required_features) {
  if (required_features.empty())
    return true;

  for (const auto& feature : required_features) {
    switch (feature) {
      case Feature::kRobustBufferAccess:
        if (available_features.robustBufferAccess == VK_FALSE)
          return false;
        break;
      case Feature::kFullDrawIndexUint32:
        if (available_features.fullDrawIndexUint32 == VK_FALSE)
          return false;
        break;
      case Feature::kImageCubeArray:
        if (available_features.imageCubeArray == VK_FALSE)
          return false;
        break;
      case Feature::kIndependentBlend:
        if (available_features.independentBlend == VK_FALSE)
          return false;
        break;
      case Feature::kGeometryShader:
        if (available_features.geometryShader == VK_FALSE)
          return false;
        break;
      case Feature::kTessellationShader:
        if (available_features.tessellationShader == VK_FALSE)
          return false;
        break;
      case Feature::kSampleRateShading:
        if (available_features.sampleRateShading == VK_FALSE)
          return false;
        break;
      case Feature::kDualSrcBlend:
        if (available_features.dualSrcBlend == VK_FALSE)
          return false;
        break;
      case Feature::kLogicOp:
        if (available_features.logicOp == VK_FALSE)
          return false;
        break;
      case Feature::kMultiDrawIndirect:
        if (available_features.multiDrawIndirect == VK_FALSE)
          return false;
        break;
      case Feature::kDrawIndirectFirstInstance:
        if (available_features.drawIndirectFirstInstance == VK_FALSE)
          return false;
        break;
      case Feature::kDepthClamp:
        if (available_features.depthClamp == VK_FALSE)
          return false;
        break;
      case Feature::kDepthBiasClamp:
        if (available_features.depthBiasClamp == VK_FALSE)
          return false;
        break;
      case Feature::kFillModeNonSolid:
        if (available_features.fillModeNonSolid == VK_FALSE)
          return false;
        break;
      case Feature::kDepthBounds:
        if (available_features.depthBounds == VK_FALSE)
          return false;
        break;
      case Feature::kWideLines:
        if (available_features.wideLines == VK_FALSE)
          return false;
        break;
      case Feature::kLargePoints:
        if (available_features.largePoints == VK_FALSE)
          return false;
        break;
      case Feature::kAlphaToOne:
        if (available_features.alphaToOne == VK_FALSE)
          return false;
        break;
      case Feature::kMultiViewport:
        if (available_features.multiViewport == VK_FALSE)
          return false;
        break;
      case Feature::kSamplerAnisotropy:
        if (available_features.samplerAnisotropy == VK_FALSE)
          return false;
        break;
      case Feature::kTextureCompressionETC2:
        if (available_features.textureCompressionETC2 == VK_FALSE)
          return false;
        break;
      case Feature::kTextureCompressionASTC_LDR:
        if (available_features.textureCompressionASTC_LDR == VK_FALSE)
          return false;
        break;
      case Feature::kTextureCompressionBC:
        if (available_features.textureCompressionBC == VK_FALSE)
          return false;
        break;
      case Feature::kOcclusionQueryPrecise:
        if (available_features.occlusionQueryPrecise == VK_FALSE)
          return false;
        break;
      case Feature::kPipelineStatisticsQuery:
        if (available_features.pipelineStatisticsQuery == VK_FALSE)
          return false;
        break;
      case Feature::kVertexPipelineStoresAndAtomics:
        if (available_features.vertexPipelineStoresAndAtomics == VK_FALSE)
          return false;
        break;
      case Feature::kFragmentStoresAndAtomics:
        if (available_features.fragmentStoresAndAtomics == VK_FALSE)
          return false;
        break;
      case Feature::kShaderTessellationAndGeometryPointSize:
        if (available_features.shaderTessellationAndGeometryPointSize ==
            VK_FALSE)
          return false;
        break;
      case Feature::kShaderImageGatherExtended:
        if (available_features.shaderImageGatherExtended == VK_FALSE)
          return false;
        break;
      case Feature::kShaderStorageImageExtendedFormats:
        if (available_features.shaderStorageImageExtendedFormats == VK_FALSE)
          return false;
        break;
      case Feature::kShaderStorageImageMultisample:
        if (available_features.shaderStorageImageMultisample == VK_FALSE)
          return false;
        break;
      case Feature::kShaderStorageImageReadWithoutFormat:
        if (available_features.shaderStorageImageReadWithoutFormat == VK_FALSE)
          return false;
        break;
      case Feature::kShaderStorageImageWriteWithoutFormat:
        if (available_features.shaderStorageImageWriteWithoutFormat == VK_FALSE)
          return false;
        break;
      case Feature::kShaderUniformBufferArrayDynamicIndexing:
        if (available_features.shaderUniformBufferArrayDynamicIndexing ==
            VK_FALSE)
          return false;
        break;
      case Feature::kShaderSampledImageArrayDynamicIndexing:
        if (available_features.shaderSampledImageArrayDynamicIndexing ==
            VK_FALSE)
          return false;
        break;
      case Feature::kShaderStorageBufferArrayDynamicIndexing:
        if (available_features.shaderStorageBufferArrayDynamicIndexing ==
            VK_FALSE)
          return false;
        break;
      case Feature::kShaderStorageImageArrayDynamicIndexing:
        if (available_features.shaderStorageImageArrayDynamicIndexing ==
            VK_FALSE)
          return false;
        break;
      case Feature::kShaderClipDistance:
        if (available_features.shaderClipDistance == VK_FALSE)
          return false;
        break;
      case Feature::kShaderCullDistance:
        if (available_features.shaderCullDistance == VK_FALSE)
          return false;
        break;
      case Feature::kShaderFloat64:
        if (available_features.shaderFloat64 == VK_FALSE)
          return false;
        break;
      case Feature::kShaderInt64:
        if (available_features.shaderInt64 == VK_FALSE)
          return false;
        break;
      case Feature::kShaderInt16:
        if (available_features.shaderInt16 == VK_FALSE)
          return false;
        break;
      case Feature::kShaderResourceResidency:
        if (available_features.shaderResourceResidency == VK_FALSE)
          return false;
        break;
      case Feature::kShaderResourceMinLod:
        if (available_features.shaderResourceMinLod == VK_FALSE)
          return false;
        break;
      case Feature::kSparseBinding:
        if (available_features.sparseBinding == VK_FALSE)
          return false;
        break;
      case Feature::kSparseResidencyBuffer:
        if (available_features.sparseResidencyBuffer == VK_FALSE)
          return false;
        break;
      case Feature::kSparseResidencyImage2D:
        if (available_features.sparseResidencyImage2D == VK_FALSE)
          return false;
        break;
      case Feature::kSparseResidencyImage3D:
        if (available_features.sparseResidencyImage3D == VK_FALSE)
          return false;
        break;
      case Feature::kSparseResidency2Samples:
        if (available_features.sparseResidency2Samples == VK_FALSE)
          return false;
        break;
      case Feature::kSparseResidency4Samples:
        if (available_features.sparseResidency4Samples == VK_FALSE)
          return false;
        break;
      case Feature::kSparseResidency8Samples:
        if (available_features.sparseResidency8Samples == VK_FALSE)
          return false;
        break;
      case Feature::kSparseResidency16Samples:
        if (available_features.sparseResidency16Samples == VK_FALSE)
          return false;
        break;
      case Feature::kSparseResidencyAliased:
        if (available_features.sparseResidencyAliased == VK_FALSE)
          return false;
        break;
      case Feature::kVariableMultisampleRate:
        if (available_features.variableMultisampleRate == VK_FALSE)
          return false;
        break;
      case Feature::kInheritedQueries:
        if (available_features.inheritedQueries == VK_FALSE)
          return false;
        break;
      case Feature::kFramebuffer:
      case Feature::kDepthStencil:
      case Feature::kFenceTimeout:
      case Feature::kUnknown:
        break;
    }
  }

  return true;
}

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

}  // namespace

Device::Device(VkInstance instance,
               VkPhysicalDevice physical_device,
               uint32_t queue_family_index,
               VkDevice device,
               VkQueue queue)
    : instance_(instance),
      physical_device_(physical_device),
      device_(device),
      queue_(queue),
      queue_family_index_(queue_family_index) {}

Device::~Device() = default;

Result Device::LoadVulkanPointers(
    PFN_vkGetInstanceProcAddr getInstanceProcAddr) {
#define AMBER_VK_FUNC(func)                                    \
  if (!(ptrs_.func = reinterpret_cast<PFN_##func>(             \
            getInstanceProcAddr(instance_, #func)))) {         \
    return Result("Vulkan: Unable to load " #func " pointer"); \
  }
#include "src/vulkan/vk-funcs.inc"
#undef AMBER_VK_FUNC
  return {};
}

Result Device::Initialize(
    PFN_vkGetInstanceProcAddr getInstanceProcAddr,
    const std::vector<Feature>& required_features,
    const std::vector<std::string>& required_extensions,
    const VkPhysicalDeviceFeatures& available_features,
    const VkPhysicalDeviceFeatures2KHR& available_features2,
    const std::vector<std::string>& available_extensions) {
  Result r = LoadVulkanPointers(getInstanceProcAddr);
  if (!r.IsSuccess())
    return r;

  bool use_physical_device_features_2 = false;
  // Determine if VkPhysicalDeviceProperties2KHR should be used
  for (auto& ext : required_extensions) {
    if (ext == "VK_KHR_get_physical_device_properties2") {
      use_physical_device_features_2 = true;
    }
  }

  VkPhysicalDeviceFeatures available_vulkan_features = {};
  if (use_physical_device_features_2) {
    available_vulkan_features = available_features2.features;
  } else {
    available_vulkan_features = available_features;
  }

  if (!AreAllRequiredFeaturesSupported(available_vulkan_features,
                                       required_features)) {
    return Result(
        "Vulkan: Device::Initialize given physical device does not support "
        "required features");
  }

  if (!AreAllExtensionsSupported(available_extensions, required_extensions)) {
    return Result(
        "Vulkan: Device::Initialize given physical device does not support "
        "required extensions");
  }

  ptrs_.vkGetPhysicalDeviceProperties(physical_device_,
                                      &physical_device_properties_);

  ptrs_.vkGetPhysicalDeviceMemoryProperties(physical_device_,
                                            &physical_memory_properties_);

  return {};
}

}  // namespace vulkan
}  // namespace amber
