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
                               const Format* depth_format) {
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
          device_, *info->buffer->GetFormat(), VK_IMAGE_ASPECT_COLOR_BIT,
          width_, height_, depth_));

      Result r = color_images_.back()->Initialize(
          VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
          VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
      if (!r.IsSuccess())
        return r;

      attachments[info->location] = color_images_.back()->GetVkImageView();
    }
  }

  if (depth_format && depth_format->IsFormatKnown()) {
    depth_image_ = MakeUnique<TransferImage>(
        device_, *depth_format,
        static_cast<VkImageAspectFlags>(depth_format->HasStencilComponent()
                                            ? VK_IMAGE_ASPECT_DEPTH_BIT |
                                                  VK_IMAGE_ASPECT_STENCIL_BIT
                                            : VK_IMAGE_ASPECT_DEPTH_BIT),
        width_, height_, depth_);

    Result r = depth_image_->Initialize(
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
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

void FrameBuffer::ChangeFrameLayout(CommandBuffer* command,
                                    VkImageLayout color_layout,
                                    VkPipelineStageFlags color_stage,
                                    VkImageLayout depth_layout,
                                    VkPipelineStageFlags depth_stage) {
  for (auto& img : color_images_)
    img->ImageBarrier(command, color_layout, color_stage);

  if (depth_image_)
    depth_image_->ImageBarrier(command, depth_layout, depth_stage);
}

void FrameBuffer::ChangeFrameToDrawLayout(CommandBuffer* command) {
  ChangeFrameLayout(command,
                    // Color attachments
                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                    // Depth attachment
                    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                    VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT);
}

void FrameBuffer::ChangeFrameToProbeLayout(CommandBuffer* command) {
  ChangeFrameLayout(
      command,
      // Color attachments
      VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT,
      // Depth attachments
      VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT);
}

void FrameBuffer::ChangeFrameToWriteLayout(CommandBuffer* command) {
  ChangeFrameLayout(
      command,
      // Color attachments
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT,
      // Depth attachments
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT);
}

void FrameBuffer::TransferColorImagesToHost(CommandBuffer* command) {
  for (auto& img : color_images_)
    img->CopyToHost(command);
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

void FrameBuffer::TransferColorImagesToDevice(CommandBuffer* command) {
  for (auto& img : color_images_)
    img->CopyToDevice(command);
}

void FrameBuffer::CopyBuffersToImages() {
  for (size_t i = 0; i < color_images_.size(); ++i) {
    auto& img = color_images_[i];
    auto* info = color_attachments_[i];
    auto* values = info->buffer->ValuePtr();
    // Nothing to do if our local buffer is empty
    if (values->empty())
      continue;

    std::memcpy(img->HostAccessibleMemoryPtr(), values->data(),
                info->buffer->GetSizeInBytes());
  }
}

}  // namespace vulkan
}  // namespace amber
