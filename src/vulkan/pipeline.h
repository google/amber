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

class ComputePipeline;
class GraphicsPipeline;

class Pipeline {
 public:
  virtual ~Pipeline();

  bool IsGraphics() const { return pipeline_type_ == PipelineType::kGraphics; }
  bool IsCompute() const { return pipeline_type_ == PipelineType::kCompute; }

  GraphicsPipeline* AsGraphics();
  ComputePipeline* AsCompute();

  Result AddDescriptor(const BufferCommand*);

  // Copy the contents of the resource bound to the given descriptor
  // to host memory.
  Result CopyDescriptorToHost(const uint32_t descriptor_set,
                              const uint32_t binding);

  // Get the information of the resource bound to the given descriptor.
  Result GetDescriptorInfo(const uint32_t descriptor_set,
                           const uint32_t binding,
                           ResourceInfo* info);

  virtual void Shutdown();
  virtual Result ProcessCommands() = 0;

 protected:
  Pipeline(
      PipelineType type,
      VkDevice device,
      const VkPhysicalDeviceMemoryProperties& properties,
      uint32_t fence_timeout_ms,
      const std::vector<VkPipelineShaderStageCreateInfo>& shader_stage_info);
  Result InitializeCommandBuffer(VkCommandPool pool, VkQueue queue);
  Result CreateVkDescriptorRelatedObjectsIfNeeded();
  Result UpdateDescriptorSetsIfNeeded();

  Result SendDescriptorDataToDeviceIfNeeded();
  void BindVkPipeline();
  void BindVkDescriptorSets();

  const std::vector<VkPipelineShaderStageCreateInfo>& GetShaderStageInfo()
      const {
    return shader_stage_info_;
  }

  uint32_t GetFenceTimeout() const { return fence_timeout_ms_; }

  VkPipeline pipeline_ = VK_NULL_HANDLE;
  VkPipelineLayout pipeline_layout_ = VK_NULL_HANDLE;

  VkDevice device_ = VK_NULL_HANDLE;
  VkPhysicalDeviceMemoryProperties memory_properties_;
  std::unique_ptr<CommandBuffer> command_;

 private:
  enum class DescriptorSetLayoutType : uint8_t {
    kEmpty = 0,
    kNonEmpty,
  };

  Result CreatePipelineLayout();

  Result CreateDescriptorSetLayouts();
  Result CreateDescriptorPools();
  Result CreateDescriptorSets();

  void DestroyDescriptorSetLayouts();
  void DestroyDescriptorPools();

  // Sort |descriptors_| in the order of |descriptors_set_| and |binding_|.
  void SortDescriptorsBySetAndBinding();

  PipelineType pipeline_type_;
  std::vector<std::unique_ptr<Descriptor>> descriptors_;
  std::vector<DescriptorSetLayoutType> descriptor_set_layout_types_;
  std::vector<VkDescriptorSetLayout> descriptor_set_layouts_;
  std::vector<VkDescriptorPool> descriptor_pools_;
  std::vector<VkDescriptorSet> descriptor_sets_;
  std::vector<VkPipelineShaderStageCreateInfo> shader_stage_info_;
  uint32_t fence_timeout_ms_ = 100;
  bool descriptor_related_objects_already_created_ = false;
  bool need_sort_descriptors_ = true;
};

}  // namespace vulkan
}  // namespace amber

#endif  // SRC_VULKAN_PIPELINE_H_
