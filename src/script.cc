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

#include "src/script.h"

namespace amber {
namespace {

std::string FeatureToName(Feature feature) {
  if (feature == Feature::kRobustBufferAccess)
    return "robustBufferAccess";
  if (feature == Feature::kFullDrawIndexUint32)
    return "fullDrawIndexUint32";
  if (feature == Feature::kImageCubeArray)
    return "imageCubeArray";
  if (feature == Feature::kIndependentBlend)
    return "independentBlend";
  if (feature == Feature::kGeometryShader)
    return "geometryShader";
  if (feature == Feature::kTessellationShader)
    return "tessellationShader";
  if (feature == Feature::kSampleRateShading)
    return "sampleRateShading";
  if (feature == Feature::kDualSrcBlend)
    return "dualSrcBlend";
  if (feature == Feature::kLogicOp)
    return "logicOp";
  if (feature == Feature::kMultiDrawIndirect)
    return "multiDrawIndirect";
  if (feature == Feature::kDrawIndirectFirstInstance)
    return "drawIndirectFirstInstance";
  if (feature == Feature::kDepthClamp)
    return "depthClamp";
  if (feature == Feature::kDepthBiasClamp)
    return "depthBiasClamp";
  if (feature == Feature::kFillModeNonSolid)
    return "fillModeNonSolid";
  if (feature == Feature::kDepthBounds)
    return "depthBounds";
  if (feature == Feature::kWideLines)
    return "wideLines";
  if (feature == Feature::kLargePoints)
    return "largePoints";
  if (feature == Feature::kAlphaToOne)
    return "alphaToOne";
  if (feature == Feature::kMultiViewport)
    return "multiViewport";
  if (feature == Feature::kSamplerAnisotropy)
    return "samplerAnisotropy";
  if (feature == Feature::kTextureCompressionETC2)
    return "textureCompressionETC2";
  if (feature == Feature::kTextureCompressionASTC_LDR)
    return "textureCompressionASTC_LDR";
  if (feature == Feature::kTextureCompressionBC)
    return "textureCompressionBC";
  if (feature == Feature::kOcclusionQueryPrecise)
    return "occlusionQueryPrecise";
  if (feature == Feature::kPipelineStatisticsQuery)
    return "pipelineStatisticsQuery";
  if (feature == Feature::kVertexPipelineStoresAndAtomics)
    return "vertexPipelineStoresAndAtomics";
  if (feature == Feature::kFragmentStoresAndAtomics)
    return "fragmentStoresAndAtomics";
  if (feature == Feature::kShaderTessellationAndGeometryPointSize)
    return "shaderTessellationAndGeometryPointSize";
  if (feature == Feature::kShaderImageGatherExtended)
    return "shaderImageGatherExtended";
  if (feature == Feature::kShaderStorageImageExtendedFormats)
    return "shaderStorageImageExtendedFormats";
  if (feature == Feature::kShaderStorageImageMultisample)
    return "shaderStorageImageMultisample";
  if (feature == Feature::kShaderStorageImageReadWithoutFormat)
    return "shaderStorageImageReadWithoutFormat";
  if (feature == Feature::kShaderStorageImageWriteWithoutFormat)
    return "shaderStorageImageWriteWithoutFormat";
  if (feature == Feature::kShaderUniformBufferArrayDynamicIndexing)
    return "shaderUniformBufferArrayDynamicIndexing";
  if (feature == Feature::kShaderSampledImageArrayDynamicIndexing)
    return "shaderSampledImageArrayDynamicIndexing";
  if (feature == Feature::kShaderStorageBufferArrayDynamicIndexing)
    return "shaderStorageBufferArrayDynamicIndexing";
  if (feature == Feature::kShaderStorageImageArrayDynamicIndexing)
    return "shaderStorageImageArrayDynamicIndexing";
  if (feature == Feature::kShaderClipDistance)
    return "shaderClipDistance";
  if (feature == Feature::kShaderCullDistance)
    return "shaderCullDistance";
  if (feature == Feature::kShaderFloat64)
    return "shaderFloat64";
  if (feature == Feature::kShaderInt64)
    return "shaderInt64";
  if (feature == Feature::kShaderInt16)
    return "shaderInt16";
  if (feature == Feature::kShaderResourceResidency)
    return "shaderResourceResidency";
  if (feature == Feature::kShaderResourceMinLod)
    return "shaderResourceMinLod";
  if (feature == Feature::kSparseBinding)
    return "sparseBinding";
  if (feature == Feature::kSparseResidencyBuffer)
    return "sparseResidencyBuffer";
  if (feature == Feature::kSparseResidencyImage2D)
    return "sparseResidencyImage2D";
  if (feature == Feature::kSparseResidencyImage3D)
    return "sparseResidencyImage3D";
  if (feature == Feature::kSparseResidency2Samples)
    return "sparseResidency2Samples";
  if (feature == Feature::kSparseResidency4Samples)
    return "sparseResidency4Samples";
  if (feature == Feature::kSparseResidency8Samples)
    return "sparseResidency8Samples";
  if (feature == Feature::kSparseResidency16Samples)
    return "sparseResidency16Samples";
  if (feature == Feature::kSparseResidencyAliased)
    return "sparseResidencyAliased";
  if (feature == Feature::kVariableMultisampleRate)
    return "variableMultisampleRate";
  if (feature == Feature::kInheritedQueries)
    return "inheritedQueries";
  return "";
}

}  // namespace

Script::Script() = default;

Script::~Script() = default;

std::vector<ShaderInfo> Script::GetShaderInfo() const {
  std::vector<ShaderInfo> ret;
  for (const auto& shader : shaders_) {
    // TODO(dsinclair): The name returned should be the
    // `pipeline_name + shader_name` instead of just shader name when we have
    // pipelines everywhere

    // TODO(dsinclair): The optimization passes should be retrieved from the
    // pipeline and returned here instead of an empty array.
    ret.emplace_back(ShaderInfo{shader->GetFormat(),
                                shader->GetType(),
                                shader->GetName(),
                                shader->GetData(),
                                {}});
  }
  return ret;
}

std::vector<std::string> Script::GetRequiredFeatures() const {
  std::vector<std::string> required_features_in_string;
  for (auto feature : engine_info_.required_features) {
    auto name = FeatureToName(feature);
    if (!name.empty())
      required_features_in_string.push_back(name);
  }
  return required_features_in_string;
}

std::vector<std::string> Script::GetRequiredExtensions() const {
  return engine_info_.required_extensions;
}

}  // namespace amber
