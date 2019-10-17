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
#include "src/vulkan/device.h"
#include "src/vulkan/resource.h"

namespace amber {
namespace vulkan {

ImageDescriptor::ImageDescriptor(Buffer* buffer,
                                 DescriptorType type,
                                 Device* device,
                                 uint32_t desc_set,
                                 uint32_t binding)
    : Descriptor(buffer, type, device, desc_set, binding) {}

ImageDescriptor::~ImageDescriptor() = default;

void ImageDescriptor::RecordCopyDataToResourceIfNeeded(CommandBuffer* command) {
  transfer_image_->ImageBarrier(command, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                VK_PIPELINE_STAGE_TRANSFER_BIT);

  Descriptor::RecordCopyDataToResourceIfNeeded(command);

  if (type_ == DescriptorType::kStorageImage) {
    // Change to general layout as it's required for storage images.
    transfer_image_->ImageBarrier(command, VK_IMAGE_LAYOUT_GENERAL,
                                  VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
  }
}

Result ImageDescriptor::CreateResourceIfNeeded() {
  if (transfer_image_) {
    return Result(
        "Vulkan: BufferDescriptor::CreateResourceIfNeeded() must be called "
        "only when |transfer_image| is empty");
  }

  if (amber_buffer_ && amber_buffer_->ValuePtr()->empty())
    return {};

  transfer_image_ = MakeUnique<TransferImage>(
      device_, *amber_buffer_->GetFormat(), VK_IMAGE_ASPECT_COLOR_BIT,
      amber_buffer_->GetWidth(), amber_buffer_->GetHeight(), 1);
  VkImageUsageFlags usage =
      VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

  if (type_ == DescriptorType::kStorageImage) {
    usage |= VK_IMAGE_USAGE_STORAGE_BIT;
  } else {
    assert(type_ == DescriptorType::kSampledImage ||
           type_ == DescriptorType::kCombinedImageSampler);
    usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
  }

  Result r = transfer_image_->Initialize(usage);

  if (!r.IsSuccess())
    return r;

  is_descriptor_set_update_needed_ = true;
  return {};
}

Result ImageDescriptor::RecordCopyDataToHost(CommandBuffer* command) {
  transfer_image_->ImageBarrier(command, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                VK_PIPELINE_STAGE_TRANSFER_BIT);

  Descriptor::RecordCopyDataToHost(command);

  return {};
}

Result ImageDescriptor::MoveResourceToBufferOutput() {
  Result r = Descriptor::MoveResourceToBufferOutput();
  transfer_image_ = nullptr;

  return r;
}

void ImageDescriptor::UpdateDescriptorSetIfNeeded(
    VkDescriptorSet descriptor_set) {
  if (!is_descriptor_set_update_needed_)
    return;

  VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

  if (type_ == DescriptorType::kStorageImage)
    layout = VK_IMAGE_LAYOUT_GENERAL;

  VkDescriptorImageInfo image_info = {
      nullptr,  // TODO: Add sampler here later if used
      transfer_image_->GetVkImageView(), layout};

  VkWriteDescriptorSet write = VkWriteDescriptorSet();
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.dstSet = descriptor_set;
  write.dstBinding = binding_;
  write.dstArrayElement = 0;
  write.descriptorCount = 1;
  write.descriptorType = GetVkDescriptorType();
  write.pImageInfo = &image_info;

  device_->GetPtrs()->vkUpdateDescriptorSets(device_->GetVkDevice(), 1, &write,
                                             0, nullptr);

  is_descriptor_set_update_needed_ = false;
}

}  // namespace vulkan
}  // namespace amber
