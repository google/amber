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
#include "src/make_unique.h"
#include "src/vulkan/compute_pipeline.h"
#include "src/vulkan/graphics_pipeline.h"
#include "src/vulkan/storage_buffer_descriptor.h"

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

  // TODO(jaebaek): destroy pipeline_cache_ and pipeline_
  DestoryDescriptorPools();
  DestoryDescriptorSetLayouts();
  command_->Shutdown();
  vkDestroyPipelineLayout(device_, pipeline_layout_, nullptr);
  vkDestroyPipeline(device_, pipeline_, nullptr);
}

void Pipeline::DestoryDescriptorSetLayouts() {
  for (size_t i = 0; i < descriptor_set_layouts_.size(); ++i) {
    vkDestroyDescriptorSetLayout(device_, descriptor_set_layouts_[i], nullptr);
  }
  descriptor_set_layouts_.clear();
}

Result Pipeline::CreateDescriptorSetLayouts() {
  std::sort(descriptors_.begin(), descriptors_.end());

  size_t i = 0;
  while (i < descriptors_.size()) {
    std::vector<VkDescriptorSetLayoutBinding> bindings;
    for (const uint32_t current_desc = descriptors_[i]->GetDescriptorSet();
         i < descriptors_.size() &&
         current_desc == descriptors_[i]->GetDescriptorSet();
         ++i) {
      bindings.emplace_back();
      bindings.back().binding = descriptors_[i]->GetBinding();
      bindings.back().descriptorType =
          ToVkDescriptorType(descriptors_[i]->GetType());
      bindings.back().descriptorCount = 1;
      // TODO(jaebaek): Amber script must contain which stages use each
      //                descriptor. Set .stageFlags attribute based on it.
      bindings.back().stageFlags = VK_SHADER_STAGE_ALL;
      // TODO(jaebaek): Properly set .pImmutableSamplers attribute.
    }

    VkDescriptorSetLayoutCreateInfo desc_info = {};
    desc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    desc_info.bindingCount = bindings.size();
    desc_info.pBindings = bindings.data();

    VkDescriptorSetLayout desc_layout = VK_NULL_HANDLE;
    if (vkCreateDescriptorSetLayout(device_, &desc_info, nullptr,
                                    &desc_layout) != VK_SUCCESS) {
      return Result("Vulkan::Calling vkCreateDescriptorSetLayout Fail");
    }
    descriptor_set_layouts_.push_back(desc_layout);
  }

  return {};
}

void Pipeline::DestoryDescriptorPools() {
  for (size_t i = 0; i < descriptor_pools_.size(); ++i) {
    vkDestroyDescriptorPool(device_, descriptor_pools_[i], nullptr);
  }
  descriptor_pools_.clear();
}

