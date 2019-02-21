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

#ifndef SRC_VULKAN_BUFFER_H_
#define SRC_VULKAN_BUFFER_H_

#include "amber/result.h"
#include "amber/vulkan_header.h"
#include "src/vulkan/resource.h"

namespace amber {
namespace vulkan {

class CommandBuffer;
class Device;

// Class managing Vulkan Buffer i.e., VkBuffer |buffer_|. |memory_|
// has VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT and
// VK_MEMORY_PROPERTY_HOST_COHERENT_BIT properties and it is mapped
// to |buffer_|.
class Buffer : public Resource {
 public:
  Buffer(Device* device,
         size_t size_in_bytes,
         const VkPhysicalDeviceMemoryProperties& properties);
  ~Buffer() override;

  // Create |buffer_| whose usage is |usage| and allocate |memory_|
  // with VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT and
  // VK_MEMORY_PROPERTY_HOST_COHERENT_BIT properties. It also maps
  // |memory_| to |buffer_|
  Result Initialize(const VkBufferUsageFlags usage);

  VkBuffer GetVkBuffer() const { return buffer_; }
  Result CreateVkBufferView(VkFormat format);
  VkBufferView GetVkBufferView() const { return view_; }

  // Since |buffer_| is mapped to host accessible and host coherent
  // memory |memory_|, this method only conducts memory barrier to
  // make it available to device domain.
  virtual Result CopyToDevice(CommandBuffer* command);

  // Resource
  VkDeviceMemory GetHostAccessMemory() const override { return memory_; }

  // Since |buffer_| is mapped to host accessible and host coherent
  // memory |memory_|, this method only conducts memory barrier to
  // make it available to host domain.
  Result CopyToHost(CommandBuffer* command) override;

  // Copy all data from |src| to |this| and wait until
  // the memory update is effective by calling vkCmdPipelineBarrier().
  // Note that this method only records the copy command and the
  // actual submission of the command must be done later.
  void CopyFromBuffer(CommandBuffer* command, const Buffer& src);

  void Shutdown() override;

 private:
  VkBuffer buffer_ = VK_NULL_HANDLE;
  VkBufferView view_ = VK_NULL_HANDLE;
  VkDeviceMemory memory_ = VK_NULL_HANDLE;
};

}  // namespace vulkan
}  // namespace amber

#endif  // SRC_VULKAN_BUFFER_H_
