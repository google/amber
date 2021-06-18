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
#include "src/vulkan/buffer_backed_descriptor.h"
#include "src/vulkan/command_buffer.h"
#include "src/vulkan/push_constant.h"
#include "src/vulkan/resource.h"

namespace amber {

class BufferCommand;

namespace vulkan {

class ComputePipeline;
class Device;
class GraphicsPipeline;

/// Base class for a pipeline in Vulkan.
class Pipeline {
 public:
  virtual ~Pipeline();

  bool IsGraphics() const { return pipeline_type_ == PipelineType::kGraphics; }
  bool IsCompute() const { return pipeline_type_ == PipelineType::kCompute; }

  GraphicsPipeline* AsGraphics();
  ComputePipeline* AsCompute();

  Result AddBufferDescriptor(const BufferCommand*);
  Result AddSamplerDescriptor(const SamplerCommand*);

  /// Add |buffer| data to the push constants at |offset|.
  Result AddPushConstantBuffer(const Buffer* buf, uint32_t offset);

  /// Reads back the contents of resources of all descriptors to a
  /// buffer data object and put it into buffer data queue in host.
  Result ReadbackDescriptorsToHostDataQueue();

  std::unordered_map<Buffer*, std::unique_ptr<Resource>>&
  GetDescriptorTransferResources() {
    return descriptor_transfer_resources_;
  }

  void SetEntryPointName(VkShaderStageFlagBits stage,
                         const std::string& entry) {
    entry_points_[stage] = entry;
  }

  CommandBuffer* GetCommandBuffer() const { return command_.get(); }
  Device* GetDevice() const { return device_; }

 protected:
  Pipeline(
      PipelineType type,
      Device* device,
      uint32_t fence_timeout_ms,
      const std::vector<VkPipelineShaderStageCreateInfo>& shader_stage_info);

  /// Initializes the pipeline.
  Result Initialize(CommandPool* pool);

  Result GetDescriptorSlot(uint32_t desc_set,
                           uint32_t binding,
                           Descriptor** desc);
  void UpdateDescriptorSetsIfNeeded();

  Result SendDescriptorDataToDeviceIfNeeded();
  void BindVkDescriptorSets(const VkPipelineLayout& pipeline_layout);

  /// Records a Vulkan command for push contant.
  Result RecordPushConstant(const VkPipelineLayout& pipeline_layout);

  const std::vector<VkPipelineShaderStageCreateInfo>& GetVkShaderStageInfo()
      const {
    return shader_stage_info_;
  }

  const char* GetEntryPointName(VkShaderStageFlagBits stage) const;
  uint32_t GetFenceTimeout() const { return fence_timeout_ms_; }

  Result CreateVkPipelineLayout(VkPipelineLayout* pipeline_layout);

  Device* device_ = nullptr;
  std::unique_ptr<CommandBuffer> command_;

 private:
  struct DescriptorSetInfo {
    bool empty = true;
    VkDescriptorSetLayout layout = VK_NULL_HANDLE;
    VkDescriptorPool pool = VK_NULL_HANDLE;
    VkDescriptorSet vk_desc_set = VK_NULL_HANDLE;
    std::vector<std::unique_ptr<Descriptor>> descriptors;
  };

  /// Creates Vulkan descriptor related objects.
  Result CreateVkDescriptorRelatedObjectsIfNeeded();
  Result CreateDescriptorSetLayouts();
  Result CreateDescriptorPools();
  Result CreateDescriptorSets();
  /// Adds a buffer used by a descriptor. The added buffers are be stored in
  /// |descriptor_buffers_| vector in the order they are added.
  Result AddDescriptorBuffer(Buffer* amber_buffer);

  PipelineType pipeline_type_;
  std::vector<DescriptorSetInfo> descriptor_set_info_;
  std::vector<VkPipelineShaderStageCreateInfo> shader_stage_info_;
  std::unordered_map<Buffer*, std::unique_ptr<Resource>>
      descriptor_transfer_resources_;
  /// Buffers used by descriptors (buffer descriptors and image descriptors).
  std::vector<Buffer*> descriptor_buffers_;

  uint32_t fence_timeout_ms_ = 1000;
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
