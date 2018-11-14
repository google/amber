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

#ifndef SRC_VULKAN_IMAGE_H_
#define SRC_VULKAN_IMAGE_H_

#include "amber/result.h"
#include "src/vulkan/resource.h"
#include "vulkan/vulkan.h"

namespace amber {
namespace vulkan {

class Image : public Resource {
 public:
  Image(VkDevice device,
        VkFormat format,
        uint32_t x,
        uint32_t y,
        uint32_t z,
        const VkPhysicalDeviceMemoryProperties& properties);
  ~Image() override;

  Result Initialize(VkImageUsageFlags usage);
  VkImage GetVkImage() const { return image_; }
  VkImageView GetVkImageView() const { return view_; }

  // TODO(jaebaek): Determine copy all or partial data
  Result CopyToHost(VkCommandBuffer command);

  // TODO(jaebaek): Implement CopyToDevice

  void ChangeLayout(VkCommandBuffer command,
                    VkImageLayout old_layout,
                    VkImageLayout new_layout,
                    VkPipelineStageFlags from,
                    VkPipelineStageFlags to);

  // Resource
  VkDeviceMemory GetHostAccessMemory() const override {
    if (is_image_host_accessible_)
      return memory_;

    return Resource::GetHostAccessMemory();
  }

  void Shutdown() override;

 private:
  Result CreateVkImageView();

  VkImageCreateInfo image_info_;

  VkImage image_ = VK_NULL_HANDLE;
  VkImageView view_ = VK_NULL_HANDLE;
  VkDeviceMemory memory_ = VK_NULL_HANDLE;
  bool is_image_host_accessible_ = false;
};

}  // namespace vulkan
}  // namespace amber

#endif  // SRC_VULKAN_IMAGE_H_
