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

#include "src/vulkan/index_buffer.h"

#include <cassert>
#include <cstring>

#include "src/make_unique.h"
#include "src/vulkan/command_buffer.h"
#include "src/vulkan/device.h"
#include "src/vulkan/format_data.h"

namespace amber {
namespace vulkan {

IndexBuffer::IndexBuffer(Device* device) : device_(device) {}

IndexBuffer::~IndexBuffer() = default;

Result IndexBuffer::SendIndexData(
    CommandBuffer* command,
    const VkPhysicalDeviceMemoryProperties& properties,
    const std::vector<Value>& values) {
  if (transfer_buffer_) {
    return Result(
        "IndexBuffer::SendIndexData must be called once when it is created");
  }

  if (values.empty())
    return Result("IndexBuffer::SendIndexData |values| is empty");

  transfer_buffer_ = MakeUnique<TransferBuffer>(
      device_, static_cast<uint32_t>(sizeof(uint32_t) * values.size()),
      properties);
  Result r = transfer_buffer_->Initialize(VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                                          VK_BUFFER_USAGE_TRANSFER_DST_BIT);
  if (!r.IsSuccess())
    return r;

  uint32_t* ptr =
      static_cast<uint32_t*>(transfer_buffer_->HostAccessibleMemoryPtr());
  for (const auto& value : values)
    *ptr++ = value.AsUint32();

  return transfer_buffer_->CopyToDevice(command);
}

Result IndexBuffer::BindToCommandBuffer(CommandBuffer* command) {
  if (!transfer_buffer_) {
    return Result(
        "IndexBuffer::BindToCommandBuffer |transfer_buffer_| is nullptr");
  }

  device_->GetPtrs()->vkCmdBindIndexBuffer(command->GetVkCommandBuffer(),
                                           transfer_buffer_->GetVkBuffer(), 0,
                                           VK_INDEX_TYPE_UINT32);
  return {};
}

}  // namespace vulkan
}  // namespace amber
