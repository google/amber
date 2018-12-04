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
#include <string>

#include "amber/amber_vulkan.h"
#include "src/feature.h"
#include "src/format_data.h"
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
const VkFormat kDefaultColorFormat = VK_FORMAT_R8G8B8A8_UNORM;

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

Result EngineVulkan::InitDeviceAndCreateCommand() {
  Result r = device_->Initialize();
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

Result EngineVulkan::Initialize(const std::vector<Feature>&,
                                const std::vector<std::string>&) {
  if (device_)
    return Result("Vulkan::Set device_ already exists");

  device_ = MakeUnique<Device>();
  return InitDeviceAndCreateCommand();
}

Result EngineVulkan::InitializeWithConfig(EngineConfig* config,
                                          const std::vector<Feature>&,
                                          const std::vector<std::string>&) {
  if (device_)
    return Result("Vulkan::Set device_ already exists");

  VulkanEngineConfig* vk_config = static_cast<VulkanEngineConfig*>(config);
  if (vk_config->device == VK_NULL_HANDLE)
    return Result("Vulkan::InitializeWithConfig device handle is null.");

  device_ = MakeUnique<Device>(vk_config->device);
  return InitDeviceAndCreateCommand();
}

Result EngineVulkan::Shutdown() {
  for (auto it = modules_.begin(); it != modules_.end(); ++it)
    vkDestroyShaderModule(device_->GetDevice(), it->second, nullptr);

  pipeline_->Shutdown();
  pool_->Shutdown();
  device_->Shutdown();
  return {};
}

Result EngineVulkan::AddRequirement(Feature feature,
                                    const Format* fmt,
                                    uint32_t val) {
  if (std::find(features_.begin(), features_.end(), feature) != features_.end())
    return Result("Vulkan::AddRequirement feature was already handled");

  features_.push_back(feature);

  if (feature == Feature::kFenceTimeout) {
    fence_timeout_ms_ = val;
    return {};
  }

  if (feature == Feature::kFramebuffer) {
    if (fmt != nullptr)
      color_frame_format_ = MakeUnique<Format>(*fmt);
    return {};
  }

  if (feature == Feature::kDepthStencil) {
    if (fmt != nullptr)
      depth_frame_format_ = MakeUnique<Format>(*fmt);
    return {};
  }

  return Result(
      "Vulkan::AddRequirement features and extensions must be handled by "
      "Initialize()");
}

Result EngineVulkan::CreatePipeline(PipelineType type) {
  if (type == PipelineType::kCompute) {
    pipeline_ = MakeUnique<ComputePipeline>(
        device_->GetDevice(), device_->GetPhysicalMemoryProperties(),
        fence_timeout_ms_, GetShaderStageInfo());
    return pipeline_->AsCompute()->Initialize(pool_->GetCommandPool(),
                                              device_->GetQueue());
  }

  VkFormat frame_buffer_format = kDefaultColorFormat;
  if (color_frame_format_)
    frame_buffer_format = ToVkFormat(color_frame_format_->GetFormatType());

  VkFormat depth_stencil_format = VK_FORMAT_UNDEFINED;
  if (depth_frame_format_)
    depth_stencil_format = ToVkFormat(depth_frame_format_->GetFormatType());

  pipeline_ = MakeUnique<GraphicsPipeline>(
      device_->GetDevice(), device_->GetPhysicalMemoryProperties(),
      frame_buffer_format, depth_stencil_format, fence_timeout_ms_,
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
