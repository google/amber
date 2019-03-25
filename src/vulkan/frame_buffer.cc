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

#include "src/vulkan/frame_buffer.h"

#include <algorithm>
#include <cstring>
#include <limits>
#include <vector>

#include "src/make_unique.h"
#include "src/vulkan/command_buffer.h"
#include "src/vulkan/device.h"
#include "src/vulkan/format_data.h"

namespace amber {
namespace vulkan {

FrameBuffer::FrameBuffer(
    Device* device,
    const std::vector<const amber::Pipeline::BufferInfo*>& color_attachments,
    uint32_t width,
    uint32_t height)
    : device_(device),
      color_attachments_(color_attachments),
      width_(width),
      height_(height) {}

FrameBuffer::~FrameBuffer() {
  if (frame_ != VK_NULL_HANDLE) {
    device_->GetPtrs()->vkDestroyFramebuffer(device_->GetVkDevice(), frame_,
                                             nullptr);
  }
}

Result FrameBuffer::Initialize(VkRenderPass render_pass,
                               VkFormat depth_format) {
  std::vector<VkImageView> attachments;

  if (!color_attachments_.empty()) {
    std::vector<int32_t> seen_idx(color_attachments_.size(), -1);
    for (auto* info : color_attachments_) {
      if (info->location >= color_attachments_.size())
        return Result("color attachment locations must be sequential from 0");
      if (seen_idx[info->location] != -1) {
        return Result("duplicate attachment location: " +
                      std::to_string(info->location));
      }
      seen_idx[info->location] = static_cast<int32_t>(info->location);
    }

    attachments.resize(color_attachments_.size());
    for (auto* info : color_attachments_) {
      color_images_.push_back(MakeUnique<TransferImage>(
          device_,
          ToVkFormat(
              info->buffer->AsFormatBuffer()->GetFormat().GetFormatType()),
          VK_IMAGE_ASPECT_COLOR_BIT, width_, height_, depth_));

      Result r =
          color_images_.back()->Initialize(VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                           VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
      if (!r.IsSuccess())
        return r;

      attachments[info->location] = color_images_.back()->GetVkImageView();
    }
  }

  if (depth_format != VK_FORMAT_UNDEFINED) {
    depth_image_ = MakeUnique<TransferImage>(
        device_, depth_format,
        static_cast<VkImageAspectFlags>(
            VkFormatHasStencilComponent(depth_format)
                ? VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT
                : VK_IMAGE_ASPECT_DEPTH_BIT),
        width_, height_, depth_);

    Result r =
        depth_image_->Initialize(VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                 VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    if (!r.IsSuccess())
      return r;

    attachments.push_back(depth_image_->GetVkImageView());
  }

  VkFramebufferCreateInfo frame_buffer_info = VkFramebufferCreateInfo();
  frame_buffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  frame_buffer_info.renderPass = render_pass;
  frame_buffer_info.attachmentCount = static_cast<uint32_t>(attachments.size());
  frame_buffer_info.pAttachments = attachments.data();
  frame_buffer_info.width = width_;
  frame_buffer_info.height = height_;
  frame_buffer_info.layers = 1;

  if (device_->GetPtrs()->vkCreateFramebuffer(device_->GetVkDevice(),
                                              &frame_buffer_info, nullptr,
                                              &frame_) != VK_SUCCESS) {
    return Result("Vulkan::Calling vkCreateFramebuffer Fail");
  }

  return {};
}

Result FrameBuffer::ChangeFrameImageLayout(CommandBuffer* command,
                                           FrameImageState layout) {
  if (layout == FrameImageState::kInit) {
    return Result(
        "FrameBuffer::ChangeFrameImageLayout new layout cannot be kInit");
  }

  if (layout == frame_image_layout_)
    return {};

  if (layout == FrameImageState::kProbe) {
    if (frame_image_layout_ != FrameImageState::kClearOrDraw) {
      return Result(
          "FrameBuffer::ChangeFrameImageLayout new layout cannot be kProbe "
          "from kInit");
    }

    for (auto& img : color_images_) {
      img->ChangeLayout(command, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                        VK_PIPELINE_STAGE_TRANSFER_BIT);
    }
    if (depth_image_) {
      depth_image_->ChangeLayout(
          command, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
          VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
          VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
          VK_PIPELINE_STAGE_TRANSFER_BIT);
    }

    frame_image_layout_ = FrameImageState::kProbe;
    return {};
  }

  VkImageLayout old_layout = frame_image_layout_ == FrameImageState::kInit
                                 ? VK_IMAGE_LAYOUT_UNDEFINED
                                 : VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

  VkPipelineStageFlagBits source_stage =
      frame_image_layout_ == FrameImageState::kInit
          ? VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT
          : VK_PIPELINE_STAGE_TRANSFER_BIT;

  for (auto& img : color_images_) {
    img->ChangeLayout(command, old_layout,
                      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, source_stage,
                      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
  }
  if (depth_image_) {
    depth_image_->ChangeLayout(
        command, old_layout, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        source_stage, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT);
  }
  frame_image_layout_ = FrameImageState::kClearOrDraw;
  return {};
}

void FrameBuffer::CopyColorImagesToHost(CommandBuffer* command) {
  for (auto& img : color_images_) {
    ChangeFrameImageLayout(command, FrameImageState::kProbe);
    img->CopyToHost(command);
  }
}

void FrameBuffer::CopyImagesToBuffers() {
  for (size_t i = 0; i < color_images_.size(); ++i) {
    auto& img = color_images_[i];
    auto* info = color_attachments_[i];
    auto* values = info->buffer->ValuePtr();
    values->resize(info->buffer->GetSizeInBytes());
    std::memcpy(values->data(), img->HostAccessibleMemoryPtr(),
                info->buffer->GetSizeInBytes());
  }
}

}  // namespace vulkan
}  // namespace amber
