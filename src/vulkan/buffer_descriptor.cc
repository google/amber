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

// TODO(jaebaek): Make this as a protected method of Descriptor.
template <typename T>
void SetValueForBuffer(void* memory, const std::vector<Value>& values) {
  T* ptr = static_cast<T*>(memory);
  for (const auto& v : values) {
    *ptr = v.IsInteger() ? static_cast<T>(v.AsUint64())
                         : static_cast<T>(v.AsDouble());
    ++ptr;
  }
}

// Return the size in bytes for a buffer that has enough capacity to
// copy all data in |buffer_data_queue|.
size_t GetBufferSizeInBytesForQueue(
    const std::vector<BufferData>& buffer_data_queue) {
  if (buffer_data_queue.empty())
    return 0;

  auto buffer_data_with_last_offset = std::max_element(
      buffer_data_queue.begin(), buffer_data_queue.end(),
      [](const BufferData& a, const BufferData& b) {
        return static_cast<size_t>(a.offset) + a.size_in_bytes <
               static_cast<size_t>(b.offset) + b.size_in_bytes;
      });
  return static_cast<size_t>(buffer_data_with_last_offset->offset) +
         buffer_data_with_last_offset->size_in_bytes;
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
void BufferDescriptor::FillBufferWithData(void* host_memory,
                                          const BufferData& data) {
  uint8_t* ptr = static_cast<uint8_t*>(host_memory) + data.offset;
  if (data.raw_data) {
    std::memcpy(ptr, data.raw_data.get(), data.size_in_bytes);
    return;
  }

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

Result BufferDescriptor::CreateResourceIfNeeded(
    const VkPhysicalDeviceMemoryProperties& properties) {
  // Amber copies back contents of |buffer_| to host and put it into
  // |buffer_data_queue_| right after draw or compute. Therefore,
  // when calling this method, |buffer_| must be always empty.
  if (buffer_) {
    return Result(
        "Vulkan: BufferDescriptor::CreateResourceIfNeeded() must be called "
        "only when |buffer_| is empty");
  }

  const auto& buffer_data_queue = GetBufferDataQueue();

  if (buffer_data_queue.empty())
    return {};

  size_t size_in_bytes = GetBufferSizeInBytesForQueue(buffer_data_queue);

  buffer_ = MakeUnique<Buffer>(GetDevice(), size_in_bytes, properties);

  Result r = buffer_->Initialize(GetVkBufferUsage() |
                                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                                 VK_BUFFER_USAGE_TRANSFER_DST_BIT);
  if (!r.IsSuccess())
    return r;

  SetUpdateDescriptorSetNeeded();
  return {};
}

void BufferDescriptor::CopyDataToResourceIfNeeded(VkCommandBuffer command) {
  const auto& buffer_data_queue = GetBufferDataQueue();

  if (buffer_data_queue.empty())
    return;

  for (const auto& data : buffer_data_queue) {
    FillBufferWithData(buffer_->HostAccessibleMemoryPtr(), data);
  }
  ClearBufferDataQueue();

  buffer_->CopyToDevice(command);
}

Result BufferDescriptor::CopyDataToHost(VkCommandBuffer command) {
  if (!buffer_) {
    return Result(
        "Vulkan: BufferDescriptor::CopyDataToHost() |buffer_| is empty");
  }

  return buffer_->CopyToHost(command);
}

Result BufferDescriptor::MoveResourceToBufferDataQueue() {
  if (!buffer_) {
    return Result(
        "Vulkan: BufferDescriptor::MoveResourceToBufferDataQueue() |buffer_| "
        "is empty");
  }

  void* resource_memory_ptr = buffer_->HostAccessibleMemoryPtr();
  if (!resource_memory_ptr) {
    return Result(
        "Vulkan: BufferDescriptor::MoveResourceToBufferDataQueue() |buffer_| "
        "has nullptr host accessible memory");
  }

  auto size_in_bytes = buffer_->GetSizeInBytes();
  auto& buffer_data_queue = GetBufferDataQueue();
  buffer_data_queue.emplace_back();
  buffer_data_queue.back().offset = 0;
  buffer_data_queue.back().size_in_bytes = size_in_bytes;
  buffer_data_queue.back().raw_data =
      std::unique_ptr<uint8_t>(new uint8_t[size_in_bytes]);

  std::memcpy(buffer_data_queue.back().raw_data.get(), resource_memory_ptr,
              size_in_bytes);

  buffer_->Shutdown();
  buffer_ = nullptr;
  return {};
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
  if (buffer_) {
    info.size_in_bytes = buffer_->GetSizeInBytes();
    info.cpu_memory = buffer_->HostAccessibleMemoryPtr();
    return info;
  }

  auto& buffer_data_queue = GetBufferDataQueue();
  if (buffer_data_queue.empty()) {
    info.size_in_bytes = 0;
    info.cpu_memory = nullptr;
    return info;
  }

  size_t size_in_bytes = GetBufferSizeInBytesForQueue(buffer_data_queue);

  auto raw_data = std::unique_ptr<uint8_t>(new uint8_t[size_in_bytes]);

  for (const auto& data : buffer_data_queue) {
    FillBufferWithData(raw_data.get(), data);
  }
  buffer_data_queue.clear();

  buffer_data_queue.emplace_back();
  buffer_data_queue.back().offset = 0;
  buffer_data_queue.back().size_in_bytes = size_in_bytes;
  buffer_data_queue.back().raw_data = std::move(raw_data);

  info.size_in_bytes = buffer_data_queue.back().size_in_bytes;
  info.cpu_memory = buffer_data_queue.back().raw_data.get();
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
