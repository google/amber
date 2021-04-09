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
  for (const auto& resource : GetResources()) {
    if (!resource.first->ValuePtr()->empty()) {
      resource.second->UpdateMemoryWithRawData(*resource.first->ValuePtr());
      // If the resource is read-only, keep the buffer data; Amber won't copy
      // read-only resources back into the host buffers, so it makes sense to
      // leave the buffer intact.
      if (!IsReadOnly())
        resource.first->ValuePtr()->clear();
    }

    resource.second->CopyToDevice(command);
  }
  return {};
}

Result BufferBackedDescriptor::RecordCopyDataToHost(CommandBuffer* command) {
  if (!IsReadOnly()) {
    if (GetResources().empty()) {
      return Result(
          "Vulkan: BufferBackedDescriptor::RecordCopyDataToHost() no transfer "
          "resources");
    }

    for (const auto& r : GetResources())
      r.second->CopyToHost(command);
  }

  return {};
}

Result BufferBackedDescriptor::MoveResourceToBufferOutput() {
  // No need to copy results of read only resources.
  if (IsReadOnly())
    return {};

  auto resources = GetResources();

  if (resources.empty()) {
    return Result(
        "Vulkan: BufferBackedDescriptor::MoveResourceToBufferOutput() no "
        "transfer resource");
  }

  for (const auto& resource : resources) {
    void* resource_memory_ptr = resource.second->HostAccessibleMemoryPtr();
    auto* buffer = resource.first;
    if (!resource_memory_ptr) {
      return Result(
          "Vulkan: BufferBackedDescriptor::MoveResourceToBufferOutput() "
          "no host accessible memory pointer");
    }

    if (!buffer->ValuePtr()->empty()) {
      return Result(
          "Vulkan: BufferBackedDescriptor::MoveResourceToBufferOutput() "
          "output buffer is not empty");
    }

    auto size_in_bytes = resource.second->GetSizeInBytes();
    buffer->SetElementCount(size_in_bytes / buffer->GetFormat()->SizeInBytes());
    buffer->ValuePtr()->resize(size_in_bytes);
    std::memcpy(buffer->ValuePtr()->data(), resource_memory_ptr, size_in_bytes);
  }

  return {};
}

bool BufferBackedDescriptor::IsReadOnly() const {
  switch (type_) {
    case DescriptorType::kUniformBuffer:
    case DescriptorType::kUniformBufferDynamic:
    case DescriptorType::kUniformTexelBuffer:
    case DescriptorType::kSampledImage:
    case DescriptorType::kCombinedImageSampler:
      return true;
    case DescriptorType::kStorageBuffer:
    case DescriptorType::kStorageBufferDynamic:
    case DescriptorType::kStorageTexelBuffer:
    case DescriptorType::kStorageImage:
      return false;
    default:
      assert(false && "Unexpected descriptor type");
      return false;
  }
}

}  // namespace vulkan
}  // namespace amber
