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

#ifndef SRC_VULKAN_TRANSFER_IMAGE_H_
#define SRC_VULKAN_TRANSFER_IMAGE_H_

#include "amber/result.h"
#include "amber/vulkan_header.h"
#include "src/format.h"
#include "src/vulkan/resource.h"

namespace amber {
namespace vulkan {

class CommandBuffer;
class Device;

class TransferImage : public Resource {
 public:
  TransferImage(Device* device,
                const Format& format,
                VkImageAspectFlags aspect,
                uint32_t x,
                uint32_t y,
                uint32_t z);
  ~TransferImage() override;

  Result Initialize(VkImageUsageFlags usage);
  VkImageView GetVkImageView() const { return view_; }

  void ImageBarrier(CommandBuffer* command,
                    VkImageLayout to_layout,
                    VkPipelineStageFlags to_stage);

  // Only record the command for copying this image to its secondary
  // host-accessible buffer. The actual submission of the command
  // must be done later.
  void CopyToHost(CommandBuffer* command) override;
  void CopyToDevice(CommandBuffer* command) override;

 private:
  Result CreateVkImageView();
  Result AllocateAndBindMemoryToVkImage(VkImage image,
                                        VkDeviceMemory* memory,
                                        VkMemoryPropertyFlags flags,
                                        bool force_flags,
                                        uint32_t* memory_type_index);
  VkBufferImageCopy CreateBufferImageCopy();

  VkBuffer host_accessible_buffer_ = VK_NULL_HANDLE;
  VkDeviceMemory host_accessible_memory_ = VK_NULL_HANDLE;

  VkImageCreateInfo image_info_;
  VkImageAspectFlags aspect_;

  VkImage image_ = VK_NULL_HANDLE;
  VkImageView view_ = VK_NULL_HANDLE;
  VkDeviceMemory memory_ = VK_NULL_HANDLE;

  VkImageLayout layout_ = VK_IMAGE_LAYOUT_UNDEFINED;
  VkPipelineStageFlags stage_ = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
};

}  // namespace vulkan
}  // namespace amber

#endif  // SRC_VULKAN_TRANSFER_IMAGE_H_
