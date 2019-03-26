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

#include "src/vulkan/command_pool.h"

#include "src/vulkan/device.h"

namespace amber {
namespace vulkan {

CommandPool::CommandPool(Device* device) : device_(device) {}

CommandPool::~CommandPool() {
  if (pool_ == VK_NULL_HANDLE)
    return;

  device_->GetPtrs()->vkDestroyCommandPool(device_->GetVkDevice(), pool_,
                                           nullptr);
}

Result CommandPool::Initialize() {
  VkCommandPoolCreateInfo pool_info = VkCommandPoolCreateInfo();
  pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  pool_info.queueFamilyIndex = device_->GetQueueFamilyIndex();

  if (device_->GetPtrs()->vkCreateCommandPool(
          device_->GetVkDevice(), &pool_info, nullptr, &pool_) != VK_SUCCESS) {
    return Result("Vulkan::Calling vkCreateCommandPool Fail");
  }

  return {};
}

}  // namespace vulkan
}  // namespace amber
