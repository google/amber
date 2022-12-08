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

#include <algorithm>
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

const char kSubgroupSupportedOperations[] = "SubgroupSupportedOperations";
const char kSubgroupSupportedOperationsBasic[] =
    "SubgroupSupportedOperations.basic";
const char kSubgroupSupportedOperationsVote[] =
    "SubgroupSupportedOperations.vote";
const char kSubgroupSupportedOperationsArithmetic[] =
    "SubgroupSupportedOperations.arithmetic";
const char kSubgroupSupportedOperationsBallot[] =
    "SubgroupSupportedOperations.ballot";
const char kSubgroupSupportedOperationsShuffle[] =
    "SubgroupSupportedOperations.shuffle";
const char kSubgroupSupportedOperationsShuffleRelative[] =
    "SubgroupSupportedOperations.shuffleRelative";
const char kSubgroupSupportedOperationsClustered[] =
    "SubgroupSupportedOperations.clustered";
const char kSubgroupSupportedOperationsQuad[] =
    "SubgroupSupportedOperations.quad";
const char kSubgroupSupportedStages[] = "SubgroupSupportedStages";
const char kSubgroupSupportedStagesVertex[] = "SubgroupSupportedStages.vertex";
const char kSubgroupSupportedStagesTessellationControl[] =
    "SubgroupSupportedStages.tessellationControl";
const char kSubgroupSupportedStagesTessellationEvaluation[] =
    "SubgroupSupportedStages.tessellationEvaluation";
const char kSubgroupSupportedStagesGeometry[] =
    "SubgroupSupportedStages.geometry";
const char kSubgroupSupportedStagesFragment[] =
    "SubgroupSupportedStages.fragment";
const char kSubgroupSupportedStagesCompute[] =
    "SubgroupSupportedStages.compute";

const char kShaderSubgroupExtendedTypes[] =
    "ShaderSubgroupExtendedTypesFeatures.shaderSubgroupExtendedTypes";

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

#include "vk-wrappers-1-0.inc"

  ptrs_.vkGetPhysicalDeviceProperties(physical_device_,
                                      &physical_device_properties_);

  if (SupportsApiVersion(1, 1, 0)) {
#include "vk-wrappers-1-1.inc"
  }

  return {};
}

bool Device::SupportsApiVersion(uint32_t major,
                                uint32_t minor,
                                uint32_t patch) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
  return physical_device_properties_.apiVersion >=
         VK_MAKE_VERSION(major, minor, patch);
#pragma clang diagnostic pop
}

