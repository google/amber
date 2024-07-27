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

#include "src/vulkan/blas.h"

#include <cstring>

#include "src/vulkan/command_buffer.h"

namespace amber {
namespace vulkan {

inline VkDeviceSize align(VkDeviceSize v, VkDeviceSize a) {
  return (v + a - 1) & ~(a - 1);
}

BLAS::BLAS(Device* device) : device_(device) {}

BLAS::~BLAS() {
  if (blas_ != VK_NULL_HANDLE) {
    device_->GetPtrs()->vkDestroyAccelerationStructureKHR(
        device_->GetVkDevice(), blas_, nullptr);
  }
}

Result BLAS::CreateBLAS(amber::BLAS* blas) {
  if (blas_ != VK_NULL_HANDLE)
    return Result("Cannot recreate acceleration structure");

  std::vector<std::unique_ptr<Geometry>>& geometries = blas->GetGeometries();
  std::vector<VkDeviceSize> vertexBufferOffsets;
  VkDeviceSize vertexBufferSize = 0;

  VkDeviceOrHostAddressConstKHR const_null_placeholder = {};
  VkDeviceOrHostAddressKHR null_placeholder = {};

  accelerationStructureGeometriesKHR_.resize(geometries.size());
  accelerationStructureBuildRangeInfoKHR_.resize(geometries.size());
  maxPrimitiveCounts_.resize(geometries.size());
  vertexBufferOffsets.resize(geometries.size());

  for (size_t geometryNdx = 0; geometryNdx < geometries.size(); ++geometryNdx) {
    const std::unique_ptr<Geometry>& geometryData = geometries[geometryNdx];
    VkDeviceOrHostAddressConstKHR vertexData = {};
    VkAccelerationStructureGeometryDataKHR geometry;
    VkGeometryTypeKHR geometryType = VK_GEOMETRY_TYPE_MAX_ENUM_KHR;

    if (geometryData->IsTriangle()) {
      VkAccelerationStructureGeometryTrianglesDataKHR
          accelerationStructureGeometryTrianglesDataKHR = {
              // NOLINTNEXTLINE(whitespace/line_length)
              VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR,
              nullptr,
              VK_FORMAT_R32G32B32_SFLOAT,
              vertexData,
              3 * sizeof(float),
              static_cast<uint32_t>(geometryData->getVertexCount()),
              VK_INDEX_TYPE_NONE_KHR,
              const_null_placeholder,
              const_null_placeholder,
          };

      geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
      geometry.triangles = accelerationStructureGeometryTrianglesDataKHR;
    } else if (geometryData->IsAABB()) {
      const VkAccelerationStructureGeometryAabbsDataKHR
          accelerationStructureGeometryAabbsDataKHR = {
              VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR,
              nullptr, vertexData, sizeof(VkAabbPositionsKHR)};

      geometryType = VK_GEOMETRY_TYPE_AABBS_KHR;
      geometry.aabbs = accelerationStructureGeometryAabbsDataKHR;
    } else {
      assert(false && "unknown geometry type");
    }

    const VkAccelerationStructureGeometryKHR accelerationStructureGeometry = {
            VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
            nullptr,
            geometryType,
            geometry,
            VkGeometryFlagsKHR(geometryData->GetFlags())
        };
    const VkAccelerationStructureBuildRangeInfoKHR
        accelerationStructureBuildRangeInfosKHR = {
            static_cast<uint32_t>(geometryData->getPrimitiveCount()), 0, 0, 0};

    accelerationStructureGeometriesKHR_[geometryNdx] =
        accelerationStructureGeometry;
    accelerationStructureBuildRangeInfoKHR_[geometryNdx] =
        accelerationStructureBuildRangeInfosKHR;
    maxPrimitiveCounts_[geometryNdx] =
        accelerationStructureBuildRangeInfosKHR.primitiveCount;
    vertexBufferOffsets[geometryNdx] = vertexBufferSize;
    size_t s1 = sizeof(geometryData->GetData()[0]);
    vertexBufferSize += align(geometryData->GetData().size() * s1, 8);
  }

  const VkAccelerationStructureGeometryKHR*
      accelerationStructureGeometriesKHRPointer =
          accelerationStructureGeometriesKHR_.data();
  accelerationStructureBuildGeometryInfoKHR_ = {
      VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
      nullptr,
      VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
      0u,
      VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
      VK_NULL_HANDLE,
      VK_NULL_HANDLE,
      static_cast<uint32_t>(accelerationStructureGeometriesKHR_.size()),
      accelerationStructureGeometriesKHRPointer,
      nullptr,
      null_placeholder,
  };
  VkAccelerationStructureBuildSizesInfoKHR sizeInfo = {
      VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR, nullptr, 0,
      0, 0};

  device_->GetPtrs()->vkGetAccelerationStructureBuildSizesKHR(
      device_->GetVkDevice(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
      &accelerationStructureBuildGeometryInfoKHR_, maxPrimitiveCounts_.data(),
      &sizeInfo);

  const uint32_t accelerationStructureSize =
      static_cast<uint32_t>(sizeInfo.accelerationStructureSize);

  buffer_ =
      MakeUnique<TransferBuffer>(device_, accelerationStructureSize, nullptr);
  buffer_->AddUsageFlags(
      VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR |
      VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);
  buffer_->AddAllocateFlags(VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT);
  buffer_->Initialize();

  const VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfoKHR{
      VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
      nullptr,
      0,
      buffer_->GetVkBuffer(),
      0,
      accelerationStructureSize,
      VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
      0};

  if (device_->GetPtrs()->vkCreateAccelerationStructureKHR(
          device_->GetVkDevice(), &accelerationStructureCreateInfoKHR, nullptr,
          &blas_) != VK_SUCCESS)
    return Result("Vulkan::Calling vkCreateAccelerationStructureKHR failed");

  accelerationStructureBuildGeometryInfoKHR_.dstAccelerationStructure = blas_;

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

  if (vertexBufferSize > 0) {
    vertex_buffer_ = MakeUnique<TransferBuffer>(
        device_, static_cast<uint32_t>(vertexBufferSize), nullptr);
    vertex_buffer_->AddUsageFlags(
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
        VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);
    vertex_buffer_->AddAllocateFlags(VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT);
    vertex_buffer_->SetMemoryPropertiesFlags(
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    vertex_buffer_->Initialize();

    void* memory_ptr = vertex_buffer_->HostAccessibleMemoryPtr();
    assert(memory_ptr != nullptr);

    for (size_t geometryNdx = 0; geometryNdx < geometries.size();
         ++geometryNdx) {
      VkDeviceOrHostAddressConstKHR p;

      p.deviceAddress = vertex_buffer_.get()->getBufferDeviceAddress() +
                        vertexBufferOffsets[geometryNdx];

      const auto& data = geometries[geometryNdx]->GetData();
      std::memcpy(reinterpret_cast<char*>(memory_ptr) +
                      vertexBufferOffsets[geometryNdx],
                  data.data(), data.size() * sizeof(*data.data()));

      if (geometries[geometryNdx]->IsTriangle()) {
        accelerationStructureGeometriesKHR_[geometryNdx]
            .geometry.triangles.vertexData = p;
      } else if (geometries[geometryNdx]->IsAABB()) {
        accelerationStructureGeometriesKHR_[geometryNdx].geometry.aabbs.data =
            p;
      } else {
        assert(false && "unknown geometry type");
      }
      accelerationStructureGeometriesKHR_[geometryNdx].flags =
          VkGeometryFlagsKHR(geometries[geometryNdx]->GetFlags());
    }
  }

  return {};
}

Result BLAS::BuildBLAS(CommandBuffer* command_buffer) {
  if (blas_ == VK_NULL_HANDLE)
    return Result("Acceleration structure should be created first");
  if (built_)
    return {};

  VkCommandBuffer cmdBuffer = command_buffer->GetVkCommandBuffer();

  vertex_buffer_->CopyToDevice(command_buffer);

  VkAccelerationStructureBuildRangeInfoKHR*
      accelerationStructureBuildRangeInfoKHRPtr =
          accelerationStructureBuildRangeInfoKHR_.data();

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

VkDeviceAddress BLAS::getVkBLASDeviceAddress() {
  VkAccelerationStructureDeviceAddressInfoKHR info = {
      VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR, nullptr,
      blas_};

  assert(blas_ != VK_NULL_HANDLE);

  return device_->GetPtrs()->vkGetAccelerationStructureDeviceAddressKHR(
      device_->GetVkDevice(), &info);
}

}  // namespace vulkan
}  // namespace amber
