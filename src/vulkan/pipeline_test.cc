// Copyright 2020 The Amber Authors.
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

#include "src/vulkan/pipeline.h"

#include "gtest/gtest.h"
#include "src/pipeline.h"
#include "src/vulkan/compute_pipeline.h"

namespace amber {
namespace vulkan {

using VulkanPipelineTest = testing::Test;

TEST_F(VulkanPipelineTest, AddBufferDescriptorAddPushConstant) {
  amber::Pipeline amber_pipeline(PipelineType::kCompute);
  std::vector<VkPipelineShaderStageCreateInfo> create_infos;
  ComputePipeline pipeline(nullptr, 0, create_infos);

  auto cmd = MakeUnique<BufferCommand>(BufferCommand::BufferType::kPushConstant,
                                       &amber_pipeline);
  // Push constant buffers should not be passed to AddBufferDescriptor().
  Result r = pipeline.AddBufferDescriptor(cmd.get());
  ASSERT_FALSE(r.IsSuccess());
}

TEST_F(VulkanPipelineTest, AddBufferDescriptorAddBufferTwice) {
  amber::Pipeline amber_pipeline(PipelineType::kCompute);
  std::vector<VkPipelineShaderStageCreateInfo> create_infos;
  ComputePipeline pipeline(nullptr, 0, create_infos);

  auto cmd = MakeUnique<BufferCommand>(BufferCommand::BufferType::kUniform,
                                       &amber_pipeline);
  Result r = pipeline.AddBufferDescriptor(cmd.get());
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  // Adding same buffer again shouldn't fail.
  r = pipeline.AddBufferDescriptor(cmd.get());
  ASSERT_TRUE(r.IsSuccess());
}

}  // namespace vulkan
}  // namespace amber
