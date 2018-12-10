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
#include <utility>
#include <vector>

#include "src/engine.h"
#include "src/make_unique.h"

namespace amber {
namespace vulkan {
namespace {

// TODO(jaebaek): Make this as a protected method of Descriptor.
template <typename T>
void SetValueForBuffer(void* memory, const std::vector<Value>& values) {
  T* ptr = static_cast<T*>(memory);
  for (const auto& v : values) {
    *ptr = static_cast<T>(v.IsInteger() ? v.AsUint64() : v.AsDouble());
    ++ptr;
  }
}

}  // namespace

BufferDescriptor::BufferDescriptor(DescriptorType type,
                                   VkDevice device,
                                   uint32_t desc_set,
                                   uint32_t binding)
    : Descriptor(type, device, desc_set, binding) {
  assert(type == DescriptorType::kStorageBuffer ||
         type == DescriptorType::kUniformBuffer);
}

BufferDescriptor::~BufferDescriptor() = default;

// TODO(jaebaek): Add unittests for this method.
void BufferDescriptor::FillBufferWithData(const BufferData& data) {
  uint8_t* ptr =
      static_cast<uint8_t*>(buffer_->HostAccessibleMemoryPtr()) + data.offset;
  switch (data.type) {
    case DataType::kInt8:
    case DataType::kUint8:
      SetValueForBuffer<uint8_t>(ptr, data.values);
      break;
    case DataType::kInt16:
    case DataType::kUint16:
      SetValueForBuffer<uint16_t>(ptr, data.values);
      break;
    case DataType::kInt32:
    case DataType::kUint32:
      SetValueForBuffer<uint32_t>(ptr, data.values);
      break;
    case DataType::kInt64:
    case DataType::kUint64:
      SetValueForBuffer<uint64_t>(ptr, data.values);
      break;
    case DataType::kFloat:
      SetValueForBuffer<float>(ptr, data.values);
      break;
    case DataType::kDouble:
      SetValueForBuffer<double>(ptr, data.values);
      break;
  }
}

Result BufferDescriptor::CreateOrResizeIfNeeded(
    VkCommandBuffer command,
    const VkPhysicalDeviceMemoryProperties& properties) {
  const auto& buffer_data_queue = GetBufferDataQueue();

  if (buffer_data_queue.empty())
    return {};

  auto buffer_data_with_last_offset = std::max_element(
      buffer_data_queue.begin(), buffer_data_queue.end(),
      [](const BufferData& a, const BufferData& b) {
        return static_cast<size_t>(a.offset) + a.size_in_bytes <
               static_cast<size_t>(b.offset) + b.size_in_bytes;
      });
  size_t new_size_in_bytes =
      static_cast<size_t>(buffer_data_with_last_offset->offset) +
      buffer_data_with_last_offset->size_in_bytes;

  if (!buffer_) {
    // Create buffer
    buffer_ = MakeUnique<Buffer>(GetDevice(), new_size_in_bytes, properties);

    Result r = buffer_->Initialize(GetVkBufferUsage() |
                                   VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                                   VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    if (!r.IsSuccess())
      return r;

    SetUpdateDescriptorSetNeeded();
  } else if (buffer_->GetSizeInBytes() < new_size_in_bytes) {
    // Resize buffer
    auto new_buffer =
        MakeUnique<Buffer>(GetDevice(), new_size_in_bytes, properties);

    Result r = new_buffer->Initialize(GetVkBufferUsage() |
                                      VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                                      VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    if (!r.IsSuccess())
      return r;

    new_buffer->CopyFromBuffer(command, *buffer_);

    buffer_ = std::move(new_buffer);

    SetUpdateDescriptorSetNeeded();
  }

  return {};
}

void BufferDescriptor::UpdateResourceIfNeeded(VkCommandBuffer command) {
  const auto& buffer_data_queue = GetBufferDataQueue();

  if (buffer_data_queue.empty())
    return;

  for (const auto& data : buffer_data_queue) {
    FillBufferWithData(data);
  }
  ClearBufferDataQueue();

  buffer_->CopyToDevice(command);
}

Result BufferDescriptor::SendDataToHostIfNeeded(VkCommandBuffer command) {
  return buffer_->CopyToHost(command);
}

Result BufferDescriptor::UpdateDescriptorSetIfNeeded(
    VkDescriptorSet descriptor_set) {
  if (!IsDescriptorSetUpdateNeeded())
    return {};

  VkDescriptorBufferInfo buffer_info = {};
  buffer_info.buffer = buffer_->GetVkBuffer();
  buffer_info.offset = 0;
  buffer_info.range = VK_WHOLE_SIZE;

  return Descriptor::UpdateDescriptorSetForBuffer(
      descriptor_set, GetVkDescriptorType(), buffer_info);
}

ResourceInfo BufferDescriptor::GetResourceInfo() {
  ResourceInfo info = {};
  info.type = ResourceInfoType::kBuffer;
  info.size_in_bytes = buffer_->GetSizeInBytes();
  info.cpu_memory = buffer_->HostAccessibleMemoryPtr();
  return info;
}

void BufferDescriptor::Shutdown() {
  buffer_->Shutdown();
}

}  // namespace vulkan
}  // namespace amber
