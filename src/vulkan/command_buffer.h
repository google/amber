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

#ifndef SRC_VULKAN_COMMAND_BUFFER_H_
#define SRC_VULKAN_COMMAND_BUFFER_H_

#include "amber/result.h"
#include "amber/vulkan_header.h"

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

class CommandBufferGuard;
class CommandPool;
class Device;

class CommandBuffer {
 public:
  CommandBuffer(Device* device, CommandPool* pool);
  ~CommandBuffer();

  Result Initialize();
  VkCommandBuffer GetVkCommandBuffer() const { return command_; }

 private:
  friend CommandBufferGuard;

  Result BeginRecording();
  Result SubmitAndReset(uint32_t timeout_ms);

  bool guarded_ = false;

  Device* device_ = nullptr;
  CommandPool* pool_ = nullptr;
  VkCommandBuffer command_ = VK_NULL_HANDLE;
  VkFence fence_ = VK_NULL_HANDLE;
};

class CommandBufferGuard {
 public:
  explicit CommandBufferGuard(CommandBuffer* buffer);
  ~CommandBufferGuard();

  bool IsRecording() const { return result_.IsSuccess(); }
  Result GetResult() { return result_; }

  Result Submit(uint32_t timeout_ms);

 private:
  Result result_;
  CommandBuffer* buffer_;
};

}  // namespace vulkan
}  // namespace amber

#endif  // SRC_VULKAN_COMMAND_BUFFER_H_
