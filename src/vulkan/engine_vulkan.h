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

#ifndef SRC_VULKAN_ENGINE_VULKAN_H_
#define SRC_VULKAN_ENGINE_VULKAN_H_

#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "amber/vulkan_header.h"
#include "src/cast_hash.h"
#include "src/engine.h"
#include "src/pipeline.h"
#include "src/vulkan/buffer_descriptor.h"
#include "src/vulkan/command_pool.h"
#include "src/vulkan/device.h"
#include "src/vulkan/pipeline.h"
#include "src/vulkan/vertex_buffer.h"

namespace amber {
namespace vulkan {

/// Engine implementation based on Vulkan.
class EngineVulkan : public Engine {
 public:
  EngineVulkan();
  ~EngineVulkan() override;

  // Engine
  Result Initialize(EngineConfig* config,
                    Delegate* delegate,
                    const std::vector<std::string>& features,
                    const std::vector<std::string>& instance_extensions,
                    const std::vector<std::string>& device_extensions) override;
  Result CreatePipeline(amber::Pipeline* type) override;

  Result DoClearColor(const ClearColorCommand* cmd) override;
  Result DoClearStencil(const ClearStencilCommand* cmd) override;
  Result DoClearDepth(const ClearDepthCommand* cmd) override;
  Result DoClear(const ClearCommand* cmd) override;
  Result DoDrawRect(const DrawRectCommand* cmd) override;
  Result DoDrawArrays(const DrawArraysCommand* cmd) override;
  Result DoCompute(const ComputeCommand* cmd) override;
  Result DoEntryPoint(const EntryPointCommand* cmd) override;
  Result DoPatchParameterVertices(
      const PatchParameterVerticesCommand* cmd) override;
  Result DoBuffer(const BufferCommand* cmd) override;

 private:
  struct PipelineInfo {
    std::unique_ptr<Pipeline> vk_pipeline;
    std::unique_ptr<VertexBuffer> vertex_buffer;
    struct ShaderInfo {
      VkShaderModule shader;
      std::unique_ptr<std::vector<VkSpecializationMapEntry>>
          specialization_entries;
      std::unique_ptr<std::vector<uint32_t>> specialization_data;
      std::unique_ptr<VkSpecializationInfo> specialization_info;
    };
    std::unordered_map<ShaderType, ShaderInfo, CastHash<ShaderType>>
        shader_info;
  };

  Result GetVkShaderStageInfo(
      amber::Pipeline* pipeline,
      std::vector<VkPipelineShaderStageCreateInfo>* out);
  bool IsFormatSupportedByPhysicalDevice(BufferType type,
                                         VkPhysicalDevice physical_device,
                                         VkFormat format);
  bool IsDescriptorSetInBounds(VkPhysicalDevice physical_device,
                               uint32_t descriptor_set);

  Result SetShader(amber::Pipeline* pipeline,
                   ShaderType type,
                   const std::vector<uint32_t>& data);

  std::unique_ptr<Device> device_;
  std::unique_ptr<CommandPool> pool_;

  std::map<amber::Pipeline*, PipelineInfo> pipeline_map_;
};

}  // namespace vulkan
}  // namespace amber

#endif  // SRC_VULKAN_ENGINE_VULKAN_H_
