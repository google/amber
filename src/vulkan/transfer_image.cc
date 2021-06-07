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

#include "src/vulkan/transfer_image.h"

#include <cstring>
#include <limits>
#include <vector>

#include "src/vulkan/command_buffer.h"
#include "src/vulkan/device.h"

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
    0,                                   /* usage */
    VK_SHARING_MODE_EXCLUSIVE,           /* sharingMode */
    0,                                   /* queueFamilyIndexCount */
    nullptr,                             /* pQueueFamilyIndices */
    VK_IMAGE_LAYOUT_UNDEFINED,           /* initialLayout */
};

VkSampleCountFlagBits GetVkSampleCount(uint32_t samples) {
  switch (samples) {
    case 1u:
      return VK_SAMPLE_COUNT_1_BIT;
    case 2u:
      return VK_SAMPLE_COUNT_2_BIT;
    case 4u:
      return VK_SAMPLE_COUNT_4_BIT;
    case 8u:
      return VK_SAMPLE_COUNT_8_BIT;
    case 16u:
      return VK_SAMPLE_COUNT_16_BIT;
    case 32u:
      return VK_SAMPLE_COUNT_32_BIT;
    case 64u:
      return VK_SAMPLE_COUNT_64_BIT;
  }

  return VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM;
}

}  // namespace

TransferImage::TransferImage(Device* device,
                             const Format& format,
                             VkImageAspectFlags aspect,
                             VkImageType image_type,
                             VkImageUsageFlags image_usage_flags,
                             uint32_t x,
                             uint32_t y,
                             uint32_t z,
                             uint32_t mip_levels,
                             uint32_t base_mip_level,
                             uint32_t used_mip_levels,
                             uint32_t samples)
    : Resource(
          device,
          x * y * z *
              (format.SizeInBytes() +
               // D24_UNORM_S8_UINT requires 32bit component for depth when
               // performing buffer copies. Reserve extra room to handle that.
               (format.GetFormatType() == FormatType::kD24_UNORM_S8_UINT ? 1
                                                                         : 0))),
      image_info_(kDefaultImageInfo),
      aspect_(aspect),
      mip_levels_(mip_levels),
      base_mip_level_(base_mip_level),
      used_mip_levels_(used_mip_levels),
      samples_(samples) {
  image_info_.format = device_->GetVkFormat(format);
  image_info_.imageType = image_type;
  image_info_.extent = {x, y, z};
  image_info_.mipLevels = mip_levels;
  image_info_.samples = GetVkSampleCount(samples);
  image_info_.usage = image_usage_flags;
}

TransferImage::~TransferImage() {
  if (view_ != VK_NULL_HANDLE) {
    device_->GetPtrs()->vkDestroyImageView(device_->GetVkDevice(), view_,
                                           nullptr);
  }

  if (image_ != VK_NULL_HANDLE)
    device_->GetPtrs()->vkDestroyImage(device_->GetVkDevice(), image_, nullptr);

  if (memory_ != VK_NULL_HANDLE)
    device_->GetPtrs()->vkFreeMemory(device_->GetVkDevice(), memory_, nullptr);

  if (host_accessible_memory_ != VK_NULL_HANDLE) {
    UnMapMemory(host_accessible_memory_);
    device_->GetPtrs()->vkFreeMemory(device_->GetVkDevice(),
                                     host_accessible_memory_, nullptr);
  }

  if (host_accessible_buffer_ != VK_NULL_HANDLE) {
    device_->GetPtrs()->vkDestroyBuffer(device_->GetVkDevice(),
                                        host_accessible_buffer_, nullptr);
  }
}

