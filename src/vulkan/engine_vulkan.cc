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
#include <set>
#include <utility>

#include "amber/amber_vulkan.h"
#include "src/make_unique.h"
#include "src/vulkan/compute_pipeline.h"
#include "src/vulkan/descriptor.h"
#include "src/vulkan/format_data.h"
#include "src/vulkan/graphics_pipeline.h"

namespace amber {
namespace vulkan {
namespace {

const FormatType kDefaultFramebufferFormat = FormatType::kB8G8R8A8_UNORM;

VkShaderStageFlagBits ToVkShaderStage(ShaderType type) {
  switch (type) {
    case kShaderTypeGeometry:
      return VK_SHADER_STAGE_GEOMETRY_BIT;
    case kShaderTypeFragment:
      return VK_SHADER_STAGE_FRAGMENT_BIT;
    case kShaderTypeVertex:
      return VK_SHADER_STAGE_VERTEX_BIT;
    case kShaderTypeTessellationControl:
      return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    case kShaderTypeTessellationEvaluation:
      return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    case kShaderTypeCompute:
      return VK_SHADER_STAGE_COMPUTE_BIT;
    case kShaderTypeMulti:
      // It's an error if this arrives here ...
      break;
  }

  assert(false && "Vulkan::Unknown shader stage");
  return VK_SHADER_STAGE_FRAGMENT_BIT;
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

EngineVulkan::EngineVulkan() : Engine() {}

EngineVulkan::~EngineVulkan() = default;

Result EngineVulkan::Initialize(
    EngineConfig* config,
    Delegate* delegate,
    const std::vector<std::string>& features,
    const std::vector<std::string>& instance_extensions,
    const std::vector<std::string>& device_extensions) {
  if (device_)
    return Result("Vulkan::Initialize device_ already exists");

  VulkanEngineConfig* vk_config = static_cast<VulkanEngineConfig*>(config);
  if (!vk_config || vk_config->vkGetInstanceProcAddr == VK_NULL_HANDLE)
    return Result("Vulkan::Initialize vkGetInstanceProcAddr must be provided.");
  if (vk_config->device == VK_NULL_HANDLE)
    return Result("Vulkan::Initialize device must be provided");
  if (vk_config->physical_device == VK_NULL_HANDLE)
    return Result("Vulkan::Initialize physical device handle is null.");
  if (vk_config->queue == VK_NULL_HANDLE)
    return Result("Vulkan::Initialize queue handle is null.");

  // Validate instance extensions
  if (!AreAllExtensionsSupported(vk_config->available_instance_extensions,
                                 instance_extensions)) {
    return Result("Vulkan::Initialize not all instance extensions supported");
  }

  device_ = MakeUnique<Device>(vk_config->instance, vk_config->physical_device,
                               vk_config->queue_family_index, vk_config->device,
                               vk_config->queue);

  Result r = device_->Initialize(
      vk_config->vkGetInstanceProcAddr, delegate, features, device_extensions,
      vk_config->available_features, vk_config->available_features2,
      vk_config->available_device_extensions);
  if (!r.IsSuccess())
    return r;

  if (!pool_) {
    pool_ = MakeUnique<CommandPool>(device_.get());
    r = pool_->Initialize(device_->GetQueueFamilyIndex());
    if (!r.IsSuccess())
      return r;
  }

  return {};
}

Result EngineVulkan::Shutdown() {
  if (!device_)
    return {};

  for (auto it = pipeline_map_.begin(); it != pipeline_map_.end(); ++it) {
    auto& info = it->second;

    for (auto mod_it = info.shaders.begin(); mod_it != info.shaders.end();
         ++mod_it) {
      auto vk_device = device_->GetDevice();
      if (vk_device != VK_NULL_HANDLE && mod_it->second != VK_NULL_HANDLE)
        device_->GetPtrs()->vkDestroyShaderModule(vk_device, mod_it->second,
                                                  nullptr);
    }

    info.vk_pipeline->Shutdown();
    if (info.vertex_buffer)
      info.vertex_buffer->Shutdown();
  }

  if (pool_)
    pool_->Shutdown();

  return {};
}

bool EngineVulkan::VerifyFormatAvailable(const Format& format,
                                         BufferType type) {
  return IsFormatSupportedByPhysicalDevice(type, device_->GetPhysicalDevice(),
                                           ToVkFormat(format.GetFormatType()));
}

Result EngineVulkan::CreatePipeline(amber::Pipeline* pipeline) {
  // Create the pipeline data early so we can access them as needed.
  pipeline_map_[pipeline] = PipelineInfo();
  auto& info = pipeline_map_[pipeline];

  for (const auto& shader_info : pipeline->GetShaders()) {
    Result r =
        SetShader(pipeline, shader_info.GetShaderType(), shader_info.GetData());
    if (!r.IsSuccess())
      return r;
  }

  for (const auto& colour_info : pipeline->GetColorAttachments()) {
    auto& fmt = colour_info.buffer->AsFormatBuffer()->GetFormat();
    if (!VerifyFormatAvailable(fmt, colour_info.type))
      return Result("Vulkan color attachment format is not supported");
  }

  FormatType depth_buffer_format = FormatType::kUnknown;
  if (pipeline->GetDepthBuffer().buffer) {
    const auto& depth_info = pipeline->GetDepthBuffer();
    auto& depth_fmt = depth_info.buffer->AsFormatBuffer()->GetFormat();
    if (!VerifyFormatAvailable(depth_fmt, depth_info.type))
      return Result("Vulkan depth attachment format is not supported");

    depth_buffer_format = depth_fmt.GetFormatType();
  }

  const auto& engine_data = GetEngineData();
  std::unique_ptr<Pipeline> vk_pipeline;
  if (pipeline->GetType() == PipelineType::kCompute) {
    vk_pipeline = MakeUnique<ComputePipeline>(
        device_.get(), device_->GetPhysicalDeviceProperties(),
        device_->GetPhysicalMemoryProperties(), engine_data.fence_timeout_ms,
        GetShaderStageInfo(pipeline));
    Result r =
        vk_pipeline->AsCompute()->Initialize(pool_.get(), device_->GetQueue());
    if (!r.IsSuccess())
      return r;
  } else {
    vk_pipeline = MakeUnique<GraphicsPipeline>(
        device_.get(), device_->GetPhysicalDeviceProperties(),
        device_->GetPhysicalMemoryProperties(), pipeline->GetColorAttachments(),
        ToVkFormat(depth_buffer_format), engine_data.fence_timeout_ms,
        GetShaderStageInfo(pipeline));

    Result r = vk_pipeline->AsGraphics()->Initialize(
        pipeline->GetFramebufferWidth(), pipeline->GetFramebufferHeight(),
        pool_.get(), device_->GetQueue());
    if (!r.IsSuccess())
      return r;
  }

  info.vk_pipeline = std::move(vk_pipeline);

  for (const auto& vtex_info : pipeline->GetVertexBuffers()) {
    auto& fmt = vtex_info.buffer->IsFormatBuffer()
                    ? vtex_info.buffer->AsFormatBuffer()->GetFormat()
                    : Format();
    if (!VerifyFormatAvailable(fmt, vtex_info.type))
      return Result("Vulkan vertex buffer format is not supported");

    if (!info.vertex_buffer)
      info.vertex_buffer = MakeUnique<VertexBuffer>(device_.get());

    info.vertex_buffer->SetData(static_cast<uint8_t>(vtex_info.location), fmt,
                                vtex_info.buffer->GetData());
  }

  if (pipeline->GetIndexBuffer()) {
    auto* buf = pipeline->GetIndexBuffer();
    info.vk_pipeline->AsGraphics()->SetIndexBuffer(buf->GetData());
  }

  return {};
}

Result EngineVulkan::SetShader(amber::Pipeline* pipeline,
                               ShaderType type,
                               const std::vector<uint32_t>& data) {
  auto& info = pipeline_map_[pipeline];

  auto it = info.shaders.find(type);
  if (it != info.shaders.end())
    return Result("Vulkan::Setting Duplicated Shader Types Fail");

  VkShaderModuleCreateInfo create_info = VkShaderModuleCreateInfo();
  create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  create_info.codeSize = data.size() * sizeof(uint32_t);
  create_info.pCode = data.data();

  VkShaderModule shader;
  if (device_->GetPtrs()->vkCreateShaderModule(
          device_->GetDevice(), &create_info, nullptr, &shader) != VK_SUCCESS) {
    return Result("Vulkan::Calling vkCreateShaderModule Fail");
  }

  info.shaders[type] = shader;
  return {};
}

std::vector<VkPipelineShaderStageCreateInfo> EngineVulkan::GetShaderStageInfo(
    amber::Pipeline* pipeline) {
  auto& info = pipeline_map_[pipeline];

  std::vector<VkPipelineShaderStageCreateInfo> stage_info(info.shaders.size());
  uint32_t stage_count = 0;
  for (auto it : info.shaders) {
    stage_info[stage_count] = VkPipelineShaderStageCreateInfo();
    stage_info[stage_count].sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stage_info[stage_count].stage = ToVkShaderStage(it.first);
    stage_info[stage_count].module = it.second;
    stage_info[stage_count].pName = nullptr;
    ++stage_count;
  }
  return stage_info;
}

Result EngineVulkan::DoClearColor(const ClearColorCommand* command) {
  auto& info = pipeline_map_[command->GetPipeline()];
  if (!info.vk_pipeline->IsGraphics())
    return Result("Vulkan::Clear Color Command for Non-Graphics Pipeline");

  return info.vk_pipeline->AsGraphics()->SetClearColor(
      command->GetR(), command->GetG(), command->GetB(), command->GetA());
}

Result EngineVulkan::DoClearStencil(const ClearStencilCommand* command) {
  auto& info = pipeline_map_[command->GetPipeline()];
  if (!info.vk_pipeline->IsGraphics())
    return Result("Vulkan::Clear Stencil Command for Non-Graphics Pipeline");

  return info.vk_pipeline->AsGraphics()->SetClearStencil(command->GetValue());
}

Result EngineVulkan::DoClearDepth(const ClearDepthCommand* command) {
  auto& info = pipeline_map_[command->GetPipeline()];
  if (!info.vk_pipeline->IsGraphics())
    return Result("Vulkan::Clear Depth Command for Non-Graphics Pipeline");

  return info.vk_pipeline->AsGraphics()->SetClearDepth(command->GetValue());
}

Result EngineVulkan::DoClear(const ClearCommand* command) {
  auto& info = pipeline_map_[command->GetPipeline()];
  if (!info.vk_pipeline->IsGraphics())
    return Result("Vulkan::Clear Command for Non-Graphics Pipeline");

  return info.vk_pipeline->AsGraphics()->Clear();
}

Result EngineVulkan::DoDrawRect(const DrawRectCommand* command) {
  auto& info = pipeline_map_[command->GetPipeline()];
  if (!info.vk_pipeline->IsGraphics())
    return Result("Vulkan::DrawRect for Non-Graphics Pipeline");

  auto* graphics = info.vk_pipeline->AsGraphics();

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

  auto vertex_buffer = MakeUnique<VertexBuffer>(device_.get());
  vertex_buffer->SetData(0, format, values);

  DrawArraysCommand draw(command->GetPipeline(), *command->GetPipelineData());
  draw.SetTopology(command->IsPatch() ? Topology::kPatchList
                                      : Topology::kTriangleStrip);
  draw.SetFirstVertexIndex(0);
  draw.SetVertexCount(4);
  draw.SetInstanceCount(1);

  Result r = graphics->Draw(&draw, vertex_buffer.get());
  if (!r.IsSuccess())
    return r;

  vertex_buffer->Shutdown();
  return {};
}

Result EngineVulkan::DoDrawArrays(const DrawArraysCommand* command) {
  auto& info = pipeline_map_[command->GetPipeline()];
  if (!info.vk_pipeline)
    return Result("Vulkan::DrawArrays for Non-Graphics Pipeline");

  return info.vk_pipeline->AsGraphics()->Draw(command,
                                              info.vertex_buffer.get());
}

Result EngineVulkan::DoCompute(const ComputeCommand* command) {
  auto& info = pipeline_map_[command->GetPipeline()];
  if (info.vk_pipeline->IsGraphics())
    return Result("Vulkan: Compute called for graphics pipeline.");

  return info.vk_pipeline->AsCompute()->Compute(
      command->GetX(), command->GetY(), command->GetZ());
}

Result EngineVulkan::DoEntryPoint(const EntryPointCommand* command) {
  auto& info = pipeline_map_[command->GetPipeline()];
  if (!info.vk_pipeline)
    return Result("Vulkan::DoEntryPoint no Pipeline exists");

  info.vk_pipeline->SetEntryPointName(ToVkShaderStage(command->GetShaderType()),
                                      command->GetEntryPointName());
  return {};
}

Result EngineVulkan::DoPatchParameterVertices(
    const PatchParameterVerticesCommand* command) {
  auto& info = pipeline_map_[command->GetPipeline()];
  if (!info.vk_pipeline->IsGraphics())
    return Result("Vulkan::DoPatchParameterVertices for Non-Graphics Pipeline");

  info.vk_pipeline->AsGraphics()->SetPatchControlPoints(
      command->GetControlPointCount());
  return {};
}

Result EngineVulkan::DoProcessCommands(amber::Pipeline* pipeline) {
  return pipeline_map_[pipeline].vk_pipeline->ProcessCommands();
}

Result EngineVulkan::GetFrameBufferInfo(amber::Pipeline* pipeline,
                                        size_t attachment_idx,
                                        ResourceInfo* resource_info) {
  if (!resource_info)
    return Result("Vulkan::GetFrameBufferInfo missing resource info");

  auto& info = pipeline_map_[pipeline];
  if (!info.vk_pipeline)
    return Result("Vulkan::GetFrameBufferInfo missing pipeline");
  if (!info.vk_pipeline->IsGraphics())
    return Result("Vulkan::GetFrameBufferInfo for Non-Graphics Pipeline");

  const auto graphics = info.vk_pipeline->AsGraphics();
  const auto frame = graphics->GetFrame();
  const auto& fmt = frame->GetFormatForAttachment(attachment_idx);
  const auto bytes_per_texel = fmt.GetByteSize();
  resource_info->type = ResourceInfoType::kImage;
  resource_info->image_info.width = frame->GetWidth();
  resource_info->image_info.height = frame->GetHeight();
  resource_info->image_info.depth = 1U;
  resource_info->image_info.texel_stride = bytes_per_texel;
  resource_info->image_info.texel_format = &fmt;
  // When copying the image to the host buffer, we specify a row length of 0
  // which results in tight packing of rows.  So the row stride is the product
  // of the texel stride and the number of texels in a row.
  const auto row_stride = bytes_per_texel * frame->GetWidth();
  resource_info->image_info.row_stride = row_stride;
  resource_info->size_in_bytes = row_stride * frame->GetHeight();
  resource_info->cpu_memory = frame->GetColorBufferPtr(attachment_idx);

  return {};
}

Result EngineVulkan::GetFrameBuffer(amber::Pipeline* pipeline,
                                    size_t attachment_idx,
                                    std::vector<Value>* values) {
  values->resize(0);

  ResourceInfo resource_info;
  GetFrameBufferInfo(pipeline, attachment_idx, &resource_info);
  if (resource_info.type != ResourceInfoType::kImage) {
    return Result(
        "Vulkan:GetFrameBuffer() is invalid for non-image framebuffer");
  }

  auto& info = pipeline_map_[pipeline];
  const auto frame = info.vk_pipeline->AsGraphics()->GetFrame();
  const auto& fmt = frame->GetFormatForAttachment(attachment_idx);
  // TODO(jaebaek): Support other formats
  if (fmt.GetFormatType() != kDefaultFramebufferFormat)
    return Result("Vulkan::GetFrameBuffer Unsupported buffer format");

  Value pixel;

  const uint8_t* cpu_memory =
      static_cast<const uint8_t*>(resource_info.cpu_memory);
  const uint32_t row_stride = resource_info.image_info.row_stride;
  const uint32_t texel_stride = resource_info.image_info.texel_stride;

  for (uint32_t y = 0; y < resource_info.image_info.height; ++y) {
    for (uint32_t x = 0; x < resource_info.image_info.width; ++x) {
      const uint8_t* ptr_8 = cpu_memory + (row_stride * y) + (texel_stride * x);
      const uint32_t* ptr_32 = reinterpret_cast<const uint32_t*>(ptr_8);
      pixel.SetIntValue(*ptr_32);
      values->push_back(pixel);
    }
  }

  return {};
}

Result EngineVulkan::GetDescriptorInfo(amber::Pipeline* pipeline,
                                       const uint32_t descriptor_set,
                                       const uint32_t binding,
                                       ResourceInfo* resource_info) {
  auto& info = pipeline_map_[pipeline];
  return info.vk_pipeline->GetDescriptorInfo(descriptor_set, binding,
                                             resource_info);
}

Result EngineVulkan::DoBuffer(const BufferCommand* command) {
  auto& info = pipeline_map_[command->GetPipeline()];
  if (command->IsPushConstant())
    return info.vk_pipeline->AddPushConstant(command);

  if (!IsDescriptorSetInBounds(device_->GetPhysicalDevice(),
                               command->GetDescriptorSet())) {
    return Result(
        "Vulkan::DoBuffer exceed maxBoundDescriptorSets limit of physical "
        "device");
  }

  return info.vk_pipeline->AddDescriptor(command);
}

bool EngineVulkan::IsFormatSupportedByPhysicalDevice(
    BufferType type,
    VkPhysicalDevice physical_device,
    VkFormat format) {
  VkFormatProperties properties = VkFormatProperties();
  device_->GetPtrs()->vkGetPhysicalDeviceFormatProperties(physical_device,
                                                          format, &properties);

  VkFormatFeatureFlagBits flag = VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT;
  bool is_buffer_type_image = false;
  switch (type) {
    case BufferType::kColor:
      flag = VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT;
      is_buffer_type_image = true;
      break;
    case BufferType::kDepth:
      flag = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
      is_buffer_type_image = true;
      break;
    case BufferType::kSampled:
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

bool EngineVulkan::IsDescriptorSetInBounds(VkPhysicalDevice physical_device,
                                           uint32_t descriptor_set) {
  VkPhysicalDeviceProperties properties = VkPhysicalDeviceProperties();
  device_->GetPtrs()->vkGetPhysicalDeviceProperties(physical_device,
                                                    &properties);
  return properties.limits.maxBoundDescriptorSets > descriptor_set;
}

}  // namespace vulkan
}  // namespace amber