Result Device::Initialize(
    PFN_vkGetInstanceProcAddr getInstanceProcAddr,
    Delegate* delegate,
    const std::vector<std::string>& required_features,
    const std::vector<std::string>& required_device_extensions,
    const VkPhysicalDeviceFeatures& available_features,
    const VkPhysicalDeviceFeatures2KHR& available_features2,
    const std::vector<std::string>& available_extensions) {
  Result r = LoadVulkanPointers(getInstanceProcAddr, delegate);
  if (!r.IsSuccess())
    return r;

  // Check for the core features. We don't know if available_features or
  // available_features2 is provided, so check both.
  if (!AreAllRequiredFeaturesSupported(available_features, required_features) &&
      !AreAllRequiredFeaturesSupported(available_features2.features,
                                       required_features)) {
    return Result(
        "Vulkan: Device::Initialize given physical device does not support "
        "required features");
  }

  // Search for additional features in case they are found in pNext field of
  // available_features2.
  VkPhysicalDeviceVariablePointerFeaturesKHR* var_ptrs = nullptr;
  VkPhysicalDeviceFloat16Int8FeaturesKHR* float16_ptrs = nullptr;
  VkPhysicalDevice8BitStorageFeaturesKHR* storage8_ptrs = nullptr;
  VkPhysicalDevice16BitStorageFeaturesKHR* storage16_ptrs = nullptr;
  VkPhysicalDeviceVulkan11Features* vulkan11_ptrs = nullptr;
  VkPhysicalDeviceVulkan12Features* vulkan12_ptrs = nullptr;
  VkPhysicalDeviceVulkan13Features* vulkan13_ptrs = nullptr;
  VkPhysicalDeviceSubgroupSizeControlFeaturesEXT*
      subgroup_size_control_features = nullptr;
  VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures*
      shader_subgroup_extended_types_ptrs = nullptr;
  void* ptr = available_features2.pNext;
  while (ptr != nullptr) {
    BaseOutStructure* s = static_cast<BaseOutStructure*>(ptr);
    switch (s->sType) {
      case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VARIABLE_POINTER_FEATURES_KHR:
        var_ptrs =
            static_cast<VkPhysicalDeviceVariablePointerFeaturesKHR*>(ptr);
        break;
      case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FLOAT16_INT8_FEATURES_KHR:
        float16_ptrs =
            static_cast<VkPhysicalDeviceFloat16Int8FeaturesKHR*>(ptr);
        break;
      case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES_KHR:
        storage8_ptrs =
            static_cast<VkPhysicalDevice8BitStorageFeaturesKHR*>(ptr);
        break;
      case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES_KHR:
        storage16_ptrs =
            static_cast<VkPhysicalDevice16BitStorageFeaturesKHR*>(ptr);
        break;
      case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_FEATURES_EXT:
        subgroup_size_control_features =
            static_cast<VkPhysicalDeviceSubgroupSizeControlFeaturesEXT*>(ptr);
        break;
      // NOLINTNEXTLINE(whitespace/line_length)
      case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_EXTENDED_TYPES_FEATURES:
        shader_subgroup_extended_types_ptrs =
            static_cast<VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures*>(
                ptr);
        break;
      case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES:
        vulkan11_ptrs = static_cast<VkPhysicalDeviceVulkan11Features*>(ptr);
        break;
      case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES:
        vulkan12_ptrs = static_cast<VkPhysicalDeviceVulkan12Features*>(ptr);
        break;
      case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES:
          vulkan13_ptrs = static_cast<VkPhysicalDeviceVulkan13Features*>(ptr);
          break;
      default:
        break;
    }
    ptr = s->pNext;
  }

  // Compare the available additional (non-core) features against the
  // requirements.
  //
  // Vulkan 1.2 added support for defining non-core physical device features
  // using VkPhysicalDeviceVulkan11Features and VkPhysicalDeviceVulkan12Features
  // structures. If |vulkan11_ptrs| and/or |vulkan12_ptrs| are null, we must
  // check for features using the old approach (by checking across various
  // feature structs); otherwise, we can check features via the new structs.
  for (const auto& feature : required_features) {
    // First check the feature structures are provided for the required
    // features.
    if ((feature == kVariablePointers ||
         feature == kVariablePointersStorageBuffer) &&
        var_ptrs == nullptr && vulkan11_ptrs == nullptr) {
      return amber::Result(
          "Variable pointers requested but feature not returned");
    }
    if ((feature == k16BitStorage_Storage ||
         feature == k16BitStorage_UniformAndStorage ||
         feature == k16BitStorage_PushConstant ||
         feature == k16BitStorage_InputOutput) &&
        storage16_ptrs == nullptr && vulkan11_ptrs == nullptr) {
      return amber::Result(
          "Shader 16-bit storage requested but feature not returned");
    }
    if ((feature == kFloat16Int8_Float16 || feature == kFloat16Int8_Int8) &&
        float16_ptrs == nullptr && vulkan12_ptrs == nullptr) {
      return amber::Result(
          "Shader float16/int8 requested but feature not returned");
    }
    if ((feature == k8BitStorage_UniformAndStorage ||
         feature == k8BitStorage_Storage ||
         feature == k8BitStorage_PushConstant) &&
        storage8_ptrs == nullptr && vulkan12_ptrs == nullptr) {
      return amber::Result(
          "Shader 8-bit storage requested but feature not returned");
    }
    if ((feature == kSubgroupSizeControl || feature == kComputeFullSubgroups) &&
        subgroup_size_control_features == nullptr && vulkan13_ptrs == nullptr) {
      return amber::Result("Missing subgroup size control features");
    }
    if (feature == kShaderSubgroupExtendedTypes &&
        shader_subgroup_extended_types_ptrs == nullptr &&
        vulkan12_ptrs == nullptr) {
      return amber::Result(
          "Subgroup extended types requested but feature not returned");
    }

    // Next check the fields of the feature structures.

    // If Vulkan 1.1 structure exists the features are set there.
    if (vulkan11_ptrs) {
      if (feature == kVariablePointers &&
          vulkan11_ptrs->variablePointers != VK_TRUE) {
        return amber::Result("Missing variable pointers feature");
      }
      if (feature == kVariablePointersStorageBuffer &&
          vulkan11_ptrs->variablePointersStorageBuffer != VK_TRUE) {
        return amber::Result(
            "Missing variable pointers storage buffer feature");
      }
      if (feature == k16BitStorage_Storage &&
          vulkan11_ptrs->storageBuffer16BitAccess != VK_TRUE) {
        return amber::Result("Missing 16-bit storage access");
      }
      if (feature == k16BitStorage_UniformAndStorage &&
          vulkan11_ptrs->uniformAndStorageBuffer16BitAccess != VK_TRUE) {
        return amber::Result("Missing 16-bit uniform and storage access");
      }
      if (feature == k16BitStorage_PushConstant &&
          vulkan11_ptrs->storagePushConstant16 != VK_TRUE) {
        return amber::Result("Missing 16-bit push constant access");
      }
      if (feature == k16BitStorage_InputOutput &&
          vulkan11_ptrs->storageInputOutput16 != VK_TRUE) {
        return amber::Result("Missing 16-bit input/output access");
      }
    } else {
      // Vulkan 1.1 structure was not found. Use separate structures per each
      // feature.
      if (feature == kVariablePointers &&
          var_ptrs->variablePointers != VK_TRUE) {
        return amber::Result("Missing variable pointers feature");
      }
      if (feature == kVariablePointersStorageBuffer &&
          var_ptrs->variablePointersStorageBuffer != VK_TRUE) {
        return amber::Result(
            "Missing variable pointers storage buffer feature");
      }
      if (feature == k16BitStorage_Storage &&
          storage16_ptrs->storageBuffer16BitAccess != VK_TRUE) {
        return amber::Result("Missing 16-bit storage access");
      }
      if (feature == k16BitStorage_UniformAndStorage &&
          storage16_ptrs->uniformAndStorageBuffer16BitAccess != VK_TRUE) {
        return amber::Result("Missing 16-bit uniform and storage access");
      }
      if (feature == k16BitStorage_PushConstant &&
          storage16_ptrs->storagePushConstant16 != VK_TRUE) {
        return amber::Result("Missing 16-bit push constant access");
      }
      if (feature == k16BitStorage_InputOutput &&
          storage16_ptrs->storageInputOutput16 != VK_TRUE) {
        return amber::Result("Missing 16-bit input/output access");
      }
    }

    // If Vulkan 1.2 structure exists the features are set there.
    if (vulkan12_ptrs) {
      if (feature == kFloat16Int8_Float16 &&
          vulkan12_ptrs->shaderFloat16 != VK_TRUE) {
        return amber::Result("Missing float16 feature");
      }
      if (feature == kFloat16Int8_Int8 &&
          vulkan12_ptrs->shaderInt8 != VK_TRUE) {
        return amber::Result("Missing int8 feature");
      }
      if (feature == k8BitStorage_Storage &&
          vulkan12_ptrs->storageBuffer8BitAccess != VK_TRUE) {
        return amber::Result("Missing 8-bit storage access");
      }
      if (feature == k8BitStorage_UniformAndStorage &&
          vulkan12_ptrs->uniformAndStorageBuffer8BitAccess != VK_TRUE) {
        return amber::Result("Missing 8-bit uniform and storage access");
      }
      if (feature == k8BitStorage_PushConstant &&
          vulkan12_ptrs->storagePushConstant8 != VK_TRUE) {
        return amber::Result("Missing 8-bit push constant access");
      }
      if (feature == kShaderSubgroupExtendedTypes &&
          vulkan12_ptrs->shaderSubgroupExtendedTypes != VK_TRUE) {
        return amber::Result("Missing subgroup extended types");
      }
    } else {
      // Vulkan 1.2 structure was not found. Use separate structures per each
      // feature.
      if (feature == kFloat16Int8_Float16 &&
          float16_ptrs->shaderFloat16 != VK_TRUE) {
        return amber::Result("Missing float16 feature");
      }
      if (feature == kFloat16Int8_Int8 && float16_ptrs->shaderInt8 != VK_TRUE) {
        return amber::Result("Missing int8 feature");
      }
      if (feature == k8BitStorage_Storage &&
          storage8_ptrs->storageBuffer8BitAccess != VK_TRUE) {
        return amber::Result("Missing 8-bit storage access");
      }
      if (feature == k8BitStorage_UniformAndStorage &&
          storage8_ptrs->uniformAndStorageBuffer8BitAccess != VK_TRUE) {
        return amber::Result("Missing 8-bit uniform and storage access");
      }
      if (feature == k8BitStorage_PushConstant &&
          storage8_ptrs->storagePushConstant8 != VK_TRUE) {
        return amber::Result("Missing 8-bit push constant access");
      }
      if (feature == kShaderSubgroupExtendedTypes &&
          shader_subgroup_extended_types_ptrs->shaderSubgroupExtendedTypes !=
              VK_TRUE) {
        return amber::Result("Missing subgroup extended types");
      }
    }

    // If Vulkan 1.3 structure exists the features are set there.
    if (vulkan13_ptrs) {
        if (feature == kSubgroupSizeControl &&
            vulkan13_ptrs->subgroupSizeControl != VK_TRUE) {
          return amber::Result("Missing subgroup size control feature");
        }
        if (feature == kComputeFullSubgroups &&
            vulkan13_ptrs->computeFullSubgroups != VK_TRUE) {
          return amber::Result("Missing compute full subgroups feature");
        }
    } else {
      if (feature == kSubgroupSizeControl &&
          subgroup_size_control_features->subgroupSizeControl != VK_TRUE) {
        return amber::Result("Missing subgroup size control feature");
      }
      if (feature == kComputeFullSubgroups &&
          subgroup_size_control_features->computeFullSubgroups != VK_TRUE) {
        return amber::Result("Missing compute full subgroups feature");
      }
    }
  }

  if (!AreAllExtensionsSupported(available_extensions,
                                 required_device_extensions)) {
    return Result(
        "Vulkan: Device::Initialize given physical device does not support "
        "required extensions");
  }

  ptrs_.vkGetPhysicalDeviceMemoryProperties(physical_device_,
                                            &physical_memory_properties_);

  subgroup_size_control_properties_ = {};
  const bool needs_subgroup_size_control =
      std::find(required_features.begin(), required_features.end(),
                kSubgroupSizeControl) != required_features.end();

  bool needs_subgroup_supported_operations = false;
  bool needs_subgroup_supported_stages = false;

  // Search for subgroup supported operations requirements.
  for (const auto& feature : required_features)
    if (feature.find(kSubgroupSupportedOperations) != std::string::npos)
      needs_subgroup_supported_operations = true;

  // Search for subgroup supported stages requirements.
  for (const auto& feature : required_features)
    if (feature.find(kSubgroupSupportedStages) != std::string::npos)
      needs_subgroup_supported_stages = true;

  const bool needs_subgroup_properties =
      needs_subgroup_supported_operations || needs_subgroup_supported_stages;

  if (needs_subgroup_size_control || needs_subgroup_properties) {
    // Always chain all physical device properties structs in case at least one
    // of them is needed.
    VkPhysicalDeviceProperties2 properties2 = {};
    VkPhysicalDeviceSubgroupProperties subgroup_properties = {};
    VkPhysicalDeviceVulkan11Properties vulkan11_properties = {};
    VkSubgroupFeatureFlags subgroup_supported_operations;
    VkShaderStageFlags subgroup_supported_stages;
    properties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    properties2.pNext = &subgroup_size_control_properties_;
    subgroup_size_control_properties_.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_PROPERTIES_EXT;
    if (SupportsApiVersion(1, 2, 0)) {
      subgroup_size_control_properties_.pNext = &vulkan11_properties;
      vulkan11_properties.sType =
          VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES;
    } else {
      subgroup_size_control_properties_.pNext = &subgroup_properties;
      subgroup_properties.sType =
          VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_PROPERTIES;
    }

    if (needs_subgroup_size_control && !SupportsApiVersion(1, 1, 0)) {
      return Result(
          "Vulkan: Device::Initialize subgroup size control feature also "
          "requires an API version of 1.1 or higher");
    }
    if (needs_subgroup_properties && !SupportsApiVersion(1, 1, 0)) {
      return Result(
          "Vulkan: Device::Initialize subgroup properties also "
          "requires an API version of 1.1 or higher");
    }
    ptrs_.vkGetPhysicalDeviceProperties2(physical_device_, &properties2);

    if (needs_subgroup_supported_operations) {
      // Read supported subgroup operations from the correct struct depending on
      // the device API
      if (SupportsApiVersion(1, 2, 0)) {
        subgroup_supported_operations =
            vulkan11_properties.subgroupSupportedOperations;
      } else {
        subgroup_supported_operations = subgroup_properties.supportedOperations;
      }

      for (const auto& feature : required_features) {
        if (feature == kSubgroupSupportedOperationsBasic &&
            !(subgroup_supported_operations & VK_SUBGROUP_FEATURE_BASIC_BIT)) {
          return amber::Result("Missing subgroup operation basic feature");
        }
        if (feature == kSubgroupSupportedOperationsVote &&
            !(subgroup_supported_operations & VK_SUBGROUP_FEATURE_VOTE_BIT)) {
          return amber::Result("Missing subgroup operation vote feature");
        }
        if (feature == kSubgroupSupportedOperationsArithmetic &&
            !(subgroup_supported_operations &
              VK_SUBGROUP_FEATURE_ARITHMETIC_BIT)) {
          return amber::Result("Missing subgroup operation arithmetic feature");
        }
        if (feature == kSubgroupSupportedOperationsBallot &&
            !(subgroup_supported_operations & VK_SUBGROUP_FEATURE_BALLOT_BIT)) {
          return amber::Result("Missing subgroup operation ballot feature");
        }
        if (feature == kSubgroupSupportedOperationsShuffle &&
            !(subgroup_supported_operations &
              VK_SUBGROUP_FEATURE_SHUFFLE_BIT)) {
          return amber::Result("Missing subgroup operation shuffle feature");
        }
        if (feature == kSubgroupSupportedOperationsShuffleRelative &&
            !(subgroup_supported_operations &
              VK_SUBGROUP_FEATURE_SHUFFLE_RELATIVE_BIT)) {
          return amber::Result(
              "Missing subgroup operation shuffle relative feature");
        }
        if (feature == kSubgroupSupportedOperationsClustered &&
            !(subgroup_supported_operations &
              VK_SUBGROUP_FEATURE_CLUSTERED_BIT)) {
          return amber::Result("Missing subgroup operation clustered feature");
        }
        if (feature == kSubgroupSupportedOperationsQuad &&
            !(subgroup_supported_operations & VK_SUBGROUP_FEATURE_QUAD_BIT)) {
          return amber::Result("Missing subgroup operation quad feature");
        }
      }
    }

    if (needs_subgroup_supported_stages) {
      // Read supported subgroup stages from the correct struct depending on the
      // device API
      if (SupportsApiVersion(1, 2, 0)) {
        subgroup_supported_stages = vulkan11_properties.subgroupSupportedStages;
      } else {
        subgroup_supported_stages = subgroup_properties.supportedStages;
      }

      for (const auto& feature : required_features) {
        if (feature == kSubgroupSupportedStagesVertex &&
            !(subgroup_supported_stages & VK_SHADER_STAGE_VERTEX_BIT)) {
          return amber::Result(
              "Subgroup operations not supported for vertex shader stage");
        }
        if (feature == kSubgroupSupportedStagesTessellationControl &&
            !(subgroup_supported_stages &
              VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT)) {
          return amber::Result(
              "Subgroup operations not supported for tessellation control "
              "shader stage");
        }
        if (feature == kSubgroupSupportedStagesTessellationEvaluation &&
            !(subgroup_supported_stages &
              VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT)) {
          return amber::Result(
              "Subgroup operations not supported for tessellation evaluation "
              "shader stage");
        }
        if (feature == kSubgroupSupportedStagesGeometry &&
            !(subgroup_supported_stages & VK_SHADER_STAGE_GEOMETRY_BIT)) {
          return amber::Result(
              "Subgroup operations not supported for geometry shader stage");
        }
        if (feature == kSubgroupSupportedStagesFragment &&
            !(subgroup_supported_stages & VK_SHADER_STAGE_FRAGMENT_BIT)) {
          return amber::Result(
              "Subgroup operations not supported for fragment shader stage");
        }
        if (feature == kSubgroupSupportedStagesCompute &&
            !(subgroup_supported_stages & VK_SHADER_STAGE_COMPUTE_BIT)) {
          return amber::Result(
              "Subgroup operations not supported for compute shader stage");
        }
      }
    }
  }

  return {};
}

