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

#ifndef SRC_VULKAN_BUFFER_BACKED_DESCRIPTOR_H_
#define SRC_VULKAN_BUFFER_BACKED_DESCRIPTOR_H_

#include <memory>
#include <vector>

#include "amber/result.h"
#include "amber/value.h"
#include "amber/vulkan_header.h"
#include "src/buffer.h"
#include "src/engine.h"
#include "src/vulkan/descriptor.h"
#include "src/vulkan/resource.h"

namespace amber {
namespace vulkan {

class BufferBackedDescriptor : public Descriptor {
 public:
  BufferBackedDescriptor(Buffer* buffer,
                         DescriptorType type,
                         Device* device,
                         uint32_t desc_set,
                         uint32_t binding);
  ~BufferBackedDescriptor() override;

  Result CreateResourceIfNeeded() override { return {}; }
  void RecordCopyDataToResourceIfNeeded(CommandBuffer* command) override;
  Result RecordCopyDataToHost(CommandBuffer* command) override;
  Result MoveResourceToBufferOutput() override;
  virtual Resource* GetResource() = 0;

  Result SetSizeInElements(uint32_t element_count) override;
  Result AddToBuffer(const std::vector<Value>& values,
                     uint32_t offset) override;
  Buffer* getAmberBuffer() { return amber_buffer_; }
  void setAmberBuffer(Buffer* buffer) { amber_buffer_ = buffer; }

 private:
  Buffer* amber_buffer_ = nullptr;
};

}  // namespace vulkan
}  // namespace amber

#endif  // SRC_VULKAN_BUFFER_BACKED_DESCRIPTOR_H_
