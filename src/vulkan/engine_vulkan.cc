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
    const std::vector<Feature>& features,
    const std::vector<std::string>& extensions) {
  Result r = device_->Initialize(features, extensions);
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

Result EngineVulkan::Initialize(const std::vector<Feature>& features,
                                const std::vector<std::string>& extensions) {
  if (device_)
    return Result("Vulkan::Set device_ already exists");

  device_ = MakeUnique<Device>();
  return InitDeviceAndCreateCommand(features, extensions);
}

Result EngineVulkan::InitializeWithConfig(
    EngineConfig* config,
    const std::vector<Feature>& features,
    const std::vector<std::string>& extensions) {
  if (device_)
    return Result("Vulkan::Set device_ already exists");

  VulkanEngineConfig* vk_config = static_cast<VulkanEngineConfig*>(config);
  if (vk_config->device == VK_NULL_HANDLE)
    return Result("Vulkan::InitializeWithConfig device handle is null.");

  device_ = MakeUnique<Device>(vk_config->device);
  return InitDeviceAndCreateCommand(features, extensions);
}

Result EngineVulkan::Shutdown() {
  for (auto it = modules_.begin(); it != modules_.end(); ++it)
    vkDestroyShaderModule(device_->GetDevice(), it->second, nullptr);

  pipeline_->Shutdown();

  if (vertex_buffer_)
    vertex_buffer_->Shutdown();

  pool_->Shutdown();
  device_->Shutdown();
  return {};
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
    stage_info[stage_count].pName = nullptr;
    ++stage_count;
  }
  return stage_info;
}

Result EngineVulkan::SetBuffer(BufferType type,
                               uint8_t location,
                               const Format& format,
                               const std::vector<Value>& values) {
  // Handle image and depth attachments special as they come in before
  // the pipeline is created.
  if (type == BufferType::kColor || type == BufferType::kDepth) {
    if (type == BufferType::kColor)
      color_frame_format_ = ToVkFormat(format.GetFormatType());
    else if (type == BufferType::kDepth)
      depth_frame_format_ = ToVkFormat(format.GetFormatType());

    return {};
  }

  if (!pipeline_)
    return Result("Vulkan::SetBuffer no Pipeline exists");

  if (!pipeline_->IsGraphics())
    return Result("Vulkan::SetBuffer for Non-Graphics Pipeline");

  if (type == BufferType::kVertex) {
    if (!vertex_buffer_)
      vertex_buffer_ = MakeUnique<VertexBuffer>(device_->GetDevice());

    pipeline_->AsGraphics()->SetVertexBuffer(location, format, values,
                                             vertex_buffer_.get());
    return {};
  }

  return Result("Vulkan::SetBuffer non-vertex buffer type not implemented");
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

Result EngineVulkan::DoDrawRect(const DrawRectCommand* command) {
  if (!pipeline_->IsGraphics())
    return Result("Vulkan::DrawRect for Non-Graphics Pipeline");

  auto* graphics = pipeline_->AsGraphics();

  Result r = graphics->ResetPipeline();
  if (!r.IsSuccess())
    return r;

  // |format| is not Format for frame buffer but for vertex buffer.
  // Since draw rect command contains its vertex information and it
  // does not include a format of vertex buffer, we can choose any
  // one that is suitable. We use VK_FORMAT_R32G32_SFLOAT for it.
  Format format;
  format.SetFormatType(FormatType::kR32G32_SFLOAT);
  format.AddComponent(FormatComponentType::kR, FormatMode::kSFloat, 32);
  format.AddComponent(FormatComponentType::kG, FormatMode::kSFloat, 32);

  float x = command->GetX();
  float y = command->GetY();
  float width = command->GetWidth();
  float height = command->GetHeight();
  if (command->IsOrtho()) {
    const float frame_width = static_cast<float>(graphics->GetWidth());
    const float frame_height = static_cast<float>(graphics->GetHeight());
    x = ((x / frame_width) * 2.0f) - 1.0f;
    y = ((y / frame_height) * 2.0f) - 1.0f;
    width = ((width / frame_width) * 2.0f) - 1.0f;
    height = ((height / frame_height) * 2.0f) - 1.0f;
  }

  std::vector<Value> values(8);
  // Bottom left
  values[0].SetDoubleValue(static_cast<double>(x));
  values[1].SetDoubleValue(static_cast<double>(y + height));
  // Top left
  values[2].SetDoubleValue(static_cast<double>(x));
  values[3].SetDoubleValue(static_cast<double>(y));
  // Bottom right
  values[4].SetDoubleValue(static_cast<double>(x + width));
  values[5].SetDoubleValue(static_cast<double>(y + height));
  // Top right
  values[6].SetDoubleValue(static_cast<double>(x + width));
  values[7].SetDoubleValue(static_cast<double>(y));

  auto vertex_buffer = MakeUnique<VertexBuffer>(device_->GetDevice());

  r = graphics->SetVertexBuffer(0, format, values, vertex_buffer.get());
  if (!r.IsSuccess())
    return r;

  PipelineData data;
  DrawArraysCommand draw(data);
  draw.SetTopology(command->IsPatch() ? Topology::kPatchList
                                      : Topology::kTriangleStrip);
  draw.SetFirstVertexIndex(0);
  draw.SetVertexCount(4);
  draw.SetInstanceCount(1);

  r = graphics->Draw(&draw, vertex_buffer.get());
  if (!r.IsSuccess())
    return r;

  r = graphics->ResetPipeline();
  if (!r.IsSuccess())
    return r;

  vertex_buffer->Shutdown();
  return {};
}

Result EngineVulkan::DoDrawArrays(const DrawArraysCommand* command) {
  if (!pipeline_->IsGraphics())
    return Result("Vulkan::DrawArrays for Non-Graphics Pipeline");

  return pipeline_->AsGraphics()->Draw(command, vertex_buffer_.get());
}

Result EngineVulkan::DoCompute(const ComputeCommand* command) {
  if (pipeline_->IsGraphics())
    return Result("Vulkan: Compute called for graphics pipeline.");

  return pipeline_->AsCompute()->Compute(command->GetX(), command->GetY(),
                                         command->GetZ());
}

Result EngineVulkan::DoEntryPoint(const EntryPointCommand* command) {
  if (!pipeline_)
    return Result("Vulkan::DoEntryPoint no Pipeline exists");

  pipeline_->SetEntryPointName(ToVkShaderStage(command->GetShaderType()),
                               command->GetEntryPointName());
  return {};
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
  if (!command->IsSSBO() && !command->IsUniform())
    return Result("Vulkan::DoBuffer not supported buffer type");

  return pipeline_->AddDescriptor(command);
}

}  // namespace vulkan
}  // namespace amber
