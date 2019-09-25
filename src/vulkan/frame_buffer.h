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

#ifndef SRC_VULKAN_FRAME_BUFFER_H_
#define SRC_VULKAN_FRAME_BUFFER_H_

#include <memory>
#include <vector>

#include "src/pipeline.h"
#include "src/vulkan/transfer_image.h"

namespace amber {
namespace vulkan {

class CommandBuffer;
class Device;

/// Wrapper around a Vulkan FrameBuffer object.
class FrameBuffer {
 public:
  FrameBuffer(
      Device* device,
      const std::vector<const amber::Pipeline::BufferInfo*>& color_attachments,
      uint32_t width,
      uint32_t height);
  ~FrameBuffer();

  Result Initialize(VkRenderPass render_pass, const Format* depth_format);

  void ChangeFrameToDrawLayout(CommandBuffer* command);
  void ChangeFrameToProbeLayout(CommandBuffer* command);
  void ChangeFrameToWriteLayout(CommandBuffer* command);

  VkFramebuffer GetVkFrameBuffer() const { return frame_; }
  const void* GetColorBufferPtr(size_t idx) const {
    return color_images_[idx]->HostAccessibleMemoryPtr();
  }

  // Only record the command for copying the image that backs this
  // framebuffer to the host accessible buffer. The actual submission
  // of the command must be done later.
  void TransferColorImagesToHost(CommandBuffer* command);
  void TransferColorImagesToDevice(CommandBuffer* command);

  void CopyImagesToBuffers();
  void CopyBuffersToImages();

  uint32_t GetWidth() const { return width_; }
  uint32_t GetHeight() const { return height_; }

 private:
  void ChangeFrameLayout(CommandBuffer* command,
                         VkImageLayout color_layout,
                         VkPipelineStageFlags color_stage,
                         VkImageLayout depth_layout,
                         VkPipelineStageFlags depth_stage);

  Device* device_ = nullptr;
  std::vector<const amber::Pipeline::BufferInfo*> color_attachments_;
  VkFramebuffer frame_ = VK_NULL_HANDLE;
  std::vector<std::unique_ptr<TransferImage>> color_images_;
  std::unique_ptr<TransferImage> depth_image_;
  uint32_t width_ = 0;
  uint32_t height_ = 0;
  uint32_t depth_ = 1;
};

}  // namespace vulkan
}  // namespace amber

#endif  // SRC_VULKAN_FRAME_BUFFER_H_
