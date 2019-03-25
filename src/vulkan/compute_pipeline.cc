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

#include "src/vulkan/compute_pipeline.h"

#include "src/vulkan/command_pool.h"
#include "src/vulkan/device.h"

namespace amber {
namespace vulkan {

ComputePipeline::ComputePipeline(
    Device* device,
    uint32_t fence_timeout_ms,
    const std::vector<VkPipelineShaderStageCreateInfo>& shader_stage_info)
    : Pipeline(PipelineType::kCompute,
               device,
               fence_timeout_ms,
               shader_stage_info) {}

ComputePipeline::~ComputePipeline() = default;

Result ComputePipeline::Initialize(CommandPool* pool) {
  return Pipeline::Initialize(pool);
}

Result ComputePipeline::CreateVkComputePipeline(
    const VkPipelineLayout& pipeline_layout,
    VkPipeline* pipeline) {
  auto shader_stage_info = GetVkShaderStageInfo();
  if (shader_stage_info.size() != 1) {
    return Result(
        "Vulkan::CreateVkComputePipeline number of shaders given to compute "
        "pipeline is not 1");
  }

  if (shader_stage_info[0].stage != VK_SHADER_STAGE_COMPUTE_BIT)
    return Result("Vulkan: Non compute shader for compute pipeline");

  shader_stage_info[0].pName = GetEntryPointName(VK_SHADER_STAGE_COMPUTE_BIT);

  VkComputePipelineCreateInfo pipeline_info = VkComputePipelineCreateInfo();
  pipeline_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
  pipeline_info.stage = shader_stage_info[0];
  pipeline_info.layout = pipeline_layout;

  if (device_->GetPtrs()->vkCreateComputePipelines(
          device_->GetVkDevice(), VK_NULL_HANDLE, 1, &pipeline_info, nullptr,
          pipeline) != VK_SUCCESS) {
    return Result("Vulkan::Calling vkCreateComputePipelines Fail");
  }

  return {};
}

Result ComputePipeline::Compute(uint32_t x, uint32_t y, uint32_t z) {
  Result r = SendDescriptorDataToDeviceIfNeeded();
  if (!r.IsSuccess())
    return r;

  VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
  r = CreateVkPipelineLayout(&pipeline_layout);
  if (!r.IsSuccess())
    return r;

  VkPipeline pipeline = VK_NULL_HANDLE;
  r = CreateVkComputePipeline(pipeline_layout, &pipeline);
  if (!r.IsSuccess())
    return r;

  // Note that a command updating a descriptor set and a command using
  // it must be submitted separately, because using a descriptor set
  // while updating it is not safe.
  UpdateDescriptorSetsIfNeeded();

  {
    CommandBufferGuard guard(GetCommandBuffer());
    if (!guard.IsRecording())
      return guard.GetResult();

    BindVkDescriptorSets(pipeline_layout);

    r = RecordPushConstant(pipeline_layout);
    if (!r.IsSuccess())
      return r;

    device_->GetPtrs()->vkCmdBindPipeline(command_->GetVkCommandBuffer(),
                                          VK_PIPELINE_BIND_POINT_COMPUTE,
                                          pipeline);
    device_->GetPtrs()->vkCmdDispatch(command_->GetVkCommandBuffer(), x, y, z);

    r = guard.Submit(GetFenceTimeout());
    if (!r.IsSuccess())
      return r;
  }

  r = ReadbackDescriptorsToHostDataQueue();
  if (!r.IsSuccess())
    return r;

  device_->GetPtrs()->vkDestroyPipeline(device_->GetVkDevice(), pipeline,
                                        nullptr);
  device_->GetPtrs()->vkDestroyPipelineLayout(device_->GetVkDevice(),
                                              pipeline_layout, nullptr);

  return {};
}

}  // namespace vulkan
}  // namespace amber
