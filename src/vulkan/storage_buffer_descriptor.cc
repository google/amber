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

#include "src/vulkan/storage_buffer_descriptor.h"

#include <vector>

#include "src/make_unique.h"

namespace amber {
namespace vulkan {
namespace {

// TODO(jaebaek): Make this as a protected method of Descriptor.
template <typename T>
void SetUintValueForBuffer(void* memory, const std::vector<Value>& values) {
  T* ptr = static_cast<T*>(memory);
  for (size_t i = 0; i < values.size(); ++i) {
    *ptr = static_cast<T>(values[i].AsUint64());
    ++ptr;
  }
}

// TODO(jaebaek): Make this as a protected method of Descriptor.
template <typename T>
void SetFloatValueForBuffer(void* memory, const std::vector<Value>& values) {
  T* ptr = static_cast<T*>(memory);
  for (size_t i = 0; i < values.size(); ++i) {
    *ptr = static_cast<T>(values[i].AsDouble());
    ++ptr;
  }
}

}  // namespace

StorageBufferDescriptor::StorageBufferDescriptor(
    VkDevice device,
    uint32_t desc_set,
    uint32_t binding,
    size_t size,
    const VkPhysicalDeviceMemoryProperties& properties)
    : Descriptor(DescriptorType::kStorageBuffer, device, desc_set, binding),
      buffer_(MakeUnique<Buffer>(device, size, properties)) {}

StorageBufferDescriptor::~StorageBufferDescriptor() = default;

Result StorageBufferDescriptor::Initialize(DataType type,
                                           const std::vector<Value>& values) {
  Result r = buffer_->Initialize(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                                 VK_BUFFER_USAGE_TRANSFER_DST_BIT);
  if (!r.IsSuccess())
    return r;

  switch (type) {
    case DataType::kInt8:
    case DataType::kUint8:
      SetUintValueForBuffer<uint8_t>(buffer_->HostAccessibleMemoryPtr(),
                                     values);
      break;
    case DataType::kInt16:
    case DataType::kUint16:
      SetUintValueForBuffer<uint16_t>(buffer_->HostAccessibleMemoryPtr(),
                                      values);
      break;
    case DataType::kInt32:
    case DataType::kUint32:
      SetUintValueForBuffer<uint32_t>(buffer_->HostAccessibleMemoryPtr(),
                                      values);
      break;
    case DataType::kInt64:
    case DataType::kUint64:
      SetUintValueForBuffer<uint64_t>(buffer_->HostAccessibleMemoryPtr(),
                                      values);
      break;
    case DataType::kFloat:
      SetFloatValueForBuffer<float>(buffer_->HostAccessibleMemoryPtr(), values);
      break;
    case DataType::kDouble:
      SetFloatValueForBuffer<double>(buffer_->HostAccessibleMemoryPtr(),
                                     values);
      break;
    default:
      return Result("StorageBufferDescriptor::Initialize unknown data type");
  }

  return {};
}

void StorageBufferDescriptor::SendDataToGPUIfNeeded(VkCommandBuffer command) {
  // TODO(jaebaek): VkRunner script allows data updating after initialiation.
  //                Support updating data.
  if (IsDataAlreadySent())
    return;

  buffer_->CopyToDevice(command);
  SetDataSent();
}

Result StorageBufferDescriptor::UpdateDescriptorSet(
    VkDescriptorSet descriptor_set) {
  VkDescriptorBufferInfo buffer_info = {};
  buffer_info.buffer = buffer_->GetVkBuffer();
  buffer_info.offset = 0;
  buffer_info.range = VK_WHOLE_SIZE;

  return Descriptor::UpdateDescriptorSetForBuffer(
      descriptor_set, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, buffer_info);
}

void StorageBufferDescriptor::Shutdown() {
  buffer_->Shutdown();
}

}  // namespace vulkan
}  // namespace amber
