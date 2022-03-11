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

#include "src/vulkan/pipeline.h"

#include <algorithm>
#include <limits>
#include <utility>

#include "src/command.h"
#include "src/engine.h"
#include "src/make_unique.h"
#include "src/vulkan/buffer_descriptor.h"
#include "src/vulkan/compute_pipeline.h"
#include "src/vulkan/device.h"
#include "src/vulkan/graphics_pipeline.h"
#include "src/vulkan/image_descriptor.h"
#include "src/vulkan/sampler_descriptor.h"

namespace amber {
namespace vulkan {
namespace {

const char* kDefaultEntryPointName = "main";

}  // namespace

Pipeline::Pipeline(
    PipelineType type,
    Device* device,
    uint32_t fence_timeout_ms,
    const std::vector<VkPipelineShaderStageCreateInfo>& shader_stage_info)
    : device_(device),
      pipeline_type_(type),
      shader_stage_info_(shader_stage_info),
      fence_timeout_ms_(fence_timeout_ms) {}

Pipeline::~Pipeline() {
  // Command must be reset before we destroy descriptors or we get a validation
  // error.
  command_ = nullptr;

  for (auto& info : descriptor_set_info_) {
    if (info.layout != VK_NULL_HANDLE) {
      device_->GetPtrs()->vkDestroyDescriptorSetLayout(device_->GetVkDevice(),
                                                       info.layout, nullptr);
    }

    if (info.empty)
      continue;

    if (info.pool != VK_NULL_HANDLE) {
      device_->GetPtrs()->vkDestroyDescriptorPool(device_->GetVkDevice(),
                                                  info.pool, nullptr);
    }
  }
}

GraphicsPipeline* Pipeline::AsGraphics() {
  return static_cast<GraphicsPipeline*>(this);
}

ComputePipeline* Pipeline::AsCompute() {
  return static_cast<ComputePipeline*>(this);
}

Result Pipeline::Initialize(CommandPool* pool) {
  push_constant_ = MakeUnique<PushConstant>(device_);

  command_ = MakeUnique<CommandBuffer>(device_, pool);
  return command_->Initialize();
}

Result Pipeline::CreateDescriptorSetLayouts() {
  for (auto& info : descriptor_set_info_) {
    VkDescriptorSetLayoutCreateInfo desc_info =
        VkDescriptorSetLayoutCreateInfo();
    desc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

    // If there are no descriptors for this descriptor set we only
    // need to create its layout and there will be no bindings.
    std::vector<VkDescriptorSetLayoutBinding> bindings;
    for (auto& desc : info.descriptors) {
      bindings.emplace_back();
      bindings.back().binding = desc->GetBinding();
      bindings.back().descriptorType = desc->GetVkDescriptorType();
      bindings.back().descriptorCount = desc->GetDescriptorCount();
      bindings.back().stageFlags = VK_SHADER_STAGE_ALL;
    }
    desc_info.bindingCount = static_cast<uint32_t>(bindings.size());
    desc_info.pBindings = bindings.data();

    if (device_->GetPtrs()->vkCreateDescriptorSetLayout(
            device_->GetVkDevice(), &desc_info, nullptr, &info.layout) !=
        VK_SUCCESS) {
      return Result("Vulkan::Calling vkCreateDescriptorSetLayout Fail");
    }
  }

  return {};
}

Result Pipeline::CreateDescriptorPools() {
  for (auto& info : descriptor_set_info_) {
    if (info.empty)
      continue;

    std::vector<VkDescriptorPoolSize> pool_sizes;
    for (auto& desc : info.descriptors) {
      VkDescriptorType type = desc->GetVkDescriptorType();
      auto it = find_if(pool_sizes.begin(), pool_sizes.end(),
                        [&type](const VkDescriptorPoolSize& size) {
                          return size.type == type;
                        });
      if (it != pool_sizes.end()) {
        it->descriptorCount += desc->GetDescriptorCount();
        continue;
      }

      pool_sizes.emplace_back();
      pool_sizes.back().type = type;
      pool_sizes.back().descriptorCount = desc->GetDescriptorCount();
    }

    VkDescriptorPoolCreateInfo pool_info = VkDescriptorPoolCreateInfo();
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.maxSets = 1;
    pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
    pool_info.pPoolSizes = pool_sizes.data();

    if (device_->GetPtrs()->vkCreateDescriptorPool(device_->GetVkDevice(),
                                                   &pool_info, nullptr,
                                                   &info.pool) != VK_SUCCESS) {
      return Result("Vulkan::Calling vkCreateDescriptorPool Fail");
    }
  }

  return {};
}

Result Pipeline::CreateDescriptorSets() {
  for (size_t i = 0; i < descriptor_set_info_.size(); ++i) {
    if (descriptor_set_info_[i].empty)
      continue;

    VkDescriptorSetAllocateInfo desc_set_info = VkDescriptorSetAllocateInfo();
    desc_set_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    desc_set_info.descriptorPool = descriptor_set_info_[i].pool;
    desc_set_info.descriptorSetCount = 1;
    desc_set_info.pSetLayouts = &descriptor_set_info_[i].layout;

    VkDescriptorSet desc_set = VK_NULL_HANDLE;
    if (device_->GetPtrs()->vkAllocateDescriptorSets(
            device_->GetVkDevice(), &desc_set_info, &desc_set) != VK_SUCCESS) {
      return Result("Vulkan::Calling vkAllocateDescriptorSets Fail");
    }
    descriptor_set_info_[i].vk_desc_set = desc_set;
  }

  return {};
}

Result Pipeline::CreateVkPipelineLayout(VkPipelineLayout* pipeline_layout) {
  Result r = CreateVkDescriptorRelatedObjectsIfNeeded();
  if (!r.IsSuccess())
    return r;

  std::vector<VkDescriptorSetLayout> descriptor_set_layouts;
  for (const auto& desc_set : descriptor_set_info_)
    descriptor_set_layouts.push_back(desc_set.layout);

  VkPipelineLayoutCreateInfo pipeline_layout_info =
      VkPipelineLayoutCreateInfo();
  pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipeline_layout_info.setLayoutCount =
      static_cast<uint32_t>(descriptor_set_layouts.size());
  pipeline_layout_info.pSetLayouts = descriptor_set_layouts.data();

  VkPushConstantRange push_const_range =
      push_constant_->GetVkPushConstantRange();
  if (push_const_range.size > 0) {
    pipeline_layout_info.pushConstantRangeCount = 1U;
    pipeline_layout_info.pPushConstantRanges = &push_const_range;
  }

  if (device_->GetPtrs()->vkCreatePipelineLayout(
          device_->GetVkDevice(), &pipeline_layout_info, nullptr,
          pipeline_layout) != VK_SUCCESS) {
    return Result("Vulkan::Calling vkCreatePipelineLayout Fail");
  }

  return {};
}

Result Pipeline::CreateVkDescriptorRelatedObjectsIfNeeded() {
  if (descriptor_related_objects_already_created_)
    return {};

  Result r = CreateDescriptorSetLayouts();
  if (!r.IsSuccess())
    return r;

  r = CreateDescriptorPools();
  if (!r.IsSuccess())
    return r;

  r = CreateDescriptorSets();
  if (!r.IsSuccess())
    return r;

  descriptor_related_objects_already_created_ = true;
  return {};
}

void Pipeline::UpdateDescriptorSetsIfNeeded() {
  for (auto& info : descriptor_set_info_) {
    for (auto& desc : info.descriptors)
      desc->UpdateDescriptorSetIfNeeded(info.vk_desc_set);
  }
}

Result Pipeline::RecordPushConstant(const VkPipelineLayout& pipeline_layout) {
  return push_constant_->RecordPushConstantVkCommand(command_.get(),
                                                     pipeline_layout);
}

Result Pipeline::AddPushConstantBuffer(const Buffer* buf, uint32_t offset) {
  if (!buf)
    return Result("Missing push constant buffer data");
  return push_constant_->AddBuffer(buf, offset);
}

Result Pipeline::GetDescriptorSlot(uint32_t desc_set,
                                   uint32_t binding,
                                   Descriptor** desc) {
  *desc = nullptr;

  if (desc_set >= descriptor_set_info_.size()) {
    for (size_t i = descriptor_set_info_.size();
         i <= static_cast<size_t>(desc_set); ++i) {
      descriptor_set_info_.emplace_back();
    }
  }

  if (descriptor_set_info_[desc_set].empty &&
      descriptor_related_objects_already_created_) {
    return Result(
        "Vulkan: Pipeline descriptor related objects were already created but "
        "try to put data on empty descriptor set '" +
        std::to_string(desc_set) +
        "'. Note that all used descriptor sets must be allocated before the "
        "first compute or draw.");
  }
  descriptor_set_info_[desc_set].empty = false;

  auto& descriptors = descriptor_set_info_[desc_set].descriptors;
  for (auto& descriptor : descriptors) {
    if (descriptor->GetBinding() == binding)
      *desc = descriptor.get();
  }

  return {};
}

Result Pipeline::AddDescriptorBuffer(Buffer* amber_buffer) {
  // Don't add the buffer if it's already added.
  const auto& buffer = std::find_if(
      descriptor_buffers_.begin(), descriptor_buffers_.end(),
      [&](const Buffer* buf) { return buf == amber_buffer; });
  if (buffer != descriptor_buffers_.end()) {
    return {};
  }
  descriptor_buffers_.push_back(amber_buffer);
  return {};
}

Result Pipeline::AddBufferDescriptor(const BufferCommand* cmd) {
  if (cmd == nullptr)
    return Result("Pipeline::AddBufferDescriptor BufferCommand is nullptr");
  if (!cmd->IsSSBO() && !cmd->IsUniform() && !cmd->IsStorageImage() &&
      !cmd->IsSampledImage() && !cmd->IsCombinedImageSampler() &&
      !cmd->IsUniformTexelBuffer() && !cmd->IsStorageTexelBuffer() &&
      !cmd->IsUniformDynamic() && !cmd->IsSSBODynamic()) {
    return Result("Pipeline::AddBufferDescriptor not supported buffer type");
  }

  Descriptor* desc;
  Result r =
      GetDescriptorSlot(cmd->GetDescriptorSet(), cmd->GetBinding(), &desc);
  if (!r.IsSuccess())
    return r;

  auto& descriptors = descriptor_set_info_[cmd->GetDescriptorSet()].descriptors;

  bool is_image = false;
  DescriptorType desc_type = DescriptorType::kUniformBuffer;

  if (cmd->IsStorageImage()) {
    desc_type = DescriptorType::kStorageImage;
    is_image = true;
  } else if (cmd->IsSampledImage()) {
    desc_type = DescriptorType::kSampledImage;
    is_image = true;
  } else if (cmd->IsCombinedImageSampler()) {
    desc_type = DescriptorType::kCombinedImageSampler;
    is_image = true;
  } else if (cmd->IsUniformTexelBuffer()) {
    desc_type = DescriptorType::kUniformTexelBuffer;
  } else if (cmd->IsStorageTexelBuffer()) {
    desc_type = DescriptorType::kStorageTexelBuffer;
  } else if (cmd->IsSSBO()) {
    desc_type = DescriptorType::kStorageBuffer;
  } else if (cmd->IsUniformDynamic()) {
    desc_type = DescriptorType::kUniformBufferDynamic;
  } else if (cmd->IsSSBODynamic()) {
    desc_type = DescriptorType::kStorageBufferDynamic;
  }

  if (desc == nullptr) {
    if (is_image) {
      auto image_desc = MakeUnique<ImageDescriptor>(
          cmd->GetBuffer(), desc_type, device_, cmd->GetBaseMipLevel(),
          cmd->GetDescriptorSet(), cmd->GetBinding(), this);
      if (cmd->IsCombinedImageSampler())
        image_desc->SetAmberSampler(cmd->GetSampler());

      descriptors.push_back(std::move(image_desc));
    } else {
      auto buffer_desc = MakeUnique<BufferDescriptor>(
          cmd->GetBuffer(), desc_type, device_, cmd->GetDescriptorSet(),
          cmd->GetBinding(), this);
      descriptors.push_back(std::move(buffer_desc));
    }
    AddDescriptorBuffer(cmd->GetBuffer());
    desc = descriptors.back().get();
  } else {
    if (desc->GetDescriptorType() != desc_type) {
      return Result(
          "Descriptors bound to the same binding needs to have matching "
          "descriptor types");
    }
    desc->AsBufferBackedDescriptor()->AddAmberBuffer(cmd->GetBuffer());
    AddDescriptorBuffer(cmd->GetBuffer());
  }

  if (cmd->IsUniformDynamic() || cmd->IsSSBODynamic())
    desc->AsBufferDescriptor()->AddDynamicOffset(cmd->GetDynamicOffset());

  if (cmd->IsUniform() || cmd->IsUniformDynamic() || cmd->IsSSBO() ||
      cmd->IsSSBODynamic()) {
    desc->AsBufferDescriptor()->AddDescriptorOffset(cmd->GetDescriptorOffset());
    desc->AsBufferDescriptor()->AddDescriptorRange(cmd->GetDescriptorRange());
  }

  if (cmd->IsSSBO() && !desc->IsStorageBuffer()) {
    return Result(
        "Vulkan::AddBufferDescriptor BufferCommand for SSBO uses wrong "
        "descriptor "
        "set and binding");
  }

  if (cmd->IsUniform() && !desc->IsUniformBuffer()) {
    return Result(
        "Vulkan::AddBufferDescriptor BufferCommand for UBO uses wrong "
        "descriptor set "
        "and binding");
  }

  return {};
}

Result Pipeline::AddSamplerDescriptor(const SamplerCommand* cmd) {
  if (cmd == nullptr)
    return Result("Pipeline::AddSamplerDescriptor SamplerCommand is nullptr");

  Descriptor* desc;
  Result r =
      GetDescriptorSlot(cmd->GetDescriptorSet(), cmd->GetBinding(), &desc);
  if (!r.IsSuccess())
    return r;

  auto& descriptors = descriptor_set_info_[cmd->GetDescriptorSet()].descriptors;

  if (desc == nullptr) {
    auto sampler_desc = MakeUnique<SamplerDescriptor>(
        cmd->GetSampler(), DescriptorType::kSampler, device_,
        cmd->GetDescriptorSet(), cmd->GetBinding());
    descriptors.push_back(std::move(sampler_desc));
  } else {
    if (desc->GetDescriptorType() != DescriptorType::kSampler) {
      return Result(
          "Descriptors bound to the same binding needs to have matching "
          "descriptor types");
    }
    desc->AsSamplerDescriptor()->AddAmberSampler(cmd->GetSampler());
  }

  return {};
}

Result Pipeline::SendDescriptorDataToDeviceIfNeeded() {
  {
    CommandBufferGuard guard(GetCommandBuffer());
    if (!guard.IsRecording())
      return guard.GetResult();

    for (auto& info : descriptor_set_info_) {
      for (auto& desc : info.descriptors) {
        Result r = desc->CreateResourceIfNeeded();
        if (!r.IsSuccess())
          return r;
      }
    }

    // Initialize transfer buffers / images.
    for (auto buffer : descriptor_buffers_) {
      if (descriptor_transfer_resources_.count(buffer) == 0) {
        return Result(
            "Vulkan: Pipeline::SendDescriptorDataToDeviceIfNeeded() "
            "descriptor's transfer resource is not found");
      }
      Result r = descriptor_transfer_resources_[buffer]->Initialize();
      if (!r.IsSuccess())
         return r;
    }

    // Note that if a buffer for a descriptor is host accessible and
    // does not need to record a command to copy data to device, it
    // directly writes data to the buffer. The direct write must be
    // done after resizing backed buffer i.e., copying data to the new
    // buffer from the old one. Thus, we must submit commands here to
    // guarantee this.
    Result r = guard.Submit(GetFenceTimeout());
    if (!r.IsSuccess())
      return r;
  }

  CommandBufferGuard guard(GetCommandBuffer());
  if (!guard.IsRecording())
    return guard.GetResult();

  // Copy descriptor data to transfer resources.
  for (auto& buffer : descriptor_buffers_) {
    if (auto transfer_buffer =
            descriptor_transfer_resources_[buffer]->AsTransferBuffer()) {
      BufferBackedDescriptor::RecordCopyBufferDataToTransferResourceIfNeeded(
          GetCommandBuffer(), buffer, transfer_buffer);
    } else if (auto transfer_image =
                   descriptor_transfer_resources_[buffer]->AsTransferImage()) {
      transfer_image->ImageBarrier(GetCommandBuffer(),
                                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                   VK_PIPELINE_STAGE_TRANSFER_BIT);

      BufferBackedDescriptor::RecordCopyBufferDataToTransferResourceIfNeeded(
          GetCommandBuffer(), buffer, transfer_image);

      transfer_image->ImageBarrier(GetCommandBuffer(), VK_IMAGE_LAYOUT_GENERAL,
                                   VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
    } else {
      return Result(
          "Vulkan: Pipeline::SendDescriptorDataToDeviceIfNeeded() "
          "this should be unreachable");
    }
  }
  return guard.Submit(GetFenceTimeout());
}

void Pipeline::BindVkDescriptorSets(const VkPipelineLayout& pipeline_layout) {
  for (size_t i = 0; i < descriptor_set_info_.size(); ++i) {
    if (descriptor_set_info_[i].empty)
      continue;

    // Sort descriptors by binding number to get correct order of dynamic
    // offsets.
    typedef std::pair<uint32_t, std::vector<uint32_t>> binding_offsets_pair;
    std::vector<binding_offsets_pair> binding_offsets;
    for (const auto& desc : descriptor_set_info_[i].descriptors) {
      binding_offsets.push_back(
          {desc->GetBinding(), desc->GetDynamicOffsets()});
    }

    std::sort(std::begin(binding_offsets), std::end(binding_offsets),
              [](const binding_offsets_pair& a, const binding_offsets_pair& b) {
                return a.first < b.first;
              });

    // Add the sorted dynamic offsets.
    std::vector<uint32_t> dynamic_offsets;
    for (const auto& binding_offset : binding_offsets) {
      for (auto offset : binding_offset.second) {
        dynamic_offsets.push_back(offset);
      }
    }

    device_->GetPtrs()->vkCmdBindDescriptorSets(
        command_->GetVkCommandBuffer(),
        IsGraphics() ? VK_PIPELINE_BIND_POINT_GRAPHICS
                     : VK_PIPELINE_BIND_POINT_COMPUTE,
        pipeline_layout, static_cast<uint32_t>(i), 1,
        &descriptor_set_info_[i].vk_desc_set,
        static_cast<uint32_t>(dynamic_offsets.size()), dynamic_offsets.data());
  }
}

Result Pipeline::ReadbackDescriptorsToHostDataQueue() {
  // Record required commands to copy the data to a host visible buffer.
  {
    CommandBufferGuard guard(GetCommandBuffer());
    if (!guard.IsRecording())
      return guard.GetResult();

    for (auto& buffer : descriptor_buffers_) {
      if (descriptor_transfer_resources_.count(buffer) == 0) {
        return Result(
            "Vulkan: Pipeline::ReadbackDescriptorsToHostDataQueue() "
            "descriptor's transfer resource is not found");
      }
      if (auto transfer_buffer =
              descriptor_transfer_resources_[buffer]->AsTransferBuffer()) {
        Result r = BufferBackedDescriptor::RecordCopyTransferResourceToHost(
            GetCommandBuffer(), transfer_buffer);
        if (!r.IsSuccess())
          return r;
      } else if (auto transfer_image = descriptor_transfer_resources_[buffer]
                                           ->AsTransferImage()) {
        transfer_image->ImageBarrier(GetCommandBuffer(),
                                     VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                     VK_PIPELINE_STAGE_TRANSFER_BIT);
        Result r = BufferBackedDescriptor::RecordCopyTransferResourceToHost(
            GetCommandBuffer(), transfer_image);
        if (!r.IsSuccess())
          return r;
      } else {
        return Result(
            "Vulkan: Pipeline::ReadbackDescriptorsToHostDataQueue() "
            "this should be unreachable");
      }
    }

    Result r = guard.Submit(GetFenceTimeout());
    if (!r.IsSuccess())
      return r;
  }

  // Move data from transfer buffers to output buffers.
  for (auto& buffer : descriptor_buffers_) {
    auto& transfer_resource = descriptor_transfer_resources_[buffer];
    Result r = BufferBackedDescriptor::MoveTransferResourceToBufferOutput(
        transfer_resource.get(), buffer);
    if (!r.IsSuccess())
      return r;
  }
  descriptor_transfer_resources_.clear();
  return {};
}

const char* Pipeline::GetEntryPointName(VkShaderStageFlagBits stage) const {
  auto it = entry_points_.find(stage);
  if (it != entry_points_.end())
    return it->second.c_str();

  return kDefaultEntryPointName;
}

}  // namespace vulkan
}  // namespace amber
