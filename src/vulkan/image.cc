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

#include "src/vulkan/image.h"

#include <limits>

#include "src/vulkan/format_data.h"

namespace amber {
namespace vulkan {
namespace {

const VkImageCreateInfo kDefaultImageInfo = {
    VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, /* sType */
    nullptr,                             /* pNext */
    0,                                   /* flags */
    VK_IMAGE_TYPE_2D,                    /* imageType */
    VK_FORMAT_R8G8B8A8_UNORM,            /* format */
    {250, 250, 1},                       /* extent */
    1,                                   /* mipLevels */
    1,                                   /* arrayLayers */
    VK_SAMPLE_COUNT_1_BIT,               /* samples */
    VK_IMAGE_TILING_OPTIMAL,             /* tiling */
    VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, /* usage */
    VK_SHARING_MODE_EXCLUSIVE,               /* sharingMode */
    0,                                       /* queueFamilyIndexCount */
    nullptr,                                 /* pQueueFamilyIndices */
    VK_IMAGE_LAYOUT_UNDEFINED,               /* initialLayout */
};

}  // namespace

Image::Image(VkDevice device,
             VkFormat format,
             uint32_t x,
             uint32_t y,
             uint32_t z,
             const VkPhysicalDeviceMemoryProperties& properties)
    : Resource(device, x * y * z * VkFormatToByteSize(format), properties),
      image_info_(kDefaultImageInfo) {
  image_info_.format = format;
  image_info_.extent = {x, y, z};
}

Image::~Image() = default;

Result Image::Initialize(VkImageUsageFlags usage) {
  if (image_ != VK_NULL_HANDLE)
    return Result("Vulkan::Image was already initalized");

  image_info_.usage = usage;

  if (vkCreateImage(GetDevice(), &image_info_, nullptr, &image_) != VK_SUCCESS)
    return Result("Vulkan::Calling vkCreateImage Fail");

  AllocateResult allocate_result = AllocateAndBindMemoryToVkImage(
      image_, &memory_, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, false);
  if (!allocate_result.r.IsSuccess())
    return allocate_result.r;

  Result r = CreateVkImageView();
  if (!r.IsSuccess())
    return r;

  // For images, we always make a secondary buffer. When the tiling of an image
  // is optimal, read/write data from CPU does not show correct values. We need
  // a secondary buffer to convert the GPU-optimial data to CPU-readable data
  // and vice versa.
  return Resource::Initialize();
}

Result Image::CreateVkImageView() {
  VkImageViewCreateInfo image_view_info = {};
  image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  image_view_info.image = image_;
  // TODO(jaebaek): Set .viewType correctly
  image_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
  image_view_info.format = image_info_.format;
  image_view_info.components = {
      VK_COMPONENT_SWIZZLE_R,
      VK_COMPONENT_SWIZZLE_G,
      VK_COMPONENT_SWIZZLE_B,
      VK_COMPONENT_SWIZZLE_A,
  };
  image_view_info.subresourceRange = {
      VK_IMAGE_ASPECT_COLOR_BIT, /* aspectMask */
      0,                         /* baseMipLevel */
      1,                         /* levelCount */
      0,                         /* baseArrayLayer */
      1,                         /* layerCount */
  };

  if (vkCreateImageView(GetDevice(), &image_view_info, nullptr, &view_) !=
      VK_SUCCESS) {
    return Result("Vulkan::Calling vkCreateImageView Fail");
  }

  return {};
}

void Image::Shutdown() {
  vkDestroyImageView(GetDevice(), view_, nullptr);
  vkDestroyImage(GetDevice(), image_, nullptr);
  vkFreeMemory(GetDevice(), memory_, nullptr);

  view_ = VK_NULL_HANDLE;
  image_ = VK_NULL_HANDLE;
  memory_ = VK_NULL_HANDLE;

  Resource::Shutdown();
}

Result Image::CopyToHost(VkCommandBuffer command) {
  VkBufferImageCopy copy_region = {};
  copy_region.bufferOffset = 0;
  // Row length of 0 results in tight packing of rows, so the row stride
  // is the number of texels times the texel stride.
  copy_region.bufferRowLength = 0;
  copy_region.bufferImageHeight = 0;
  copy_region.imageSubresource = {
      VK_IMAGE_ASPECT_COLOR_BIT, /* aspectMask */
      0,                         /* mipLevel */
      0,                         /* baseArrayLayer */
      1,                         /* layerCount */
  };
  copy_region.imageOffset = {0, 0, 0};
  copy_region.imageExtent = {image_info_.extent.width,
                             image_info_.extent.height, 1};

  vkCmdCopyImageToBuffer(command, image_, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                         GetHostAccessibleBuffer(), 1, &copy_region);

  MemoryBarrier(command);
  return {};
}

void Image::ChangeLayout(VkCommandBuffer command,
                         VkImageLayout old_layout,
                         VkImageLayout new_layout,
                         VkPipelineStageFlags from,
                         VkPipelineStageFlags to) {
  VkImageMemoryBarrier barrier = {};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = old_layout;
  barrier.newLayout = new_layout;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = image_;
  barrier.subresourceRange = {
      VK_IMAGE_ASPECT_COLOR_BIT, /* aspectMask */
      0,                         /* baseMipLevel */
      1,                         /* levelCount */
      0,                         /* baseArrayLayer */
      1,                         /* layerCount */
  };

  switch (old_layout) {
    case VK_IMAGE_LAYOUT_PREINITIALIZED:
      // Based on Vulkan spec, image in VK_IMAGE_LAYOUT_PREINITIALIZED is not
      // accessible by GPU.
      barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
      break;
    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
      barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
      break;
    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
      barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
      break;
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
      // An image becomes "transfer dst" only when we send a buffer data to
      // it.
      barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT |
                              VK_ACCESS_SHADER_WRITE_BIT |
                              VK_ACCESS_TRANSFER_WRITE_BIT;
      break;
    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
      barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
      break;
    default:
      barrier.srcAccessMask = 0;
      break;
  }

  switch (new_layout) {
    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
      barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
      break;
    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
      barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
      break;
    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
      barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
      break;
    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
      barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
      break;
    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
      barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
      break;
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
      // An image becomes "transfer dst" only when we send a buffer data to
      // it.
      barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT |
                              VK_ACCESS_SHADER_WRITE_BIT |
                              VK_ACCESS_TRANSFER_WRITE_BIT;
      break;
    default:
      barrier.dstAccessMask = 0;
      break;
  }

  vkCmdPipelineBarrier(command, from, to, 0, 0, NULL, 0, NULL, 1, &barrier);
}

}  // namespace vulkan
}  // namespace amber
