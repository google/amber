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

#include "src/vulkan/buffer_backed_descriptor.h"
#include <cassert>
#include <cstring>
#include "src/vulkan/command_buffer.h"
#include "src/vulkan/device.h"

namespace amber {
namespace vulkan {

BufferBackedDescriptor::BufferBackedDescriptor(Buffer* buffer,
                                               DescriptorType type,
                                               Device* device,
                                               uint32_t desc_set,
                                               uint32_t binding)
    : Descriptor(type, device, desc_set, binding), amber_buffer_(buffer) {}

BufferBackedDescriptor::~BufferBackedDescriptor() = default;

void BufferBackedDescriptor::RecordCopyDataToResourceIfNeeded(
    CommandBuffer* command) {
  if (!GetResource())
    return;

  if (amber_buffer_ && !amber_buffer_->ValuePtr()->empty()) {
    GetResource()->UpdateMemoryWithRawData(*amber_buffer_->ValuePtr());
    amber_buffer_->ValuePtr()->clear();
  }

  GetResource()->CopyToDevice(command);
}

Result BufferBackedDescriptor::RecordCopyDataToHost(CommandBuffer* command) {
  if (!GetResource()) {
    return Result(
        "Vulkan: BufferBackedDescriptor::RecordCopyDataToHost() no transfer "
        "resource");
  }

  GetResource()->CopyToHost(command);
  return {};
}

Result BufferBackedDescriptor::MoveResourceToBufferOutput() {
  if (!GetResource()) {
    return Result(
        "Vulkan: BufferBackedDescriptor::MoveResourceToBufferOutput() no "
        "transfer"
        " resource");
  }

  // Only need to copy the buffer back if we have an attached amber buffer to
  // write to.
  if (amber_buffer_) {
    void* resource_memory_ptr = GetResource()->HostAccessibleMemoryPtr();
    if (!resource_memory_ptr) {
      return Result(
          "Vulkan: BufferBackedDescriptor::MoveResourceToBufferOutput() "
          "no host accessible memory pointer");
    }

    if (!amber_buffer_->ValuePtr()->empty()) {
      return Result(
          "Vulkan: BufferBackedDescriptor::MoveResourceToBufferOutput() "
          "output buffer is not empty");
    }

    auto size_in_bytes = GetResource()->GetSizeInBytes();
    amber_buffer_->SetElementCount(size_in_bytes /
                                   amber_buffer_->GetFormat()->SizeInBytes());
    amber_buffer_->ValuePtr()->resize(size_in_bytes);
    std::memcpy(amber_buffer_->ValuePtr()->data(), resource_memory_ptr,
                size_in_bytes);
  }

  return {};
}

Result BufferBackedDescriptor::SetSizeInElements(uint32_t element_count) {
  if (!amber_buffer_)
    return Result("missing amber_buffer for SetSizeInElements call");

  amber_buffer_->SetSizeInElements(element_count);
  return {};
}

Result BufferBackedDescriptor::AddToBuffer(const std::vector<Value>& values,
                                           uint32_t offset) {
  if (!amber_buffer_)
    return Result("missing amber_buffer for AddToBuffer call");

  return amber_buffer_->SetDataWithOffset(values, offset);
}

}  // namespace vulkan
}  // namespace amber
