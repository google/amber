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
#include <utility>
#include <vector>

#include "amber/result.h"
#include "amber/value.h"
#include "amber/vulkan_header.h"
#include "src/buffer.h"
#include "src/engine.h"
#include "src/vulkan/descriptor.h"
#include "src/vulkan/pipeline.h"
#include "src/vulkan/resource.h"

namespace amber {
namespace vulkan {

class BufferBackedDescriptor : public Descriptor {
 public:
  BufferBackedDescriptor(Buffer* buffer,
                         DescriptorType type,
                         Device* device,
                         uint32_t desc_set,
                         uint32_t binding,
                         Pipeline* pipeline);
  ~BufferBackedDescriptor() override;

  Result CreateResourceIfNeeded() override { return {}; }
  static Result RecordCopyBufferDataToTransferResourceIfNeeded(
      CommandBuffer* command_buffer,
      Buffer* buffer,
      Resource* transfer_resource);
  static Result RecordCopyTransferResourceToHost(CommandBuffer* command_buffer,
                                                 Resource* transfer_resource);
  static Result MoveTransferResourceToBufferOutput(Resource* transfer_resource,
                                                   Buffer* buffer);
  uint32_t GetDescriptorCount() override {
    return static_cast<uint32_t>(amber_buffers_.size());
  }
  const std::vector<Buffer*>& GetAmberBuffers() const { return amber_buffers_; }
  void AddAmberBuffer(Buffer* buffer) { amber_buffers_.push_back(buffer); }
  BufferBackedDescriptor* AsBufferBackedDescriptor() override { return this; }
  bool IsReadOnly() const;

 protected:
  // Pipeline where this descriptor is attached to.
  Pipeline* pipeline_;

 private:
  std::vector<Buffer*> amber_buffers_;
};

}  // namespace vulkan
}  // namespace amber

#endif  // SRC_VULKAN_BUFFER_BACKED_DESCRIPTOR_H_
