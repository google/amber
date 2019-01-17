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
#include <string>
#include <unordered_map>
#include <vector>

#include "amber/result.h"
#include "amber/vulkan_header.h"
#include "src/cast_hash.h"
#include "src/engine.h"
#include "src/vulkan/command.h"
#include "src/vulkan/descriptor.h"
#include "src/vulkan/push_constant.h"

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

  // Add information of how and what to do with push constant.
  Result AddPushConstant(const BufferCommand* command);

  // Copy the contents of the resource bound to the given descriptor
  // to host memory.
  Result CopyDescriptorToHost(const uint32_t descriptor_set,
                              const uint32_t binding);

  // Get the information of the resource bound to the given descriptor.
  Result GetDescriptorInfo(const uint32_t descriptor_set,
                           const uint32_t binding,
                           ResourceInfo* info);

  void SetEntryPointName(VkShaderStageFlagBits stage,
                         const std::string& entry) {
    entry_points_[stage] = entry;
  }

  virtual void Shutdown();
  virtual Result ProcessCommands() = 0;

 protected:
  Pipeline(
      PipelineType type,
      VkDevice device,
      const VkPhysicalDeviceProperties& properties,
      const VkPhysicalDeviceMemoryProperties& memory_properties,
      uint32_t fence_timeout_ms,
      const std::vector<VkPipelineShaderStageCreateInfo>& shader_stage_info);

  // Create |push_constant_| and |command_|. This method also
  // initializes |command_| that abstracts VkCommandBuffer and
  // VkFence objects.
  Result Initialize(VkCommandPool pool, VkQueue queue);

  // Create Vulkan descriptor related objects i.e.,
  // VkDescriptorSetLayout, VkDescriptorPool, VkDescriptorSet if
  // |descriptor_related_objects_already_created_| is false. This
  // method also creates VkPipelineLayout if |pipeline_layout_| is
  // VK_NULL_HANDLE.
  Result CreateVkDescriptorRelatedObjectsAndPipelineLayoutIfNeeded();

  Result UpdateDescriptorSetsIfNeeded();

  Result SendDescriptorDataToDeviceIfNeeded();
  void BindVkPipeline();
  void BindVkDescriptorSets();

  // Record a Vulkan command for push contant.
  Result RecordPushConstant();

  const std::vector<VkPipelineShaderStageCreateInfo>& GetShaderStageInfo()
      const {
    return shader_stage_info_;
  }

  const char* GetEntryPointName(VkShaderStageFlagBits stage) const;
  uint32_t GetFenceTimeout() const { return fence_timeout_ms_; }

  VkPipeline pipeline_ = VK_NULL_HANDLE;
  VkPipelineLayout pipeline_layout_ = VK_NULL_HANDLE;

  VkDevice device_ = VK_NULL_HANDLE;
  VkPhysicalDeviceMemoryProperties memory_properties_;
  std::unique_ptr<CommandBuffer> command_;

 private:
  struct DescriptorSetInfo {
    bool empty = true;
    VkDescriptorSetLayout layout = VK_NULL_HANDLE;
    VkDescriptorPool pool = VK_NULL_HANDLE;
    VkDescriptorSet vk_desc_set = VK_NULL_HANDLE;
    std::vector<std::unique_ptr<Descriptor>> descriptors_;
  };

  void DestroyVkDescriptorAndPipelineRelatedObjects();
  void ResetVkPipelineRelatedObjects();
  Result CreatePipelineLayout();

  Result CreateDescriptorSetLayouts();
  Result CreateDescriptorPools();
  Result CreateDescriptorSets();

  PipelineType pipeline_type_;
  VkPhysicalDeviceProperties physical_device_properties_;
  std::vector<DescriptorSetInfo> descriptor_set_info_;
  std::vector<VkPipelineShaderStageCreateInfo> shader_stage_info_;

  uint32_t fence_timeout_ms_ = 100;
  bool descriptor_related_objects_already_created_ = false;
  std::unordered_map<VkShaderStageFlagBits,
                     std::string,
                     CastHash<VkShaderStageFlagBits>>
      entry_points_;

  std::unique_ptr<PushConstant> push_constant_;
};

}  // namespace vulkan
}  // namespace amber

#endif  // SRC_VULKAN_PIPELINE_H_
