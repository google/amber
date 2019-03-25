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

#include <cassert>

#include "src/vulkan/command_pool.h"
#include "src/vulkan/device.h"

namespace amber {
namespace vulkan {

CommandBuffer::CommandBuffer(Device* device, CommandPool* pool)
    : device_(device), pool_(pool) {}

CommandBuffer::~CommandBuffer() {
  if (fence_ != VK_NULL_HANDLE)
    device_->GetPtrs()->vkDestroyFence(device_->GetVkDevice(), fence_, nullptr);

  if (command_ != VK_NULL_HANDLE) {
    device_->GetPtrs()->vkFreeCommandBuffers(
        device_->GetVkDevice(), pool_->GetVkCommandPool(), 1, &command_);
  }
}

Result CommandBuffer::Initialize() {
  VkCommandBufferAllocateInfo command_info = VkCommandBufferAllocateInfo();
  command_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  command_info.commandPool = pool_->GetVkCommandPool();
  command_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  command_info.commandBufferCount = 1;

  if (device_->GetPtrs()->vkAllocateCommandBuffers(
          device_->GetVkDevice(), &command_info, &command_) != VK_SUCCESS) {
    return Result("Vulkan::Calling vkAllocateCommandBuffers Fail");
  }

  VkFenceCreateInfo fence_info = VkFenceCreateInfo();
  fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  if (device_->GetPtrs()->vkCreateFence(device_->GetVkDevice(), &fence_info,
                                        nullptr, &fence_) != VK_SUCCESS) {
    return Result("Vulkan::Calling vkCreateFence Fail");
  }

  return {};
}

Result CommandBuffer::BeginRecording() {
  VkCommandBufferBeginInfo command_begin_info = VkCommandBufferBeginInfo();
  command_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  command_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  if (device_->GetPtrs()->vkBeginCommandBuffer(command_, &command_begin_info) !=
      VK_SUCCESS) {
    return Result("Vulkan::Calling vkBeginCommandBuffer Fail");
  }

  return {};
}

Result CommandBuffer::SubmitAndReset(uint32_t timeout_ms) {
  if (device_->GetPtrs()->vkEndCommandBuffer(command_) != VK_SUCCESS)
    return Result("Vulkan::Calling vkEndCommandBuffer Fail");

  if (device_->GetPtrs()->vkResetFences(device_->GetVkDevice(), 1, &fence_) !=
      VK_SUCCESS) {
    return Result("Vulkan::Calling vkResetFences Fail");
  }

  VkSubmitInfo submit_info = VkSubmitInfo();
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &command_;
  if (device_->GetPtrs()->vkQueueSubmit(device_->GetVkQueue(), 1, &submit_info,
                                        fence_) != VK_SUCCESS) {
    return Result("Vulkan::Calling vkQueueSubmit Fail");
  }

  VkResult r = device_->GetPtrs()->vkWaitForFences(
      device_->GetVkDevice(), 1, &fence_, VK_TRUE,
      static_cast<uint64_t>(timeout_ms) * 1000ULL * 1000ULL /* nanosecond */);
  if (r == VK_TIMEOUT)
    return Result("Vulkan::Calling vkWaitForFences Timeout");
  if (r != VK_SUCCESS)
    return Result("Vulkan::Calling vkWaitForFences Fail");

  if (device_->GetPtrs()->vkResetCommandBuffer(command_, 0) != VK_SUCCESS)
    return Result("Vulkan::Calling vkResetCommandBuffer Fail");

  return {};
}

CommandBufferGuard::CommandBufferGuard(CommandBuffer* buffer)
    : buffer_(buffer) {
  assert(!buffer_->guarded_);

  buffer_->guarded_ = true;
  result_ = buffer_->BeginRecording();
}

CommandBufferGuard::~CommandBufferGuard() = default;

Result CommandBufferGuard::Submit(uint32_t timeout_ms) {
  assert(buffer_->guarded_);

  result_ = buffer_->SubmitAndReset(timeout_ms);
  buffer_->guarded_ = false;
  return result_;
}

}  // namespace vulkan
}  // namespace amber
