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

#include "src/make_unique.h"
#include "src/type_parser.h"

namespace amber {

Script::Script() : virtual_files_(MakeUnique<VirtualFileStore>()) {}

Script::~Script() = default;

std::vector<ShaderInfo> Script::GetShaderInfo() const {
  std::vector<ShaderInfo> ret;
  for (const auto& shader : shaders_) {
    bool in_pipeline = false;
    // A given shader could be in multiple pipelines with different
    // optimizations so make sure we check and report all pipelines.
    for (const auto& pipeline : pipelines_) {
      auto shader_info = pipeline->GetShader(shader.get());
      if (shader_info) {
        ret.emplace_back(
            ShaderInfo{shader->GetFormat(), shader->GetType(),
                       pipeline->GetName() + "-" + shader->GetName(),
                       shader->GetData(), shader_info->GetShaderOptimizations(),
                       shader->GetTargetEnv(), shader_info->GetData()});

        in_pipeline = true;
      }
    }

    if (!in_pipeline) {
      ret.emplace_back(ShaderInfo{shader->GetFormat(),
                                  shader->GetType(),
                                  shader->GetName(),
                                  shader->GetData(),
                                  {},
                                  shader->GetTargetEnv(),
                                  {}});
    }
  }
  return ret;
}

void Script::AddRequiredExtension(const std::string& ext) {
  // Make this smarter when we have more instance extensions to match.
  if (ext == "VK_KHR_get_physical_device_properties2")
    AddRequiredInstanceExtension(ext);
  else
    AddRequiredDeviceExtension(ext);
}

bool Script::IsKnownFeature(const std::string& name) const {
  return name == "robustBufferAccess" || name == "fullDrawIndexUint32" ||
         name == "imageCubeArray" || name == "independentBlend" ||
         name == "geometryShader" || name == "tessellationShader" ||
         name == "sampleRateShading" || name == "dualSrcBlend" ||
         name == "logicOp" || name == "multiDrawIndirect" ||
         name == "drawIndirectFirstInstance" || name == "depthClamp" ||
         name == "depthBiasClamp" || name == "fillModeNonSolid" ||
         name == "depthBounds" || name == "wideLines" ||
         name == "largePoints" || name == "alphaToOne" ||
         name == "multiViewport" || name == "samplerAnisotropy" ||
         name == "textureCompressionETC2" ||
         name == "textureCompressionASTC_LDR" ||
         name == "textureCompressionBC" || name == "occlusionQueryPrecise" ||
         name == "pipelineStatisticsQuery" ||
         name == "vertexPipelineStoresAndAtomics" ||
         name == "fragmentStoresAndAtomics" ||
         name == "shaderTessellationAndGeometryPointSize" ||
         name == "shaderImageGatherExtended" ||
         name == "shaderStorageImageExtendedFormats" ||
         name == "shaderStorageImageMultisample" ||
         name == "shaderStorageImageReadWithoutFormat" ||
         name == "shaderStorageImageWriteWithoutFormat" ||
         name == "shaderUniformBufferArrayDynamicIndexing" ||
         name == "shaderSampledImageArrayDynamicIndexing" ||
         name == "shaderStorageBufferArrayDynamicIndexing" ||
         name == "shaderStorageImageArrayDynamicIndexing" ||
         name == "shaderClipDistance" || name == "shaderCullDistance" ||
         name == "shaderFloat64" || name == "shaderInt64" ||
         name == "shaderInt16" || name == "shaderResourceResidency" ||
         name == "shaderResourceMinLod" || name == "sparseBinding" ||
         name == "sparseResidencyBuffer" || name == "sparseResidencyImage2D" ||
         name == "sparseResidencyImage3D" ||
         name == "sparseResidency2Samples" ||
         name == "sparseResidency4Samples" ||
         name == "sparseResidency8Samples" ||
         name == "sparseResidency16Samples" ||
         name == "sparseResidencyAliased" ||
         name == "variableMultisampleRate" || name == "inheritedQueries" ||
         name == "VariablePointerFeatures.variablePointers" ||
         name == "VariablePointerFeatures.variablePointersStorageBuffer" ||
         name == "Float16Int8Features.shaderFloat16" ||
         name == "Float16Int8Features.shaderInt8" ||
         name == "Storage8BitFeatures.storageBuffer8BitAccess" ||
         name == "Storage8BitFeatures.uniformAndStorageBuffer8BitAccess" ||
         name == "Storage8BitFeatures.storagePushConstant8" ||
         name == "Storage16BitFeatures.storageBuffer16BitAccess" ||
         name == "Storage16BitFeatures.uniformAndStorageBuffer16BitAccess" ||
         name == "Storage16BitFeatures.storagePushConstant16" ||
         name == "Storage16BitFeatures.storageInputOutput16" ||
         name == "SubgroupSizeControl.subgroupSizeControl" ||
         name == "SubgroupSizeControl.computeFullSubgroups" ||
         name == "SubgroupSupportedOperations.basic" ||
         name == "SubgroupSupportedOperations.vote" ||
         name == "SubgroupSupportedOperations.arithmetic" ||
         name == "SubgroupSupportedOperations.ballot" ||
         name == "SubgroupSupportedOperations.shuffle" ||
         name == "SubgroupSupportedOperations.shuffleRelative" ||
         name == "SubgroupSupportedOperations.clustered" ||
         name == "SubgroupSupportedOperations.quad" ||
         name == "SubgroupSupportedStages.vertex" ||
         name == "SubgroupSupportedStages.tessellationControl" ||
         name == "SubgroupSupportedStages.tessellationEvaluation" ||
         name == "SubgroupSupportedStages.geometry" ||
         name == "SubgroupSupportedStages.fragment" ||
         name == "SubgroupSupportedStages.compute" ||
         name ==
             "ShaderSubgroupExtendedTypesFeatures.shaderSubgroupExtendedTypes";
}

type::Type* Script::ParseType(const std::string& str) {
  auto type = GetType(str);
  if (type)
    return type;

  TypeParser parser;
  auto new_type = parser.Parse(str);
  if (new_type != nullptr) {
    type = new_type.get();
    RegisterType(std::move(new_type));
  }
  return type;
}

}  // namespace amber
