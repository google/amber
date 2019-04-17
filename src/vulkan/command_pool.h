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

#ifndef SRC_VULKAN_COMMAND_POOL_H_
#define SRC_VULKAN_COMMAND_POOL_H_

#include "amber/result.h"
#include "amber/vulkan_header.h"

namespace amber {
namespace vulkan {

class Device;

/// Wrapper around a Vulkan command pool. The `Initialize` method must be called
/// before using the command pool.
class CommandPool {
 public:
  explicit CommandPool(Device* device);
  ~CommandPool();

  Result Initialize();
  VkCommandPool GetVkCommandPool() const { return pool_; }

 private:
  Device* device_ = nullptr;
  VkCommandPool pool_ = VK_NULL_HANDLE;
};

}  // namespace vulkan
}  // namespace amber

#endif  // SRC_VULKAN_COMMAND_POOL_H_
