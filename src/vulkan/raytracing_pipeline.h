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

#ifndef SRC_VULKAN_RAYTRACING_PIPELINE_H_
#define SRC_VULKAN_RAYTRACING_PIPELINE_H_

#include <memory>
#include <vector>

#include "amber/result.h"
#include "amber/vulkan_header.h"
#include "src/vulkan/pipeline.h"

namespace amber {
namespace vulkan {

/// Pipepline to handle compute commands.
class RayTracingPipeline : public Pipeline {
 public:
  RayTracingPipeline(
      Device* device,
      BlasesMap* blases,
      TlasesMap* tlases,
      uint32_t fence_timeout_ms,
      bool pipeline_runtime_layer_enabled,
      const std::vector<VkPipelineShaderStageCreateInfo>& shader_stage_info,
      VkPipelineCreateFlags create_flags);
  ~RayTracingPipeline() override;

  Result AddTLASDescriptor(const TLASCommand* cmd);

  Result Initialize(CommandPool* pool,
                    std::vector<VkRayTracingShaderGroupCreateInfoKHR>&
                        shader_group_create_info);

  Result getVulkanSBTRegion(VkPipeline pipeline,
                            amber::SBT* aSBT,
                            VkStridedDeviceAddressRegionKHR* region);

  Result InitLibrary(const std::vector<VkPipeline>& lib,
                     uint32_t maxPipelineRayPayloadSize,
                     uint32_t maxPipelineRayHitAttributeSize,
                     uint32_t maxPipelineRayRecursionDepth);

  Result TraceRays(amber::SBT* rSBT,
                   amber::SBT* mSBT,
                   amber::SBT* hSBT,
                   amber::SBT* cSBT,
                   uint32_t x,
                   uint32_t y,
                   uint32_t z,
                   uint32_t maxPipelineRayPayloadSize,
                   uint32_t maxPipelineRayHitAttributeSize,
                   uint32_t maxPipelineRayRecursionDepth,
                   const std::vector<VkPipeline>& lib,
                   bool is_timed_execution);

  BlasesMap* GetBlases() override { return blases_; }
  TlasesMap* GetTlases() override { return tlases_; }

 private:
  Result CreateVkRayTracingPipeline(const VkPipelineLayout& pipeline_layout,
                                    VkPipeline* pipeline,
                                    const std::vector<VkPipeline>& libs,
                                    uint32_t maxPipelineRayPayloadSize,
                                    uint32_t maxPipelineRayHitAttributeSize,
                                    uint32_t maxPipelineRayRecursionDepth);

  std::vector<VkRayTracingShaderGroupCreateInfoKHR> shader_group_create_info_;
  BlasesMap* blases_;
  TlasesMap* tlases_;
  SbtsMap sbtses_;
  std::vector<std::unique_ptr<amber::vulkan::SBT>> sbts_;
};

}  // namespace vulkan
}  // namespace amber

#endif  // SRC_VULKAN_RAYTRACING_PIPELINE_H_
