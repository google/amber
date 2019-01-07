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

#include "amber/amber_vulkan.h"

#include <cassert>
#include <set>

#include "src/feature.h"
#include "src/script.h"

namespace amber {
namespace {

void AddRequiredFeatures(
    VkPhysicalDeviceFeatures* required_features,
    const std::vector<Feature>& required_features_from_script) {
  for (const auto& feature : required_features_from_script) {
    switch (feature) {
      case Feature::kRobustBufferAccess:
        required_features->robustBufferAccess = VK_TRUE;
        break;
      case Feature::kFullDrawIndexUint32:
        required_features->fullDrawIndexUint32 = VK_TRUE;
        break;
      case Feature::kImageCubeArray:
        required_features->imageCubeArray = VK_TRUE;
        break;
      case Feature::kIndependentBlend:
        required_features->independentBlend = VK_TRUE;
        break;
      case Feature::kGeometryShader:
        required_features->geometryShader = VK_TRUE;
        break;
      case Feature::kTessellationShader:
        required_features->tessellationShader = VK_TRUE;
        break;
      case Feature::kSampleRateShading:
        required_features->sampleRateShading = VK_TRUE;
        break;
      case Feature::kDualSrcBlend:
        required_features->dualSrcBlend = VK_TRUE;
        break;
      case Feature::kLogicOp:
        required_features->logicOp = VK_TRUE;
        break;
      case Feature::kMultiDrawIndirect:
        required_features->multiDrawIndirect = VK_TRUE;
        break;
      case Feature::kDrawIndirectFirstInstance:
        required_features->drawIndirectFirstInstance = VK_TRUE;
        break;
      case Feature::kDepthClamp:
        required_features->depthClamp = VK_TRUE;
        break;
      case Feature::kDepthBiasClamp:
        required_features->depthBiasClamp = VK_TRUE;
        break;
      case Feature::kFillModeNonSolid:
        required_features->fillModeNonSolid = VK_TRUE;
        break;
      case Feature::kDepthBounds:
        required_features->depthBounds = VK_TRUE;
        break;
      case Feature::kWideLines:
        required_features->wideLines = VK_TRUE;
        break;
      case Feature::kLargePoints:
        required_features->largePoints = VK_TRUE;
        break;
      case Feature::kAlphaToOne:
        required_features->alphaToOne = VK_TRUE;
        break;
      case Feature::kMultiViewport:
        required_features->multiViewport = VK_TRUE;
        break;
      case Feature::kSamplerAnisotropy:
        required_features->samplerAnisotropy = VK_TRUE;
        break;
      case Feature::kTextureCompressionETC2:
        required_features->textureCompressionETC2 = VK_TRUE;
        break;
      case Feature::kTextureCompressionASTC_LDR:
        required_features->textureCompressionASTC_LDR = VK_TRUE;
        break;
      case Feature::kTextureCompressionBC:
        required_features->textureCompressionBC = VK_TRUE;
        break;
      case Feature::kOcclusionQueryPrecise:
        required_features->occlusionQueryPrecise = VK_TRUE;
        break;
      case Feature::kPipelineStatisticsQuery:
        required_features->pipelineStatisticsQuery = VK_TRUE;
        break;
      case Feature::kVertexPipelineStoresAndAtomics:
        required_features->vertexPipelineStoresAndAtomics = VK_TRUE;
        break;
      case Feature::kFragmentStoresAndAtomics:
        required_features->fragmentStoresAndAtomics = VK_TRUE;
        break;
      case Feature::kShaderTessellationAndGeometryPointSize:
        required_features->shaderTessellationAndGeometryPointSize = VK_TRUE;
        break;
      case Feature::kShaderImageGatherExtended:
        required_features->shaderImageGatherExtended = VK_TRUE;
        break;
      case Feature::kShaderStorageImageExtendedFormats:
        required_features->shaderStorageImageExtendedFormats = VK_TRUE;
        break;
      case Feature::kShaderStorageImageMultisample:
        required_features->shaderStorageImageMultisample = VK_TRUE;
        break;
      case Feature::kShaderStorageImageReadWithoutFormat:
        required_features->shaderStorageImageReadWithoutFormat = VK_TRUE;
        break;
      case Feature::kShaderStorageImageWriteWithoutFormat:
        required_features->shaderStorageImageWriteWithoutFormat = VK_TRUE;
        break;
      case Feature::kShaderUniformBufferArrayDynamicIndexing:
        required_features->shaderUniformBufferArrayDynamicIndexing = VK_TRUE;
        break;
      case Feature::kShaderSampledImageArrayDynamicIndexing:
        required_features->shaderSampledImageArrayDynamicIndexing = VK_TRUE;
        break;
      case Feature::kShaderStorageBufferArrayDynamicIndexing:
        required_features->shaderStorageBufferArrayDynamicIndexing = VK_TRUE;
        break;
      case Feature::kShaderStorageImageArrayDynamicIndexing:
        required_features->shaderStorageImageArrayDynamicIndexing = VK_TRUE;
        break;
      case Feature::kShaderClipDistance:
        required_features->shaderClipDistance = VK_TRUE;
        break;
      case Feature::kShaderCullDistance:
        required_features->shaderCullDistance = VK_TRUE;
        break;
      case Feature::kShaderFloat64:
        required_features->shaderFloat64 = VK_TRUE;
        break;
      case Feature::kShaderInt64:
        required_features->shaderInt64 = VK_TRUE;
        break;
      case Feature::kShaderInt16:
        required_features->shaderInt16 = VK_TRUE;
        break;
      case Feature::kShaderResourceResidency:
        required_features->shaderResourceResidency = VK_TRUE;
        break;
      case Feature::kShaderResourceMinLod:
        required_features->shaderResourceMinLod = VK_TRUE;
        break;
      case Feature::kSparseBinding:
        required_features->sparseBinding = VK_TRUE;
        break;
      case Feature::kSparseResidencyBuffer:
        required_features->sparseResidencyBuffer = VK_TRUE;
        break;
      case Feature::kSparseResidencyImage2D:
        required_features->sparseResidencyImage2D = VK_TRUE;
        break;
      case Feature::kSparseResidencyImage3D:
        required_features->sparseResidencyImage3D = VK_TRUE;
        break;
      case Feature::kSparseResidency2Samples:
        required_features->sparseResidency2Samples = VK_TRUE;
        break;
      case Feature::kSparseResidency4Samples:
        required_features->sparseResidency4Samples = VK_TRUE;
        break;
      case Feature::kSparseResidency8Samples:
        required_features->sparseResidency8Samples = VK_TRUE;
        break;
      case Feature::kSparseResidency16Samples:
        required_features->sparseResidency16Samples = VK_TRUE;
        break;
      case Feature::kSparseResidencyAliased:
        required_features->sparseResidencyAliased = VK_TRUE;
        break;
      case Feature::kVariableMultisampleRate:
        required_features->variableMultisampleRate = VK_TRUE;
        break;
      case Feature::kInheritedQueries:
        required_features->inheritedQueries = VK_TRUE;
        break;
      case Feature::kFramebuffer:
      case Feature::kDepthStencil:
      case Feature::kFenceTimeout:
      case Feature::kUnknown:
        break;
    }
  }
}

}  // namespace

VkPhysicalDeviceFeatures GetRequiredVulkanFeatures(
    const std::vector<const Recipe*>& recipes) {
  VkPhysicalDeviceFeatures required_features = {};
  for (auto recipe : recipes) {
    Script* script = static_cast<Script*>(recipe->GetImpl());
    if (!script)
      assert(false && "Recipe must contain a parsed script");

    AddRequiredFeatures(&required_features, script->RequiredFeatures());
  }
  return required_features;
}

std::vector<std::string> GetRequiredVulkanExtensions(
    const std::vector<const Recipe*>& recipes) {
  std::set<std::string> required_extensions;
  for (auto recipe : recipes) {
    Script* script = static_cast<Script*>(recipe->GetImpl());
    if (!script)
      assert(false && "Recipe must contain a parsed script");

    const auto& required_extensions_from_script = script->RequiredExtensions();
    required_extensions.insert(required_extensions_from_script.begin(),
                               required_extensions_from_script.end());
  }
  return std::vector<std::string>(required_extensions.begin(),
                                  required_extensions.end());
}

}  // namespace amber
