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

#include "src/vulkan/command.h"

#include <memory>

#include "src/make_unique.h"

namespace amber {

namespace vulkan {

CommandPool::CommandPool(VkDevice device) : device_(device) {}

CommandPool::~CommandPool() = default;

Result CommandPool::Initialize(uint32_t queue_family_index) {
  VkCommandPoolCreateInfo pool_info = {};
  pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  pool_info.queueFamilyIndex = queue_family_index;

  if (vkCreateCommandPool(device_, &pool_info, nullptr, &pool_) != VK_SUCCESS)
    return Result("Vulkan::Calling vkCreateCommandPool Fail");

  return {};
}

void CommandPool::Shutdown() {
  vkDestroyCommandPool(device_, pool_, nullptr);
}

CommandBuffer::CommandBuffer(VkDevice device, VkCommandPool pool, VkQueue queue)
    : device_(device), pool_(pool), queue_(queue) {}

CommandBuffer::~CommandBuffer() = default;

Result CommandBuffer::Initialize() {
  VkCommandBufferAllocateInfo command_info = {};
  command_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  command_info.commandPool = pool_;
  command_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  command_info.commandBufferCount = 1;

  if (vkAllocateCommandBuffers(device_, &command_info, &command_) != VK_SUCCESS)
    return Result("Vulkan::Calling vkAllocateCommandBuffers Fail");

  VkFenceCreateInfo fence_info = {};
  fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  if (vkCreateFence(device_, &fence_info, nullptr, &fence_) != VK_SUCCESS)
    return Result("Vulkan::Calling vkCreateFence Fail");

  return {};
}

Result CommandBuffer::BeginIfNotInRecording() {
  if (state_ == CommandBufferState::kRecording)
    return {};

  if (state_ != CommandBufferState::kInitial)
    return Result("Vulkan::Begin CommandBuffer from Not Valid State");

  VkCommandBufferBeginInfo command_begin_info = {};
  command_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  command_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  if (vkBeginCommandBuffer(command_, &command_begin_info) != VK_SUCCESS)
    return Result("Vulkan::Calling vkBeginCommandBuffer Fail");

  state_ = CommandBufferState::kRecording;
  return {};
}

Result CommandBuffer::End() {
  if (state_ != CommandBufferState::kRecording)
    return Result("Vulkan::End CommandBuffer from Not Valid State");

  if (vkEndCommandBuffer(command_) != VK_SUCCESS)
    return Result("Vulkan::Calling vkEndCommandBuffer Fail");

  state_ = CommandBufferState::kExecutable;
  return {};
}

Result CommandBuffer::SubmitAndReset(uint32_t timeout_ms) {
  if (state_ != CommandBufferState::kExecutable)
    return Result("Vulkan::Submit CommandBuffer from Not Valid State");

  if (vkResetFences(device_, 1, &fence_) != VK_SUCCESS)
    return Result("Vulkan::Calling vkResetFences Fail");

  VkSubmitInfo submit_info = {};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &command_;
  if (vkQueueSubmit(queue_, 1, &submit_info, fence_) != VK_SUCCESS)
    return Result("Vulkan::Calling vkQueueSubmit Fail");

  VkResult r = vkWaitForFences(
      device_, 1, &fence_, VK_TRUE,
      static_cast<uint64_t>(timeout_ms) * 1000ULL * 1000ULL /* nanosecond */);
  if (r == VK_TIMEOUT)
    return Result("Vulkan::Calling vkWaitForFences Timeout");
  if (r != VK_SUCCESS)
    return Result("Vulkan::Calling vkWaitForFences Fail");

  if (vkResetCommandBuffer(command_, 0) != VK_SUCCESS)
    return Result("Vulkan::Calling vkResetCommandBuffer Fail");

  state_ = CommandBufferState::kInitial;
  return {};
}

void CommandBuffer::Shutdown() {
  vkDestroyFence(device_, fence_, nullptr);
  vkFreeCommandBuffers(device_, pool_, 1, &command_);
}

}  // namespace vulkan

}  // namespace amber
