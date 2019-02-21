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
#include <cassert>
#include <limits>
#include <utility>

#include "src/command.h"
#include "src/engine.h"
#include "src/make_unique.h"
#include "src/vulkan/buffer_descriptor.h"
#include "src/vulkan/compute_pipeline.h"
#include "src/vulkan/device.h"
#include "src/vulkan/graphics_pipeline.h"

namespace amber {
namespace vulkan {
namespace {

const char* kDefaultEntryPointName = "main";

}  // namespace

Pipeline::Pipeline(
    PipelineType type,
    Device* device,
    const VkPhysicalDeviceProperties& properties,
    const VkPhysicalDeviceMemoryProperties& memory_properties,
    uint32_t fence_timeout_ms,
    const std::vector<VkPipelineShaderStageCreateInfo>& shader_stage_info)
    : device_(device),
      memory_properties_(memory_properties),
      pipeline_type_(type),
      physical_device_properties_(properties),
      shader_stage_info_(shader_stage_info),
      fence_timeout_ms_(fence_timeout_ms) {}

Pipeline::~Pipeline() = default;

GraphicsPipeline* Pipeline::AsGraphics() {
  return static_cast<GraphicsPipeline*>(this);
}

ComputePipeline* Pipeline::AsCompute() {
  return static_cast<ComputePipeline*>(this);
}

Result Pipeline::Initialize(CommandPool* pool, VkQueue queue) {
  push_constant_ = MakeUnique<PushConstant>(
      device_, physical_device_properties_.limits.maxPushConstantsSize);

  command_ = MakeUnique<CommandBuffer>(device_, pool, queue);
  return command_->Initialize();
}

void Pipeline::Shutdown() {
  if (command_) {
    Result r = command_->End();
    if (r.IsSuccess())
      command_->SubmitAndReset(fence_timeout_ms_);

    command_->Shutdown();
  }

  DestroyVkDescriptorAndPipelineRelatedObjects();
}

void Pipeline::DestroyVkDescriptorAndPipelineRelatedObjects() {
  for (auto& info : descriptor_set_info_) {
    if (info.layout != VK_NULL_HANDLE) {
      device_->GetPtrs()->vkDestroyDescriptorSetLayout(device_->GetDevice(),
                                                       info.layout, nullptr);
    }

    if (info.empty)
      continue;

    if (info.pool != VK_NULL_HANDLE) {
      device_->GetPtrs()->vkDestroyDescriptorPool(device_->GetDevice(),
                                                  info.pool, nullptr);
    }

    for (auto& desc : info.descriptors_) {
      if (desc)
        desc->Shutdown();
    }
  }

  ResetVkPipelineRelatedObjects();
}

void Pipeline::ResetVkPipelineRelatedObjects() {
  if (pipeline_layout_ != VK_NULL_HANDLE) {
    device_->GetPtrs()->vkDestroyPipelineLayout(device_->GetDevice(),
                                                pipeline_layout_, nullptr);
    pipeline_layout_ = VK_NULL_HANDLE;
  }

  if (pipeline_ != VK_NULL_HANDLE) {
    device_->GetPtrs()->vkDestroyPipeline(device_->GetDevice(), pipeline_,
                                          nullptr);
    pipeline_ = VK_NULL_HANDLE;
  }
}

Result Pipeline::CreateDescriptorSetLayouts() {
  for (auto& info : descriptor_set_info_) {
    VkDescriptorSetLayoutCreateInfo desc_info =
        VkDescriptorSetLayoutCreateInfo();
    desc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

    // If there are no descriptors for this descriptor set we only
    // need to create its layout and there will be no bindings.
    std::vector<VkDescriptorSetLayoutBinding> bindings;
    for (auto& desc : info.descriptors_) {
      bindings.emplace_back();
      bindings.back().binding = desc->GetBinding();
      bindings.back().descriptorType = ToVkDescriptorType(desc->GetType());
      bindings.back().descriptorCount = 1;
      bindings.back().stageFlags = VK_SHADER_STAGE_ALL;
    }
    desc_info.bindingCount = static_cast<uint32_t>(bindings.size());
    desc_info.pBindings = bindings.data();

    if (device_->GetPtrs()->vkCreateDescriptorSetLayout(
            device_->GetDevice(), &desc_info, nullptr, &info.layout) !=
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
    for (auto& desc : info.descriptors_) {
      auto type = ToVkDescriptorType(desc->GetType());
      auto it = find_if(pool_sizes.begin(), pool_sizes.end(),
                        [&type](const VkDescriptorPoolSize& size) {
                          return size.type == type;
                        });
      if (it != pool_sizes.end()) {
        it->descriptorCount += 1;
        continue;
      }

      pool_sizes.emplace_back();
      pool_sizes.back().type = type;
      pool_sizes.back().descriptorCount = 1;
    }

    VkDescriptorPoolCreateInfo pool_info = VkDescriptorPoolCreateInfo();
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.maxSets = 1;
    pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
    pool_info.pPoolSizes = pool_sizes.data();

    if (device_->GetPtrs()->vkCreateDescriptorPool(device_->GetDevice(),
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
            device_->GetDevice(), &desc_set_info, &desc_set) != VK_SUCCESS) {
      return Result("Vulkan::Calling vkAllocateDescriptorSets Fail");
    }
    descriptor_set_info_[i].vk_desc_set = desc_set;
  }

  return {};
}

Result Pipeline::CreatePipelineLayout() {
  std::vector<VkDescriptorSetLayout> descriptor_set_layouts;
  for (const auto& desc_set : descriptor_set_info_)
    descriptor_set_layouts.push_back(desc_set.layout);

  VkPipelineLayoutCreateInfo pipeline_layout_info =
      VkPipelineLayoutCreateInfo();
  pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipeline_layout_info.setLayoutCount =
      static_cast<uint32_t>(descriptor_set_layouts.size());
  pipeline_layout_info.pSetLayouts = descriptor_set_layouts.data();

  VkPushConstantRange push_const_range = push_constant_->GetPushConstantRange();
  if (push_const_range.size) {
    pipeline_layout_info.pushConstantRangeCount = 1U;
    pipeline_layout_info.pPushConstantRanges = &push_const_range;
  }

  if (device_->GetPtrs()->vkCreatePipelineLayout(
          device_->GetDevice(), &pipeline_layout_info, nullptr,
          &pipeline_layout_) != VK_SUCCESS) {
    return Result("Vulkan::Calling vkCreatePipelineLayout Fail");
  }

  return {};
}

Result Pipeline::CreateVkDescriptorRelatedObjectsAndPipelineLayoutIfNeeded() {
  if (descriptor_related_objects_already_created_) {
    return pipeline_layout_ == VK_NULL_HANDLE ? CreatePipelineLayout()
                                              : Result();
  }

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
  return CreatePipelineLayout();
}

Result Pipeline::UpdateDescriptorSetsIfNeeded() {
  for (auto& info : descriptor_set_info_) {
    for (auto& desc : info.descriptors_) {
      Result r = desc->UpdateDescriptorSetIfNeeded(info.vk_desc_set);
      if (!r.IsSuccess())
        return r;
    }
  }

  return {};
}

Result Pipeline::RecordPushConstant() {
  return push_constant_->RecordPushConstantVkCommand(command_.get(),
                                                     pipeline_layout_);
}

Result Pipeline::AddPushConstant(const BufferCommand* command) {
  if (!command->IsPushConstant())
    return Result(
        "Pipeline::AddPushConstant BufferCommand type is not push constant");

  ResetVkPipelineRelatedObjects();

  return push_constant_->AddBufferData(command);
}

Result Pipeline::AddDescriptor(const BufferCommand* buffer_command) {
  if (buffer_command == nullptr)
    return Result("Pipeline::AddDescriptor BufferCommand is nullptr");

  if (!buffer_command->IsSSBO() && !buffer_command->IsUniform())
    return Result("Pipeline::AddDescriptor not supported buffer type");

  const uint32_t desc_set = buffer_command->GetDescriptorSet();
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

  auto& descriptors = descriptor_set_info_[desc_set].descriptors_;
  Descriptor* desc = nullptr;
  for (auto& descriptor : descriptors) {
    if (descriptor->GetBinding() == buffer_command->GetBinding())
      desc = descriptor.get();
  }

  if (desc == nullptr) {
    auto desc_type = buffer_command->IsSSBO() ? DescriptorType::kStorageBuffer
                                              : DescriptorType::kUniformBuffer;
    auto buffer_desc = MakeUnique<BufferDescriptor>(
        desc_type, device_, buffer_command->GetDescriptorSet(),
        buffer_command->GetBinding());
    descriptors.push_back(std::move(buffer_desc));

    desc = descriptors.back().get();
  }

  if (buffer_command->IsSSBO() && !desc->IsStorageBuffer()) {
    return Result(
        "Vulkan::AddDescriptor BufferCommand for SSBO uses wrong descriptor "
        "set and binding");
  }

  if (buffer_command->IsUniform() && !desc->IsUniformBuffer()) {
    return Result(
        "Vulkan::AddDescriptor BufferCommand for UBO uses wrong descriptor set "
        "and binding");
  }

  desc->AddToBufferInputQueue(
      buffer_command->GetDatumType().GetType(), buffer_command->GetOffset(),
      buffer_command->GetSize(), buffer_command->GetValues());

  return {};
}

Result Pipeline::SendDescriptorDataToDeviceIfNeeded() {
  bool data_send_needed = false;
  for (auto& info : descriptor_set_info_) {
    for (auto& desc : info.descriptors_) {
      if (desc->HasDataNotSent()) {
        data_send_needed = true;
        break;
      }
    }

    if (data_send_needed)
      break;
  }

  if (!data_send_needed)
    return {};

  Result r = command_->BeginIfNotInRecording();
  if (!r.IsSuccess())
    return r;

  for (auto& info : descriptor_set_info_) {
    for (auto& desc : info.descriptors_) {
      r = desc->CreateResourceIfNeeded(memory_properties_);
      if (!r.IsSuccess())
        return r;
    }
  }

  r = command_->End();
  if (!r.IsSuccess())
    return r;

  // Note that if a buffer for a descriptor is host accessible and
  // does not need to record a command to copy data to device, it
  // directly writes data to the buffer. The direct write must be
  // done after resizing backed buffer i.e., copying data to the new
  // buffer from the old one. Thus, we must submit commands here to
  // guarantee this.
  r = command_->SubmitAndReset(GetFenceTimeout());
  if (!r.IsSuccess())
    return r;

  r = command_->BeginIfNotInRecording();
  if (!r.IsSuccess())
    return r;

  for (auto& info : descriptor_set_info_) {
    for (auto& desc : info.descriptors_) {
      r = desc->RecordCopyDataToResourceIfNeeded(command_.get());
      if (!r.IsSuccess())
        return r;
    }
  }

  return {};
}

void Pipeline::BindVkDescriptorSets() {
  for (size_t i = 0; i < descriptor_set_info_.size(); ++i) {
    if (descriptor_set_info_[i].empty)
      continue;

    device_->GetPtrs()->vkCmdBindDescriptorSets(
        command_->GetCommandBuffer(),
        IsGraphics() ? VK_PIPELINE_BIND_POINT_GRAPHICS
                     : VK_PIPELINE_BIND_POINT_COMPUTE,
        pipeline_layout_, static_cast<uint32_t>(i), 1,
        &descriptor_set_info_[i].vk_desc_set, 0, nullptr);
  }
}

void Pipeline::BindVkPipeline() {
  device_->GetPtrs()->vkCmdBindPipeline(command_->GetCommandBuffer(),
                                        IsGraphics()
                                            ? VK_PIPELINE_BIND_POINT_GRAPHICS
                                            : VK_PIPELINE_BIND_POINT_COMPUTE,
                                        pipeline_);
}

Result Pipeline::ReadbackDescriptorsToHostDataQueue() {
  Result r = command_->BeginIfNotInRecording();
  if (!r.IsSuccess())
    return r;

  for (auto& desc_set : descriptor_set_info_) {
    for (auto& desc : desc_set.descriptors_) {
      r = desc->RecordCopyDataToHost(command_.get());
      if (!r.IsSuccess())
        return r;
    }
  }

  r = command_->End();
  if (!r.IsSuccess())
    return r;

  r = command_->SubmitAndReset(GetFenceTimeout());
  if (!r.IsSuccess())
    return r;

  for (auto& desc_set : descriptor_set_info_) {
    for (auto& desc : desc_set.descriptors_) {
      r = desc->MoveResourceToBufferOutput();
      if (!r.IsSuccess())
        return r;
    }
  }

  return {};
}

Result Pipeline::GetDescriptorInfo(const uint32_t descriptor_set,
                                   const uint32_t binding,
                                   ResourceInfo* info) {
  assert(info);

  if (descriptor_set_info_.size() <= descriptor_set) {
    return Result(
        "Pipeline::GetDescriptorInfo no Descriptor class has given descriptor "
        "set: " +
        std::to_string(descriptor_set));
  }

  for (auto& desc : descriptor_set_info_[descriptor_set].descriptors_) {
    if (desc->GetBinding() == binding) {
      *info = desc->GetResourceInfo();
      return {};
    }
  }

  return Result("Vulkan::Pipeline descriptor with descriptor set: " +
                std::to_string(descriptor_set) +
                ", binding: " + std::to_string(binding) + " does not exist");
}

const char* Pipeline::GetEntryPointName(VkShaderStageFlagBits stage) const {
  auto it = entry_points_.find(stage);
  if (it != entry_points_.end())
    return it->second.c_str();

  return kDefaultEntryPointName;
}

Result Pipeline::ProcessCommands() {
  Result r = command_->BeginIfNotInRecording();
  if (!r.IsSuccess())
    return r;

  r = command_->End();
  if (!r.IsSuccess())
    return r;

  return command_->SubmitAndReset(GetFenceTimeout());
}

}  // namespace vulkan
}  // namespace amber
