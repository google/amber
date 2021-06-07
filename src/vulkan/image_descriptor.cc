// Copyright 2019 The Amber Authors.
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

#include "src/vulkan/image_descriptor.h"

#include <algorithm>
#include <unordered_map>
#include <utility>

#include "src/vulkan/device.h"
#include "src/vulkan/resource.h"

namespace amber {
namespace vulkan {

ImageDescriptor::ImageDescriptor(Buffer* buffer,
                                 DescriptorType type,
                                 Device* device,
                                 uint32_t base_mip_level,
                                 uint32_t desc_set,
                                 uint32_t binding,
                                 Pipeline* pipeline)
    : BufferBackedDescriptor(buffer, type, device, desc_set, binding, pipeline),
      base_mip_level_(base_mip_level),
      vulkan_sampler_(device) {}

ImageDescriptor::~ImageDescriptor() = default;

Result ImageDescriptor::CreateResourceIfNeeded() {
  auto& transfer_resources = pipeline_->GetDescriptorTransferResources();

  for (const auto& amber_buffer : GetAmberBuffers()) {
    if (amber_buffer->ValuePtr()->empty())
      continue;

    // Check if the transfer image is already created.
    if (transfer_resources.count(amber_buffer) > 0) {
      continue;
    }

    // Default to 2D image.
    VkImageType image_type = VK_IMAGE_TYPE_2D;
    switch (amber_buffer->GetImageDimension()) {
      case ImageDimension::k1D:
        image_type = VK_IMAGE_TYPE_1D;
        break;
      case ImageDimension::k2D:
        image_type = VK_IMAGE_TYPE_2D;
        break;
      case ImageDimension::k3D:
        image_type = VK_IMAGE_TYPE_3D;
        break;
      default:
        break;
    }

    Format* fmt = amber_buffer->GetFormat();
    VkImageAspectFlags aspect = 0;
    if (fmt->HasDepthComponent() && fmt->HasStencilComponent()) {
      aspect = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    } else if (fmt->HasDepthComponent()) {
      aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
    } else if (fmt->HasStencilComponent()) {
      aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
    } else {
      aspect = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    VkImageUsageFlags usage =
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    if (type_ == DescriptorType::kStorageImage) {
      usage |= VK_IMAGE_USAGE_STORAGE_BIT;
    } else {
      assert(type_ == DescriptorType::kSampledImage ||
             type_ == DescriptorType::kCombinedImageSampler);
      usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
    }

    auto transfer_image = MakeUnique<TransferImage>(
        device_, *fmt, aspect, image_type, usage, amber_buffer->GetWidth(),
        amber_buffer->GetHeight(), amber_buffer->GetDepth(),
        amber_buffer->GetMipLevels(), base_mip_level_, VK_REMAINING_MIP_LEVELS,
        amber_buffer->GetSamples());

    // Store the transfer image to the pipeline's map of transfer images.
    transfer_resources[amber_buffer] = std::move(transfer_image);
  }

  if (amber_sampler_) {
    Result r = vulkan_sampler_.CreateSampler(amber_sampler_);
    if (!r.IsSuccess())
      return r;
  }

  is_descriptor_set_update_needed_ = true;
  return {};
}

void ImageDescriptor::UpdateDescriptorSetIfNeeded(
    VkDescriptorSet descriptor_set) {
  if (!is_descriptor_set_update_needed_)
    return;

  // Always use general layout.
  VkImageLayout layout = VK_IMAGE_LAYOUT_GENERAL;

  std::vector<VkDescriptorImageInfo> image_infos;

  // Create VkDescriptorImageInfo for every descriptor image.
  for (const auto& amber_buffer : GetAmberBuffers()) {
    const auto& image =
        pipeline_->GetDescriptorTransferResources()[amber_buffer]
            ->AsTransferImage();
    VkDescriptorImageInfo image_info = {vulkan_sampler_.GetVkSampler(),
                                        image->GetVkImageView(), layout};
    image_infos.push_back(image_info);
  }

  VkWriteDescriptorSet write = VkWriteDescriptorSet();
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.dstSet = descriptor_set;
  write.dstBinding = binding_;
  write.dstArrayElement = 0;
  write.descriptorCount = static_cast<uint32_t>(image_infos.size());
  write.descriptorType = GetVkDescriptorType();
  write.pImageInfo = image_infos.data();

  device_->GetPtrs()->vkUpdateDescriptorSets(device_->GetVkDevice(), 1, &write,
                                             0, nullptr);

  is_descriptor_set_update_needed_ = false;
}

}  // namespace vulkan
}  // namespace amber
