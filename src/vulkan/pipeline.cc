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

#include <limits>

#include "src/command.h"
#include "src/make_unique.h"
#include "src/vulkan/graphics_pipeline.h"

namespace amber {
namespace vulkan {

Pipeline::Pipeline(PipelineType type,
                   VkDevice device,
                   const VkPhysicalDeviceMemoryProperties& properties)
    : device_(device), memory_properties_(properties), pipeline_type_(type) {}

Pipeline::~Pipeline() = default;

GraphicsPipeline* Pipeline::AsGraphics() {
  return static_cast<GraphicsPipeline*>(this);
}

Result Pipeline::InitializeCommandBuffer(VkCommandPool pool, VkQueue queue) {
  command_ = MakeUnique<CommandBuffer>(device_, pool, queue);
  Result r = command_->Initialize();
  if (!r.IsSuccess())
    return r;

  return {};
}

void Pipeline::Shutdown() {
  // TODO(jaebaek): destroy pipeline_cache_ and pipeline_
  command_->Shutdown();
  vkDestroyPipelineLayout(device_, pipeline_layout_, nullptr);
}

Result Pipeline::CreatePipelineLayout() {
  VkPipelineLayoutCreateInfo pipeline_layout_info = {};
  pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipeline_layout_info.setLayoutCount = 0;
  pipeline_layout_info.pSetLayouts = nullptr;
  // TODO(jaebaek): Push constant for pipeline_layout_info.

  if (vkCreatePipelineLayout(device_, &pipeline_layout_info, nullptr,
                             &pipeline_layout_) != VK_SUCCESS) {
    return Result("Vulkan::Calling vkCreatePipelineLayout Fail");
  }

  return {};
}

}  // namespace vulkan
}  // namespace amber
