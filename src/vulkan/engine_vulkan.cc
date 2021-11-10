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
#include <set>
#include <utility>

#include "amber/amber_vulkan.h"
#include "src/make_unique.h"
#include "src/type_parser.h"
#include "src/vulkan/compute_pipeline.h"
#include "src/vulkan/graphics_pipeline.h"

namespace amber {
namespace vulkan {
namespace {

const uint32_t kTrianglesPerCell = 2;
const uint32_t kVerticesPerTriangle = 3;

Result ToVkShaderStage(ShaderType type, VkShaderStageFlagBits* ret) {
  switch (type) {
    case kShaderTypeGeometry:
      *ret = VK_SHADER_STAGE_GEOMETRY_BIT;
      break;
    case kShaderTypeFragment:
      *ret = VK_SHADER_STAGE_FRAGMENT_BIT;
      break;
    case kShaderTypeVertex:
      *ret = VK_SHADER_STAGE_VERTEX_BIT;
      break;
    case kShaderTypeTessellationControl:
      *ret = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
      break;
    case kShaderTypeTessellationEvaluation:
      *ret = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
      break;
    case kShaderTypeCompute:
      *ret = VK_SHADER_STAGE_COMPUTE_BIT;
      break;
    case kShaderTypeMulti:
      *ret = VK_SHADER_STAGE_FRAGMENT_BIT;
      return Result("Vulkan::Unknown shader stage");
  }

  return {};
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

EngineVulkan::~EngineVulkan() {
  auto vk_device = device_->GetVkDevice();
  if (vk_device != VK_NULL_HANDLE) {
    for (auto shader : shaders_) {
      device_->GetPtrs()->vkDestroyShaderModule(vk_device, shader.second,
                                                nullptr);
    }
  }
}

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
    r = pool_->Initialize();
    if (!r.IsSuccess())
      return r;
  }

