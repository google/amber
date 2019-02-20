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

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "amber/vulkan_header.h"
#include "src/cast_hash.h"
#include "src/engine.h"
#include "src/pipeline.h"
#include "src/vulkan/command_pool.h"
#include "src/vulkan/device.h"
#include "src/vulkan/pipeline.h"
#include "src/vulkan/vertex_buffer.h"

namespace amber {
namespace vulkan {

class EngineVulkan : public Engine {
 public:
  EngineVulkan();
  ~EngineVulkan() override;

  // Engine
  Result Initialize(EngineConfig* config,
                    const std::vector<Feature>& features,
                    const std::vector<std::string>& instance_extensions,
                    const std::vector<std::string>& device_extensions) override;
  Result Shutdown() override;
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
  Result DoProcessCommands() override;
  Result GetFrameBufferInfo(ResourceInfo* info) override;
  Result GetFrameBuffer(std::vector<Value>* values) override;
  Result GetDescriptorInfo(const uint32_t descriptor_set,
                           const uint32_t binding,
                           ResourceInfo* info) override;

 private:
  std::vector<VkPipelineShaderStageCreateInfo> GetShaderStageInfo();
  bool IsFormatSupportedByPhysicalDevice(BufferType type,
                                         VkPhysicalDevice physical_device,
                                         VkFormat format);
  bool IsDescriptorSetInBounds(VkPhysicalDevice physical_device,
                               uint32_t descriptor_set);
  Result SetShader(ShaderType type, const std::vector<uint32_t>& data);
  Result SetBuffer(BufferType type,
                   uint8_t location,
                   const Format& format,
                   const std::vector<Value>& data);

  std::unique_ptr<Device> device_;
  std::unique_ptr<CommandPool> pool_;
  std::unique_ptr<Pipeline> pipeline_;
  std::unique_ptr<VertexBuffer> vertex_buffer_;

  std::unordered_map<ShaderType, VkShaderModule, CastHash<ShaderType>> modules_;

  std::unique_ptr<Format> color_frame_format_;
  std::unique_ptr<Format> depth_frame_format_;
};

}  // namespace vulkan
}  // namespace amber

#endif  // SRC_VULKAN_ENGINE_VULKAN_H_
