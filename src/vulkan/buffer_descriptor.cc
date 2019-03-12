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

#include "src/vulkan/buffer_descriptor.h"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <utility>
#include <vector>

#include "src/engine.h"
#include "src/make_unique.h"
#include "src/vulkan/command_buffer.h"

namespace amber {
namespace vulkan {

BufferDescriptor::BufferDescriptor(Buffer* buffer,
                                   DescriptorType type,
                                   Device* device,
                                   uint32_t desc_set,
                                   uint32_t binding)
    : Descriptor(type, device, desc_set, binding), amber_buffer_(buffer) {
  assert(type == DescriptorType::kStorageBuffer ||
         type == DescriptorType::kUniformBuffer);
}

BufferDescriptor::~BufferDescriptor() = default;

Result BufferDescriptor::CreateResourceIfNeeded(
    const VkPhysicalDeviceMemoryProperties& properties) {
  if (transfer_buffer_) {
    return Result(
        "Vulkan: BufferDescriptor::CreateResourceIfNeeded() must be called "
        "only when |transfer_buffer| is empty");
  }

  if (amber_buffer_ && amber_buffer_->ValuePtr()->empty())
    return {};

  size_t size_in_bytes = amber_buffer_ ? amber_buffer_->ValuePtr()->size() : 0;
  transfer_buffer_ =
      MakeUnique<TransferBuffer>(device_, size_in_bytes, properties);

  Result r = transfer_buffer_->Initialize(GetVkBufferUsage() |
                                          VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                                          VK_BUFFER_USAGE_TRANSFER_DST_BIT);
  if (!r.IsSuccess())
    return r;

  SetUpdateDescriptorSetNeeded();
  return {};
}

Result BufferDescriptor::RecordCopyDataToResourceIfNeeded(
    CommandBuffer* command) {
  if (amber_buffer_ && !amber_buffer_->ValuePtr()->empty()) {
    transfer_buffer_->UpdateMemoryWithRawData(*amber_buffer_->ValuePtr());
    amber_buffer_->ValuePtr()->clear();
  }

  transfer_buffer_->CopyToDevice(command);
  return {};
}

Result BufferDescriptor::RecordCopyDataToHost(CommandBuffer* command) {
  if (!transfer_buffer_) {
    return Result(
        "Vulkan: BufferDescriptor::RecordCopyDataToHost() |vk_buffer| is "
        "empty");
  }

  return transfer_buffer_->CopyToHost(command);
}

Result BufferDescriptor::MoveResourceToBufferOutput() {
  if (!transfer_buffer_) {
    return Result(
        "Vulkan: BufferDescriptor::MoveResourceToBufferOutput() |vk_buffer| "
        "is empty");
  }

  // Only need to copy the buffer back if we have an attached amber buffer to
  // write too.
  if (amber_buffer_) {
    void* resource_memory_ptr = transfer_buffer_->HostAccessibleMemoryPtr();
    if (!resource_memory_ptr) {
      return Result(
          "Vulkan: BufferDescriptor::MoveResourceToBufferOutput() |vk_buffer| "
          "has nullptr host accessible memory");
    }

    if (!amber_buffer_->ValuePtr()->empty()) {
      return Result(
          "Vulkan: BufferDescriptor::MoveResourceToBufferOutput() "
          "output buffer is not empty");
    }

    auto size_in_bytes = transfer_buffer_->GetSizeInBytes();
    amber_buffer_->SetSize(size_in_bytes);
    amber_buffer_->ValuePtr()->resize(size_in_bytes);
    std::memcpy(amber_buffer_->ValuePtr()->data(), resource_memory_ptr,
                size_in_bytes);
  }

  transfer_buffer_->Shutdown();
  transfer_buffer_ = nullptr;
  return {};
}

Result BufferDescriptor::UpdateDescriptorSetIfNeeded(
    VkDescriptorSet descriptor_set) {
  if (!IsDescriptorSetUpdateNeeded())
    return {};

  VkDescriptorBufferInfo buffer_info = VkDescriptorBufferInfo();
  buffer_info.buffer = transfer_buffer_->GetVkBuffer();
  buffer_info.offset = 0;
  buffer_info.range = VK_WHOLE_SIZE;

  return Descriptor::UpdateDescriptorSetForBuffer(
      descriptor_set, GetVkDescriptorType(), buffer_info);
}

void BufferDescriptor::Shutdown() {
  if (transfer_buffer_)
    transfer_buffer_->Shutdown();
}

Result BufferDescriptor::AddToBuffer(DataType type,
                                     uint32_t offset,
                                     size_t size_in_bytes,
                                     const std::vector<Value>& values) {
  if (!amber_buffer_)
    return Result("missing amber_buffer for AddToBuffer call");

  if (amber_buffer_->ValuePtr()->size() < offset + size_in_bytes) {
    amber_buffer_->ValuePtr()->resize(offset + size_in_bytes);
    amber_buffer_->SetSize(offset + size_in_bytes);
  }

  BufferInput in{offset, size_in_bytes, type, values};
  in.UpdateBufferWithValues(amber_buffer_->GetMemPtr());

  return {};
}

}  // namespace vulkan
}  // namespace amber
