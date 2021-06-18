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

#include "src/vulkan/vertex_buffer.h"

#include <cassert>
#include <cstring>

#include "src/make_unique.h"
#include "src/vulkan/command_buffer.h"
#include "src/vulkan/device.h"

namespace amber {
namespace vulkan {
namespace {

VkVertexInputRate GetVkInputRate(InputRate rate) {
  return rate == InputRate::kVertex ? VK_VERTEX_INPUT_RATE_VERTEX
                                    : VK_VERTEX_INPUT_RATE_INSTANCE;
}

}  // namespace

VertexBuffer::VertexBuffer(Device* device) : device_(device) {}

VertexBuffer::~VertexBuffer() = default;

void VertexBuffer::SetData(uint8_t location,
                           Buffer* buffer,
                           InputRate rate,
                           Format* format,
                           uint32_t offset,
                           uint32_t stride) {
  const uint32_t binding = static_cast<uint32_t>(vertex_attr_desc_.size());
  vertex_attr_desc_.emplace_back();
  vertex_attr_desc_.back().binding = binding;
  vertex_attr_desc_.back().location = location;
  vertex_attr_desc_.back().offset = offset;
  vertex_attr_desc_.back().format = device_->GetVkFormat(*format);

  vertex_binding_desc_.emplace_back();
  vertex_binding_desc_.back().binding = binding;
  vertex_binding_desc_.back().stride = stride;
  vertex_binding_desc_.back().inputRate = GetVkInputRate(rate);

  data_.push_back(buffer);
}

void VertexBuffer::BindToCommandBuffer(CommandBuffer* command) {
  std::vector<VkBuffer> buffers;
  std::vector<VkDeviceSize> offsets;

  for (const auto& buf : data_) {
    buffers.push_back(buffer_to_vk_buffer_[buf]);
    offsets.push_back(0);
  }
  device_->GetPtrs()->vkCmdBindVertexBuffers(
      command->GetVkCommandBuffer(), 0, static_cast<uint32_t>(buffers.size()),
      buffers.data(), offsets.data());
}

Result VertexBuffer::SendVertexData(CommandBuffer* command) {
  if (!is_vertex_data_pending_)
    return Result("Vulkan::Vertices data was already sent");

  buffer_to_vk_buffer_.clear();

  for (const auto& buf : data_) {
    if (buffer_to_vk_buffer_.count(buf) != 0) {
      continue;
    }

    // Create a new transfer buffer to hold vertex data.
    uint32_t bytes = buf->GetSizeInBytes();
    transfer_buffers_.push_back(
        MakeUnique<TransferBuffer>(device_, bytes, nullptr));
    Result r = transfer_buffers_.back()->AddUsageFlags(
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    if (!r.IsSuccess())
      return r;
    r = transfer_buffers_.back()->Initialize();

    std::memcpy(transfer_buffers_.back()->HostAccessibleMemoryPtr(),
                buf->GetValues<void>(), bytes);
    transfer_buffers_.back()->CopyToDevice(command);

    if (!r.IsSuccess())
      return r;

    buffer_to_vk_buffer_[buf] = transfer_buffers_.back()->GetVkBuffer();
  }

  is_vertex_data_pending_ = false;
  return {};
}

}  // namespace vulkan
}  // namespace amber
