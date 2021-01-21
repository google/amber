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

#include <map>
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

  void SetData(uint8_t location,
               Buffer* buffer,
               InputRate rate,
               Format* format,
               uint32_t offset,
               uint32_t stride);

  const std::vector<VkVertexInputAttributeDescription>& GetVkVertexInputAttr()
      const {
    return vertex_attr_desc_;
  }
  const std::vector<VkVertexInputBindingDescription>& GetVkVertexInputBinding()
      const {
    return vertex_binding_desc_;
  }

  void BindToCommandBuffer(CommandBuffer* command);

 private:
  Device* device_ = nullptr;

  bool is_vertex_data_pending_ = true;

  std::vector<std::unique_ptr<TransferBuffer>> transfer_buffers_;

  std::vector<Buffer*> data_;
  std::vector<VkVertexInputBindingDescription> vertex_binding_desc_;
  std::vector<VkVertexInputAttributeDescription> vertex_attr_desc_;
  std::map<Buffer*, VkBuffer> buffer_to_vk_buffer_;
};

}  // namespace vulkan
}  // namespace amber

#endif  // SRC_VULKAN_VERTEX_BUFFER_H_