bool Device::IsFormatSupportedByPhysicalDevice(const Format& format,
                                               BufferType type) {
  VkFormat vk_format = GetVkFormat(format);
  VkFormatProperties properties = VkFormatProperties();
  GetPtrs()->vkGetPhysicalDeviceFormatProperties(physical_device_, vk_format,
                                                 &properties);

  VkFormatFeatureFlagBits flag = VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT;
  bool is_buffer_type_image = false;
  switch (type) {
    case BufferType::kColor:
    case BufferType::kResolve:
    case BufferType::kStorageImage:
      flag = VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT;
      is_buffer_type_image = true;
      break;
    case BufferType::kDepthStencil:
      flag = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
      is_buffer_type_image = true;
      break;
    case BufferType::kSampledImage:
    case BufferType::kCombinedImageSampler:
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

bool Device::IsRequiredSubgroupSizeSupported(
    const ShaderType type,
    const uint32_t required_subgroup_size) const {
  VkShaderStageFlagBits stage = {};
  switch (type) {
    case kShaderTypeGeometry:
      stage = VK_SHADER_STAGE_GEOMETRY_BIT;
      break;
    case kShaderTypeFragment:
      stage = VK_SHADER_STAGE_FRAGMENT_BIT;
      break;
    case kShaderTypeVertex:
      stage = VK_SHADER_STAGE_VERTEX_BIT;
      break;
    case kShaderTypeTessellationControl:
      stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
      break;
    case kShaderTypeTessellationEvaluation:
      stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
      break;
    case kShaderTypeCompute:
      stage = VK_SHADER_STAGE_COMPUTE_BIT;
      break;
    default:
      return false;
  }
  if ((stage & subgroup_size_control_properties_.requiredSubgroupSizeStages) ==
      0) {
    return false;
  }
  if (required_subgroup_size == 0 ||
      required_subgroup_size <
          subgroup_size_control_properties_.minSubgroupSize ||
      required_subgroup_size >
          subgroup_size_control_properties_.maxSubgroupSize) {
    return false;
  }

  return true;
}

uint32_t Device::GetMinSubgroupSize() const {
  return subgroup_size_control_properties_.minSubgroupSize;
}

uint32_t Device::GetMaxSubgroupSize() const {
  return subgroup_size_control_properties_.maxSubgroupSize;
}

}  // namespace vulkan
}  // namespace amber
