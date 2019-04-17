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

/// Command buffer states.
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

/// Wrapper around a Vulkan command buffer. This is designed to not be used
/// directly, but should always be used through the `CommandBufferGuard` class.
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

/// Wrapper around a `CommandBuffer`.
///
/// Usage follows the pattern:
/// ```
/// CommandBufferGuard guard(cb);
/// if (!guard.IsRecording())
///   return guard.GetResult();
/// ...
/// Result r = guard.Submit(timeout);
/// if (!r.IsSuccess())
///   return r;
/// ```
class CommandBufferGuard {
 public:
  /// Creates a command buffer guard and sets the command buffer to recording.
  explicit CommandBufferGuard(CommandBuffer* buffer);
  ~CommandBufferGuard();

  /// Returns true if the command buffer was successfully set to recording.
  bool IsRecording() const { return result_.IsSuccess(); }
  /// Returns the result object if the command buffer recording failed.
  Result GetResult() { return result_; }

  /// Submits and resets the internal command buffer.
  Result Submit(uint32_t timeout_ms);

 private:
  Result result_;
  CommandBuffer* buffer_;
};

}  // namespace vulkan
}  // namespace amber

#endif  // SRC_VULKAN_COMMAND_BUFFER_H_