Result Pipeline::CreateDescriptorPools() {
  size_t i = 0;
  while (i < descriptors_.size()) {
    std::vector<VkDescriptorPoolSize> pool_sizes;
    for (const uint32_t current_desc = descriptors_[i]->GetDescriptorSet();
         i < descriptors_.size() &&
         current_desc == descriptors_[i]->GetDescriptorSet();
         ++i) {
      auto type = ToVkDescriptorType(descriptors_[i]->GetType());
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

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.maxSets = 1;
    pool_info.poolSizeCount = pool_sizes.size();
    pool_info.pPoolSizes = pool_sizes.data();

    VkDescriptorPool desc_pool = VK_NULL_HANDLE;
    if (vkCreateDescriptorPool(device_, &pool_info, nullptr, &desc_pool) !=
        VK_SUCCESS) {
      return Result("Vulkan::Calling vkCreateDescriptorPool Fail");
    }
    descriptor_pools_.push_back(desc_pool);
  }

  return {};
}

Result Pipeline::CreateDescriptorSets() {
  for (size_t i = 0; i < descriptor_set_layouts_.size(); ++i) {
    VkDescriptorSetAllocateInfo desc_set_info = {};
    desc_set_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    desc_set_info.descriptorPool = descriptor_pools_[i];
    desc_set_info.descriptorSetCount = 1;
    desc_set_info.pSetLayouts = &descriptor_set_layouts_[i];

    VkDescriptorSet desc_set = VK_NULL_HANDLE;
    if (vkAllocateDescriptorSets(device_, &desc_set_info, &desc_set) !=
        VK_SUCCESS) {
      return Result("Vulkan::Calling vkAllocateDescriptorSets Fail");
    }
    descriptor_sets_.push_back(desc_set);
  }

  return {};
}

Result Pipeline::CreatePipelineLayout() {
  VkPipelineLayoutCreateInfo pipeline_layout_info = {};
  pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipeline_layout_info.setLayoutCount = descriptor_set_layouts_.size();
  pipeline_layout_info.pSetLayouts = descriptor_set_layouts_.data();
  // TODO(jaebaek): Push constant for pipeline_layout_info.

  if (vkCreatePipelineLayout(device_, &pipeline_layout_info, nullptr,
                             &pipeline_layout_) != VK_SUCCESS) {
    return Result("Vulkan::Calling vkCreatePipelineLayout Fail");
  }

  return {};
}

Result Pipeline::CreateVkDescriptorRelatedObjects() {
  // TODO(jaebaek): Update SSBO dynamically.
  Result r = CreateDescriptorSetLayouts();
  if (!r.IsSuccess())
    return r;

  r = CreateDescriptorPools();
  if (!r.IsSuccess())
    return r;

  r = CreateDescriptorSets();
  if (!r.IsSuccess())
    return r;

  r = CreatePipelineLayout();
  if (!r.IsSuccess())
    return r;

  for (size_t i = 0, j = 0;
       i < descriptors_.size() && j < descriptor_sets_.size(); ++j) {
    for (const uint32_t current_desc = descriptors_[i]->GetDescriptorSet();
         i < descriptors_.size() &&
         current_desc == descriptors_[i]->GetDescriptorSet();
         ++i) {
      r = descriptors_[i]->UpdateDescriptorSet(descriptor_sets_[j]);
      if (!r.IsSuccess())
        return r;
    }
  }

  return {};
}

Result Pipeline::AddDescriptor(const BufferCommand* buffer_command) {
  if (!buffer_command->IsSSBO())
    return Result("Vulkan::AddDescriptor non-SSBO not implemented");

  auto ssbo = MakeUnique<StorageBufferDescriptor>(
      device_, buffer_command->GetDescriptorSet(), buffer_command->GetBinding(),
      buffer_command->GetSize(), memory_properties_);

  Result r = ssbo->Initialize(buffer_command->GetDatumType().GetType(),
                              buffer_command->GetValues());
  if (!r.IsSuccess())
    return r;

  descriptors_.push_back(std::move(ssbo));

  return {};
}

Result Pipeline::SendDescriptorDataToGPUIfNeeded() {
  if (descriptors_.size() == 0)
    return {};

  bool data_send_needed = false;
  for (const auto& desc : descriptors_) {
    if (!desc->IsDataAlreadySent()) {
      data_send_needed = true;
      break;
    }
  }

  if (!data_send_needed)
    return {};

  Result r = command_->BeginIfNotInRecording();
  if (!r.IsSuccess())
    return r;

  for (const auto& desc : descriptors_)
    desc->SendDataToGPUIfNeeded(command_->GetCommandBuffer());

  return {};
}

void Pipeline::BindVkDescriptorSets() {
  vkCmdBindDescriptorSets(command_->GetCommandBuffer(),
                          IsGraphics() ? VK_PIPELINE_BIND_POINT_GRAPHICS
                                       : VK_PIPELINE_BIND_POINT_COMPUTE,
                          pipeline_layout_, 0, descriptor_sets_.size(),
                          descriptor_sets_.data(), 0, nullptr);
}

void Pipeline::BindVkPipeline() {
  vkCmdBindPipeline(command_->GetCommandBuffer(),
                    IsGraphics() ? VK_PIPELINE_BIND_POINT_GRAPHICS
                                 : VK_PIPELINE_BIND_POINT_COMPUTE,
                    pipeline_);
}

}  // namespace vulkan
}  // namespace amber
