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

#ifndef SRC_VULKAN_COMPUTE_PIPELINE_H_
#define SRC_VULKAN_COMPUTE_PIPELINE_H_

#include <vector>

#include "amber/result.h"
#include "amber/vulkan_header.h"
#include "src/vulkan/pipeline.h"

namespace amber {
namespace vulkan {

/// Pipepline to handle compute commands.
class ComputePipeline : public Pipeline {
 public:
  ComputePipeline(
      Device* device,
      uint32_t fence_timeout_ms,
      const std::vector<VkPipelineShaderStageCreateInfo>& shader_stage_info);
  ~ComputePipeline() override;

  Result Initialize(CommandPool* pool);

  Result Compute(uint32_t x, uint32_t y, uint32_t z);

 private:
  Result CreateVkComputePipeline(const VkPipelineLayout& pipeline_layout,
                                 VkPipeline* pipeline);
};

}  // namespace vulkan
}  // namespace amber

#endif  // SRC_VULKAN_COMPUTE_PIPELINE_H_
