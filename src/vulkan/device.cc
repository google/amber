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
#include <iomanip>  // Vulkan wrappers: std::setw(), std::left/right
#include <iostream>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "src/make_unique.h"

namespace amber {
namespace vulkan {
namespace {

const char kVariablePointers[] = "VariablePointerFeatures.variablePointers";
const char kVariablePointersStorageBuffer[] =
    "VariablePointerFeatures.variablePointersStorageBuffer";

struct BaseOutStructure {
  VkStructureType sType;
  void* pNext;
};

bool AreAllRequiredFeaturesSupported(
    const VkPhysicalDeviceFeatures& available_features,
    const std::vector<std::string>& required_features) {
  if (required_features.empty())
    return true;

  for (const auto& feature : required_features) {
    if (feature == "robustBufferAccess") {
      if (available_features.robustBufferAccess == VK_FALSE)
        return false;
      continue;
    }
    if (feature == "fullDrawIndexUint32") {
      if (available_features.fullDrawIndexUint32 == VK_FALSE)
        return false;
      continue;
    }
    if (feature == "imageCubeArray") {
      if (available_features.imageCubeArray == VK_FALSE)
        return false;
      continue;
    }
    if (feature == "independentBlend") {
      if (available_features.independentBlend == VK_FALSE)
        return false;
      continue;
    }
    if (feature == "geometryShader") {
      if (available_features.geometryShader == VK_FALSE)
        return false;
      continue;
    }
    if (feature == "tessellationShader") {
      if (available_features.tessellationShader == VK_FALSE)
        return false;
      continue;
    }
    if (feature == "sampleRateShading") {
      if (available_features.sampleRateShading == VK_FALSE)
        return false;
      continue;
    }
    if (feature == "dualSrcBlend") {
      if (available_features.dualSrcBlend == VK_FALSE)
        return false;
      continue;
    }
    if (feature == "logicOp") {
      if (available_features.logicOp == VK_FALSE)
        return false;
      continue;
    }
    if (feature == "multiDrawIndirect") {
      if (available_features.multiDrawIndirect == VK_FALSE)
        return false;
      continue;
    }
    if (feature == "drawIndirectFirstInstance") {
      if (available_features.drawIndirectFirstInstance == VK_FALSE)
        return false;
      continue;
    }
    if (feature == "depthClamp") {
      if (available_features.depthClamp == VK_FALSE)
        return false;
      continue;
    }
    if (feature == "depthBiasClamp") {
      if (available_features.depthBiasClamp == VK_FALSE)
        return false;
      continue;
    }
    if (feature == "fillModeNonSolid") {
      if (available_features.fillModeNonSolid == VK_FALSE)
        return false;
      continue;
    }
    if (feature == "depthBounds") {
      if (available_features.depthBounds == VK_FALSE)
        return false;
      continue;
    }
    if (feature == "wideLines") {
      if (available_features.wideLines == VK_FALSE)
        return false;
      continue;
    }
    if (feature == "largePoints") {
      if (available_features.largePoints == VK_FALSE)
        return false;
      continue;
    }
    if (feature == "alphaToOne") {
      if (available_features.alphaToOne == VK_FALSE)
        return false;
      continue;
    }
    if (feature == "multiViewport") {
      if (available_features.multiViewport == VK_FALSE)
        return false;
      continue;
    }
    if (feature == "samplerAnisotropy") {
      if (available_features.samplerAnisotropy == VK_FALSE)
        return false;
      continue;
    }
    if (feature == "textureCompressionETC2") {
      if (available_features.textureCompressionETC2 == VK_FALSE)
        return false;
      continue;
    }
    if (feature == "textureCompressionASTC_LDR") {
      if (available_features.textureCompressionASTC_LDR == VK_FALSE)
        return false;
      continue;
    }
    if (feature == "textureCompressionBC") {
      if (available_features.textureCompressionBC == VK_FALSE)
        return false;
      continue;
    }
    if (feature == "occlusionQueryPrecise") {
      if (available_features.occlusionQueryPrecise == VK_FALSE)
        return false;
      continue;
    }
    if (feature == "pipelineStatisticsQuery") {
      if (available_features.pipelineStatisticsQuery == VK_FALSE)
        return false;
      continue;
    }
    if (feature == "vertexPipelineStoresAndAtomics") {
      if (available_features.vertexPipelineStoresAndAtomics == VK_FALSE)
        return false;
      continue;
    }
    if (feature == "fragmentStoresAndAtomics") {
      if (available_features.fragmentStoresAndAtomics == VK_FALSE)
        return false;
      continue;
    }
    if (feature == "shaderTessellationAndGeometryPointSize") {
      if (available_features.shaderTessellationAndGeometryPointSize == VK_FALSE)
        return false;
      continue;
    }
    if (feature == "shaderImageGatherExtended") {
      if (available_features.shaderImageGatherExtended == VK_FALSE)
        return false;
      continue;
    }
    if (feature == "shaderStorageImageExtendedFormats") {
      if (available_features.shaderStorageImageExtendedFormats == VK_FALSE)
        return false;
      continue;
    }
    if (feature == "shaderStorageImageMultisample") {
      if (available_features.shaderStorageImageMultisample == VK_FALSE)
        return false;
      continue;
    }
    if (feature == "shaderStorageImageReadWithoutFormat") {
      if (available_features.shaderStorageImageReadWithoutFormat == VK_FALSE)
        return false;
      continue;
    }
    if (feature == "shaderStorageImageWriteWithoutFormat") {
      if (available_features.shaderStorageImageWriteWithoutFormat == VK_FALSE)
        return false;
      continue;
    }
    if (feature == "shaderUniformBufferArrayDynamicIndexing") {
      if (available_features.shaderUniformBufferArrayDynamicIndexing ==
          VK_FALSE)
        return false;
      continue;
    }
    if (feature == "shaderSampledImageArrayDynamicIndexing") {
      if (available_features.shaderSampledImageArrayDynamicIndexing == VK_FALSE)
        return false;
      continue;
    }
    if (feature == "shaderStorageBufferArrayDynamicIndexing") {
      if (available_features.shaderStorageBufferArrayDynamicIndexing ==
          VK_FALSE)
        return false;
      continue;
    }
    if (feature == "shaderStorageImageArrayDynamicIndexing") {
      if (available_features.shaderStorageImageArrayDynamicIndexing == VK_FALSE)
        return false;
      continue;
    }
    if (feature == "shaderClipDistance") {
      if (available_features.shaderClipDistance == VK_FALSE)
        return false;
      continue;
    }
    if (feature == "shaderCullDistance") {
      if (available_features.shaderCullDistance == VK_FALSE)
        return false;
      continue;
    }
    if (feature == "shaderFloat64") {
      if (available_features.shaderFloat64 == VK_FALSE)
        return false;
      continue;
    }
    if (feature == "shaderInt64") {
      if (available_features.shaderInt64 == VK_FALSE)
        return false;
      continue;
    }
    if (feature == "shaderInt16") {
      if (available_features.shaderInt16 == VK_FALSE)
        return false;
      continue;
    }
    if (feature == "shaderResourceResidency") {
      if (available_features.shaderResourceResidency == VK_FALSE)
        return false;
      continue;
    }
    if (feature == "shaderResourceMinLod") {
      if (available_features.shaderResourceMinLod == VK_FALSE)
        return false;
      continue;
    }
    if (feature == "sparseBinding") {
      if (available_features.sparseBinding == VK_FALSE)
        return false;
      continue;
    }
    if (feature == "sparseResidencyBuffer") {
      if (available_features.sparseResidencyBuffer == VK_FALSE)
        return false;
      continue;
    }
    if (feature == "sparseResidencyImage2D") {
      if (available_features.sparseResidencyImage2D == VK_FALSE)
        return false;
      continue;
    }
    if (feature == "sparseResidencyImage3D") {
      if (available_features.sparseResidencyImage3D == VK_FALSE)
        return false;
      continue;
    }
    if (feature == "sparseResidency2Samples") {
      if (available_features.sparseResidency2Samples == VK_FALSE)
        return false;
      continue;
    }
    if (feature == "sparseResidency4Samples") {
      if (available_features.sparseResidency4Samples == VK_FALSE)
        return false;
      continue;
    }
    if (feature == "sparseResidency8Samples") {
      if (available_features.sparseResidency8Samples == VK_FALSE)
        return false;
      continue;
    }
    if (feature == "sparseResidency16Samples") {
      if (available_features.sparseResidency16Samples == VK_FALSE)
        return false;
      continue;
    }
    if (feature == "sparseResidencyAliased") {
      if (available_features.sparseResidencyAliased == VK_FALSE)
        return false;
      continue;
    }
    if (feature == "variableMultisampleRate") {
      if (available_features.variableMultisampleRate == VK_FALSE)
        return false;
      continue;
    }
    if (feature == "inheritedQueries") {
      if (available_features.inheritedQueries == VK_FALSE)
        return false;
      continue;
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

Result Device::LoadVulkanPointers(PFN_vkGetInstanceProcAddr getInstanceProcAddr,
                                  Delegate* delegate) {
  // Note: logging Vulkan calls is done via the delegate rather than a Vulkan
  // layer because we want such logging even when Amber is built as a native
  // executable on Android, where Vulkan layers are usable only with APKs.
  if (delegate && delegate->LogGraphicsCalls())
    delegate->Log("Loading Vulkan Pointers");

#include "vk-wrappers.inc"

  return {};
}

Result Device::Initialize(
    PFN_vkGetInstanceProcAddr getInstanceProcAddr,
    Delegate* delegate,
    const std::vector<std::string>& required_features,
    const std::vector<std::string>& required_extensions,
    const VkPhysicalDeviceFeatures& available_features,
    const VkPhysicalDeviceFeatures2KHR& available_features2,
    const std::vector<std::string>& available_extensions) {
  Result r = LoadVulkanPointers(getInstanceProcAddr, delegate);
  if (!r.IsSuccess())
    return r;

  bool use_physical_device_features_2 = false;
  // Determine if VkPhysicalDeviceProperties2KHR should be used
  for (auto& ext : required_extensions) {
    if (ext == "VK_KHR_get_physical_device_properties2") {
      use_physical_device_features_2 = true;
    }
  }

  VkPhysicalDeviceFeatures available_vulkan_features =
      VkPhysicalDeviceFeatures();
  if (use_physical_device_features_2) {
    available_vulkan_features = available_features2.features;

    VkPhysicalDeviceVariablePointerFeaturesKHR* var_ptrs = nullptr;
    void* ptr = available_features2.pNext;
    while (ptr != nullptr) {
      BaseOutStructure* s = static_cast<BaseOutStructure*>(ptr);
      if (s->sType ==
          VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VARIABLE_POINTER_FEATURES_KHR) {
        var_ptrs =
            static_cast<VkPhysicalDeviceVariablePointerFeaturesKHR*>(ptr);
        break;
      }
      ptr = s->pNext;
    }

    std::vector<std::string> required_features1;
    for (const auto& feature : required_features) {
      // No dot means this is a features1 feature.
      if (feature.find_first_of('.') == std::string::npos) {
        required_features1.push_back(feature);
        continue;
      }

      if ((feature == kVariablePointers ||
           feature == kVariablePointersStorageBuffer) &&
          var_ptrs == nullptr) {
        return amber::Result(
            "Variable pointers requested but feature not returned");
      }

      if (feature == kVariablePointers &&
          var_ptrs->variablePointers != VK_TRUE) {
        return amber::Result("Missing variable pointers feature");
      }
      if (feature == kVariablePointersStorageBuffer &&
          var_ptrs->variablePointersStorageBuffer != VK_TRUE) {
        return amber::Result(
            "Missing variable pointers storage buffer feature");
      }
    }

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

bool Device::IsFormatSupportedByPhysicalDevice(const Format& format,
                                               Buffer* buffer) {
  VkFormat vk_format = GetVkFormat(format);
  VkFormatProperties properties = VkFormatProperties();
  GetPtrs()->vkGetPhysicalDeviceFormatProperties(physical_device_, vk_format,
                                                 &properties);

  VkFormatFeatureFlagBits flag = VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT;
  bool is_buffer_type_image = false;
  switch (buffer->GetBufferType()) {
    case BufferType::kColor:
      flag = VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT;
      is_buffer_type_image = true;
      break;
    case BufferType::kDepth:
      flag = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
      is_buffer_type_image = true;
      break;
    case BufferType::kSampled:
      flag = VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT;
      is_buffer_type_image = true;
      break;
    case BufferType::kVertex:
      flag = VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT;
      is_buffer_type_image = false;
      break;
    default:
      return false;
  }

  return ((is_buffer_type_image ? properties.optimalTilingFeatures
                                : properties.bufferFeatures) &
          flag) == flag;
}

bool Device::HasMemoryFlags(uint32_t memory_type_index,
                            const VkMemoryPropertyFlags flags) const {
  return (physical_memory_properties_.memoryTypes[memory_type_index]
              .propertyFlags &
          flags) == flags;
}

bool Device::IsMemoryHostAccessible(uint32_t memory_type_index) const {
  return HasMemoryFlags(memory_type_index, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
}

bool Device::IsMemoryHostCoherent(uint32_t memory_type_index) const {
  return HasMemoryFlags(memory_type_index,
                        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
}

uint32_t Device::GetMaxPushConstants() const {
  return physical_device_properties_.limits.maxPushConstantsSize;
}

bool Device::IsDescriptorSetInBounds(uint32_t descriptor_set) const {
  VkPhysicalDeviceProperties properties = VkPhysicalDeviceProperties();
  GetPtrs()->vkGetPhysicalDeviceProperties(physical_device_, &properties);
  return properties.limits.maxBoundDescriptorSets > descriptor_set;
}

VkFormat Device::GetVkFormat(const Format& format) const {
  VkFormat ret = VK_FORMAT_UNDEFINED;
  switch (format.GetFormatType()) {
    case FormatType::kUnknown:
      ret = VK_FORMAT_UNDEFINED;
      break;
    case FormatType::kA1R5G5B5_UNORM_PACK16:
      ret = VK_FORMAT_A1R5G5B5_UNORM_PACK16;
      break;
    case FormatType::kA2B10G10R10_SINT_PACK32:
      ret = VK_FORMAT_A2B10G10R10_SINT_PACK32;
      break;
    case FormatType::kA2B10G10R10_SNORM_PACK32:
      ret = VK_FORMAT_A2B10G10R10_SNORM_PACK32;
      break;
    case FormatType::kA2B10G10R10_SSCALED_PACK32:
      ret = VK_FORMAT_A2B10G10R10_SSCALED_PACK32;
      break;
    case FormatType::kA2B10G10R10_UINT_PACK32:
      ret = VK_FORMAT_A2B10G10R10_UINT_PACK32;
      break;
    case FormatType::kA2B10G10R10_UNORM_PACK32:
      ret = VK_FORMAT_A2B10G10R10_UNORM_PACK32;
      break;
    case FormatType::kA2B10G10R10_USCALED_PACK32:
      ret = VK_FORMAT_A2B10G10R10_USCALED_PACK32;
      break;
    case FormatType::kA2R10G10B10_SINT_PACK32:
      ret = VK_FORMAT_A2R10G10B10_SINT_PACK32;
      break;
    case FormatType::kA2R10G10B10_SNORM_PACK32:
      ret = VK_FORMAT_A2R10G10B10_SNORM_PACK32;
      break;
    case FormatType::kA2R10G10B10_SSCALED_PACK32:
      ret = VK_FORMAT_A2R10G10B10_SSCALED_PACK32;
      break;
    case FormatType::kA2R10G10B10_UINT_PACK32:
      ret = VK_FORMAT_A2R10G10B10_UINT_PACK32;
      break;
    case FormatType::kA2R10G10B10_UNORM_PACK32:
      ret = VK_FORMAT_A2R10G10B10_UNORM_PACK32;
      break;
    case FormatType::kA2R10G10B10_USCALED_PACK32:
      ret = VK_FORMAT_A2R10G10B10_USCALED_PACK32;
      break;
    case FormatType::kA8B8G8R8_SINT_PACK32:
      ret = VK_FORMAT_A8B8G8R8_SINT_PACK32;
      break;
    case FormatType::kA8B8G8R8_SNORM_PACK32:
      ret = VK_FORMAT_A8B8G8R8_SNORM_PACK32;
      break;
    case FormatType::kA8B8G8R8_SRGB_PACK32:
      ret = VK_FORMAT_A8B8G8R8_SRGB_PACK32;
      break;
    case FormatType::kA8B8G8R8_SSCALED_PACK32:
      ret = VK_FORMAT_A8B8G8R8_SSCALED_PACK32;
      break;
    case FormatType::kA8B8G8R8_UINT_PACK32:
      ret = VK_FORMAT_A8B8G8R8_UINT_PACK32;
      break;
    case FormatType::kA8B8G8R8_UNORM_PACK32:
      ret = VK_FORMAT_A8B8G8R8_UNORM_PACK32;
      break;
    case FormatType::kA8B8G8R8_USCALED_PACK32:
      ret = VK_FORMAT_A8B8G8R8_USCALED_PACK32;
      break;
    case FormatType::kB10G11R11_UFLOAT_PACK32:
      ret = VK_FORMAT_B10G11R11_UFLOAT_PACK32;
      break;
    case FormatType::kB4G4R4A4_UNORM_PACK16:
      ret = VK_FORMAT_B4G4R4A4_UNORM_PACK16;
      break;
    case FormatType::kB5G5R5A1_UNORM_PACK16:
      ret = VK_FORMAT_B5G5R5A1_UNORM_PACK16;
      break;
    case FormatType::kB5G6R5_UNORM_PACK16:
      ret = VK_FORMAT_B5G6R5_UNORM_PACK16;
      break;
    case FormatType::kB8G8R8A8_SINT:
      ret = VK_FORMAT_B8G8R8A8_SINT;
      break;
    case FormatType::kB8G8R8A8_SNORM:
      ret = VK_FORMAT_B8G8R8A8_SNORM;
      break;
    case FormatType::kB8G8R8A8_SRGB:
      ret = VK_FORMAT_B8G8R8A8_SRGB;
      break;
    case FormatType::kB8G8R8A8_SSCALED:
      ret = VK_FORMAT_B8G8R8A8_SSCALED;
      break;
    case FormatType::kB8G8R8A8_UINT:
      ret = VK_FORMAT_B8G8R8A8_UINT;
      break;
    case FormatType::kB8G8R8A8_UNORM:
      ret = VK_FORMAT_B8G8R8A8_UNORM;
      break;
    case FormatType::kB8G8R8A8_USCALED:
      ret = VK_FORMAT_B8G8R8A8_USCALED;
      break;
    case FormatType::kB8G8R8_SINT:
      ret = VK_FORMAT_B8G8R8_SINT;
      break;
    case FormatType::kB8G8R8_SNORM:
      ret = VK_FORMAT_B8G8R8_SNORM;
      break;
    case FormatType::kB8G8R8_SRGB:
      ret = VK_FORMAT_B8G8R8_SRGB;
      break;
    case FormatType::kB8G8R8_SSCALED:
      ret = VK_FORMAT_B8G8R8_SSCALED;
      break;
    case FormatType::kB8G8R8_UINT:
      ret = VK_FORMAT_B8G8R8_UINT;
      break;
    case FormatType::kB8G8R8_UNORM:
      ret = VK_FORMAT_B8G8R8_UNORM;
      break;
    case FormatType::kB8G8R8_USCALED:
      ret = VK_FORMAT_B8G8R8_USCALED;
      break;
    case FormatType::kD16_UNORM:
      ret = VK_FORMAT_D16_UNORM;
      break;
    case FormatType::kD16_UNORM_S8_UINT:
      ret = VK_FORMAT_D16_UNORM_S8_UINT;
      break;
    case FormatType::kD24_UNORM_S8_UINT:
      ret = VK_FORMAT_D24_UNORM_S8_UINT;
      break;
    case FormatType::kD32_SFLOAT:
      ret = VK_FORMAT_D32_SFLOAT;
      break;
    case FormatType::kD32_SFLOAT_S8_UINT:
      ret = VK_FORMAT_D32_SFLOAT_S8_UINT;
      break;
    case FormatType::kR16G16B16A16_SFLOAT:
      ret = VK_FORMAT_R16G16B16A16_SFLOAT;
      break;
    case FormatType::kR16G16B16A16_SINT:
      ret = VK_FORMAT_R16G16B16A16_SINT;
      break;
    case FormatType::kR16G16B16A16_SNORM:
      ret = VK_FORMAT_R16G16B16A16_SNORM;
      break;
    case FormatType::kR16G16B16A16_SSCALED:
      ret = VK_FORMAT_R16G16B16A16_SSCALED;
      break;
    case FormatType::kR16G16B16A16_UINT:
      ret = VK_FORMAT_R16G16B16A16_UINT;
      break;
    case FormatType::kR16G16B16A16_UNORM:
      ret = VK_FORMAT_R16G16B16A16_UNORM;
      break;
    case FormatType::kR16G16B16A16_USCALED:
      ret = VK_FORMAT_R16G16B16A16_USCALED;
      break;
    case FormatType::kR16G16B16_SFLOAT:
      ret = VK_FORMAT_R16G16B16_SFLOAT;
      break;
    case FormatType::kR16G16B16_SINT:
      ret = VK_FORMAT_R16G16B16_SINT;
      break;
    case FormatType::kR16G16B16_SNORM:
      ret = VK_FORMAT_R16G16B16_SNORM;
      break;
    case FormatType::kR16G16B16_SSCALED:
      ret = VK_FORMAT_R16G16B16_SSCALED;
      break;
    case FormatType::kR16G16B16_UINT:
      ret = VK_FORMAT_R16G16B16_UINT;
      break;
    case FormatType::kR16G16B16_UNORM:
      ret = VK_FORMAT_R16G16B16_UNORM;
      break;
    case FormatType::kR16G16B16_USCALED:
      ret = VK_FORMAT_R16G16B16_USCALED;
      break;
    case FormatType::kR16G16_SFLOAT:
      ret = VK_FORMAT_R16G16_SFLOAT;
      break;
    case FormatType::kR16G16_SINT:
      ret = VK_FORMAT_R16G16_SINT;
      break;
    case FormatType::kR16G16_SNORM:
      ret = VK_FORMAT_R16G16_SNORM;
      break;
    case FormatType::kR16G16_SSCALED:
      ret = VK_FORMAT_R16G16_SSCALED;
      break;
    case FormatType::kR16G16_UINT:
      ret = VK_FORMAT_R16G16_UINT;
      break;
    case FormatType::kR16G16_UNORM:
      ret = VK_FORMAT_R16G16_UNORM;
      break;
    case FormatType::kR16G16_USCALED:
      ret = VK_FORMAT_R16G16_USCALED;
      break;
    case FormatType::kR16_SFLOAT:
      ret = VK_FORMAT_R16_SFLOAT;
      break;
    case FormatType::kR16_SINT:
      ret = VK_FORMAT_R16_SINT;
      break;
    case FormatType::kR16_SNORM:
      ret = VK_FORMAT_R16_SNORM;
      break;
    case FormatType::kR16_SSCALED:
      ret = VK_FORMAT_R16_SSCALED;
      break;
    case FormatType::kR16_UINT:
      ret = VK_FORMAT_R16_UINT;
      break;
    case FormatType::kR16_UNORM:
      ret = VK_FORMAT_R16_UNORM;
      break;
    case FormatType::kR16_USCALED:
      ret = VK_FORMAT_R16_USCALED;
      break;
    case FormatType::kR32G32B32A32_SFLOAT:
      ret = VK_FORMAT_R32G32B32A32_SFLOAT;
      break;
    case FormatType::kR32G32B32A32_SINT:
      ret = VK_FORMAT_R32G32B32A32_SINT;
      break;
    case FormatType::kR32G32B32A32_UINT:
      ret = VK_FORMAT_R32G32B32A32_UINT;
      break;
    case FormatType::kR32G32B32_SFLOAT:
      ret = VK_FORMAT_R32G32B32_SFLOAT;
      break;
    case FormatType::kR32G32B32_SINT:
      ret = VK_FORMAT_R32G32B32_SINT;
      break;
    case FormatType::kR32G32B32_UINT:
      ret = VK_FORMAT_R32G32B32_UINT;
      break;
    case FormatType::kR32G32_SFLOAT:
      ret = VK_FORMAT_R32G32_SFLOAT;
      break;
    case FormatType::kR32G32_SINT:
      ret = VK_FORMAT_R32G32_SINT;
      break;
    case FormatType::kR32G32_UINT:
      ret = VK_FORMAT_R32G32_UINT;
      break;
    case FormatType::kR32_SFLOAT:
      ret = VK_FORMAT_R32_SFLOAT;
      break;
    case FormatType::kR32_SINT:
      ret = VK_FORMAT_R32_SINT;
      break;
    case FormatType::kR32_UINT:
      ret = VK_FORMAT_R32_UINT;
      break;
    case FormatType::kR4G4B4A4_UNORM_PACK16:
      ret = VK_FORMAT_R4G4B4A4_UNORM_PACK16;
      break;
    case FormatType::kR4G4_UNORM_PACK8:
      ret = VK_FORMAT_R4G4_UNORM_PACK8;
      break;
    case FormatType::kR5G5B5A1_UNORM_PACK16:
      ret = VK_FORMAT_R5G5B5A1_UNORM_PACK16;
      break;
    case FormatType::kR5G6B5_UNORM_PACK16:
      ret = VK_FORMAT_R5G6B5_UNORM_PACK16;
      break;
    case FormatType::kR64G64B64A64_SFLOAT:
      ret = VK_FORMAT_R64G64B64A64_SFLOAT;
      break;
    case FormatType::kR64G64B64A64_SINT:
      ret = VK_FORMAT_R64G64B64A64_SINT;
      break;
    case FormatType::kR64G64B64A64_UINT:
      ret = VK_FORMAT_R64G64B64A64_UINT;
      break;
    case FormatType::kR64G64B64_SFLOAT:
      ret = VK_FORMAT_R64G64B64_SFLOAT;
      break;
    case FormatType::kR64G64B64_SINT:
      ret = VK_FORMAT_R64G64B64_SINT;
      break;
    case FormatType::kR64G64B64_UINT:
      ret = VK_FORMAT_R64G64B64_UINT;
      break;
    case FormatType::kR64G64_SFLOAT:
      ret = VK_FORMAT_R64G64_SFLOAT;
      break;
    case FormatType::kR64G64_SINT:
      ret = VK_FORMAT_R64G64_SINT;
      break;
    case FormatType::kR64G64_UINT:
      ret = VK_FORMAT_R64G64_UINT;
      break;
    case FormatType::kR64_SFLOAT:
      ret = VK_FORMAT_R64_SFLOAT;
      break;
    case FormatType::kR64_SINT:
      ret = VK_FORMAT_R64_SINT;
      break;
    case FormatType::kR64_UINT:
      ret = VK_FORMAT_R64_UINT;
      break;
    case FormatType::kR8G8B8A8_SINT:
      ret = VK_FORMAT_R8G8B8A8_SINT;
      break;
    case FormatType::kR8G8B8A8_SNORM:
      ret = VK_FORMAT_R8G8B8A8_SNORM;
      break;
    case FormatType::kR8G8B8A8_SRGB:
      ret = VK_FORMAT_R8G8B8A8_SRGB;
      break;
    case FormatType::kR8G8B8A8_SSCALED:
      ret = VK_FORMAT_R8G8B8A8_SSCALED;
      break;
    case FormatType::kR8G8B8A8_UINT:
      ret = VK_FORMAT_R8G8B8A8_UINT;
      break;
    case FormatType::kR8G8B8A8_UNORM:
      ret = VK_FORMAT_R8G8B8A8_UNORM;
      break;
    case FormatType::kR8G8B8A8_USCALED:
      ret = VK_FORMAT_R8G8B8A8_USCALED;
      break;
    case FormatType::kR8G8B8_SINT:
      ret = VK_FORMAT_R8G8B8_SINT;
      break;
    case FormatType::kR8G8B8_SNORM:
      ret = VK_FORMAT_R8G8B8_SNORM;
      break;
    case FormatType::kR8G8B8_SRGB:
      ret = VK_FORMAT_R8G8B8_SRGB;
      break;
    case FormatType::kR8G8B8_SSCALED:
      ret = VK_FORMAT_R8G8B8_SSCALED;
      break;
    case FormatType::kR8G8B8_UINT:
      ret = VK_FORMAT_R8G8B8_UINT;
      break;
    case FormatType::kR8G8B8_UNORM:
      ret = VK_FORMAT_R8G8B8_UNORM;
      break;
    case FormatType::kR8G8B8_USCALED:
      ret = VK_FORMAT_R8G8B8_USCALED;
      break;
    case FormatType::kR8G8_SINT:
      ret = VK_FORMAT_R8G8_SINT;
      break;
    case FormatType::kR8G8_SNORM:
      ret = VK_FORMAT_R8G8_SNORM;
      break;
    case FormatType::kR8G8_SRGB:
      ret = VK_FORMAT_R8G8_SRGB;
      break;
    case FormatType::kR8G8_SSCALED:
      ret = VK_FORMAT_R8G8_SSCALED;
      break;
    case FormatType::kR8G8_UINT:
      ret = VK_FORMAT_R8G8_UINT;
      break;
    case FormatType::kR8G8_UNORM:
      ret = VK_FORMAT_R8G8_UNORM;
      break;
    case FormatType::kR8G8_USCALED:
      ret = VK_FORMAT_R8G8_USCALED;
      break;
    case FormatType::kR8_SINT:
      ret = VK_FORMAT_R8_SINT;
      break;
    case FormatType::kR8_SNORM:
      ret = VK_FORMAT_R8_SNORM;
      break;
    case FormatType::kR8_SRGB:
      ret = VK_FORMAT_R8_SRGB;
      break;
    case FormatType::kR8_SSCALED:
      ret = VK_FORMAT_R8_SSCALED;
      break;
    case FormatType::kR8_UINT:
      ret = VK_FORMAT_R8_UINT;
      break;
    case FormatType::kR8_UNORM:
      ret = VK_FORMAT_R8_UNORM;
      break;
    case FormatType::kR8_USCALED:
      ret = VK_FORMAT_R8_USCALED;
      break;
    case FormatType::kS8_UINT:
      ret = VK_FORMAT_S8_UINT;
      break;
    case FormatType::kX8_D24_UNORM_PACK32:
      ret = VK_FORMAT_X8_D24_UNORM_PACK32;
      break;
  }
  return ret;
}

}  // namespace vulkan
}  // namespace amber
