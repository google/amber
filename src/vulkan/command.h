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

#ifndef SRC_VULKAN_COMMAND_H_
#define SRC_VULKAN_COMMAND_H_

#include "amber/result.h"
#include "vulkan/vulkan.h"

namespace amber {

namespace vulkan {

// Command buffer states based on "5.1. Command Buffer Lifecycle" of Vulkan
// spec
enum class CommandBufferState : uint8_t {
  kInitial = 0,
  kRecording,
  kExecutable,
  kPending,
  kInvalid,
};

class CommandPool {
 public:
  explicit CommandPool(VkDevice device);
  ~CommandPool();

  Result Initialize(uint32_t queue_family_index);
  VkCommandPool GetCommandPool() const { return pool_; }
  void Shutdown();

 private:
  VkDevice device_ = VK_NULL_HANDLE;
  VkCommandPool pool_ = VK_NULL_HANDLE;
};

class CommandBuffer {
 public:
  CommandBuffer(VkDevice device, VkCommandPool pool, VkQueue queue);
  ~CommandBuffer();

  Result Initialize();
  VkCommandBuffer GetCommandBuffer() const { return command_; }
  void Shutdown();

  // Do nothing and return if it is already ready to record. If it is in
  // initial state, call command begin API and make it ready to record.
  // Otherwise, report error.
  Result BeginIfNotInRecording();

  Result End();
  Result SubmitAndReset();

 private:
  VkDevice device_ = VK_NULL_HANDLE;
  VkCommandPool pool_ = VK_NULL_HANDLE;
  VkQueue queue_ = VK_NULL_HANDLE;
  VkCommandBuffer command_ = VK_NULL_HANDLE;
  VkFence fence_ = VK_NULL_HANDLE;
  CommandBufferState state_ = CommandBufferState::kInitial;
};

}  // namespace vulkan

}  // namespace amber

#endif  // SRC_VULKAN_COMMAND_H_
