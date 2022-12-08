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

#include <vulkan/vulkan.h>

#include <algorithm>
#include <cstring>
#include <iostream>
#include <iterator>
#include <set>
#include <sstream>
#include <utility>

#include "samples/log.h"

namespace sample {
namespace {

const char* const kRequiredValidationLayers[] = {
#ifdef __ANDROID__
    // Note that the order of enabled layers is important. It is
    // based on Android NDK Vulkan document.
    "VK_LAYER_GOOGLE_threading",      "VK_LAYER_LUNARG_parameter_validation",
    "VK_LAYER_LUNARG_object_tracker", "VK_LAYER_LUNARG_core_validation",
    "VK_LAYER_GOOGLE_unique_objects",
#else   // __ANDROID__
    "VK_LAYER_KHRONOS_validation",
#endif  // __ANDROID__
};

const size_t kNumberOfRequiredValidationLayers =
    sizeof(kRequiredValidationLayers) / sizeof(const char*);

const char kVariablePointers[] = "VariablePointerFeatures.variablePointers";
const char kVariablePointersStorageBuffer[] =
    "VariablePointerFeatures.variablePointersStorageBuffer";
const char kFloat16Int8_Float16[] = "Float16Int8Features.shaderFloat16";
const char kFloat16Int8_Int8[] = "Float16Int8Features.shaderInt8";
const char k8BitStorage_Storage[] =
    "Storage8BitFeatures.storageBuffer8BitAccess";
const char k8BitStorage_UniformAndStorage[] =
    "Storage8BitFeatures.uniformAndStorageBuffer8BitAccess";
const char k8BitStorage_PushConstant[] =
    "Storage8BitFeatures.storagePushConstant8";
const char k16BitStorage_Storage[] =
    "Storage16BitFeatures.storageBuffer16BitAccess";
const char k16BitStorage_UniformAndStorage[] =
    "Storage16BitFeatures.uniformAndStorageBuffer16BitAccess";
const char k16BitStorage_PushConstant[] =
    "Storage16BitFeatures.storagePushConstant16";
const char k16BitStorage_InputOutput[] =
    "Storage16BitFeatures.storageInputOutput16";

const char kSubgroupSizeControl[] = "SubgroupSizeControl.subgroupSizeControl";
const char kComputeFullSubgroups[] = "SubgroupSizeControl.computeFullSubgroups";

const char kShaderSubgroupExtendedTypes[] =
    "ShaderSubgroupExtendedTypesFeatures.shaderSubgroupExtendedTypes";

const char kExtensionForValidationLayer[] = "VK_EXT_debug_report";

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flag,
                                             VkDebugReportObjectTypeEXT,
                                             uint64_t,
                                             size_t,
                                             int32_t,
                                             const char* layerPrefix,
                                             const char* msg,
                                             void*) {
  std::string flag_message;
  switch (flag) {
    case VK_DEBUG_REPORT_ERROR_BIT_EXT:
      flag_message = "[ERROR]";
      break;
    case VK_DEBUG_REPORT_WARNING_BIT_EXT:
      flag_message = "[WARNING]";
      break;
    default:
      flag_message = "[UNKNOWN]";
      break;
  }

  LogError(flag_message + " validation layer (" + layerPrefix + "):\n" + msg);
  return VK_FALSE;
}

// Convert required features given as a string array to
// VkPhysicalDeviceFeatures.
amber::Result NamesToVulkanFeatures(
    const std::vector<std::string>& required_feature_names,
    VkPhysicalDeviceFeatures* features) {
  for (const auto& name : required_feature_names) {
    if (name == "robustBufferAccess") {
      features->robustBufferAccess = VK_TRUE;
    } else if (name == "fullDrawIndexUint32") {
      features->fullDrawIndexUint32 = VK_TRUE;
    } else if (name == "imageCubeArray") {
      features->imageCubeArray = VK_TRUE;
    } else if (name == "independentBlend") {
      features->independentBlend = VK_TRUE;
    } else if (name == "geometryShader") {
      features->geometryShader = VK_TRUE;
    } else if (name == "tessellationShader") {
      features->tessellationShader = VK_TRUE;
    } else if (name == "sampleRateShading") {
      features->sampleRateShading = VK_TRUE;
    } else if (name == "dualSrcBlend") {
      features->dualSrcBlend = VK_TRUE;
    } else if (name == "logicOp") {
      features->logicOp = VK_TRUE;
    } else if (name == "multiDrawIndirect") {
      features->multiDrawIndirect = VK_TRUE;
    } else if (name == "drawIndirectFirstInstance") {
      features->drawIndirectFirstInstance = VK_TRUE;
    } else if (name == "depthClamp") {
      features->depthClamp = VK_TRUE;
    } else if (name == "depthBiasClamp") {
      features->depthBiasClamp = VK_TRUE;
    } else if (name == "fillModeNonSolid") {
      features->fillModeNonSolid = VK_TRUE;
    } else if (name == "depthBounds") {
      features->depthBounds = VK_TRUE;
    } else if (name == "wideLines") {
      features->wideLines = VK_TRUE;
    } else if (name == "largePoints") {
      features->largePoints = VK_TRUE;
    } else if (name == "alphaToOne") {
      features->alphaToOne = VK_TRUE;
    } else if (name == "multiViewport") {
      features->multiViewport = VK_TRUE;
    } else if (name == "samplerAnisotropy") {
      features->samplerAnisotropy = VK_TRUE;
    } else if (name == "textureCompressionETC2") {
      features->textureCompressionETC2 = VK_TRUE;
    } else if (name == "textureCompressionASTC_LDR") {
      features->textureCompressionASTC_LDR = VK_TRUE;
    } else if (name == "textureCompressionBC") {
      features->textureCompressionBC = VK_TRUE;
    } else if (name == "occlusionQueryPrecise") {
      features->occlusionQueryPrecise = VK_TRUE;
    } else if (name == "pipelineStatisticsQuery") {
      features->pipelineStatisticsQuery = VK_TRUE;
    } else if (name == "vertexPipelineStoresAndAtomics") {
      features->vertexPipelineStoresAndAtomics = VK_TRUE;
    } else if (name == "fragmentStoresAndAtomics") {
      features->fragmentStoresAndAtomics = VK_TRUE;
    } else if (name == "shaderTessellationAndGeometryPointSize") {
      features->shaderTessellationAndGeometryPointSize = VK_TRUE;
    } else if (name == "shaderImageGatherExtended") {
      features->shaderImageGatherExtended = VK_TRUE;
    } else if (name == "shaderStorageImageExtendedFormats") {
      features->shaderStorageImageExtendedFormats = VK_TRUE;
    } else if (name == "shaderStorageImageMultisample") {
      features->shaderStorageImageMultisample = VK_TRUE;
    } else if (name == "shaderStorageImageReadWithoutFormat") {
      features->shaderStorageImageReadWithoutFormat = VK_TRUE;
    } else if (name == "shaderStorageImageWriteWithoutFormat") {
      features->shaderStorageImageWriteWithoutFormat = VK_TRUE;
    } else if (name == "shaderUniformBufferArrayDynamicIndexing") {
      features->shaderUniformBufferArrayDynamicIndexing = VK_TRUE;
    } else if (name == "shaderSampledImageArrayDynamicIndexing") {
      features->shaderSampledImageArrayDynamicIndexing = VK_TRUE;
    } else if (name == "shaderStorageBufferArrayDynamicIndexing") {
      features->shaderStorageBufferArrayDynamicIndexing = VK_TRUE;
    } else if (name == "shaderStorageImageArrayDynamicIndexing") {
      features->shaderStorageImageArrayDynamicIndexing = VK_TRUE;
    } else if (name == "shaderClipDistance") {
      features->shaderClipDistance = VK_TRUE;
    } else if (name == "shaderCullDistance") {
      features->shaderCullDistance = VK_TRUE;
    } else if (name == "shaderFloat64") {
      features->shaderFloat64 = VK_TRUE;
    } else if (name == "shaderInt64") {
      features->shaderInt64 = VK_TRUE;
    } else if (name == "shaderInt16") {
      features->shaderInt16 = VK_TRUE;
    } else if (name == "shaderResourceResidency") {
      features->shaderResourceResidency = VK_TRUE;
    } else if (name == "shaderResourceMinLod") {
      features->shaderResourceMinLod = VK_TRUE;
    } else if (name == "sparseBinding") {
      features->sparseBinding = VK_TRUE;
    } else if (name == "sparseResidencyBuffer") {
      features->sparseResidencyBuffer = VK_TRUE;
    } else if (name == "sparseResidencyImage2D") {
      features->sparseResidencyImage2D = VK_TRUE;
    } else if (name == "sparseResidencyImage3D") {
      features->sparseResidencyImage3D = VK_TRUE;
    } else if (name == "sparseResidency2Samples") {
      features->sparseResidency2Samples = VK_TRUE;
    } else if (name == "sparseResidency4Samples") {
      features->sparseResidency4Samples = VK_TRUE;
    } else if (name == "sparseResidency8Samples") {
      features->sparseResidency8Samples = VK_TRUE;
    } else if (name == "sparseResidency16Samples") {
      features->sparseResidency16Samples = VK_TRUE;
    } else if (name == "sparseResidencyAliased") {
      features->sparseResidencyAliased = VK_TRUE;
    } else if (name == "variableMultisampleRate") {
      features->variableMultisampleRate = VK_TRUE;
    } else if (name == "inheritedQueries") {
      features->inheritedQueries = VK_TRUE;
    } else {
      return amber::Result("Sample: Unknown Vulkan feature: " + name);
    }
  }
  return {};
}

bool AreAllValidationLayersSupported() {
  std::vector<VkLayerProperties> available_layer_properties;
  uint32_t available_layer_count = 0;
  if (vkEnumerateInstanceLayerProperties(&available_layer_count, nullptr) !=
      VK_SUCCESS) {
    return false;
  }
  available_layer_properties.resize(available_layer_count);
  if (vkEnumerateInstanceLayerProperties(&available_layer_count,
                                         available_layer_properties.data()) !=
      VK_SUCCESS) {
    return false;
  }

  std::set<std::string> required_layer_set(
      kRequiredValidationLayers,
      &kRequiredValidationLayers[kNumberOfRequiredValidationLayers]);
  for (const auto& property : available_layer_properties) {
    required_layer_set.erase(property.layerName);
  }

  if (required_layer_set.empty())
    return true;

  std::string missing_layers;
  for (const auto& missing_layer : required_layer_set)
    missing_layers = missing_layers + missing_layer + ",\n\t\t";
  LogError("Vulkan: missing validation layers:\n\t\t" + missing_layers);
  return true;
}

bool AreAllValidationExtensionsSupported() {
  for (const auto& layer : kRequiredValidationLayers) {
    uint32_t available_extension_count = 0;
    std::vector<VkExtensionProperties> extension_properties;

    if (vkEnumerateInstanceExtensionProperties(
            layer, &available_extension_count, nullptr) != VK_SUCCESS) {
      return false;
    }
    extension_properties.resize(available_extension_count);
    if (vkEnumerateInstanceExtensionProperties(
            layer, &available_extension_count, extension_properties.data()) !=
        VK_SUCCESS) {
      return false;
    }

    for (const auto& ext : extension_properties) {
      if (!strcmp(kExtensionForValidationLayer, ext.extensionName))
        return true;
    }
  }

  return false;
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

// Get all available instance extensions.
std::vector<std::string> GetAvailableInstanceExtensions() {
  std::vector<std::string> available_extensions;
  uint32_t available_extension_count = 0;
  std::vector<VkExtensionProperties> available_extension_properties;

  if (vkEnumerateInstanceExtensionProperties(
          nullptr, &available_extension_count, nullptr) != VK_SUCCESS) {
    return available_extensions;
  }

  if (available_extension_count == 0)
    return available_extensions;

  available_extension_properties.resize(available_extension_count);
  if (vkEnumerateInstanceExtensionProperties(
          nullptr, &available_extension_count,
          available_extension_properties.data()) != VK_SUCCESS) {
    return available_extensions;
  }

  for (const auto& property : available_extension_properties)
    available_extensions.push_back(property.extensionName);

  return available_extensions;
}

// Get all available extensions of |physical_device|.
std::vector<std::string> GetAvailableDeviceExtensions(
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
  uint32_t count = 0;
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

std::string deviceTypeToName(VkPhysicalDeviceType type) {
  switch (type) {
    case VK_PHYSICAL_DEVICE_TYPE_OTHER:
      return "other";
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
      return "integrated gpu";
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
      return "discrete gpu";
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
      return "virtual gpu";
    case VK_PHYSICAL_DEVICE_TYPE_CPU:
      return "cpu";
    default:
      break;
  }
  return "unknown";
}

std::string stageFlagBitsToNames(const VkShaderStageFlags bits) {
  static const std::pair<VkShaderStageFlagBits, const char*> stages[] = {
      std::make_pair(VK_SHADER_STAGE_VERTEX_BIT, "vert"),
      std::make_pair(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, "tessc"),
      std::make_pair(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, "tesse"),
      std::make_pair(VK_SHADER_STAGE_GEOMETRY_BIT, "geom"),
      std::make_pair(VK_SHADER_STAGE_FRAGMENT_BIT, "frag"),
      std::make_pair(VK_SHADER_STAGE_COMPUTE_BIT, "comp")};
  std::ostringstream result;
  bool addSeparator = false;
  for (const auto& stage : stages) {
    if ((bits & stage.first) != 0) {
      if (addSeparator)
        result << ", ";
      result << stage.second;
      addSeparator = true;
    }
  }
  return result.str();
}

}  // namespace

ConfigHelperVulkan::ConfigHelperVulkan()
    : available_features_(VkPhysicalDeviceFeatures()),
      available_features2_(VkPhysicalDeviceFeatures2KHR()),
      variable_pointers_feature_(VkPhysicalDeviceVariablePointerFeaturesKHR()),
      float16_int8_feature_(VkPhysicalDeviceFloat16Int8FeaturesKHR()),
      storage_8bit_feature_(VkPhysicalDevice8BitStorageFeaturesKHR()),
      storage_16bit_feature_(VkPhysicalDevice16BitStorageFeaturesKHR()),
      subgroup_size_control_feature_(
          VkPhysicalDeviceSubgroupSizeControlFeaturesEXT()) {}

ConfigHelperVulkan::~ConfigHelperVulkan() {
  if (vulkan_device_)
    vkDestroyDevice(vulkan_device_, nullptr);

  if (vulkan_callback_) {
    auto vkDestroyDebugReportCallbackEXT =
        reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(
            vkGetInstanceProcAddr(vulkan_instance_,
                                  "vkDestroyDebugReportCallbackEXT"));
    if (vkDestroyDebugReportCallbackEXT) {
      vkDestroyDebugReportCallbackEXT(vulkan_instance_, vulkan_callback_,
                                      nullptr);
    }
  }

  if (vulkan_instance_)
    vkDestroyInstance(vulkan_instance_, nullptr);
}

amber::Result ConfigHelperVulkan::CreateVulkanInstance(
    uint32_t engine_major,
    uint32_t engine_minor,
    std::vector<std::string> required_extensions,
    bool disable_validation_layer) {
  VkApplicationInfo app_info = VkApplicationInfo();
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
  app_info.apiVersion = VK_MAKE_VERSION(engine_major, engine_minor, 0);
#pragma clang diagnostic pop

  VkInstanceCreateInfo instance_info = VkInstanceCreateInfo();
  instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instance_info.pApplicationInfo = &app_info;

  if (!disable_validation_layer) {
    if (!AreAllValidationLayersSupported())
      return amber::Result("Sample: not all validation layers are supported");
    if (!AreAllValidationExtensionsSupported()) {
      return amber::Result(
          "Sample: extensions of validation layers are not supported");
    }
    instance_info.enabledLayerCount = kNumberOfRequiredValidationLayers;
    instance_info.ppEnabledLayerNames = kRequiredValidationLayers;

    required_extensions.push_back(kExtensionForValidationLayer);
  }

  available_instance_extensions_ = GetAvailableInstanceExtensions();
  if (!required_extensions.empty()) {
    if (!AreAllExtensionsSupported(available_instance_extensions_,
                                   required_extensions)) {
      return amber::Result("Missing required instance extensions");
    }
  }

  if (std::find(available_instance_extensions_.begin(),
                available_instance_extensions_.end(),
                "VK_KHR_get_physical_device_properties2") !=
      available_instance_extensions_.end()) {
    required_extensions.push_back("VK_KHR_get_physical_device_properties2");
  }

  // Determine if VkPhysicalDeviceProperties2KHR should be used
  for (auto& ext : required_extensions) {
    if (ext == "VK_KHR_get_physical_device_properties2")
      supports_get_physical_device_properties2_ = true;
  }

  std::vector<const char*> required_extensions_in_char;
  std::transform(
      required_extensions.begin(), required_extensions.end(),
      std::back_inserter(required_extensions_in_char),
      [](const std::string& ext) -> const char* { return ext.c_str(); });

  instance_info.enabledExtensionCount =
      static_cast<uint32_t>(required_extensions_in_char.size());
  instance_info.ppEnabledExtensionNames = required_extensions_in_char.data();

  if (vkCreateInstance(&instance_info, nullptr, &vulkan_instance_) !=
      VK_SUCCESS) {
    return amber::Result("Unable to create vulkan instance");
  }
  return {};
}

amber::Result ConfigHelperVulkan::CreateDebugReportCallback() {
  VkDebugReportCallbackCreateInfoEXT info =
      VkDebugReportCallbackCreateInfoEXT();
  info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
  info.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
  info.pfnCallback = debugCallback;

  auto vkCreateDebugReportCallbackEXT =
      reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(
          vkGetInstanceProcAddr(vulkan_instance_,
                                "vkCreateDebugReportCallbackEXT"));
  if (!vkCreateDebugReportCallbackEXT)
    return amber::Result("Sample: vkCreateDebugReportCallbackEXT is nullptr");

  if (vkCreateDebugReportCallbackEXT(vulkan_instance_, &info, nullptr,
                                     &vulkan_callback_) != VK_SUCCESS) {
    return amber::Result("Sample: vkCreateDebugReportCallbackEXT fail");
  }
  return {};
}

amber::Result ConfigHelperVulkan::CheckVulkanPhysicalDeviceRequirements(
    const VkPhysicalDevice physical_device,
    const std::vector<std::string>& required_features,
    const std::vector<std::string>& required_extensions) {
  available_device_extensions_ = GetAvailableDeviceExtensions(physical_device);
  if (!AreAllExtensionsSupported(available_device_extensions_,
                                 required_extensions)) {
    return amber::Result("Device does not support all required extensions");
  }
  for (const auto& ext : available_device_extensions_) {
    if (ext == "VK_KHR_shader_float16_int8")
      supports_shader_float16_int8_ = true;
    else if (ext == "VK_KHR_8bit_storage")
      supports_shader_8bit_storage_ = true;
    else if (ext == "VK_KHR_16bit_storage")
      supports_shader_16bit_storage_ = true;
    else if (ext == "VK_EXT_subgroup_size_control")
      supports_subgroup_size_control_ = true;
    else if (ext == "VK_KHR_shader_subgroup_extended_types")
      supports_shader_subgroup_extended_types_ = true;
  }

  VkPhysicalDeviceFeatures required_vulkan_features =
      VkPhysicalDeviceFeatures();

  if (supports_get_physical_device_properties2_) {
    VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures
        shader_subgroup_extended_types_features = {};
    VkPhysicalDeviceSubgroupSizeControlFeaturesEXT
        subgroup_size_control_features = {};
    VkPhysicalDeviceVariablePointerFeaturesKHR variable_pointers_features = {};
    VkPhysicalDeviceFloat16Int8FeaturesKHR float16_int8_features = {};
    VkPhysicalDevice8BitStorageFeaturesKHR storage_8bit_features = {};
    VkPhysicalDevice16BitStorageFeaturesKHR storage_16bit_features = {};

    subgroup_size_control_features.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_FEATURES_EXT;
    subgroup_size_control_features.pNext = nullptr;

    // Add subgroup size control struct into the chain only if
    // VK_EXT_subgroup_size_control is supported.
    variable_pointers_features.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VARIABLE_POINTER_FEATURES_KHR;
    variable_pointers_features.pNext = supports_subgroup_size_control_
                                           ? &subgroup_size_control_features
                                           : nullptr;

    shader_subgroup_extended_types_features.sType =
    // NOLINTNEXTLINE(whitespace/line_length)
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_EXTENDED_TYPES_FEATURES;
    shader_subgroup_extended_types_features.pNext = &variable_pointers_features;

    float16_int8_features.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FLOAT16_INT8_FEATURES_KHR;
    float16_int8_features.pNext = &shader_subgroup_extended_types_features;

    storage_8bit_features.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES_KHR;
    storage_8bit_features.pNext = &float16_int8_features;

    storage_16bit_features.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES_KHR;
    storage_16bit_features.pNext = &storage_8bit_features;

    VkPhysicalDeviceFeatures2KHR features2 = VkPhysicalDeviceFeatures2KHR();
    features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2_KHR;
    features2.pNext = &storage_16bit_features;

    auto vkGetPhysicalDeviceFeatures2KHR =
        reinterpret_cast<PFN_vkGetPhysicalDeviceFeatures2KHR>(
            vkGetInstanceProcAddr(vulkan_instance_,
                                  "vkGetPhysicalDeviceFeatures2KHR"));
    vkGetPhysicalDeviceFeatures2KHR(physical_device, &features2);
    available_features_ = features2.features;

    std::vector<std::string> required_features1;
    for (const auto& feature : required_features) {
      // No dot means this is a features1 feature.
      if (feature.find_first_of('.') == std::string::npos) {
        required_features1.push_back(feature);
        continue;
      }

      if ((feature == kVariablePointers &&
           variable_pointers_features.variablePointers == VK_FALSE) ||
          (feature == kVariablePointersStorageBuffer &&
           variable_pointers_features.variablePointersStorageBuffer ==
               VK_FALSE) ||
          (feature == kSubgroupSizeControl &&
           subgroup_size_control_features.subgroupSizeControl == VK_FALSE) ||
          (feature == kComputeFullSubgroups &&
           subgroup_size_control_features.computeFullSubgroups == VK_FALSE) ||
          (feature == kFloat16Int8_Float16 &&
           float16_int8_features.shaderFloat16 == VK_FALSE) ||
          (feature == kFloat16Int8_Int8 &&
           float16_int8_features.shaderInt8 == VK_FALSE) ||
          (feature == k8BitStorage_Storage &&
           storage_8bit_features.storageBuffer8BitAccess == VK_FALSE) ||
          (feature == k8BitStorage_UniformAndStorage &&
           storage_8bit_features.uniformAndStorageBuffer8BitAccess ==
               VK_FALSE) ||
          (feature == k8BitStorage_PushConstant &&
           storage_8bit_features.storagePushConstant8 == VK_FALSE) ||
          (feature == k16BitStorage_Storage &&
           storage_16bit_features.storageBuffer16BitAccess == VK_FALSE) ||
          (feature == k16BitStorage_InputOutput &&
           storage_16bit_features.storageInputOutput16 == VK_FALSE) ||
          (feature == k16BitStorage_PushConstant &&
           storage_16bit_features.storagePushConstant16 == VK_FALSE) ||
          (feature == k16BitStorage_UniformAndStorage &&
           storage_16bit_features.uniformAndStorageBuffer16BitAccess ==
               VK_FALSE) ||
          (feature == kShaderSubgroupExtendedTypes &&
           shader_subgroup_extended_types_features
                   .shaderSubgroupExtendedTypes == VK_FALSE)) {
        return amber::Result("Device does not support all required features");
      }
    }

    amber::Result r =
        NamesToVulkanFeatures(required_features1, &required_vulkan_features);
    if (!r.IsSuccess())
      return r;

  } else {
    amber::Result r =
        NamesToVulkanFeatures(required_features, &required_vulkan_features);
    if (!r.IsSuccess())
      return r;

    vkGetPhysicalDeviceFeatures(physical_device, &available_features_);
  }
  if (!AreAllRequiredFeaturesSupported(available_features_,
                                       required_vulkan_features)) {
    return amber::Result("Device does not support all required features");
  }

  vulkan_queue_family_index_ = ChooseQueueFamilyIndex(physical_device);
  if (vulkan_queue_family_index_ == std::numeric_limits<uint32_t>::max()) {
    return amber::Result("Device does not support required queue flags");
  }

  return {};
}

amber::Result ConfigHelperVulkan::ChooseVulkanPhysicalDevice(
    const std::vector<std::string>& required_features,
    const std::vector<std::string>& required_extensions,
    const int32_t selected_device) {
  uint32_t count = 0;
  std::vector<VkPhysicalDevice> physical_devices;

  if (vkEnumeratePhysicalDevices(vulkan_instance_, &count, nullptr) !=
      VK_SUCCESS) {
    return amber::Result("Unable to enumerate physical devices");
  }

  physical_devices.resize(count);
  if (vkEnumeratePhysicalDevices(vulkan_instance_, &count,
                                 physical_devices.data()) != VK_SUCCESS) {
    return amber::Result("Unable to enumerate physical devices");
  }

  if (selected_device > -1) {
    uint32_t deviceID = static_cast<uint32_t>(selected_device);
    if (deviceID >= count) {
      return amber::Result("Unable to find Vulkan device with ID " +
                           std::to_string(deviceID));
    }
    amber::Result r = CheckVulkanPhysicalDeviceRequirements(
        physical_devices[deviceID], required_features, required_extensions);
    if (!r.IsSuccess())
      return r;
    vulkan_physical_device_ = physical_devices[deviceID];
    return {};
  } else {
    for (uint32_t i = 0; i < count; ++i) {
      amber::Result r = CheckVulkanPhysicalDeviceRequirements(
          physical_devices[i], required_features, required_extensions);
      if (!r.IsSuccess())
        continue;
      vulkan_physical_device_ = physical_devices[i];
      return {};
    }
  }

  std::ostringstream out;
  out << "Unable to find Vulkan device supporting:" << std::endl;
  for (const auto& str : required_features)
    out << "  " << str << std::endl;
  for (const auto& str : required_extensions)
    out << "  " << str << std::endl;

  return amber::Result(out.str());
}

amber::Result ConfigHelperVulkan::CreateVulkanDevice(
    const std::vector<std::string>& required_features,
    const std::vector<std::string>& required_extensions) {
  VkDeviceQueueCreateInfo queue_info = VkDeviceQueueCreateInfo();
  const float priorities[] = {1.0f};

  queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queue_info.queueFamilyIndex = vulkan_queue_family_index_;
  queue_info.queueCount = 1;
  queue_info.pQueuePriorities = priorities;

  std::vector<const char*> required_extensions_in_char;
  std::transform(
      required_extensions.begin(), required_extensions.end(),
      std::back_inserter(required_extensions_in_char),
      [](const std::string& ext) -> const char* { return ext.c_str(); });

  VkDeviceCreateInfo info = VkDeviceCreateInfo();
  info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  info.pQueueCreateInfos = &queue_info;
  info.queueCreateInfoCount = 1;
  info.enabledExtensionCount =
      static_cast<uint32_t>(required_extensions_in_char.size());
  info.ppEnabledExtensionNames = required_extensions_in_char.data();

  if (supports_get_physical_device_properties2_)
    return CreateDeviceWithFeatures2(required_features, &info);
  return CreateDeviceWithFeatures1(required_features, &info);
}

amber::Result ConfigHelperVulkan::CreateDeviceWithFeatures1(
    const std::vector<std::string>& required_features,
    VkDeviceCreateInfo* info) {
  VkPhysicalDeviceFeatures required_vulkan_features =
      VkPhysicalDeviceFeatures();
  amber::Result r =
      NamesToVulkanFeatures(required_features, &required_vulkan_features);
  if (!r.IsSuccess())
    return r;

  info->pEnabledFeatures = &required_vulkan_features;
  return DoCreateDevice(info);
}

amber::Result ConfigHelperVulkan::CreateDeviceWithFeatures2(
    const std::vector<std::string>& required_features,
    VkDeviceCreateInfo* info) {
  variable_pointers_feature_.sType =
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VARIABLE_POINTER_FEATURES_KHR;
  variable_pointers_feature_.pNext = nullptr;

  float16_int8_feature_.sType =
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FLOAT16_INT8_FEATURES_KHR;
  float16_int8_feature_.pNext = nullptr;

  storage_8bit_feature_.sType =
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES_KHR;
  storage_8bit_feature_.pNext = nullptr;

  storage_16bit_feature_.sType =
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES_KHR;
  storage_16bit_feature_.pNext = nullptr;

  subgroup_size_control_feature_.sType =
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_FEATURES_EXT;
  subgroup_size_control_feature_.pNext = nullptr;

  shader_subgroup_extended_types_feature_.sType =
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_EXTENDED_TYPES_FEATURES;
  shader_subgroup_extended_types_feature_.pNext = nullptr;

  void** next_ptr = &variable_pointers_feature_.pNext;

  if (supports_shader_float16_int8_) {
    *next_ptr = &float16_int8_feature_;
    next_ptr = &float16_int8_feature_.pNext;
  }

  if (supports_shader_8bit_storage_) {
    *next_ptr = &storage_8bit_feature_;
    next_ptr = &storage_8bit_feature_.pNext;
  }

  if (supports_shader_16bit_storage_) {
    *next_ptr = &storage_16bit_feature_;
    next_ptr = &storage_16bit_feature_.pNext;
  }

  if (supports_subgroup_size_control_) {
    *next_ptr = &subgroup_size_control_feature_;
    next_ptr = &subgroup_size_control_feature_.pNext;
  }

  if (supports_shader_subgroup_extended_types_) {
    *next_ptr = &shader_subgroup_extended_types_feature_;
    next_ptr = &shader_subgroup_extended_types_feature_.pNext;
  }

  available_features2_.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2_KHR;
  available_features2_.pNext = &variable_pointers_feature_;

  std::vector<std::string> feature1_names;
  for (const auto& feature : required_features) {
    // No dot means this is a features1 feature.
    if (feature.find_first_of('.') == std::string::npos) {
      feature1_names.push_back(feature);
      continue;
    }

    if (feature == kVariablePointers)
      variable_pointers_feature_.variablePointers = VK_TRUE;
    else if (feature == kVariablePointersStorageBuffer)
      variable_pointers_feature_.variablePointersStorageBuffer = VK_TRUE;
    else if (feature == kFloat16Int8_Float16)
      float16_int8_feature_.shaderFloat16 = VK_TRUE;
    else if (feature == kFloat16Int8_Int8)
      float16_int8_feature_.shaderInt8 = VK_TRUE;
    else if (feature == k8BitStorage_Storage)
      storage_8bit_feature_.storageBuffer8BitAccess = VK_TRUE;
    else if (feature == k8BitStorage_UniformAndStorage)
      storage_8bit_feature_.uniformAndStorageBuffer8BitAccess = VK_TRUE;
    else if (feature == k8BitStorage_PushConstant)
      storage_8bit_feature_.storagePushConstant8 = VK_TRUE;
    else if (feature == k16BitStorage_Storage)
      storage_16bit_feature_.storageBuffer16BitAccess = VK_TRUE;
    else if (feature == k16BitStorage_UniformAndStorage)
      storage_16bit_feature_.uniformAndStorageBuffer16BitAccess = VK_TRUE;
    else if (feature == k16BitStorage_PushConstant)
      storage_16bit_feature_.storagePushConstant16 = VK_TRUE;
    else if (feature == k16BitStorage_InputOutput)
      storage_16bit_feature_.storageInputOutput16 = VK_TRUE;
    else if (feature == kSubgroupSizeControl)
      subgroup_size_control_feature_.subgroupSizeControl = VK_TRUE;
    else if (feature == kComputeFullSubgroups)
      subgroup_size_control_feature_.computeFullSubgroups = VK_TRUE;
    else if (feature == kShaderSubgroupExtendedTypes)
      shader_subgroup_extended_types_feature_.shaderSubgroupExtendedTypes =
          VK_TRUE;
  }

  VkPhysicalDeviceFeatures required_vulkan_features =
      VkPhysicalDeviceFeatures();
  amber::Result r =
      NamesToVulkanFeatures(feature1_names, &required_vulkan_features);
  if (!r.IsSuccess())
    return r;

  available_features2_.features = required_vulkan_features;

  info->pNext = &available_features2_;
  info->pEnabledFeatures = nullptr;
  return DoCreateDevice(info);
}

amber::Result ConfigHelperVulkan::DoCreateDevice(VkDeviceCreateInfo* info) {
  if (vkCreateDevice(vulkan_physical_device_, info, nullptr, &vulkan_device_) !=
      VK_SUCCESS) {
    return amber::Result("Unable to create vulkan device");
  }
  return {};
}

void ConfigHelperVulkan::DumpPhysicalDeviceInfo() {
  VkPhysicalDeviceProperties2KHR properties2 = {
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2_KHR,
      nullptr,  // pNext: will point to our driver_properties struct if the
                // "VK_KHR_get_physical_device_properties2" and
                // "VK_KHR_driver_properties" extensions are both available.
      {},  // properties: this is the older VkPhysicalDeviceProperties struct,
           // wrapped by this newer struct that adds the pNext member. We use
           // this older struct if the "VK_KHR_get_physical_device_properties2"
           // extension is unavailable.
  };

  VkPhysicalDeviceDriverPropertiesKHR driver_properties = {
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES_KHR,
      nullptr,
      {},
      {},
      {},
      {},
  };

  VkPhysicalDeviceSubgroupSizeControlPropertiesEXT
      subgroup_size_control_properties = {
          VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_PROPERTIES_EXT,  // NOLINT(whitespace/line_length)
          nullptr,
          {},
          {},
          {},
          {}};

  // If the vkGetPhysicalDeviceProperties2KHR function is unavailable (because
  // the "VK_KHR_get_physical_device_properties2" extension is unavailable or
  // because vkGetInstanceProcAddr failed) or the "VK_KHR_driver_properties"
  // extension is unavailable, then this will stay as nullptr and we will
  // instead call the older vkGetPhysicalDeviceProperties function.
  PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR =
      nullptr;

  if (supports_get_physical_device_properties2_ &&
      std::find(available_device_extensions_.begin(),
                available_device_extensions_.end(),
                "VK_KHR_driver_properties") !=
          available_device_extensions_.end()) {
    properties2.pNext = &driver_properties;

    vkGetPhysicalDeviceProperties2KHR =
        reinterpret_cast<PFN_vkGetPhysicalDeviceProperties2KHR>(
            vkGetInstanceProcAddr(vulkan_instance_,
                                  "vkGetPhysicalDeviceProperties2KHR"));
    if (!vkGetPhysicalDeviceProperties2KHR) {
      std::cout << "Warning: device claimed to support "
                   "vkGetPhysicalDeviceProperties2KHR but could not find this "
                   "function."
                << std::endl;
    }
    if (supports_subgroup_size_control_) {
      driver_properties.pNext = &subgroup_size_control_properties;
    }
  }

  if (vkGetPhysicalDeviceProperties2KHR) {
    vkGetPhysicalDeviceProperties2KHR(vulkan_physical_device_, &properties2);
  } else {
    vkGetPhysicalDeviceProperties(vulkan_physical_device_,
                                  &properties2.properties);
  }

  const VkPhysicalDeviceProperties& props = properties2.properties;

  uint32_t api_version = props.apiVersion;

  std::cout << std::endl;
  std::cout << "Physical device properties:" << std::endl;
  std::cout << "  apiVersion: " << static_cast<uint32_t>(api_version >> 22u)
            << "." << static_cast<uint32_t>((api_version >> 12u) & 0x3ffu)
            << "." << static_cast<uint32_t>(api_version & 0xfffu) << std::endl;
  std::cout << "  driverVersion: " << props.driverVersion << std::endl;
  std::cout << "  vendorID: " << props.vendorID << std::endl;
  std::cout << "  deviceID: " << props.deviceID << std::endl;
  std::cout << "  deviceType: " << deviceTypeToName(props.deviceType)
            << std::endl;
  std::cout << "  deviceName: " << props.deviceName << std::endl;
  if (vkGetPhysicalDeviceProperties2KHR) {
    std::cout << "  driverName: " << driver_properties.driverName << std::endl;
    std::cout << "  driverInfo: " << driver_properties.driverInfo << std::endl;
    if (supports_subgroup_size_control_) {
      std::cout << "  minSubgroupSize: "
                << subgroup_size_control_properties.minSubgroupSize
                << std::endl;
      std::cout << "  maxSubgroupSize: "
                << subgroup_size_control_properties.maxSubgroupSize
                << std::endl;
      std::cout << "  maxComputeWorkgroupSubgroups: "
                << subgroup_size_control_properties.maxComputeWorkgroupSubgroups
                << std::endl;
      std::cout << "  requiredSubgroupSizeStages: "
                << stageFlagBitsToNames(subgroup_size_control_properties
                                            .requiredSubgroupSizeStages)
                << std::endl;
    }
  }
  std::cout << "End of physical device properties." << std::endl;
}

amber::Result ConfigHelperVulkan::CreateConfig(
    uint32_t engine_major,
    uint32_t engine_minor,
    int32_t selected_device,
    const std::vector<std::string>& required_features,
    const std::vector<std::string>& required_instance_extensions,
    const std::vector<std::string>& required_device_extensions,
    bool disable_validation_layer,
    bool show_version_info,
    std::unique_ptr<amber::EngineConfig>* cfg_holder) {
  amber::Result r = CreateVulkanInstance(engine_major, engine_minor,
                                         required_instance_extensions,
                                         disable_validation_layer);
  if (!r.IsSuccess())
    return r;

  if (!disable_validation_layer) {
    r = CreateDebugReportCallback();
    if (!r.IsSuccess())
      return r;
  }

  r = ChooseVulkanPhysicalDevice(required_features, required_device_extensions,
                                 selected_device);
  if (!r.IsSuccess())
    return r;

  if (show_version_info)
    DumpPhysicalDeviceInfo();

  r = CreateVulkanDevice(required_features, required_device_extensions);
  if (!r.IsSuccess())
    return r;

  vkGetDeviceQueue(vulkan_device_, vulkan_queue_family_index_, 0,
                   &vulkan_queue_);

  *cfg_holder =
      std::unique_ptr<amber::EngineConfig>(new amber::VulkanEngineConfig());
  amber::VulkanEngineConfig* config =
      static_cast<amber::VulkanEngineConfig*>(cfg_holder->get());
  config->physical_device = vulkan_physical_device_;
  config->available_features = available_features_;
  config->available_features2 = available_features2_;
  config->available_instance_extensions = available_instance_extensions_;
  config->available_device_extensions = available_device_extensions_;
  config->instance = vulkan_instance_;
  config->queue_family_index = vulkan_queue_family_index_;
  config->queue = vulkan_queue_;
  config->device = vulkan_device_;
  config->vkGetInstanceProcAddr = vkGetInstanceProcAddr;

  return {};
}

}  // namespace sample
