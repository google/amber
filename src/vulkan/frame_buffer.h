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

#include "src/vulkan/image.h"

namespace amber {
namespace vulkan {

enum class FrameImageState : uint8_t {
  kInit = 0,
  kClearOrDraw,
  kProbe,
};

class FrameBuffer {
 public:
  FrameBuffer(VkDevice device, uint32_t width, uint32_t height);
  ~FrameBuffer();

  Result Initialize(VkRenderPass render_pass,
                    VkFormat color_format,
                    VkFormat depth_format,
                    const VkPhysicalDeviceMemoryProperties& properties);
  void Shutdown();

  void ChangeFrameImageLayout(VkCommandBuffer command, FrameImageState layout);

  VkFramebuffer GetFrameBuffer() const { return frame_; }
  const void* GetColorBufferPtr() const {
    return color_image_->HostAccessibleMemoryPtr();
  }
  VkImage GetColorImage() const { return color_image_->GetVkImage(); }
  Result CopyColorImageToHost(VkCommandBuffer command) {
    ChangeFrameImageLayout(command, FrameImageState::kProbe);
    return color_image_->CopyToHost(command);
  }

  uint32_t GetWidth() const { return width_; }
  uint32_t GetHeight() const { return height_; }

 private:
  VkDevice device_ = VK_NULL_HANDLE;
  VkFramebuffer frame_ = VK_NULL_HANDLE;
  std::unique_ptr<Image> color_image_;
  std::unique_ptr<Image> depth_image_;
  uint32_t width_ = 0;
  uint32_t height_ = 0;
  uint32_t depth_ = 1;
  FrameImageState frame_image_layout_ = FrameImageState::kInit;
};

}  // namespace vulkan
}  // namespace amber

#endif  // SRC_VULKAN_FRAME_BUFFER_H_
