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

#include "src/make_unique.h"
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

  // Unreachable
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

Result EngineVulkan::Initialize() {
  if (device_)
    return Result("Vulkan::Set device_ already exists");

  device_ = MakeUnique<Device>();
  return InitDeviceAndCreateCommand();
}

Result EngineVulkan::InitializeWithDevice(void* default_device) {
  if (device_)
    return Result("Vulkan::Set device_ already exists");

  VkDevice device = static_cast<VkDevice>(default_device);
  if (device == VK_NULL_HANDLE)
    return Result("Vulkan::Set VK_NULL_HANDLE is given");

  device_ = MakeUnique<Device>(device);
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

Result EngineVulkan::AddRequirement(Feature feature, const Format* fmt) {
  auto it = std::find_if(requirements_.begin(), requirements_.end(),
                         [&feature](const EngineVulkan::Requirement& req) {
                           return req.feature == feature;
                         });
  if (it != requirements_.end())
    return Result("Vulkan::Feature Already Exists");

  requirements_.push_back({feature, fmt});
  return {};
}

Result EngineVulkan::CreatePipeline(PipelineType type) {
  if (type == PipelineType::kCompute)
    return Result("Vulkan::Compute Pipeline Not Implemented");

  VkFormat frame_buffer_format = kDefaultColorFormat;
  auto it_frame_buffer =
      std::find_if(requirements_.begin(), requirements_.end(),
                   [](const EngineVulkan::Requirement& req) {
                     return req.feature == Feature::kFramebuffer;
                   });
  if (it_frame_buffer != requirements_.end()) {
    frame_buffer_format = ToVkFormat(it_frame_buffer->format->GetFormatType());
  }

  VkFormat depth_stencil_format = VK_FORMAT_UNDEFINED;
  auto it_depth_stencil =
      std::find_if(requirements_.begin(), requirements_.end(),
                   [](const EngineVulkan::Requirement& req) {
                     return req.feature == Feature::kDepthStencil;
                   });
  if (it_depth_stencil != requirements_.end()) {
    depth_stencil_format =
        ToVkFormat(it_depth_stencil->format->GetFormatType());
  }

  pipeline_ = MakeUnique<GraphicsPipeline>(
      type, device_->GetDevice(), device_->GetPhysicalMemoryProperties(),
      frame_buffer_format, depth_stencil_format, GetShaderStageInfo());

  return pipeline_->AsGraphics()->Initialize(
      kFramebufferWidth, kFramebufferHeight, pool_->GetCommandPool(),
      device_->GetQueue());
}

Result EngineVulkan::SetShader(ShaderType type,
                               const std::vector<uint32_t>& data) {
  if (type == ShaderType::kCompute)
    return Result("Vulkan::Compute Pipeline Not Implemented");

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

Result EngineVulkan::DoDrawArrays(const DrawArraysCommand*) {
  if (!pipeline_->IsGraphics())
    return Result("Vulkan::DrawArrays for Non-Graphics Pipeline");

  return pipeline_->AsGraphics()->Draw();
}

Result EngineVulkan::DoCompute(const ComputeCommand*) {
  return Result("Vulkan::DoCompute Not Implemented");
}

Result EngineVulkan::DoEntryPoint(const EntryPointCommand*) {
  return Result("Vulkan::DoEntryPoint Not Implemented");
}

Result EngineVulkan::DoPatchParameterVertices(
    const PatchParameterVerticesCommand*) {
  return Result("Vulkan::DoPatch Not Implemented");
}

Result EngineVulkan::DoProbe(const ProbeCommand* command) {
  if (!pipeline_->IsGraphics())
    return Result("Vulkan::Probe FrameBuffer for Non-Graphics Pipeline");

  return pipeline_->AsGraphics()->Probe(command);
}

Result EngineVulkan::DoProbeSSBO(const ProbeSSBOCommand*) {
  return Result("Vulkan::DoProbeSSBO Not Implemented");
}

Result EngineVulkan::DoBuffer(const BufferCommand*) {
  return Result("Vulkan::DoBuffer Not Implemented");
}

Result EngineVulkan::DoTolerance(const ToleranceCommand*) {
  return Result("Vulkan::DoTolerance Not Implemented");
}

}  // namespace vulkan
}  // namespace amber
