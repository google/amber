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
#include "amber/vulkan_header.h"
#include "src/format.h"
#include "src/value.h"
#include "src/vulkan/buffer.h"

namespace amber {
namespace vulkan {

class VertexBuffer {
 public:
  explicit VertexBuffer(VkDevice device);
  ~VertexBuffer();

  void Shutdown();

  Result SendVertexData(VkCommandBuffer command,
                        const VkPhysicalDeviceMemoryProperties& properties);
  bool VertexDataSent() const { return !is_vertex_data_pending_; }

  void SetData(uint8_t location,
               const Format& format,
               const std::vector<Value>& values);

  const std::vector<VkVertexInputAttributeDescription>& GetVertexInputAttr()
      const {
    return vertex_attr_desc_;
  }

  VkVertexInputBindingDescription GetVertexInputBinding() const {
    VkVertexInputBindingDescription vertex_binding_desc =
        VkVertexInputBindingDescription();
    vertex_binding_desc.binding = 0;
    vertex_binding_desc.stride = Get4BytesAlignedStride();
    vertex_binding_desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return vertex_binding_desc;
  }

  size_t GetVertexCount() const {
    if (data_.empty())
      return 0;

    return data_[0].size() / formats_[0].GetComponents().size();
  }

  void BindToCommandBuffer(VkCommandBuffer command);

 private:
  void FillVertexBufferWithData(VkCommandBuffer command);

  // Return |stride_in_bytes_| rounded up by 4.
  uint32_t Get4BytesAlignedStride() const {
    return ((stride_in_bytes_ + 3) / 4) * 4;
  }

  VkDevice device_ = VK_NULL_HANDLE;

  bool is_vertex_data_pending_ = true;

  std::unique_ptr<Buffer> buffer_;
  uint32_t stride_in_bytes_ = 0;

  std::vector<Format> formats_;
  std::vector<std::vector<Value>> data_;

  std::vector<VkVertexInputAttributeDescription> vertex_attr_desc_;
};

}  // namespace vulkan
}  // namespace amber

#endif  // SRC_VULKAN_VERTEX_BUFFER_H_
