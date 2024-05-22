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

#ifndef SRC_VULKAN_BLAS_H_
#define SRC_VULKAN_BLAS_H_

#include <vector>
#include <memory>

#include "src/acceleration_structure.h"
#include "src/vulkan/device.h"
#include "src/vulkan/transfer_buffer.h"

namespace amber {
namespace vulkan {

class BLAS {
 public:
  explicit BLAS(Device* device);
  ~BLAS();

  Result CreateBLAS(amber::BLAS* blas);
  Result BuildBLAS(CommandBuffer* command_buffer);
  VkAccelerationStructureKHR GetVkBLAS() { return blas_; }
  VkDeviceAddress getVkBLASDeviceAddress();

 private:
  Device* device_ = nullptr;
  VkAccelerationStructureKHR blas_ = VK_NULL_HANDLE;
  bool built_ = false;
  std::unique_ptr<TransferBuffer> buffer_;
  std::unique_ptr<TransferBuffer> scratch_buffer_;
  std::unique_ptr<TransferBuffer> vertex_buffer_;
  VkAccelerationStructureBuildGeometryInfoKHR
      accelerationStructureBuildGeometryInfoKHR_;
  std::vector<VkAccelerationStructureGeometryKHR>
      accelerationStructureGeometriesKHR_;
  std::vector<VkAccelerationStructureBuildRangeInfoKHR>
      accelerationStructureBuildRangeInfoKHR_;
  std::vector<uint32_t> maxPrimitiveCounts_;
};

}  // namespace vulkan
}  // namespace amber

#endif  // SRC_VULKAN_BLAS_H_
