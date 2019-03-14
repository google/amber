// Copyright 2019 The Amber Authors.
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

#include "src/vulkan/command_buffer.h"

#include "src/vulkan/command_pool.h"
#include "src/vulkan/device.h"

namespace amber {
namespace vulkan {

CommandBuffer::CommandBuffer(Device* device, CommandPool* pool, VkQueue queue)
    : device_(device), pool_(pool), queue_(queue) {}

CommandBuffer::~CommandBuffer() {
  if (fence_ != VK_NULL_HANDLE)
    device_->GetPtrs()->vkDestroyFence(device_->GetDevice(), fence_, nullptr);

  if (command_ != VK_NULL_HANDLE) {
    device_->GetPtrs()->vkFreeCommandBuffers(
        device_->GetDevice(), pool_->GetCommandPool(), 1, &command_);
  }
}

Result CommandBuffer::Initialize() {
  VkCommandBufferAllocateInfo command_info = VkCommandBufferAllocateInfo();
  command_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  command_info.commandPool = pool_->GetCommandPool();
  command_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  command_info.commandBufferCount = 1;

  if (device_->GetPtrs()->vkAllocateCommandBuffers(
          device_->GetDevice(), &command_info, &command_) != VK_SUCCESS) {
    return Result("Vulkan::Calling vkAllocateCommandBuffers Fail");
  }

  VkFenceCreateInfo fence_info = VkFenceCreateInfo();
  fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  if (device_->GetPtrs()->vkCreateFence(device_->GetDevice(), &fence_info,
                                        nullptr, &fence_) != VK_SUCCESS) {
    return Result("Vulkan::Calling vkCreateFence Fail");
  }

  return {};
}

Result CommandBuffer::BeginIfNotInRecording() {
  if (state_ == CommandBufferState::kRecording)
    return {};

  if (state_ != CommandBufferState::kInitial)
    return Result("Vulkan::Begin CommandBuffer from Not Valid State");

  VkCommandBufferBeginInfo command_begin_info = VkCommandBufferBeginInfo();
  command_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  command_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  if (device_->GetPtrs()->vkBeginCommandBuffer(command_, &command_begin_info) !=
      VK_SUCCESS) {
    return Result("Vulkan::Calling vkBeginCommandBuffer Fail");
  }

  state_ = CommandBufferState::kRecording;
  return {};
}

Result CommandBuffer::End() {
  if (state_ != CommandBufferState::kRecording)
    return Result("Vulkan::End CommandBuffer from Not Valid State");

  if (device_->GetPtrs()->vkEndCommandBuffer(command_) != VK_SUCCESS)
    return Result("Vulkan::Calling vkEndCommandBuffer Fail");

  state_ = CommandBufferState::kExecutable;
  return {};
}

Result CommandBuffer::SubmitAndReset(uint32_t timeout_ms) {
  if (state_ != CommandBufferState::kExecutable)
    return Result("Vulkan::Submit CommandBuffer from Not Valid State");

  if (device_->GetPtrs()->vkResetFences(device_->GetDevice(), 1, &fence_) !=
      VK_SUCCESS) {
    return Result("Vulkan::Calling vkResetFences Fail");
  }

  VkSubmitInfo submit_info = VkSubmitInfo();
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &command_;
  if (device_->GetPtrs()->vkQueueSubmit(queue_, 1, &submit_info, fence_) !=
      VK_SUCCESS) {
    return Result("Vulkan::Calling vkQueueSubmit Fail");
  }

  VkResult r = device_->GetPtrs()->vkWaitForFences(
      device_->GetDevice(), 1, &fence_, VK_TRUE,
      static_cast<uint64_t>(timeout_ms) * 1000ULL * 1000ULL /* nanosecond */);
  if (r == VK_TIMEOUT)
    return Result("Vulkan::Calling vkWaitForFences Timeout");
  if (r != VK_SUCCESS)
    return Result("Vulkan::Calling vkWaitForFences Fail");

  if (device_->GetPtrs()->vkResetCommandBuffer(command_, 0) != VK_SUCCESS)
    return Result("Vulkan::Calling vkResetCommandBuffer Fail");

  state_ = CommandBufferState::kInitial;
  return {};
}

}  // namespace vulkan
}  // namespace amber
