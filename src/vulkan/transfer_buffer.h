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

#ifndef SRC_VULKAN_TRANSFER_BUFFER_H_
#define SRC_VULKAN_TRANSFER_BUFFER_H_

#include <vector>

#include "amber/result.h"
#include "amber/vulkan_header.h"
#include "src/format.h"
#include "src/vulkan/resource.h"

namespace amber {
namespace vulkan {

class CommandBuffer;
class Device;

/// Wrapper around a Vulkan VkBuffer object.
class TransferBuffer : public Resource {
 public:
  TransferBuffer(Device* device, uint32_t size_in_bytes, Format* format);
  ~TransferBuffer() override;

  TransferBuffer* AsTransferBuffer() override { return this; }
  Result AddUsageFlags(VkBufferUsageFlags flags) {
    if (buffer_ != VK_NULL_HANDLE) {
      return Result(
          "Vulkan: TransferBuffer::AddUsageFlags Usage flags can't be changed "
          "after initializing the buffer.");
    }
    usage_flags_ |= flags;
    return {};
  }
  Result Initialize() override;
  const VkBufferView* GetVkBufferView() const { return &view_; }

  VkBuffer GetVkBuffer() const { return buffer_; }

  /// Records a command on |command_buffer| to copy the buffer contents from the
  /// host to the device.
  void CopyToDevice(CommandBuffer* command_buffer) override;
  /// Records a command on |command_buffer| to copy the buffer contents from the
  /// device to the host.
  void CopyToHost(CommandBuffer* command_buffer) override;

 private:
  VkBufferUsageFlags usage_flags_ = 0;
  VkBuffer buffer_ = VK_NULL_HANDLE;
  VkDeviceMemory memory_ = VK_NULL_HANDLE;
  VkBufferView view_ = VK_NULL_HANDLE;
  VkFormat format_ = VK_FORMAT_UNDEFINED;
};

}  // namespace vulkan
}  // namespace amber

#endif  // SRC_VULKAN_TRANSFER_BUFFER_H_
