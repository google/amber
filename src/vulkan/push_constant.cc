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

namespace amber {
namespace vulkan {

PushConstant::PushConstant(uint32_t max_push_constant_size)
    : max_push_constant_size_(max_push_constant_size) {}

PushConstant::~PushConstant() = default;

VkPushConstantRange PushConstant::GetPushConstantRange() {
  if (push_constant_data_.empty())
    return {};

  auto it =
      std::min_element(push_constant_data_.begin(), push_constant_data_.end(),
                       [](const BufferData& a, const BufferData& b) {
                         return a.offset < b.offset;
                       });
  if (it == push_constant_data_.end())
    return {};

  uint32_t first_offset = it->offset;

  it = std::max_element(
      push_constant_data_.begin(), push_constant_data_.end(),
      [](const BufferData& a, const BufferData& b) {
        return a.offset + static_cast<uint32_t>(a.size_in_bytes) <
               b.offset + static_cast<uint32_t>(b.size_in_bytes);
      });
  if (it == push_constant_data_.end())
    return {};

  uint32_t size_in_bytes =
      it->offset + static_cast<uint32_t>(it->size_in_bytes) - first_offset;

  VkPushConstantRange range = {};
  range.stageFlags = VK_SHADER_STAGE_ALL;

  // Based on Vulkan spec, range.offset must be multiple of 4.
  range.offset = (first_offset / 4U) * 4U;

  // Based on Vulkan spec, range.size must be multiple of 4.
  assert(size_in_bytes + 3U <= std::numeric_limits<uint32_t>::max());
  range.size = ((size_in_bytes + 3U) / 4U) * 4U;

  return range;
}

void PushConstant::RecordPushConstantVkCommand(VkCommandBuffer command_buffer,
                                               VkPipelineLayout pipeline_layout) {
  if (push_constant_data_.empty())
    return;

  auto push_const_range = GetPushConstantRange();

  std::vector<uint8_t> memory(push_const_range.offset + push_const_range.size);
  SetMemoryPtr(memory.data());

  for (const auto& data : push_constant_data_) {
    UpdateMemoryWithData(data);
  }

  // Based on spec, offset and size in bytes of push constant must
  // be multiple of 4.
  assert(push_const_range.offset % 4U == 0 && push_const_range.size % 4U == 0);

  vkCmdPushConstants(command_buffer, pipeline_layout,
                     VK_SHADER_STAGE_ALL, push_const_range.offset,
                     push_const_range.size, &memory[push_const_range.offset]);
  SetMemoryPtr(nullptr);
}

Result PushConstant::AddBufferData(const BufferCommand* command) {
  if (!command->IsPushConstant())
    return Result(
        "PushConstant::AddBufferData BufferCommand type is not push constant");

  push_constant_data_.push_back({command->GetDatumType().GetType(),
                                 command->GetOffset(), command->GetSize(),
                                 command->GetValues()});

  return {};
}

}  // namespace vulkan
}  // namespace amber
