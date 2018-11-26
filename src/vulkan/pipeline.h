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

#ifndef SRC_VULKAN_PIPELINE_H_
#define SRC_VULKAN_PIPELINE_H_

#include <memory>
#include <vector>

#include "amber/result.h"
#include "src/engine.h"
#include "src/vulkan/command.h"
#include "src/vulkan/descriptor.h"
#include "vulkan/vulkan.h"

namespace amber {

class BufferCommand;

namespace vulkan {

class GraphicsPipeline;

class Pipeline {
 public:
  virtual ~Pipeline();

  bool IsGraphics() const { return pipeline_type_ == PipelineType::kGraphics; }
  bool IsCompute() const { return pipeline_type_ == PipelineType::kCompute; }

  GraphicsPipeline* AsGraphics();

  Result AddDescriptor(const BufferCommand*);

  virtual void Shutdown();

 protected:
  Pipeline(PipelineType type,
           VkDevice device,
           const VkPhysicalDeviceMemoryProperties& properties);
  Result InitializeCommandBuffer(VkCommandPool pool, VkQueue queue);
  Result CreateVkDescriptorRelatedObjects();

  Result SendDescriptorDataToGPUIfNeeded();
  void BindVkPipeline();
  void BindVkDescriptorSets();

  VkPipelineCache pipeline_cache_ = VK_NULL_HANDLE;
  VkPipeline pipeline_ = VK_NULL_HANDLE;
  VkPipelineLayout pipeline_layout_ = VK_NULL_HANDLE;

  std::vector<VkDescriptorSetLayout> descriptor_set_layouts_;
  std::vector<VkDescriptorPool> descriptor_pools_;
  std::vector<VkDescriptorSet> descriptor_sets_;

  VkDevice device_ = VK_NULL_HANDLE;
  VkPhysicalDeviceMemoryProperties memory_properties_;
  std::unique_ptr<CommandBuffer> command_;

 private:
  Result CreatePipelineLayout();

  Result CreateDescriptorSetLayouts();
  Result CreateDescriptorPools();
  Result CreateDescriptorSets();

  void DestoryDescriptorSetLayouts();
  void DestoryDescriptorPools();

  PipelineType pipeline_type_;
  std::vector<std::unique_ptr<Descriptor>> descriptors_;
};

}  // namespace vulkan
}  // namespace amber

#endif  // SRC_VULKAN_PIPELINE_H_
