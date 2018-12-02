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
#include "src/vulkan/resource.h"
#include "vulkan/vulkan.h"

namespace amber {
namespace vulkan {

class Buffer : public Resource {
 public:
  Buffer(VkDevice device,
         size_t size_in_bytes,
         const VkPhysicalDeviceMemoryProperties& properties);
  ~Buffer() override;

  Result Initialize(const VkBufferUsageFlags usage);
  VkBuffer GetVkBuffer() const { return buffer_; }
  Result CreateVkBufferView(VkFormat format);
  VkBufferView GetVkBufferView() const { return view_; }

  // If this buffer is host accessible and has coherent memory, this
  // method does nothing. If this buffer is host accessible but it does
  // not have coherent memory, this method flush the memory to make the
  // writes from host effective to device. Otherwise, this method records
  // the command for copying the secondary host-accessible buffer to
  // this device local buffer. Note that it only records the command and
  // the actual submission must be done later.
  Result CopyToDevice(VkCommandBuffer command);

  // Resource
  VkDeviceMemory GetHostAccessMemory() const override {
    if (is_buffer_host_accessible_)
      return memory_;

    return Resource::GetHostAccessMemory();
  }

  // If this buffer is host accessible and has coherent memory, this
  // method does nothing. If this buffer is host accessible but it does
  // not have coherent memory, this method invalidate the memory to make the
  // writes from device visible to host. Otherwise, this method records
  // the command for copying this buffer to its secondary host-accessible
  // buffer. Note that it only records the command and the actual
  // submission must be done later.
  Result CopyToHost(VkCommandBuffer command) override;

  // Copy all data from |src.buffer_| to |buffer_| and wait until
  // the memory update is effective by calling vkCmdPipelineBarrier().
  // Note that this method only records the copy command and the
  // actual submission of the command must be done later.
  void CopyFromBuffer(VkCommandBuffer command, const Buffer& src);

  void Shutdown() override;

 private:
  Result InvalidateMemoryIfNeeded();
  Result FlushMemoryIfNeeded();

  VkBuffer buffer_ = VK_NULL_HANDLE;
  VkBufferView view_ = VK_NULL_HANDLE;
  VkDeviceMemory memory_ = VK_NULL_HANDLE;
  bool is_buffer_host_accessible_ = false;
  bool is_buffer_host_coherent_ = false;
};

}  // namespace vulkan
}  // namespace amber

#endif  // SRC_VULKAN_BUFFER_H_
