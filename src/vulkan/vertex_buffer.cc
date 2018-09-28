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
#include "src/vulkan/bit_copy.h"
#include "src/vulkan/format_data.h"

namespace amber {
namespace vulkan {

VertexBuffer::VertexBuffer(VkDevice device) : device_(device) {}

VertexBuffer::~VertexBuffer() = default;

void VertexBuffer::SetData(uint8_t location,
                           const Format& format,
                           const std::vector<Value>& values) {
  vertex_attr_desc_.emplace_back();
  // TODO(jaebaek): Support multiple binding
  vertex_attr_desc_.back().binding = 0;
  vertex_attr_desc_.back().location = location;
  vertex_attr_desc_.back().format = ToVkFormat(format.GetFormatType());
  vertex_attr_desc_.back().offset = stride_in_bytes_;

  stride_in_bytes_ += format.GetByteSize();

  formats_.push_back(format);
  data_.push_back(values);
}

void VertexBuffer::FillVertexBufferWithData(VkCommandBuffer command) {
  // Send vertex data from host to device.
  uint8_t* ptr = static_cast<uint8_t*>(buffer_->HostAccessibleMemoryPtr());
  for (uint32_t i = 0; i < GetVertexCount(); ++i) {
    for (uint32_t j = 0; j < formats_.size(); ++j) {
      const auto pack_size = formats_[j].GetPackSize();
      if (pack_size) {
        BitCopy::CopyValueToBuffer(ptr, data_[j][i], 0, pack_size);
        ptr += pack_size / 8;
        continue;
      }

      const auto& components = formats_[j].GetComponents();
      uint8_t bit_offset = 0;

      for (uint32_t k = 0; k < components.size(); ++k) {
        uint8_t bits = components[k].num_bits;
        BitCopy::CopyValueToBuffer(ptr, data_[j][i * components.size() + k],
                                   bit_offset, bits);

        assert(k == components.size() - 1 ||
               static_cast<uint32_t>(bit_offset) + static_cast<uint32_t>(bits) <
                   256);
        bit_offset += bits;
      }

      ptr += formats_[j].GetByteSize();
    }
  }

  ptr = static_cast<uint8_t*>(buffer_->HostAccessibleMemoryPtr());
  buffer_->CopyToDevice(command);
}

void VertexBuffer::BindToCommandBuffer(VkCommandBuffer command) {
  const VkDeviceSize offset = 0;
  const VkBuffer buffer = buffer_->GetVkBuffer();
  // TODO(jaebaek): Support multiple binding
  vkCmdBindVertexBuffers(command, 0, 1, &buffer, &offset);
}

Result VertexBuffer::SendVertexData(
    VkCommandBuffer command,
    const VkPhysicalDeviceMemoryProperties& properties) {
  if (!is_vertex_data_pending_)
    return Result("Vulkan::Vertices data was already sent");

  const size_t n_vertices = GetVertexCount();
  if (n_vertices == 0)
    return Result("Vulkan::Data for VertexBuffer is empty");

  size_t bytes = stride_in_bytes_ * n_vertices;

  if (!buffer_) {
    buffer_ = MakeUnique<Buffer>(device_, bytes, properties);
    Result r = buffer_->Initialize(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                                   VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    if (!r.IsSuccess())
      return r;
  }

  if (formats_.empty() || formats_[0].GetComponents().empty())
    return Result("Vulkan::Formats for VertexBuffer is empty");

  FillVertexBufferWithData(command);

  is_vertex_data_pending_ = false;
  return {};
}

}  // namespace vulkan
}  // namespace amber