Result TransferImage::Initialize() {
  if (image_ != VK_NULL_HANDLE)
    return Result("Vulkan::TransferImage was already initialized");

  if (device_->GetPtrs()->vkCreateImage(device_->GetVkDevice(), &image_info_,
                                        nullptr, &image_) != VK_SUCCESS) {
    return Result("Vulkan::Calling vkCreateImage Fail");
  }

  uint32_t memory_type_index = 0;
  Result r = AllocateAndBindMemoryToVkImage(image_, &memory_,
                                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                            false, &memory_type_index);
  if (!r.IsSuccess())
    return r;

  if (aspect_ & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT) &&
      !(image_info_.usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
    // Combined depth/stencil image used as a descriptor. Only one aspect can be
    // used for the image view.
    r = CreateVkImageView(VK_IMAGE_ASPECT_DEPTH_BIT);
  } else {
    r = CreateVkImageView(aspect_);
  }

  if (!r.IsSuccess())
    return r;

  // For images, we always make a secondary buffer. When the tiling of an image
  // is optimal, read/write data from CPU does not show correct values. We need
  // a secondary buffer to convert the GPU-optimal data to CPU-readable data
  // and vice versa.
  r = CreateVkBuffer(
      &host_accessible_buffer_,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
  if (!r.IsSuccess())
    return r;

  memory_type_index = 0;
  r = AllocateAndBindMemoryToVkBuffer(host_accessible_buffer_,
                                      &host_accessible_memory_,
                                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                      true, &memory_type_index);
  if (!r.IsSuccess())
    return r;

  return MapMemory(host_accessible_memory_);
}

VkImageViewType TransferImage::GetImageViewType() const {
  // TODO(alan-baker): handle other view types.
  // 1D-array, 2D-array, Cube, Cube-array.
  switch (image_info_.imageType) {
    case VK_IMAGE_TYPE_1D:
      return VK_IMAGE_VIEW_TYPE_1D;
    case VK_IMAGE_TYPE_2D:
      return VK_IMAGE_VIEW_TYPE_2D;
    case VK_IMAGE_TYPE_3D:
      return VK_IMAGE_VIEW_TYPE_3D;
    default:
      break;
  }

  // Default to 2D image view.
  return VK_IMAGE_VIEW_TYPE_2D;
}

Result TransferImage::CreateVkImageView(VkImageAspectFlags aspect) {
  VkImageViewCreateInfo image_view_info = VkImageViewCreateInfo();
  image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  image_view_info.image = image_;
  image_view_info.viewType = GetImageViewType();
  image_view_info.format = image_info_.format;
  image_view_info.components = {
      VK_COMPONENT_SWIZZLE_R,
      VK_COMPONENT_SWIZZLE_G,
      VK_COMPONENT_SWIZZLE_B,
      VK_COMPONENT_SWIZZLE_A,
  };
  image_view_info.subresourceRange = {
      aspect,           /* aspectMask */
      base_mip_level_,  /* baseMipLevel */
      used_mip_levels_, /* levelCount */
      0,                /* baseArrayLayer */
      1,                /* layerCount */
  };

  if (device_->GetPtrs()->vkCreateImageView(device_->GetVkDevice(),
                                            &image_view_info, nullptr,
                                            &view_) != VK_SUCCESS) {
    return Result("Vulkan::Calling vkCreateImageView Fail");
  }

  return {};
}

VkBufferImageCopy TransferImage::CreateBufferImageCopy(
    VkImageAspectFlags aspect,
    uint32_t mip_level) {
  VkBufferImageCopy copy_region = VkBufferImageCopy();
  if (aspect == VK_IMAGE_ASPECT_STENCIL_BIT) {
    // Store stencil data at the end of the buffer after depth data.
    copy_region.bufferOffset =
        GetSizeInBytes() - image_info_.extent.width * image_info_.extent.height;
  } else {
    copy_region.bufferOffset = 0;
  }
  // Row length of 0 results in tight packing of rows, so the row stride
  // is the number of texels times the texel stride.
  copy_region.bufferRowLength = 0;
  copy_region.bufferImageHeight = 0;
  copy_region.imageSubresource = {
      aspect,    /* aspectMask */
      mip_level, /* mipLevel */
      0,         /* baseArrayLayer */
      1,         /* layerCount */
  };
  copy_region.imageOffset = {0, 0, 0};
  copy_region.imageExtent = {image_info_.extent.width >> mip_level,
                             image_info_.extent.height >> mip_level,
                             image_info_.extent.depth};
  return copy_region;
}

void TransferImage::CopyToHost(CommandBuffer* command_buffer) {
  const VkImageAspectFlagBits aspects[] = {VK_IMAGE_ASPECT_COLOR_BIT,
                                           VK_IMAGE_ASPECT_DEPTH_BIT,
                                           VK_IMAGE_ASPECT_STENCIL_BIT};
  // Copy operations don't support multisample images.
  if (samples_ > 1)
    return;

  std::vector<VkBufferImageCopy> copy_regions;
  uint32_t last_mip_level = used_mip_levels_ == VK_REMAINING_MIP_LEVELS
                                ? mip_levels_
                                : base_mip_level_ + used_mip_levels_;
  for (uint32_t i = base_mip_level_; i < last_mip_level; i++) {
    for (auto aspect : aspects) {
      if (aspect_ & aspect) {
        copy_regions.push_back(CreateBufferImageCopy(aspect, i));
      }
    }
  }

  device_->GetPtrs()->vkCmdCopyImageToBuffer(
      command_buffer->GetVkCommandBuffer(), image_,
      VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, host_accessible_buffer_,
      static_cast<uint32_t>(copy_regions.size()), copy_regions.data());

  MemoryBarrier(command_buffer);
}

void TransferImage::CopyToDevice(CommandBuffer* command_buffer) {
  // Copy operations don't support multisample images.
  if (samples_ > 1)
    return;

  const VkImageAspectFlagBits aspects[] = {VK_IMAGE_ASPECT_COLOR_BIT,
                                           VK_IMAGE_ASPECT_DEPTH_BIT,
                                           VK_IMAGE_ASPECT_STENCIL_BIT};
  std::vector<VkBufferImageCopy> copy_regions;
  uint32_t last_mip_level = used_mip_levels_ == VK_REMAINING_MIP_LEVELS
                                ? mip_levels_
                                : base_mip_level_ + used_mip_levels_;
  for (uint32_t i = base_mip_level_; i < last_mip_level; i++) {
    for (auto aspect : aspects) {
      if (aspect_ & aspect) {
        copy_regions.push_back(CreateBufferImageCopy(aspect, i));
      }
    }
  }

  device_->GetPtrs()->vkCmdCopyBufferToImage(
      command_buffer->GetVkCommandBuffer(), host_accessible_buffer_, image_,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      static_cast<uint32_t>(copy_regions.size()), copy_regions.data());

  MemoryBarrier(command_buffer);
}

void TransferImage::ImageBarrier(CommandBuffer* command_buffer,
                                 VkImageLayout to_layout,
                                 VkPipelineStageFlags to_stage) {
  if (to_layout == layout_ && to_stage == stage_)
    return;

  VkImageMemoryBarrier barrier = VkImageMemoryBarrier();
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = layout_;
  barrier.newLayout = to_layout;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = image_;
  barrier.subresourceRange = {
      aspect_,                 /* aspectMask */
      0,                       /* baseMipLevel */
      VK_REMAINING_MIP_LEVELS, /* levelCount */
      0,                       /* baseArrayLayer */
      1,                       /* layerCount */
  };

  switch (layout_) {
    case VK_IMAGE_LAYOUT_PREINITIALIZED:
      barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
      break;
    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
      barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
      break;
    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
      barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
      break;
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
      barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      break;
    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
      barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
      break;
    default:
      barrier.srcAccessMask = 0;
      break;
  }

  switch (to_layout) {
    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
      barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                              VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
      break;
    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
      barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                              VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
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
      barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      break;
    default:
      barrier.dstAccessMask = 0;
      break;
  }

  device_->GetPtrs()->vkCmdPipelineBarrier(command_buffer->GetVkCommandBuffer(),
                                           stage_, to_stage, 0, 0, NULL, 0,
                                           NULL, 1, &barrier);

  layout_ = to_layout;
  stage_ = to_stage;
}

Result TransferImage::AllocateAndBindMemoryToVkImage(
    VkImage image,
    VkDeviceMemory* memory,
    VkMemoryPropertyFlags flags,
    bool force_flags,
    uint32_t* memory_type_index) {
  if (memory_type_index == nullptr) {
    return Result(
        "Vulkan: TransferImage::AllocateAndBindMemoryToVkImage "
        "memory_type_index is "
        "nullptr");
  }

  *memory_type_index = 0;

  if (image == VK_NULL_HANDLE)
    return Result("Vulkan::Given VkImage is VK_NULL_HANDLE");
  if (memory == nullptr)
    return Result("Vulkan::Given VkDeviceMemory pointer is nullptr");

  VkMemoryRequirements requirement;
  device_->GetPtrs()->vkGetImageMemoryRequirements(device_->GetVkDevice(),
                                                   image, &requirement);

  *memory_type_index =
      ChooseMemory(requirement.memoryTypeBits, flags, force_flags);
  if (*memory_type_index == std::numeric_limits<uint32_t>::max())
    return Result("Vulkan::Find Proper Memory Fail");

  Result r = AllocateMemory(memory, requirement.size, *memory_type_index);
  if (!r.IsSuccess())
    return r;

  if (device_->GetPtrs()->vkBindImageMemory(device_->GetVkDevice(), image,
                                            *memory, 0) != VK_SUCCESS) {
    return Result("Vulkan::Calling vkBindImageMemory Fail");
  }

  return {};
}

}  // namespace vulkan
}  // namespace amber