  return {};
}

Result EngineVulkan::CreatePipeline(amber::Pipeline* pipeline) {
  // Create the pipeline data early so we can access them as needed.
  pipeline_map_[pipeline] = PipelineInfo();
  auto& info = pipeline_map_[pipeline];

  for (const auto& shader_info : pipeline->GetShaders()) {
    Result r = SetShader(pipeline, shader_info);
    if (!r.IsSuccess())
      return r;
  }

  for (const auto& colour_info : pipeline->GetColorAttachments()) {
    auto fmt = colour_info.buffer->GetFormat();
    if (!device_->IsFormatSupportedByPhysicalDevice(*fmt, colour_info.type))
      return Result("Vulkan color attachment format is not supported");
  }

  if (pipeline->GetDepthStencilBuffer().buffer) {
    const auto& depth_stencil_info = pipeline->GetDepthStencilBuffer();

    auto fmt = depth_stencil_info.buffer->GetFormat();
    if (!device_->IsFormatSupportedByPhysicalDevice(*fmt,
                                                    depth_stencil_info.type)) {
      return Result("Vulkan depth attachment format is not supported");
    }
  }

  std::vector<VkPipelineShaderStageCreateInfo> stage_create_info;
  Result r = GetVkShaderStageInfo(pipeline, &stage_create_info);
  if (!r.IsSuccess())
    return r;

  const auto& engine_data = GetEngineData();
  std::unique_ptr<Pipeline> vk_pipeline;
  if (pipeline->GetType() == PipelineType::kCompute) {
    vk_pipeline = MakeUnique<ComputePipeline>(
        device_.get(), engine_data.fence_timeout_ms, stage_create_info);
    r = vk_pipeline->AsCompute()->Initialize(pool_.get());
    if (!r.IsSuccess())
      return r;
  } else {
    vk_pipeline = MakeUnique<GraphicsPipeline>(
        device_.get(), pipeline->GetColorAttachments(),
        pipeline->GetDepthStencilBuffer(), pipeline->GetResolveTargets(),
        engine_data.fence_timeout_ms, stage_create_info);

    vk_pipeline->AsGraphics()->SetPatchControlPoints(
        pipeline->GetPipelineData()->GetPatchControlPoints());

    r = vk_pipeline->AsGraphics()->Initialize(pipeline->GetFramebufferWidth(),
                                              pipeline->GetFramebufferHeight(),
                                              pool_.get());
    if (!r.IsSuccess())
      return r;
  }

  info.vk_pipeline = std::move(vk_pipeline);

  // Set the entry point names for the pipeline.
  for (const auto& shader_info : pipeline->GetShaders()) {
    VkShaderStageFlagBits stage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
    r = ToVkShaderStage(shader_info.GetShaderType(), &stage);
    if (!r.IsSuccess())
      return r;
    const auto& name = shader_info.GetEntryPoint();
    if (!name.empty()) {
      info.vk_pipeline->SetEntryPointName(stage, name);
    }
  }

  for (const auto& vtex_info : pipeline->GetVertexBuffers()) {
    auto fmt = vtex_info.buffer->GetFormat();
    if (!device_->IsFormatSupportedByPhysicalDevice(*fmt, vtex_info.type))
      return Result("Vulkan vertex buffer format is not supported");
    if (!info.vertex_buffer)
      info.vertex_buffer = MakeUnique<VertexBuffer>(device_.get());

    info.vertex_buffer->SetData(static_cast<uint8_t>(vtex_info.location),
                                vtex_info.buffer, vtex_info.input_rate,
                                vtex_info.format, vtex_info.offset,
                                vtex_info.stride);
  }

  if (pipeline->GetIndexBuffer()) {
    auto* buf = pipeline->GetIndexBuffer();
    info.vk_pipeline->AsGraphics()->SetIndexBuffer(buf);
  }

  if (pipeline->GetPushConstantBuffer().buffer != nullptr) {
    r = info.vk_pipeline->AddPushConstantBuffer(
        pipeline->GetPushConstantBuffer().buffer, 0);
    if (!r.IsSuccess())
      return r;
  }

  for (const auto& buf_info : pipeline->GetBuffers()) {
    auto type = BufferCommand::BufferType::kSSBO;
    if (buf_info.type == BufferType::kStorageImage) {
      type = BufferCommand::BufferType::kStorageImage;
    } else if (buf_info.type == BufferType::kSampledImage) {
      type = BufferCommand::BufferType::kSampledImage;
    } else if (buf_info.type == BufferType::kCombinedImageSampler) {
      type = BufferCommand::BufferType::kCombinedImageSampler;
    } else if (buf_info.type == BufferType::kUniformTexelBuffer) {
      type = BufferCommand::BufferType::kUniformTexelBuffer;
    } else if (buf_info.type == BufferType::kStorageTexelBuffer) {
      type = BufferCommand::BufferType::kStorageTexelBuffer;
    } else if (buf_info.type == BufferType::kUniform) {
      type = BufferCommand::BufferType::kUniform;
    } else if (buf_info.type == BufferType::kUniformDynamic) {
      type = BufferCommand::BufferType::kUniformDynamic;
    } else if (buf_info.type == BufferType::kStorageDynamic) {
      type = BufferCommand::BufferType::kSSBODynamic;
    } else if (buf_info.type != BufferType::kStorage) {
      return Result("Vulkan: CreatePipeline - unknown buffer type: " +
                    std::to_string(static_cast<uint32_t>(buf_info.type)));
    }

    auto cmd = MakeUnique<BufferCommand>(type, pipeline);
    cmd->SetDescriptorSet(buf_info.descriptor_set);
    cmd->SetBinding(buf_info.binding);
    cmd->SetBaseMipLevel(buf_info.base_mip_level);
    cmd->SetDynamicOffset(buf_info.dynamic_offset);
    cmd->SetDescriptorOffset(buf_info.descriptor_offset);
    cmd->SetDescriptorRange(buf_info.descriptor_range);
    cmd->SetBuffer(buf_info.buffer);
    cmd->SetSampler(buf_info.sampler);

    if (cmd->GetValues().empty()) {
      cmd->GetBuffer()->SetSizeInElements(cmd->GetBuffer()->ElementCount());
    } else {
      cmd->GetBuffer()->SetDataWithOffset(cmd->GetValues(), cmd->GetOffset());
    }

    r = info.vk_pipeline->AddBufferDescriptor(cmd.get());
    if (!r.IsSuccess())
      return r;
  }

  for (const auto& sampler_info : pipeline->GetSamplers()) {
    auto cmd = MakeUnique<SamplerCommand>(pipeline);
    cmd->SetDescriptorSet(sampler_info.descriptor_set);
    cmd->SetBinding(sampler_info.binding);
    cmd->SetSampler(sampler_info.sampler);

    r = info.vk_pipeline->AddSamplerDescriptor(cmd.get());
    if (!r.IsSuccess())
      return r;
  }

  return {};
}

Result EngineVulkan::SetShader(amber::Pipeline* pipeline,
                               const amber::Pipeline::ShaderInfo& shader) {
  const auto type = shader.GetShaderType();
  const auto& data = shader.GetData();
  const auto shader_name = shader.GetShader()->GetName();
  auto& info = pipeline_map_[pipeline];

  auto it = info.shader_info.find(type);
  if (it != info.shader_info.end())
    return Result("Vulkan::Setting Duplicated Shader Types Fail");

  VkShaderModule shader_module;
  if (shaders_.find(shader_name) != shaders_.end()) {
    shader_module = shaders_[shader_name];
  } else {
    VkShaderModuleCreateInfo create_info = VkShaderModuleCreateInfo();
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = data.size() * sizeof(uint32_t);
    create_info.pCode = data.data();

    if (device_->GetPtrs()->vkCreateShaderModule(
            device_->GetVkDevice(), &create_info, nullptr, &shader_module) !=
        VK_SUCCESS) {
      return Result("Vulkan::Calling vkCreateShaderModule Fail");
    }

    shaders_[shader_name] = shader_module;
  }

  info.shader_info[type].shader = shader_module;

  for (auto& shader_info : pipeline->GetShaders()) {
    if (shader_info.GetShaderType() != type)
      continue;

    const auto required_subgroup_size_setting =
        shader_info.GetRequiredSubgroupSizeSetting();
    uint32_t required_subgroup_size_uint = 0;
    switch (required_subgroup_size_setting) {
      case amber::Pipeline::ShaderInfo::RequiredSubgroupSizeSetting::
          kSetToMinimumSize:
        required_subgroup_size_uint = device_->GetMinSubgroupSize();
        break;
      case amber::Pipeline::ShaderInfo::RequiredSubgroupSizeSetting::
          kSetToMaximumSize:
        required_subgroup_size_uint = device_->GetMaxSubgroupSize();
        break;
      default:
        required_subgroup_size_uint = shader_info.GetRequiredSubgroupSize();
        break;
    }
    if (required_subgroup_size_uint > 0) {
      if (!device_->IsRequiredSubgroupSizeSupported(
              type, required_subgroup_size_uint)) {
        return Result(
            "Vulkan::Setting Required subgroup size is not supported by the "
            "device.");
      }
    }
    info.shader_info[type].required_subgroup_size = required_subgroup_size_uint;

    info.shader_info[type].create_flags = 0;
    if (shader_info.GetVaryingSubgroupSize()) {
      info.shader_info[type].create_flags |=
          VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT_EXT;
    }
    if (shader_info.GetRequireFullSubgroups()) {
      info.shader_info[type].create_flags |=
          VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT;
    }

    const auto& shader_spec_info = shader_info.GetSpecialization();
    if (shader_spec_info.empty())
      continue;

    auto& entries = info.shader_info[type].specialization_entries;
    entries.reset(new std::vector<VkSpecializationMapEntry>());
    auto& entry_data = info.shader_info[type].specialization_data;
    entry_data.reset(new std::vector<uint32_t>());
    uint32_t i = 0;
    for (auto pair : shader_spec_info) {
      entries->push_back({pair.first,
                          static_cast<uint32_t>(i * sizeof(uint32_t)),
                          static_cast<uint32_t>(sizeof(uint32_t))});
      entry_data->push_back(pair.second);
      ++i;
    }
    auto& spec_info = info.shader_info[type].specialization_info;
    spec_info.reset(new VkSpecializationInfo());
    spec_info->mapEntryCount = static_cast<uint32_t>(shader_spec_info.size());
    spec_info->pMapEntries = entries->data();
    spec_info->dataSize = sizeof(uint32_t) * shader_spec_info.size();
    spec_info->pData = entry_data->data();
  }

  return {};
}

Result EngineVulkan::GetVkShaderStageInfo(
    amber::Pipeline* pipeline,
    std::vector<VkPipelineShaderStageCreateInfo>* out) {
  auto& info = pipeline_map_[pipeline];

  std::vector<VkPipelineShaderStageCreateInfo> stage_info(
      info.shader_info.size());
  uint32_t stage_count = 0;
  for (auto& it : info.shader_info) {
    VkShaderStageFlagBits stage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
    Result r = ToVkShaderStage(it.first, &stage);
    if (!r.IsSuccess())
      return r;

    stage_info[stage_count] = VkPipelineShaderStageCreateInfo();
    stage_info[stage_count].sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stage_info[stage_count].flags = it.second.create_flags;
    stage_info[stage_count].stage = stage;
    stage_info[stage_count].module = it.second.shader;
    stage_info[stage_count].pName = nullptr;
    if (it.second.specialization_entries &&
        !it.second.specialization_entries->empty()) {
      stage_info[stage_count].pSpecializationInfo =
          it.second.specialization_info.get();
    }

    if (stage == VK_SHADER_STAGE_COMPUTE_BIT &&
        it.second.required_subgroup_size > 0) {
      VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT* pSubgroupSize =
          new VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT();
      pSubgroupSize->sType =
          VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_REQUIRED_SUBGROUP_SIZE_CREATE_INFO_EXT;  // NOLINT(whitespace/line_length)
      pSubgroupSize->pNext = nullptr;
      pSubgroupSize->requiredSubgroupSize = it.second.required_subgroup_size;
      stage_info[stage_count].pNext = pSubgroupSize;
    }
    ++stage_count;
  }
  *out = stage_info;
  return {};
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

  float x = command->GetX();
  float y = command->GetY();
  float width = command->GetWidth();
  float height = command->GetHeight();

  if (command->IsOrtho()) {
    const float frame_width = static_cast<float>(graphics->GetWidth());
    const float frame_height = static_cast<float>(graphics->GetHeight());
    x = ((x / frame_width) * 2.0f) - 1.0f;
    y = ((y / frame_height) * 2.0f) - 1.0f;
    width = (width / frame_width) * 2.0f;
    height = (height / frame_height) * 2.0f;
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

  // |format| is not Format for frame buffer but for vertex buffer.
  // Since draw rect command contains its vertex information and it
  // does not include a format of vertex buffer, we can choose any
  // one that is suitable. We use VK_FORMAT_R32G32_SFLOAT for it.
  TypeParser parser;
  auto type = parser.Parse("R32G32_SFLOAT");
  Format fmt(type.get());

  auto buf = MakeUnique<Buffer>();
  buf->SetFormat(&fmt);
  buf->SetData(std::move(values));

  auto vertex_buffer = MakeUnique<VertexBuffer>(device_.get());
  vertex_buffer->SetData(0, buf.get(), InputRate::kVertex, buf->GetFormat(), 0,
                         buf->GetFormat()->SizeInBytes());

  DrawArraysCommand draw(command->GetPipeline(), *command->GetPipelineData());
  draw.SetTopology(command->IsPatch() ? Topology::kPatchList
                                      : Topology::kTriangleStrip);
  draw.SetFirstVertexIndex(0);
  draw.SetVertexCount(4);
  draw.SetInstanceCount(1);

  Result r = graphics->Draw(&draw, vertex_buffer.get());
  if (!r.IsSuccess())
    return r;

  return {};
}

Result EngineVulkan::DoDrawGrid(const DrawGridCommand* command) {
  auto& info = pipeline_map_[command->GetPipeline()];
  if (!info.vk_pipeline->IsGraphics())
    return Result("Vulkan::DrawGrid for Non-Graphics Pipeline");

  auto* graphics = info.vk_pipeline->AsGraphics();

  float x = command->GetX();
  float y = command->GetY();
  float width = command->GetWidth();
  float height = command->GetHeight();
  const uint32_t columns = command->GetColumns();
  const uint32_t rows = command->GetRows();
  const uint32_t vertices =
      columns * rows * kVerticesPerTriangle * kTrianglesPerCell;

  // Ortho calculation
  const float frame_width = static_cast<float>(graphics->GetWidth());
  const float frame_height = static_cast<float>(graphics->GetHeight());
  x = ((x / frame_width) * 2.0f) - 1.0f;
  y = ((y / frame_height) * 2.0f) - 1.0f;
  width = (width / frame_width) * 2.0f;
  height = (height / frame_height) * 2.0f;

  std::vector<Value> values(vertices * 2);

  const float cell_width = width / static_cast<float>(columns);
  const float cell_height = height / static_cast<float>(rows);

  for (uint32_t i = 0, c = 0; i < rows; i++) {
    for (uint32_t j = 0; j < columns; j++, c += 12) {
      // Calculate corners
      float x0 = x + cell_width * static_cast<float>(j);
      float y0 = y + cell_height * static_cast<float>(i);
      float x1 = x + cell_width * static_cast<float>(j + 1);
      float y1 = y + cell_height * static_cast<float>(i + 1);

      // Bottom right
      values[c + 0].SetDoubleValue(static_cast<double>(x1));
      values[c + 1].SetDoubleValue(static_cast<double>(y1));
      // Bottom left
      values[c + 2].SetDoubleValue(static_cast<double>(x0));
      values[c + 3].SetDoubleValue(static_cast<double>(y1));
      // Top left
      values[c + 4].SetDoubleValue(static_cast<double>(x0));
      values[c + 5].SetDoubleValue(static_cast<double>(y0));
      // Bottom right
      values[c + 6].SetDoubleValue(static_cast<double>(x1));
      values[c + 7].SetDoubleValue(static_cast<double>(y1));
      // Top left
      values[c + 8].SetDoubleValue(static_cast<double>(x0));
      values[c + 9].SetDoubleValue(static_cast<double>(y0));
      // Top right
      values[c + 10].SetDoubleValue(static_cast<double>(x1));
      values[c + 11].SetDoubleValue(static_cast<double>(y0));
    }
  }

  // |format| is not Format for frame buffer but for vertex buffer.
  // Since draw rect command contains its vertex information and it
  // does not include a format of vertex buffer, we can choose any
  // one that is suitable. We use VK_FORMAT_R32G32_SFLOAT for it.
  TypeParser parser;
  auto type = parser.Parse("R32G32_SFLOAT");
  Format fmt(type.get());

  auto buf = MakeUnique<Buffer>();
  buf->SetFormat(&fmt);
  buf->SetData(std::move(values));

  auto vertex_buffer = MakeUnique<VertexBuffer>(device_.get());
  vertex_buffer->SetData(0, buf.get(), InputRate::kVertex, buf->GetFormat(), 0,
                         buf->GetFormat()->SizeInBytes());

  DrawArraysCommand draw(command->GetPipeline(), *command->GetPipelineData());
  draw.SetTopology(Topology::kTriangleList);
  draw.SetFirstVertexIndex(0);
  draw.SetVertexCount(vertices);
  draw.SetInstanceCount(1);

  Result r = graphics->Draw(&draw, vertex_buffer.get());
  if (!r.IsSuccess())
    return r;

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

  VkShaderStageFlagBits stage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
  Result r = ToVkShaderStage(command->GetShaderType(), &stage);
  if (!r.IsSuccess())
    return r;

  info.vk_pipeline->SetEntryPointName(stage, command->GetEntryPointName());
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

Result EngineVulkan::DoBuffer(const BufferCommand* cmd) {
  if (!device_->IsDescriptorSetInBounds(cmd->GetDescriptorSet())) {
    return Result(
        "Vulkan::DoBuffer exceed maxBoundDescriptorSets limit of physical "
        "device");
  }
  if (cmd->GetValues().empty()) {
    cmd->GetBuffer()->SetSizeInElements(cmd->GetBuffer()->ElementCount());
  } else {
    cmd->GetBuffer()->SetDataWithOffset(cmd->GetValues(), cmd->GetOffset());
  }
  if (cmd->IsPushConstant()) {
    auto& info = pipeline_map_[cmd->GetPipeline()];
    return info.vk_pipeline->AddPushConstantBuffer(cmd->GetBuffer(),
                                                   cmd->GetOffset());
  }
  return {};
}

}  // namespace vulkan
}  // namespace amber
