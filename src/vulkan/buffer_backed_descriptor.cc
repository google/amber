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
                                               uint32_t binding,
                                               Pipeline* pipeline)
    : Descriptor(type, device, desc_set, binding), pipeline_(pipeline) {
  AddAmberBuffer(buffer);
}

BufferBackedDescriptor::~BufferBackedDescriptor() = default;

Result BufferBackedDescriptor::RecordCopyBufferDataToTransferResourceIfNeeded(
    CommandBuffer* command_buffer,
    Buffer* buffer,
    Resource* transfer_resource) {
  transfer_resource->UpdateMemoryWithRawData(*buffer->ValuePtr());
  // If the resource is read-only, keep the buffer data; Amber won't copy
  // read-only resources back into the host buffers, so it makes sense to
  // leave the buffer intact.
  if (!transfer_resource->IsReadOnly())
    buffer->ValuePtr()->clear();

  transfer_resource->CopyToDevice(command_buffer);
  return {};
}

Result BufferBackedDescriptor::RecordCopyTransferResourceToHost(
    CommandBuffer* command_buffer,
    Resource* transfer_resource) {
  if (!transfer_resource->IsReadOnly()) {
    transfer_resource->CopyToHost(command_buffer);
  }

  return {};
}

Result BufferBackedDescriptor::MoveTransferResourceToBufferOutput(
    Resource* transfer_resource,
    Buffer* buffer) {
  // No need to move read only resources to an output buffer.
  if (transfer_resource->IsReadOnly()) {
    return {};
  }

  void* resource_memory_ptr = transfer_resource->HostAccessibleMemoryPtr();
  if (!resource_memory_ptr) {
    return Result(
        "Vulkan: BufferBackedDescriptor::MoveTransferResourceToBufferOutput() "
        "no host accessible memory pointer");
  }

  if (!buffer->ValuePtr()->empty()) {
    return Result(
        "Vulkan: BufferBackedDescriptor::MoveTransferResourceToBufferOutput() "
        "output buffer is not empty");
  }

  auto size_in_bytes = transfer_resource->GetSizeInBytes();
  buffer->SetElementCount(size_in_bytes / buffer->GetFormat()->SizeInBytes());
  buffer->ValuePtr()->resize(size_in_bytes);
  std::memcpy(buffer->ValuePtr()->data(), resource_memory_ptr, size_in_bytes);

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
