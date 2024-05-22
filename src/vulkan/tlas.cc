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

#include "src/vulkan/tlas.h"
#include "src/vulkan/blas.h"

namespace amber {
namespace vulkan {

static VkTransformMatrixKHR makeVkMatrix(const float* m) {
  const VkTransformMatrixKHR identityMatrix3x4 = {{{1.0f, 0.0f, 0.0f, 0.0f},
                                                   {0.0f, 1.0f, 0.0f, 0.0f},
                                                   {0.0f, 0.0f, 1.0f, 0.0f}}};
  VkTransformMatrixKHR v;

  if (m == nullptr)
    return identityMatrix3x4;

  for (size_t i = 0; i < 12; i++) {
    const size_t r = i / 4;
    const size_t c = i % 4;
    v.matrix[r][c] = m[i];
  }

  return v;
}

TLAS::TLAS(Device* device) : device_(device) {}

Result TLAS::CreateTLAS(amber::TLAS* tlas,
                        BlasesMap* blases) {
  if (tlas_ != VK_NULL_HANDLE)
    return {};

  assert(tlas != nullptr);

  VkDeviceOrHostAddressConstKHR const_default_ptr;
  VkDeviceOrHostAddressKHR default_ptr;

  const_default_ptr.hostAddress = nullptr;
  default_ptr.hostAddress = nullptr;

  instances_count_ = static_cast<uint32_t>(tlas->GetInstances().size());

  const uint32_t ib_size =
      uint32_t(instances_count_ * sizeof(VkAccelerationStructureInstanceKHR));

  instance_buffer_ = MakeUnique<TransferBuffer>(device_, ib_size, nullptr);
  instance_buffer_->AddUsageFlags(
      VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
      VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);
  instance_buffer_->AddAllocateFlags(VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT);
  instance_buffer_->Initialize();

  VkAccelerationStructureInstanceKHR* instances_ptr =
      reinterpret_cast<VkAccelerationStructureInstanceKHR*>
          (instance_buffer_->HostAccessibleMemoryPtr());

  for (auto& instance : tlas->GetInstances()) {
    auto blas = instance->GetUsedBLAS();

    assert(blas != nullptr);

    auto blas_vulkan_it = blases->find(blas);
    amber::vulkan::BLAS* blas_vulkan_ptr = nullptr;

    if (blas_vulkan_it == blases->end()) {
      auto blas_vulkan =
          blases->emplace(blas, new amber::vulkan::BLAS(device_));
      blas_vulkan_ptr = blas_vulkan.first->second.get();

      Result r = blas_vulkan_ptr->CreateBLAS(blas);

      if (!r.IsSuccess())
        return r;
    } else {
      blas_vulkan_ptr = blas_vulkan_it->second.get();
    }

    VkDeviceAddress accelerationStructureAddress =
        blas_vulkan_ptr->getVkBLASDeviceAddress();

    *instances_ptr = VkAccelerationStructureInstanceKHR{
        makeVkMatrix(instance->GetTransform()),
        instance->GetInstanceIndex(),
        instance->GetMask(),
        instance->GetOffset(),
        instance->GetFlags(),
        static_cast<uint64_t>(accelerationStructureAddress)};

    instances_ptr++;
  }

  VkAccelerationStructureGeometryInstancesDataKHR
      accelerationStructureGeometryInstancesDataKHR = {
          VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR,
          nullptr,
          VK_FALSE,
          const_default_ptr,
      };
  VkAccelerationStructureGeometryDataKHR geometry = {};
  geometry.instances = accelerationStructureGeometryInstancesDataKHR;

  accelerationStructureGeometryKHR_ = {
      VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
      nullptr,
      VK_GEOMETRY_TYPE_INSTANCES_KHR,
      geometry,
      0,
  };

  accelerationStructureBuildGeometryInfoKHR_ = {
      VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
      nullptr,
      VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
      0,
      VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
      VK_NULL_HANDLE,
      VK_NULL_HANDLE,
      1,
      &accelerationStructureGeometryKHR_,
      nullptr,
      default_ptr,
  };

  VkAccelerationStructureBuildSizesInfoKHR sizeInfo = {
      VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR,
      nullptr,
      0,
      0,
      0,
  };

  device_->GetPtrs()->vkGetAccelerationStructureBuildSizesKHR(
      device_->GetVkDevice(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
      &accelerationStructureBuildGeometryInfoKHR_, &instances_count_,
      &sizeInfo);

  const uint32_t as_size =
      static_cast<uint32_t>(sizeInfo.accelerationStructureSize);

  buffer_ = MakeUnique<TransferBuffer>(device_, as_size, nullptr);
  buffer_->AddUsageFlags(
      VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR |
      VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);
  buffer_->AddAllocateFlags(VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT);
  buffer_->Initialize();

  const VkAccelerationStructureCreateInfoKHR
      accelerationStructureCreateInfoKHR = {
          VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
          nullptr,
          0,
          buffer_->GetVkBuffer(),
          0,
          as_size,
          VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
          0,
      };

  if (device_->GetPtrs()->vkCreateAccelerationStructureKHR(
          device_->GetVkDevice(), &accelerationStructureCreateInfoKHR, nullptr,
          &tlas_) != VK_SUCCESS) {
    return Result(
        "Vulkan::Calling vkCreateAccelerationStructureKHR "
        "failed");
  }

  accelerationStructureBuildGeometryInfoKHR_.dstAccelerationStructure = tlas_;

  if (sizeInfo.buildScratchSize > 0) {
    scratch_buffer_ = MakeUnique<TransferBuffer>(
        device_, static_cast<uint32_t>(sizeInfo.buildScratchSize), nullptr);
    scratch_buffer_->AddUsageFlags(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                                   VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);
    scratch_buffer_->AddAllocateFlags(VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT);
    scratch_buffer_->Initialize();

    accelerationStructureBuildGeometryInfoKHR_.scratchData.deviceAddress =
        scratch_buffer_->getBufferDeviceAddress();
  }

  accelerationStructureGeometryKHR_.geometry.instances.data.deviceAddress =
      instance_buffer_->getBufferDeviceAddress();

  return {};
}

Result TLAS::BuildTLAS(VkCommandBuffer cmdBuffer) {
  if (tlas_ == VK_NULL_HANDLE)
    return Result("Acceleration structure should be created first");
  if (built_)
    return {};

  VkAccelerationStructureBuildRangeInfoKHR
      accelerationStructureBuildRangeInfoKHR = {instances_count_, 0, 0, 0};
  VkAccelerationStructureBuildRangeInfoKHR*
      accelerationStructureBuildRangeInfoKHRPtr =
          &accelerationStructureBuildRangeInfoKHR;

  device_->GetPtrs()->vkCmdBuildAccelerationStructuresKHR(
      cmdBuffer, 1, &accelerationStructureBuildGeometryInfoKHR_,
      &accelerationStructureBuildRangeInfoKHRPtr);

  const VkAccessFlags accessMasks =
      VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR |
      VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
  const VkMemoryBarrier memBarrier{
      VK_STRUCTURE_TYPE_MEMORY_BARRIER,
      nullptr,
      accessMasks,
      accessMasks,
  };

  device_->GetPtrs()->vkCmdPipelineBarrier(
      cmdBuffer, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
      VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 1, &memBarrier, 0, nullptr, 0,
      nullptr);

  built_ = true;

  return {};
}

TLAS::~TLAS() {
  if (tlas_ != VK_NULL_HANDLE) {
    device_->GetPtrs()->vkDestroyAccelerationStructureKHR(
        device_->GetVkDevice(), tlas_, nullptr);
  }
}

}  // namespace vulkan
}  // namespace amber
