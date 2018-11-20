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

#ifndef SRC_FEATURE_H_
#define SRC_FEATURE_H_

namespace amber {

enum class Feature {
  kUnknown = 0,
  kRobustBufferAccess,
  kFullDrawIndexUint32,
  kImageCubeArray,
  kIndependentBlend,
  kGeometryShader,
  kTessellationShader,
  kSampleRateShading,
  kDualSrcBlend,
  kLogicOp,
  kMultiDrawIndirect,
  kDrawIndirectFirstInstance,
  kDepthClamp,
  kDepthBiasClamp,
  kFillModeNonSolid,
  kDepthBounds,
  kWideLines,
  kLargePoints,
  kAlphaToOne,
  kMultiViewport,
  kSamplerAnisotropy,
  kTextureCompressionETC2,
  kTextureCompressionASTC_LDR,
  kTextureCompressionBC,
  kOcclusionQueryPrecise,
  kPipelineStatisticsQuery,
  kVertexPipelineStoresAndAtomics,
  kFragmentStoresAndAtomics,
  kShaderTessellationAndGeometryPointSize,
  kShaderImageGatherExtended,
  kShaderStorageImageExtendedFormats,
  kShaderStorageImageMultisample,
  kShaderStorageImageReadWithoutFormat,
  kShaderStorageImageWriteWithoutFormat,
  kShaderUniformBufferArrayDynamicIndexing,
  kShaderSampledImageArrayDynamicIndexing,
  kShaderStorageBufferArrayDynamicIndexing,
  kShaderStorageImageArrayDynamicIndexing,
  kShaderClipDistance,
  kShaderCullDistance,
  kShaderFloat64,
  kShaderInt64,
  kShaderInt16,
  kShaderResourceResidency,
  kShaderResourceMinLod,
  kSparseBinding,
  kSparseResidencyBuffer,
  kSparseResidencyImage2D,
  kSparseResidencyImage3D,
  kSparseResidency2Samples,
  kSparseResidency4Samples,
  kSparseResidency8Samples,
  kSparseResidency16Samples,
  kSparseResidencyAliased,
  kVariableMultisampleRate,
  kInheritedQueries,
  kFramebuffer,
  kDepthStencil,
  kFenceTimeout,
};

}  // namespace amber

#endif  // SRC_FEATURE_H_
