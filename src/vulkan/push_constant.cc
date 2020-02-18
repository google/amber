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

#include "src/vulkan/push_constant.h"

#include <algorithm>
#include <cassert>
#include <limits>

#include "src/make_unique.h"
#include "src/vulkan/command_buffer.h"
#include "src/vulkan/device.h"

namespace amber {
namespace vulkan {

PushConstant::PushConstant(Device* device)
    : device_(device), buffer_(MakeUnique<Buffer>()) {}

PushConstant::~PushConstant() = default;

VkPushConstantRange PushConstant::GetVkPushConstantRange() {
  if (push_constant_data_.empty())
    return VkPushConstantRange();

  auto it =
      std::min_element(push_constant_data_.begin(), push_constant_data_.end(),
                       [](const BufferInput& a, const BufferInput& b) {
                         return a.offset < b.offset;
                       });
  assert(it != push_constant_data_.end());

  uint32_t first_offset = it->offset;

  it = std::max_element(
      push_constant_data_.begin(), push_constant_data_.end(),
      [](const BufferInput& a, const BufferInput& b) {
        return a.offset + static_cast<uint32_t>(a.buffer->GetSizeInBytes()) <
               b.offset + static_cast<uint32_t>(b.buffer->GetSizeInBytes());
      });
  assert(it != push_constant_data_.end());

  uint32_t size_in_bytes = it->offset +
                           static_cast<uint32_t>(it->buffer->GetSizeInBytes()) -
                           first_offset;

  VkPushConstantRange range = VkPushConstantRange();
  range.stageFlags = VK_SHADER_STAGE_ALL;

  // Based on Vulkan spec, range.offset must be multiple of 4.
  range.offset = (first_offset / 4U) * 4U;

  // Based on Vulkan spec, range.size must be multiple of 4.
  assert(size_in_bytes + 3U <= std::numeric_limits<uint32_t>::max());
  range.size = ((size_in_bytes + 3U) / 4U) * 4U;

  return range;
}

Result PushConstant::RecordPushConstantVkCommand(
    CommandBuffer* command,
    VkPipelineLayout pipeline_layout) {
  if (push_constant_data_.empty())
    return {};

  auto push_const_range = GetVkPushConstantRange();
  if (push_const_range.offset + push_const_range.size >
      device_->GetMaxPushConstants()) {
    return Result(
        "PushConstant::RecordPushConstantVkCommand push constant size in bytes "
        "exceeds maxPushConstantsSize of VkPhysicalDeviceLimits");
  }

  for (const auto& data : push_constant_data_) {
    Result r = UpdateMemoryWithInput(data);
    if (!r.IsSuccess())
      return r;
  }

  // Based on spec, offset and size in bytes of push constant must
  // be multiple of 4.
  if (push_const_range.offset % 4U != 0)
    return Result("PushConstant:: Offset must be a multiple of 4");
  if (push_const_range.size % 4U != 0)
    return Result("PushConstant:: Size must be a multiple of 4");

  device_->GetPtrs()->vkCmdPushConstants(
      command->GetVkCommandBuffer(), pipeline_layout, VK_SHADER_STAGE_ALL,
      push_const_range.offset, push_const_range.size,
      buffer_->GetValues<uint8_t*>() + push_const_range.offset);
  return {};
}

Result PushConstant::AddBuffer(const Buffer* buffer, uint32_t offset) {
  push_constant_data_.emplace_back();
  push_constant_data_.back().offset = offset;
  push_constant_data_.back().buffer = buffer;
  return {};
}

Result PushConstant::UpdateMemoryWithInput(const BufferInput& input) {
  if (static_cast<size_t>(input.offset) >= device_->GetMaxPushConstants()) {
    return Result(
        "Vulkan: UpdateMemoryWithInput BufferInput offset exceeds memory size");
  }

  if (input.buffer->GetSizeInBytes() >
      (device_->GetMaxPushConstants() - input.offset)) {
    return Result(
        "Vulkan: UpdateMemoryWithInput BufferInput offset + size_in_bytes "
        " exceeds memory size");
  }

  if (!buffer_->GetFormat()) {
    buffer_->SetFormat(input.buffer->GetFormat());
  } else if (!buffer_->GetFormat()->Equal(input.buffer->GetFormat())) {
    return Result("Vulkan: push constants must all have the same format");
  }

  buffer_->SetDataFromBuffer(input.buffer, input.offset);

  return {};
}

}  // namespace vulkan
}  // namespace amber
