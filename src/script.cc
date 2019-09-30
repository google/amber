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

#include "src/type_parser.h"

namespace amber {

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
         name == "VariablePointerFeatures.variablePointersStorageBuffer";
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
