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

#ifndef SRC_VULKAN_VERTEX_BUFFER_H_
#define SRC_VULKAN_VERTEX_BUFFER_H_

#include <memory>
#include <vector>

#include "amber/result.h"
#include "amber/value.h"
#include "amber/vulkan_header.h"
#include "src/buffer.h"
#include "src/format.h"
#include "src/vulkan/transfer_buffer.h"

namespace amber {
namespace vulkan {

class CommandBuffer;
class Device;

/// Wrapper around vertex data information.
class VertexBuffer {
 public:
  explicit VertexBuffer(Device* device);
  ~VertexBuffer();

  Result SendVertexData(CommandBuffer* command);
  bool VertexDataSent() const { return !is_vertex_data_pending_; }

  void SetData(uint8_t location, Buffer* buffer);

  const std::vector<VkVertexInputAttributeDescription>& GetVkVertexInputAttr()
      const {
    return vertex_attr_desc_;
  }

  VkVertexInputBindingDescription GetVkVertexInputBinding() const {
    VkVertexInputBindingDescription vertex_binding_desc =
        VkVertexInputBindingDescription();
    vertex_binding_desc.binding = 0;
    vertex_binding_desc.stride = Get4BytesAlignedStride();
    vertex_binding_desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return vertex_binding_desc;
  }

  uint32_t GetVertexCount() const {
    if (data_.empty())
      return 0;
    return data_[0]->ElementCount();
  }

  void BindToCommandBuffer(CommandBuffer* command);

  void SetBufferForTest(std::unique_ptr<TransferBuffer> buffer);

 private:
  Result FillVertexBufferWithData(CommandBuffer* command);

  uint32_t Get4BytesAlignedStride() const {
    return ((stride_in_bytes_ + 3) / 4) * 4;
  }

  Device* device_ = nullptr;

  bool is_vertex_data_pending_ = true;

  std::unique_ptr<TransferBuffer> transfer_buffer_;
  uint32_t stride_in_bytes_ = 0;

  std::vector<Buffer*> data_;
  std::vector<VkVertexInputAttributeDescription> vertex_attr_desc_;
};

}  // namespace vulkan
}  // namespace amber

#endif  // SRC_VULKAN_VERTEX_BUFFER_H_
