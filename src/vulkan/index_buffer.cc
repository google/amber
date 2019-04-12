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

#include <cstring>

#include "src/make_unique.h"
#include "src/vulkan/command_buffer.h"
#include "src/vulkan/device.h"

namespace amber {
namespace vulkan {

IndexBuffer::IndexBuffer(Device* device) : device_(device) {}

IndexBuffer::~IndexBuffer() = default;

Result IndexBuffer::SendIndexData(CommandBuffer* command, Buffer* buffer) {
  if (transfer_buffer_) {
    return Result(
        "IndexBuffer::SendIndexData must be called once when it is created");
  }

  if (buffer->ElementCount() == 0)
    return Result("IndexBuffer::SendIndexData |buffer| is empty");

  transfer_buffer_ =
      MakeUnique<TransferBuffer>(device_, buffer->GetSizeInBytes());
  Result r = transfer_buffer_->Initialize(VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                                          VK_BUFFER_USAGE_TRANSFER_DST_BIT);
  if (!r.IsSuccess())
    return r;

  std::memcpy(transfer_buffer_->HostAccessibleMemoryPtr(),
              buffer->ValuePtr()->data(), buffer->GetSizeInBytes());

  transfer_buffer_->CopyToDevice(command);
  return {};
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
