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

VertexBuffer::VertexBuffer(Device* device) : device_(device) {}

VertexBuffer::~VertexBuffer() = default;

void VertexBuffer::SetData(uint8_t location, Buffer* buffer) {
  auto format = buffer->GetFormat();

  vertex_attr_desc_.emplace_back();
  // TODO(jaebaek): Support multiple binding
  vertex_attr_desc_.back().binding = 0;
  vertex_attr_desc_.back().location = location;
  vertex_attr_desc_.back().offset = stride_in_bytes_;
  vertex_attr_desc_.back().format = device_->GetVkFormat(*format);

  stride_in_bytes_ += format->SizeInBytes();
  data_.push_back(buffer);
}

Result VertexBuffer::FillVertexBufferWithData(CommandBuffer* command) {
  // Send vertex data from host to device.
  uint8_t* ptr_in_stride_begin =
      static_cast<uint8_t*>(transfer_buffer_->HostAccessibleMemoryPtr());
  for (uint32_t i = 0; i < GetVertexCount(); ++i) {
    uint8_t* ptr = ptr_in_stride_begin;
    for (uint32_t j = 0; j < data_.size(); ++j) {
      size_t bytes = data_[j]->GetFormat()->SizeInBytes();
      std::memcpy(ptr, data_[j]->GetValues<uint8_t>() + (i * bytes), bytes);
      ptr += bytes;
    }
    ptr_in_stride_begin += Get4BytesAlignedStride();
  }

  transfer_buffer_->CopyToDevice(command);
  return {};
}

void VertexBuffer::BindToCommandBuffer(CommandBuffer* command) {
  const VkDeviceSize offset = 0;
  const VkBuffer buffer = transfer_buffer_->GetVkBuffer();
  // TODO(jaebaek): Support multiple binding
  device_->GetPtrs()->vkCmdBindVertexBuffers(command->GetVkCommandBuffer(), 0,
                                             1, &buffer, &offset);
}

Result VertexBuffer::SendVertexData(CommandBuffer* command) {
  if (!is_vertex_data_pending_)
    return Result("Vulkan::Vertices data was already sent");

  const uint32_t n_vertices = GetVertexCount();
  if (n_vertices == 0)
    return Result("Vulkan::Data for VertexBuffer is empty");

  uint32_t bytes = Get4BytesAlignedStride() * n_vertices;

  if (!transfer_buffer_) {
    transfer_buffer_ = MakeUnique<TransferBuffer>(device_, bytes);
    Result r = transfer_buffer_->Initialize(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                                            VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    if (!r.IsSuccess())
      return r;
  }

  FillVertexBufferWithData(command);

  is_vertex_data_pending_ = false;
  return {};
}

}  // namespace vulkan
}  // namespace amber
