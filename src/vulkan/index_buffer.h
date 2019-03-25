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

#ifndef SRC_VULKAN_INDEX_BUFFER_H_
#define SRC_VULKAN_INDEX_BUFFER_H_

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

// A class providing abstraction for index buffer. Only a single
// instance of this class must exist as a member of GraphicsPipeline
// class. It must be created once when
// GraphicsPipeline::SetIndexBuffer() is called.
class IndexBuffer {
 public:
  explicit IndexBuffer(Device* device);
  ~IndexBuffer();

  // Assuming that |buffer_| is nullptr and |values| is not empty,
  // it creates |buffer_| whose size is |sizeof(uint32_t) * values.size()|
  // and coverts |values| as uint32 values and copies them to |buffer_|.
  Result SendIndexData(CommandBuffer* command, Buffer* buffer);

  // Bind |buffer_| as index buffer if it is not nullptr.
  Result BindToCommandBuffer(CommandBuffer* command);

 private:
  Device* device_ = nullptr;
  std::unique_ptr<TransferBuffer> transfer_buffer_;
};

}  // namespace vulkan
}  // namespace amber

#endif  // SRC_VULKAN_INDEX_BUFFER_H_
