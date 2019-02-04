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

namespace amber {
namespace vulkan {
namespace {

// Return the size in bytes for a buffer that has enough capacity to
// copy all data in |buffer_input_queue|.
size_t GetBufferSizeInBytesForQueue(
    const std::vector<BufferInput>& buffer_input_queue) {
  if (buffer_input_queue.empty())
    return 0;

  auto buffer_data_with_last_offset = std::max_element(
      buffer_input_queue.begin(), buffer_input_queue.end(),
      [](const BufferInput& a, const BufferInput& b) {
        return static_cast<size_t>(a.offset) + a.size_in_bytes <
               static_cast<size_t>(b.offset) + b.size_in_bytes;
      });
  return static_cast<size_t>(buffer_data_with_last_offset->offset) +
         buffer_data_with_last_offset->size_in_bytes;
}

}  // namespace

BufferDescriptor::BufferDescriptor(DescriptorType type,
                                   Device* device,
                                   uint32_t desc_set,
                                   uint32_t binding)
    : Descriptor(type, device, desc_set, binding) {
  assert(type == DescriptorType::kStorageBuffer ||
         type == DescriptorType::kUniformBuffer);
}

BufferDescriptor::~BufferDescriptor() = default;

Result BufferDescriptor::CreateResourceIfNeeded(
    const VkPhysicalDeviceMemoryProperties& properties) {
  // Amber copies back contents of |buffer_| to host and put it into
  // |buffer_input_queue_| right after draw or compute. Therefore,
  // when calling this method, |buffer_| must be always empty.
  if (buffer_) {
    return Result(
        "Vulkan: BufferDescriptor::CreateResourceIfNeeded() must be called "
        "only when |buffer_| is empty");
  }

  const auto& buffer_input_queue = GetBufferInputQueue();
  const auto& buffer_output = GetBufferOutput();

  if (buffer_input_queue.empty() && buffer_output.empty())
    return {};

  size_t size_in_bytes = GetBufferSizeInBytesForQueue(buffer_input_queue);
  if (buffer_output.size() > size_in_bytes)
    size_in_bytes = buffer_output.size();

  buffer_ = MakeUnique<Buffer>(device_, size_in_bytes, properties);

  Result r = buffer_->Initialize(GetVkBufferUsage() |
                                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                                 VK_BUFFER_USAGE_TRANSFER_DST_BIT);
  if (!r.IsSuccess())
    return r;

  SetUpdateDescriptorSetNeeded();
  return {};
}

Result BufferDescriptor::RecordCopyDataToResourceIfNeeded(
    VkCommandBuffer command) {
  auto& buffer_output = GetBufferOutput();
  if (!buffer_output.empty()) {
    buffer_->UpdateMemoryWithRawData(buffer_output);
    buffer_output.clear();
  }

  const auto& buffer_input_queue = GetBufferInputQueue();
  if (buffer_input_queue.empty())
    return {};

  for (const auto& input : buffer_input_queue) {
    Result r = buffer_->UpdateMemoryWithInput(input);
    if (!r.IsSuccess())
      return r;
  }

  ClearBufferInputQueue();

  buffer_->CopyToDevice(command);
  return {};
}

Result BufferDescriptor::RecordCopyDataToHost(VkCommandBuffer command) {
  if (!buffer_) {
    return Result(
        "Vulkan: BufferDescriptor::RecordCopyDataToHost() |buffer_| is empty");
  }

  return buffer_->CopyToHost(command);
}

Result BufferDescriptor::MoveResourceToBufferOutput() {
  if (!buffer_) {
    return Result(
        "Vulkan: BufferDescriptor::MoveResourceToBufferOutput() |buffer_| "
        "is empty");
  }

  void* resource_memory_ptr = buffer_->HostAccessibleMemoryPtr();
  if (!resource_memory_ptr) {
    return Result(
        "Vulkan: BufferDescriptor::MoveResourceToBufferOutput() |buffer_| "
        "has nullptr host accessible memory");
  }

  auto& buffer_output = GetBufferOutput();
  if (!buffer_output.empty()) {
    return Result(
        "Vulkan: BufferDescriptor::MoveResourceToBufferOutput() "
        "|buffer_output_| is not empty");
  }

  auto size_in_bytes = buffer_->GetSizeInBytes();
  buffer_output.resize(size_in_bytes);
  std::memcpy(buffer_output.data(), resource_memory_ptr, size_in_bytes);

  buffer_->Shutdown();
  buffer_ = nullptr;
  return {};
}

Result BufferDescriptor::UpdateDescriptorSetIfNeeded(
    VkDescriptorSet descriptor_set) {
  if (!IsDescriptorSetUpdateNeeded())
    return {};

  VkDescriptorBufferInfo buffer_info = VkDescriptorBufferInfo();
  buffer_info.buffer = buffer_->GetVkBuffer();
  buffer_info.offset = 0;
  buffer_info.range = VK_WHOLE_SIZE;

  return Descriptor::UpdateDescriptorSetForBuffer(
      descriptor_set, GetVkDescriptorType(), buffer_info);
}

ResourceInfo BufferDescriptor::GetResourceInfo() {
  auto& buffer_input_queue = GetBufferInputQueue();
  auto& buffer_output = GetBufferOutput();

  ResourceInfo info = ResourceInfo();
  info.descriptor_set = GetDescriptorSet();
  info.binding = GetBinding();
  info.type = ResourceInfoType::kBuffer;
  if (buffer_) {
    assert(buffer_input_queue.empty() && buffer_output.empty());

    info.size_in_bytes = buffer_->GetSizeInBytes();
    info.cpu_memory = buffer_->HostAccessibleMemoryPtr();
    return info;
  }

  if (buffer_input_queue.empty()) {
    info.size_in_bytes = buffer_output.size();
    info.cpu_memory = buffer_output.data();
    return info;
  }

  // Squash elements of |buffer_input_queue_| into |buffer_output_|.
  size_t size_in_bytes = GetBufferSizeInBytesForQueue(buffer_input_queue);
  std::vector<uint8_t> new_buffer_output = buffer_output;
  if (size_in_bytes > new_buffer_output.size())
    new_buffer_output.resize(size_in_bytes);

  for (const auto& input : buffer_input_queue) {
    input.UpdateBufferWithValues(new_buffer_output.data());
  }
  buffer_input_queue.clear();

  buffer_output = new_buffer_output;

  info.size_in_bytes = buffer_output.size();
  info.cpu_memory = buffer_output.data();
  return info;
}

void BufferDescriptor::Shutdown() {
  if (buffer_)
    buffer_->Shutdown();

  for (auto& buffer : not_destroyed_buffers_) {
    if (buffer)
      buffer->Shutdown();
  }
}

}  // namespace vulkan
}  // namespace amber
