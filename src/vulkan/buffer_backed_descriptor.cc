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
    : Descriptor(type, device, desc_set, binding) {
  AddAmberBuffer(buffer);
}

BufferBackedDescriptor::~BufferBackedDescriptor() = default;

Result BufferBackedDescriptor::RecordCopyDataToResourceIfNeeded(
    CommandBuffer* command) {
  auto resources = GetResources();
  auto buffers = GetAmberBuffers();
  if (resources.size() != buffers.size())
    return Result(
        "Vulkan: BufferBackedDescriptor::RecordCopyDataToResourceIfNeeded() "
        "resource and buffer vector sizes are not matching");

  for (size_t i = 0; i < resources.size(); i++) {
    if (!buffers[i]->ValuePtr()->empty()) {
      resources[i]->UpdateMemoryWithRawData(*buffers[i]->ValuePtr());
      buffers[i]->ValuePtr()->clear();
    }

    resources[i]->CopyToDevice(command);
  }

  return {};
}

Result BufferBackedDescriptor::RecordCopyDataToHost(CommandBuffer* command) {
  if (GetResources().empty()) {
    return Result(
        "Vulkan: BufferBackedDescriptor::RecordCopyDataToHost() no transfer "
        "resources");
  }

  for (const auto& r : GetResources())
    r->CopyToHost(command);

  return {};
}

Result BufferBackedDescriptor::MoveResourceToBufferOutput() {
  auto resources = GetResources();
  auto buffers = GetAmberBuffers();
  if (resources.size() != buffers.size())
    return Result(
        "Vulkan: BufferBackedDescriptor::MoveResourceToBufferOutput() resource "
        "and buffer vector sizes are not matching");

  if (resources.empty()) {
    return Result(
        "Vulkan: BufferBackedDescriptor::MoveResourceToBufferOutput() no "
        "transfer"
        " resource");
  }

  for (size_t i = 0; i < resources.size(); i++) {
    void* resource_memory_ptr = resources[i]->HostAccessibleMemoryPtr();
    if (!resource_memory_ptr) {
      return Result(
          "Vulkan: BufferBackedDescriptor::MoveResourceToBufferOutput() "
          "no host accessible memory pointer");
    }

    if (!buffers[i]->ValuePtr()->empty()) {
      return Result(
          "Vulkan: BufferBackedDescriptor::MoveResourceToBufferOutput() "
          "output buffer is not empty");
    }

    auto size_in_bytes = resources[i]->GetSizeInBytes();
    buffers[i]->SetElementCount(size_in_bytes /
                                buffers[i]->GetFormat()->SizeInBytes());
    buffers[i]->ValuePtr()->resize(size_in_bytes);
    std::memcpy(buffers[i]->ValuePtr()->data(), resource_memory_ptr,
                size_in_bytes);
  }

  return {};
}

}  // namespace vulkan
}  // namespace amber
