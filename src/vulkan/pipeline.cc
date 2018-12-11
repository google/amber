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
#include "src/vulkan/graphics_pipeline.h"

namespace amber {
namespace vulkan {

Pipeline::Pipeline(
    PipelineType type,
    VkDevice device,
    const VkPhysicalDeviceMemoryProperties& properties,
    uint32_t fence_timeout_ms,
    const std::vector<VkPipelineShaderStageCreateInfo>& shader_stage_info)
    : device_(device),
      memory_properties_(properties),
      pipeline_type_(type),
      shader_stage_info_(shader_stage_info),
      fence_timeout_ms_(fence_timeout_ms) {}

Pipeline::~Pipeline() = default;

GraphicsPipeline* Pipeline::AsGraphics() {
  return static_cast<GraphicsPipeline*>(this);
}

ComputePipeline* Pipeline::AsCompute() {
  return static_cast<ComputePipeline*>(this);
}

Result Pipeline::InitializeCommandBuffer(VkCommandPool pool, VkQueue queue) {
  command_ = MakeUnique<CommandBuffer>(device_, pool, queue);
  Result r = command_->Initialize();
  if (!r.IsSuccess())
    return r;

  return {};
}

void Pipeline::Shutdown() {
  Result r = command_->End();
  if (r.IsSuccess())
    command_->SubmitAndReset(fence_timeout_ms_);

  for (size_t i = 0; i < descriptor_sets_.size(); ++i) {
    vkDestroyDescriptorSetLayout(device_, descriptor_sets_[i].layout, nullptr);
    if (descriptor_sets_[i].empty)
      continue;

    vkDestroyDescriptorPool(device_, descriptor_sets_[i].pool, nullptr);
  }

  command_->Shutdown();
  vkDestroyPipelineLayout(device_, pipeline_layout_, nullptr);
  for (size_t i = 0; i < descriptors_.size(); ++i)
    descriptors_[i]->Shutdown();
  vkDestroyPipeline(device_, pipeline_, nullptr);
}

void Pipeline::SortDescriptorsBySetAndBinding() {
  if (!need_sort_descriptors_)
    return;

  std::sort(descriptors_.begin(), descriptors_.end(),
            [](const std::unique_ptr<Descriptor>& a,
               const std::unique_ptr<Descriptor>& b) {
              if (a->GetDescriptorSet() == b->GetDescriptorSet())
                return a->GetBinding() < b->GetBinding();
              return a->GetDescriptorSet() < b->GetDescriptorSet();
            });
  need_sort_descriptors_ = false;
}

Result Pipeline::CreateDescriptorSetLayouts() {
  SortDescriptorsBySetAndBinding();

  size_t i = 0;
  uint32_t next_desc = 0;
  while (i < descriptors_.size()) {
    std::vector<VkDescriptorSetLayoutBinding> bindings;
    const uint32_t current_desc = descriptors_[i]->GetDescriptorSet();
    while (i < descriptors_.size() &&
           current_desc == descriptors_[i]->GetDescriptorSet()) {
      bindings.emplace_back();
      bindings.back().binding = descriptors_[i]->GetBinding();
      bindings.back().descriptorType =
          ToVkDescriptorType(descriptors_[i]->GetType());
      bindings.back().descriptorCount = 1;
      bindings.back().stageFlags = VK_SHADER_STAGE_ALL;
      ++i;
    }

    while (next_desc <= current_desc) {
      VkDescriptorSetLayoutCreateInfo desc_info = {};
      desc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

      // If two descriptor sets are discontinuous, we must put empty
      // descriptor set layouts between them.
      DescriptorSet descriptor_set = {};
      if (next_desc == current_desc) {
        desc_info.bindingCount = bindings.size();
        desc_info.pBindings = bindings.data();
        descriptor_set.empty = false;
      }

      VkDescriptorSetLayout desc_layout = VK_NULL_HANDLE;
      if (vkCreateDescriptorSetLayout(device_, &desc_info, nullptr,
                                      &desc_layout) != VK_SUCCESS) {
        return Result("Vulkan::Calling vkCreateDescriptorSetLayout Fail");
      }

      descriptor_set.layout = desc_layout;
      descriptor_sets_.push_back(descriptor_set);
      ++next_desc;
    }
  }

  return {};
}

Result Pipeline::CreateDescriptorPools() {
  SortDescriptorsBySetAndBinding();

  size_t i = 0;
  while (i < descriptors_.size()) {
    std::vector<VkDescriptorPoolSize> pool_sizes;
    const uint32_t current_desc = descriptors_[i]->GetDescriptorSet();
    while (i < descriptors_.size() &&
           current_desc == descriptors_[i]->GetDescriptorSet()) {
      auto type = ToVkDescriptorType(descriptors_[i]->GetType());
      auto it = find_if(pool_sizes.begin(), pool_sizes.end(),
                        [&type](const VkDescriptorPoolSize& size) {
                          return size.type == type;
                        });
      if (it != pool_sizes.end()) {
        it->descriptorCount += 1;
        ++i;
        continue;
      }

      pool_sizes.emplace_back();
      pool_sizes.back().type = type;
      pool_sizes.back().descriptorCount = 1;
      ++i;
    }

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.maxSets = 1;
    pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
    pool_info.pPoolSizes = pool_sizes.data();

    VkDescriptorPool desc_pool = VK_NULL_HANDLE;
    if (vkCreateDescriptorPool(device_, &pool_info, nullptr, &desc_pool) !=
        VK_SUCCESS) {
      return Result("Vulkan::Calling vkCreateDescriptorPool Fail");
    }

    if (static_cast<uint32_t>(descriptor_sets_.size()) <= current_desc) {
      return Result(
          "Pipeline::CreateDescriptorPools: More descriptor sets requested "
          "than descriptor set layouts provided");
    }

    if (descriptor_sets_[current_desc].empty) {
      return Result(
          "Pipeline::CreateDescriptorPools: Used a descriptor set with an "
          "empty layout");
    }

    descriptor_sets_[current_desc].pool = desc_pool;
  }

  return {};
}

Result Pipeline::CreateDescriptorSets() {
  for (size_t i = 0; i < descriptor_sets_.size(); ++i) {
    if (descriptor_sets_[i].empty)
      continue;

    VkDescriptorSetAllocateInfo desc_set_info = {};
    desc_set_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    desc_set_info.descriptorPool = descriptor_sets_[i].pool;
    desc_set_info.descriptorSetCount = 1;
    desc_set_info.pSetLayouts = &descriptor_sets_[i].layout;

    VkDescriptorSet desc_set = VK_NULL_HANDLE;
    if (vkAllocateDescriptorSets(device_, &desc_set_info, &desc_set) !=
        VK_SUCCESS) {
      return Result("Vulkan::Calling vkAllocateDescriptorSets Fail");
    }
    descriptor_sets_[i].vk_desc_set = desc_set;
  }

  return {};
}

Result Pipeline::CreatePipelineLayout() {
  std::vector<VkDescriptorSetLayout> descriptor_set_layouts;
  for (const auto& desc_set : descriptor_sets_)
    descriptor_set_layouts.push_back(desc_set.layout);

  VkPipelineLayoutCreateInfo pipeline_layout_info = {};
  pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipeline_layout_info.setLayoutCount =
      static_cast<uint32_t>(descriptor_set_layouts.size());
  pipeline_layout_info.pSetLayouts = descriptor_set_layouts.data();
  // TODO(jaebaek): Push constant for pipeline_layout_info.

  if (vkCreatePipelineLayout(device_, &pipeline_layout_info, nullptr,
                             &pipeline_layout_) != VK_SUCCESS) {
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

  return CreatePipelineLayout();
}

Result Pipeline::UpdateDescriptorSetsIfNeeded() {
  SortDescriptorsBySetAndBinding();

  size_t i = 0;
  while (i < descriptors_.size()) {
    for (const uint32_t current_desc = descriptors_[i]->GetDescriptorSet();
         i < descriptors_.size() &&
         current_desc == descriptors_[i]->GetDescriptorSet();
         ++i) {
      if (static_cast<uint32_t>(descriptor_sets_.size()) <= current_desc) {
        return Result(
            "Pipeline::UpdateDescriptorSetsIfNeeded: More descriptor sets "
            "requested than descriptor set layouts provided");
      }

      if (descriptor_sets_[current_desc].empty) {
        return Result(
            "Pipeline::UpdateDescriptorSetsIfNeeded: Used a descriptor set "
            "with an empty layout");
      }

      Result r = descriptors_[i]->UpdateDescriptorSetIfNeeded(
          descriptor_sets_[current_desc].vk_desc_set);
      if (!r.IsSuccess())
        return r;
    }
  }

  descriptor_related_objects_already_created_ = true;
  return {};
}

Result Pipeline::AddDescriptor(const BufferCommand* buffer_command) {
  if (!buffer_command->IsSSBO() && !buffer_command->IsUniform())
    return Result("Pipeline::AddDescriptor not supported buffer type");

  Descriptor* desc = nullptr;
  for (size_t i = 0; i < descriptors_.size(); ++i) {
    if (descriptors_[i]->GetDescriptorSet() ==
            buffer_command->GetDescriptorSet() &&
        descriptors_[i]->GetBinding() == buffer_command->GetBinding()) {
      desc = descriptors_[i].get();
    }
  }

  if (desc == nullptr) {
    auto desc_type = buffer_command->IsSSBO() ? DescriptorType::kStorageBuffer
                                              : DescriptorType::kUniformBuffer;
    auto buffer_desc = MakeUnique<BufferDescriptor>(
        desc_type, device_, buffer_command->GetDescriptorSet(),
        buffer_command->GetBinding());
    descriptors_.push_back(std::move(buffer_desc));
    need_sort_descriptors_ = true;

    desc = descriptors_.back().get();
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

  desc->AddToBufferDataQueue(
      buffer_command->GetDatumType().GetType(), buffer_command->GetOffset(),
      buffer_command->GetSize(), buffer_command->GetValues());

  return {};
}

Result Pipeline::SendDescriptorDataToDeviceIfNeeded() {
  if (descriptors_.size() == 0)
    return {};

  bool data_send_needed = false;
  for (const auto& desc : descriptors_) {
    if (desc->HasDataNotSent()) {
      data_send_needed = true;
      break;
    }
  }

  if (!data_send_needed)
    return {};

  Result r = command_->BeginIfNotInRecording();
  if (!r.IsSuccess())
    return r;

  for (const auto& desc : descriptors_) {
    r = desc->CreateOrResizeIfNeeded(command_->GetCommandBuffer(),
                                     memory_properties_);
    if (!r.IsSuccess())
      return r;
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

  for (const auto& desc : descriptors_) {
    desc->UpdateResourceIfNeeded(command_->GetCommandBuffer());
  }

  return {};
}

void Pipeline::BindVkDescriptorSets() {
  for (size_t i = 0; i < descriptor_sets_.size(); ++i) {
    if (descriptor_sets_[i].empty)
      continue;

    vkCmdBindDescriptorSets(command_->GetCommandBuffer(),
                            IsGraphics() ? VK_PIPELINE_BIND_POINT_GRAPHICS
                                         : VK_PIPELINE_BIND_POINT_COMPUTE,
                            pipeline_layout_, i, 1,
                            &descriptor_sets_[i].vk_desc_set, 0, nullptr);
  }
}

void Pipeline::BindVkPipeline() {
  vkCmdBindPipeline(command_->GetCommandBuffer(),
                    IsGraphics() ? VK_PIPELINE_BIND_POINT_GRAPHICS
                                 : VK_PIPELINE_BIND_POINT_COMPUTE,
                    pipeline_);
}

Result Pipeline::CopyDescriptorToHost(const uint32_t descriptor_set,
                                      const uint32_t binding) {
  Result r = command_->BeginIfNotInRecording();
  if (!r.IsSuccess())
    return r;

  for (size_t i = 0; i < descriptors_.size(); ++i) {
    if (descriptors_[i]->GetDescriptorSet() == descriptor_set &&
        descriptors_[i]->GetBinding() == binding) {
      return descriptors_[i]->SendDataToHostIfNeeded(
          command_->GetCommandBuffer());
    }
  }

  return Result("Vulkan::Pipeline descriptor with descriptor set: " +
                std::to_string(descriptor_set) +
                ", binding: " + std::to_string(binding) + " does not exist");
}

Result Pipeline::GetDescriptorInfo(const uint32_t descriptor_set,
                                   const uint32_t binding,
                                   ResourceInfo* info) {
  assert(info);
  for (size_t i = 0; i < descriptors_.size(); ++i) {
    if (descriptors_[i]->GetDescriptorSet() == descriptor_set &&
        descriptors_[i]->GetBinding() == binding) {
      *info = descriptors_[i]->GetResourceInfo();
      return {};
    }
  }

  return Result("Vulkan::Pipeline descriptor with descriptor set: " +
                std::to_string(descriptor_set) +
                ", binding: " + std::to_string(binding) + " does not exist");
}

}  // namespace vulkan
}  // namespace amber
