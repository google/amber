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
#include <utility>
#include <vector>

#include "src/make_unique.h"
#include "src/vulkan/command_buffer.h"
#include "src/vulkan/device.h"

namespace amber {
namespace vulkan {

BufferDescriptor::BufferDescriptor(Buffer* buffer,
                                   DescriptorType type,
                                   Device* device,
                                   uint32_t desc_set,
                                   uint32_t binding,
                                   Pipeline* pipeline)
    : BufferBackedDescriptor(buffer,
                             type,
                             device,
                             desc_set,
                             binding,
                             pipeline) {}

BufferDescriptor::~BufferDescriptor() = default;

Result BufferDescriptor::CreateResourceIfNeeded() {
  auto& transfer_resources = pipeline_->GetDescriptorTransferResources();

  VkBufferUsageFlags flags =
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
  if (IsUniformBuffer() || IsUniformBufferDynamic()) {
    flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
  } else if (IsStorageBuffer() || IsStorageBufferDynamic()) {
    flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
  } else if (IsUniformTexelBuffer()) {
    flags |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
  } else if (IsStorageTexelBuffer()) {
    flags |= VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
  } else {
    return Result("Unexpected buffer type when deciding usage flags");
  }

  for (const auto& amber_buffer : GetAmberBuffers()) {
    // Create (but don't initialize) the transfer buffer if not already created.
    if (transfer_resources.count(amber_buffer) == 0) {
      auto size_in_bytes =
          static_cast<uint32_t>(amber_buffer->ValuePtr()->size());
      auto transfer_buffer = MakeUnique<TransferBuffer>(
          device_, size_in_bytes, amber_buffer->GetFormat());
      transfer_buffer->SetReadOnly(IsReadOnly());
      transfer_resources[amber_buffer] = std::move(transfer_buffer);
    } else {
      // Unset transfer buffer's read only property if needed.
      if (!IsReadOnly()) {
        transfer_resources[amber_buffer]->SetReadOnly(false);
      }
    }

    // Update the buffer create flags.
    Result r =
        transfer_resources[amber_buffer]->AsTransferBuffer()->AddUsageFlags(
            flags);
    if (!r.IsSuccess())
      return r;
  }
  is_descriptor_set_update_needed_ = true;

  descriptor_offsets_.reserve(GetAmberBuffers().size());
  descriptor_ranges_.reserve(GetAmberBuffers().size());

  return {};
}

void BufferDescriptor::UpdateDescriptorSetIfNeeded(
    VkDescriptorSet descriptor_set) {
  if (!is_descriptor_set_update_needed_)
    return;

  std::vector<VkDescriptorBufferInfo> buffer_infos;
  std::vector<VkBufferView> buffer_views;

  // Create VkDescriptorBufferInfo for every descriptor buffer.
  for (uint32_t i = 0; i < GetAmberBuffers().size(); i++) {
    const auto& buffer =
        pipeline_->GetDescriptorTransferResources()[GetAmberBuffers()[i]]
            ->AsTransferBuffer();
    assert(buffer->GetVkBuffer() && "Unexpected descriptor type");
    // Add buffer infos for uniform and storage buffers.
    if (IsUniformBuffer() || IsUniformBufferDynamic() || IsStorageBuffer() ||
        IsStorageBufferDynamic()) {
      uint64_t range = descriptor_ranges_[i];
      // If dynamic offset is used, we must change range with VK_WHOLE_SIZE to
      // an actual range.
      // From vulkan spec: For each dynamic uniform or storage buffer binding in
      // pDescriptorSets, the sum of the effective offset, as defined above, and
      // the range of the binding must be less than or equal to the size of the
      // buffer.
      if ((IsUniformBufferDynamic() || IsStorageBufferDynamic()) &&
          descriptor_ranges_[i] == VK_WHOLE_SIZE) {
        range = buffer->GetSizeInBytes() - dynamic_offsets_[i] -
                descriptor_offsets_[i];
      }

      VkDescriptorBufferInfo buffer_info;
      buffer_info.buffer = buffer->GetVkBuffer();
      buffer_info.offset = descriptor_offsets_[i];
      buffer_info.range = range;

      buffer_infos.push_back(buffer_info);
    }

    if (IsUniformTexelBuffer() || IsStorageTexelBuffer()) {
      buffer_views.push_back(*buffer->GetVkBufferView());
    }
  }

  VkWriteDescriptorSet write = VkWriteDescriptorSet();
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.dstSet = descriptor_set;
  write.dstBinding = binding_;
  write.dstArrayElement = 0;
  write.descriptorCount = static_cast<uint32_t>(GetAmberBuffers().size());
  write.descriptorType = GetVkDescriptorType();
  write.pBufferInfo = buffer_infos.data();
  write.pTexelBufferView = buffer_views.data();

  device_->GetPtrs()->vkUpdateDescriptorSets(device_->GetVkDevice(), 1, &write,
                                             0, nullptr);
  is_descriptor_set_update_needed_ = false;
}

}  // namespace vulkan
}  // namespace amber
