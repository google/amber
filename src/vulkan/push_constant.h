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

#ifndef SRC_VULKAN_RESOURCE_H_
#define SRC_VULKAN_RESOURCE_H_

#include <vector>

#include "amber/result.h"
#include "src/vulkan/resource.h"
#include "vulkan/vulkan.h"

namespace amber {
namespace vulkan {

// Class to handle push constant.
class PushConstant : public Resource {
 public:
  PushConstant();
  ~PushConstant() override;

  // Return a VkPushConstantRange structure that contains shader
  // stage flag, offset, and size in bytes of contents of push
  // constant.
  VkPushConstantRange GetPushConstantRange();

  // Call vkCmdPushConstants() to record a command for push
  // constant. |command_buffer| is a Vulkan command buffer that
  // keeps the recorded command and |pipeline_layout| is the
  // graphics / compute pipeline that it currently uses.
  void RecordPushConstantVkCommand(VkCommandBuffer command_buffer,
                                   VkPipelineLayout pipeline_layout);

  // Add information of how and what to do with push constant to
  // |push_constant_data_|.
  Result AddBufferData(const BufferCommand* command);

  // Resource
  VkDeviceMemory GetHostAccessMemory() const override {
    return memory_;
  }
  Result CopyToHost(VkCommandBuffer) override {
    return Result("Vulkan: should not call CopyToHost() for PushConstant");
  }
  void Shutdown() override;

 private:
  // Keep the information of what and how to conduct push constant.
  std::vector<BufferData> push_constant_data_;
  void* memory_ptr_ = nullptr;
};

}  // namespace vulkan
}  // namespace amber

#endif  // SRC_VULKAN_RESOURCE_H_
