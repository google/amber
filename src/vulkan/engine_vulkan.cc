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

#include "src/vulkan/engine_vulkan.h"

#include <algorithm>
#include <cassert>

#include "amber/amber_vulkan.h"
#include "src/make_unique.h"
#include "src/vulkan/compute_pipeline.h"
#include "src/vulkan/descriptor.h"
#include "src/vulkan/format_data.h"
#include "src/vulkan/graphics_pipeline.h"

namespace amber {
namespace vulkan {
namespace {

const uint32_t kFramebufferWidth = 250;
const uint32_t kFramebufferHeight = 250;

VkShaderStageFlagBits ToVkShaderStage(ShaderType type) {
  switch (type) {
    case ShaderType::kGeometry:
      return VK_SHADER_STAGE_GEOMETRY_BIT;
    case ShaderType::kFragment:
      return VK_SHADER_STAGE_FRAGMENT_BIT;
    case ShaderType::kVertex:
      return VK_SHADER_STAGE_VERTEX_BIT;
    case ShaderType::kTessellationControl:
      return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    case ShaderType::kTessellationEvaluation:
      return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    case ShaderType::kCompute:
      return VK_SHADER_STAGE_COMPUTE_BIT;
  }

  assert(false && "Vulkan::Unknown shader stage");
  return VK_SHADER_STAGE_FRAGMENT_BIT;
}

}  // namespace

EngineVulkan::EngineVulkan() : Engine() {}

EngineVulkan::~EngineVulkan() = default;

Result EngineVulkan::InitDeviceAndCreateCommand(
    const VkPhysicalDeviceFeatures& features) {
  Result r = device_->Initialize(features);
  if (!r.IsSuccess())
    return r;

  if (!pool_) {
    pool_ = MakeUnique<CommandPool>(device_->GetDevice());
    r = pool_->Initialize(device_->GetQueueFamilyIndex());
    if (!r.IsSuccess())
      return r;
  }

  return {};
}

VkPhysicalDeviceFeatures EngineVulkan::RequiredFeatures(
    const std::vector<Feature>& required_features) {
  VkPhysicalDeviceFeatures features = {};
  for (const auto& feature : required_features) {
    switch (feature) {
      case Feature::kRobustBufferAccess:
        features.robustBufferAccess = VK_TRUE;
        break;
      case Feature::kFullDrawIndexUint32:
        features.fullDrawIndexUint32 = VK_TRUE;
        break;
      case Feature::kImageCubeArray:
        features.imageCubeArray = VK_TRUE;
        break;
      case Feature::kIndependentBlend:
        features.independentBlend = VK_TRUE;
        break;
      case Feature::kGeometryShader:
        features.geometryShader = VK_TRUE;
        break;
      case Feature::kTessellationShader:
        features.tessellationShader = VK_TRUE;
        break;
      case Feature::kSampleRateShading:
        features.sampleRateShading = VK_TRUE;
        break;
      case Feature::kDualSrcBlend:
        features.dualSrcBlend = VK_TRUE;
        break;
      case Feature::kLogicOp:
        features.logicOp = VK_TRUE;
        break;
      case Feature::kMultiDrawIndirect:
        features.multiDrawIndirect = VK_TRUE;
        break;
      case Feature::kDrawIndirectFirstInstance:
        features.drawIndirectFirstInstance = VK_TRUE;
        break;
      case Feature::kDepthClamp:
        features.depthClamp = VK_TRUE;
        break;
      case Feature::kDepthBiasClamp:
        features.depthBiasClamp = VK_TRUE;
        break;
      case Feature::kFillModeNonSolid:
        features.fillModeNonSolid = VK_TRUE;
        break;
      case Feature::kDepthBounds:
        features.depthBounds = VK_TRUE;
        break;
      case Feature::kWideLines:
        features.wideLines = VK_TRUE;
        break;
      case Feature::kLargePoints:
        features.largePoints = VK_TRUE;
        break;
      case Feature::kAlphaToOne:
        features.alphaToOne = VK_TRUE;
        break;
      case Feature::kMultiViewport:
        features.multiViewport = VK_TRUE;
        break;
      case Feature::kSamplerAnisotropy:
        features.samplerAnisotropy = VK_TRUE;
        break;
      case Feature::kTextureCompressionETC2:
        features.textureCompressionETC2 = VK_TRUE;
        break;
      case Feature::kTextureCompressionASTC_LDR:
        features.textureCompressionASTC_LDR = VK_TRUE;
        break;
      case Feature::kTextureCompressionBC:
        features.textureCompressionBC = VK_TRUE;
        break;
      case Feature::kOcclusionQueryPrecise:
        features.occlusionQueryPrecise = VK_TRUE;
        break;
      case Feature::kPipelineStatisticsQuery:
        features.pipelineStatisticsQuery = VK_TRUE;
        break;
      case Feature::kVertexPipelineStoresAndAtomics:
        features.vertexPipelineStoresAndAtomics = VK_TRUE;
        break;
      case Feature::kFragmentStoresAndAtomics:
        features.fragmentStoresAndAtomics = VK_TRUE;
        break;
      case Feature::kShaderTessellationAndGeometryPointSize:
        features.shaderTessellationAndGeometryPointSize = VK_TRUE;
        break;
      case Feature::kShaderImageGatherExtended:
        features.shaderImageGatherExtended = VK_TRUE;
        break;
      case Feature::kShaderStorageImageExtendedFormats:
        features.shaderStorageImageExtendedFormats = VK_TRUE;
        break;
      case Feature::kShaderStorageImageMultisample:
        features.shaderStorageImageMultisample = VK_TRUE;
        break;
      case Feature::kShaderStorageImageReadWithoutFormat:
        features.shaderStorageImageReadWithoutFormat = VK_TRUE;
        break;
      case Feature::kShaderStorageImageWriteWithoutFormat:
        features.shaderStorageImageWriteWithoutFormat = VK_TRUE;
        break;
      case Feature::kShaderUniformBufferArrayDynamicIndexing:
        features.shaderUniformBufferArrayDynamicIndexing = VK_TRUE;
        break;
      case Feature::kShaderSampledImageArrayDynamicIndexing:
        features.shaderSampledImageArrayDynamicIndexing = VK_TRUE;
        break;
      case Feature::kShaderStorageBufferArrayDynamicIndexing:
        features.shaderStorageBufferArrayDynamicIndexing = VK_TRUE;
        break;
      case Feature::kShaderStorageImageArrayDynamicIndexing:
        features.shaderStorageImageArrayDynamicIndexing = VK_TRUE;
        break;
      case Feature::kShaderClipDistance:
        features.shaderClipDistance = VK_TRUE;
        break;
      case Feature::kShaderCullDistance:
        features.shaderCullDistance = VK_TRUE;
        break;
      case Feature::kShaderFloat64:
        features.shaderFloat64 = VK_TRUE;
        break;
      case Feature::kShaderInt64:
        features.shaderInt64 = VK_TRUE;
        break;
      case Feature::kShaderInt16:
        features.shaderInt16 = VK_TRUE;
        break;
      case Feature::kShaderResourceResidency:
        features.shaderResourceResidency = VK_TRUE;
        break;
      case Feature::kShaderResourceMinLod:
        features.shaderResourceMinLod = VK_TRUE;
        break;
      case Feature::kSparseBinding:
        features.sparseBinding = VK_TRUE;
        break;
      case Feature::kSparseResidencyBuffer:
        features.sparseResidencyBuffer = VK_TRUE;
        break;
      case Feature::kSparseResidencyImage2D:
        features.sparseResidencyImage2D = VK_TRUE;
        break;
      case Feature::kSparseResidencyImage3D:
        features.sparseResidencyImage3D = VK_TRUE;
        break;
      case Feature::kSparseResidency2Samples:
        features.sparseResidency2Samples = VK_TRUE;
        break;
      case Feature::kSparseResidency4Samples:
        features.sparseResidency4Samples = VK_TRUE;
        break;
      case Feature::kSparseResidency8Samples:
        features.sparseResidency8Samples = VK_TRUE;
        break;
      case Feature::kSparseResidency16Samples:
        features.sparseResidency16Samples = VK_TRUE;
        break;
      case Feature::kSparseResidencyAliased:
        features.sparseResidencyAliased = VK_TRUE;
        break;
      case Feature::kVariableMultisampleRate:
        features.variableMultisampleRate = VK_TRUE;
        break;
      case Feature::kInheritedQueries:
        features.inheritedQueries = VK_TRUE;
        break;
      case Feature::kFramebuffer:
      case Feature::kDepthStencil:
      case Feature::kFenceTimeout:
      case Feature::kUnknown:
        break;
    }
  }
  return features;
}

Result EngineVulkan::Initialize(const std::vector<Feature>& features,
                                const std::vector<std::string>&) {
  if (device_)
    return Result("Vulkan::Set device_ already exists");

  device_ = MakeUnique<Device>();
  return InitDeviceAndCreateCommand(RequiredFeatures(features));
}

Result EngineVulkan::InitializeWithConfig(EngineConfig* config,
                                          const std::vector<Feature>& features,
                                          const std::vector<std::string>&) {
  if (device_)
    return Result("Vulkan::Set device_ already exists");

  VulkanEngineConfig* vk_config = static_cast<VulkanEngineConfig*>(config);
  if (vk_config->device == VK_NULL_HANDLE)
    return Result("Vulkan::InitializeWithConfig device handle is null.");

  device_ = MakeUnique<Device>(vk_config->device);
  return InitDeviceAndCreateCommand(RequiredFeatures(features));
}

Result EngineVulkan::Shutdown() {
  for (auto it = modules_.begin(); it != modules_.end(); ++it)
    vkDestroyShaderModule(device_->GetDevice(), it->second, nullptr);

  pipeline_->Shutdown();
  pool_->Shutdown();
  device_->Shutdown();
  return {};
}

Result EngineVulkan::AddRequirement(Feature feature, const Format* fmt) {
  if (feature == Feature::kFramebuffer) {
    if (fmt != nullptr)
      color_frame_format_ = ToVkFormat(fmt->GetFormatType());
    return {};
  }

  if (feature == Feature::kDepthStencil) {
    if (fmt != nullptr)
      depth_frame_format_ = ToVkFormat(fmt->GetFormatType());
    return {};
  }

  return Result(
      "Vulkan::AddRequirement features and extensions must be handled by "
      "Initialize()");
}

Result EngineVulkan::CreatePipeline(PipelineType type) {
  const auto& engine_data = GetEngineData();

  if (type == PipelineType::kCompute) {
    pipeline_ = MakeUnique<ComputePipeline>(
        device_->GetDevice(), device_->GetPhysicalMemoryProperties(),
        engine_data.fence_timeout_ms, GetShaderStageInfo());
    return pipeline_->AsCompute()->Initialize(pool_->GetCommandPool(),
                                              device_->GetQueue());
  }

  pipeline_ = MakeUnique<GraphicsPipeline>(
      device_->GetDevice(), device_->GetPhysicalMemoryProperties(),
      color_frame_format_, depth_frame_format_, engine_data.fence_timeout_ms,
      GetShaderStageInfo());

  return pipeline_->AsGraphics()->Initialize(
      kFramebufferWidth, kFramebufferHeight, pool_->GetCommandPool(),
      device_->GetQueue());
}

Result EngineVulkan::SetShader(ShaderType type,
                               const std::vector<uint32_t>& data) {
  VkShaderModuleCreateInfo info = {};
  info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  info.codeSize = data.size() * sizeof(uint32_t);
  info.pCode = data.data();

  auto it = modules_.find(type);
  if (it != modules_.end())
    return Result("Vulkan::Setting Duplicated Shader Types Fail");

  VkShaderModule shader;
  if (vkCreateShaderModule(device_->GetDevice(), &info, nullptr, &shader) !=
      VK_SUCCESS) {
    return Result("Vulkan::Calling vkCreateShaderModule Fail");
  }

  modules_[type] = shader;
  return {};
}

std::vector<VkPipelineShaderStageCreateInfo>
EngineVulkan::GetShaderStageInfo() {
  std::vector<VkPipelineShaderStageCreateInfo> stage_info(modules_.size());
  uint32_t stage_count = 0;
  for (auto it : modules_) {
    stage_info[stage_count] = {};
    stage_info[stage_count].sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stage_info[stage_count].stage = ToVkShaderStage(it.first);
    stage_info[stage_count].module = it.second;
    // TODO(jaebaek): Handle entry point command
    stage_info[stage_count].pName = "main";
    ++stage_count;
  }
  return stage_info;
}

Result EngineVulkan::SetBuffer(BufferType type,
                               uint8_t location,
                               const Format& format,
                               const std::vector<Value>& values) {
  if (!pipeline_)
    return Result("Vulkan::SetBuffer no Pipeline exists");

  // TODO(jaebaek): Doublecheck those buffers are only for the graphics
  //                pipeline.
  if (!pipeline_->IsGraphics())
    return Result("Vulkan::SetBuffer for Non-Graphics Pipeline");

  pipeline_->AsGraphics()->SetBuffer(type, location, format, values);
  return {};
}

Result EngineVulkan::DoClearColor(const ClearColorCommand* command) {
  if (!pipeline_->IsGraphics())
    return Result("Vulkan::Clear Color Command for Non-Graphics Pipeline");

  return pipeline_->AsGraphics()->SetClearColor(
      command->GetR(), command->GetG(), command->GetB(), command->GetA());
}

Result EngineVulkan::DoClearStencil(const ClearStencilCommand* command) {
  if (!pipeline_->IsGraphics())
    return Result("Vulkan::Clear Stencil Command for Non-Graphics Pipeline");

  return pipeline_->AsGraphics()->SetClearStencil(command->GetValue());
}

Result EngineVulkan::DoClearDepth(const ClearDepthCommand* command) {
  if (!pipeline_->IsGraphics())
    return Result("Vulkan::Clear Depth Command for Non-Graphics Pipeline");

  return pipeline_->AsGraphics()->SetClearDepth(command->GetValue());
}

Result EngineVulkan::DoClear(const ClearCommand*) {
  if (!pipeline_->IsGraphics())
    return Result("Vulkan::Clear Command for Non-Graphics Pipeline");

  return pipeline_->AsGraphics()->Clear();
}

Result EngineVulkan::DoDrawRect(const DrawRectCommand*) {
  return Result("Vulkan::DoDrawRect Not Implemented");
}

Result EngineVulkan::DoDrawArrays(const DrawArraysCommand* command) {
  if (!pipeline_->IsGraphics())
    return Result("Vulkan::DrawArrays for Non-Graphics Pipeline");

  return pipeline_->AsGraphics()->Draw(command);
}

Result EngineVulkan::DoCompute(const ComputeCommand* command) {
  if (pipeline_->IsGraphics())
    return Result("Vulkan: Compute called for graphics pipeline.");

  return pipeline_->AsCompute()->Compute(command->GetX(), command->GetY(),
                                         command->GetZ());
}

Result EngineVulkan::DoEntryPoint(const EntryPointCommand*) {
  return Result("Vulkan::DoEntryPoint Not Implemented");
}

Result EngineVulkan::DoPatchParameterVertices(
    const PatchParameterVerticesCommand*) {
  return Result("Vulkan::DoPatch Not Implemented");
}

Result EngineVulkan::DoProcessCommands() {
  return pipeline_->ProcessCommands();
}

Result EngineVulkan::GetFrameBufferInfo(ResourceInfo* info) {
  assert(info);

  if (!pipeline_->IsGraphics())
    return Result("Vulkan::GetFrameBufferInfo for Non-Graphics Pipeline");

  const auto graphics = pipeline_->AsGraphics();
  const auto frame = graphics->GetFrame();
  const auto bytes_per_texel = VkFormatToByteSize(graphics->GetColorFormat());
  info->type = ResourceInfoType::kImage;
  info->image_info.width = frame->GetWidth();
  info->image_info.height = frame->GetHeight();
  info->image_info.depth = 1U;
  info->image_info.texel_stride = bytes_per_texel;
  // When copying the image to the host buffer, we specify a row length of 0
  // which results in tight packing of rows.  So the row stride is the product
  // of the texel stride and the number of texels in a row.
  const auto row_stride = bytes_per_texel * frame->GetWidth();
  info->image_info.row_stride = row_stride;
  info->size_in_bytes = row_stride * frame->GetHeight();
  info->cpu_memory = frame->GetColorBufferPtr();

  return {};
}

Result EngineVulkan::GetDescriptorInfo(const uint32_t descriptor_set,
                                       const uint32_t binding,
                                       ResourceInfo* info) {
  assert(info);
  Result r = pipeline_->CopyDescriptorToHost(descriptor_set, binding);
  if (!r.IsSuccess())
    return r;

  pipeline_->GetDescriptorInfo(descriptor_set, binding, info);
  return {};
}

Result EngineVulkan::DoBuffer(const BufferCommand* command) {
  if (!command->IsSSBO())
    return Result("Vulkan::DoBuffer non-SSBO descriptor not implemented");

  return pipeline_->AddDescriptor(command);
}

}  // namespace vulkan
}  // namespace amber
