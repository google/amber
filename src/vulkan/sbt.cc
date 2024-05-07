// Copyright 2024 The Amber Authors.
// Copyright (C) 2024 Advanced Micro Devices, Inc. All rights reserved.
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

#include <cstring>

#include "src/vulkan/sbt.h"
#include "src/vulkan/pipeline.h"

namespace amber {
namespace vulkan {

SBT::SBT(Device* device) : device_(device) {}

Result SBT::Create(amber::SBT* sbt, VkPipeline pipeline) {
  uint32_t handles_count = 0;
  for (auto& x : sbt->GetSBTRecords())
    handles_count += x->GetCount();

  if (handles_count == 0)
    return Result("SBT must contain at least one record");

  const uint32_t handle_size = device_->GetRayTracingShaderGroupHandleSize();
  const uint32_t buffer_size = handle_size * handles_count;
  std::vector<uint8_t> handles(buffer_size);

  buffer_ = MakeUnique<TransferBuffer>(device_, buffer_size, nullptr);
  buffer_->AddUsageFlags(VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                         VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR |
                         VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);
  buffer_->AddAllocateFlags(VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT);
  Result r = buffer_->Initialize();
  if (!r.IsSuccess())
    return r;

  size_t start = 0;
  for (auto& x : sbt->GetSBTRecords()) {
    const uint32_t index = x->GetIndex();
    const uint32_t count = x->GetCount();
    if (index != static_cast<uint32_t>(-1)) {
      VkResult vr = device_->GetPtrs()->vkGetRayTracingShaderGroupHandlesKHR(
          device_->GetVkDevice(), pipeline, index, count, count * handle_size,
          &handles[start * handle_size]);

      if (vr != VK_SUCCESS)
        return Result("vkGetRayTracingShaderGroupHandlesKHR has failed");
    }

    start += count;
  }

  memcpy(buffer_->HostAccessibleMemoryPtr(), handles.data(), handles.size());

  // Skip flush as memory allocated for buffer is coherent

  return r;
}

SBT::~SBT() = default;

}  // namespace vulkan
}  // namespace amber
