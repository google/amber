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

#include "src/vulkan/device.h"

#include <memory>
#include <vector>

#include "src/make_unique.h"

namespace amber {
namespace vulkan {
namespace {

bool AreAllRequiredFeaturesSupported(
    const VkPhysicalDeviceFeatures& features,
    const VkPhysicalDeviceFeatures& required_features) {
  const VkBool32* supported = reinterpret_cast<const VkBool32*>(&features);
  const VkBool32* required =
      reinterpret_cast<const VkBool32*>(&required_features);
  for (size_t i = 0; i < (sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32));
       ++i) {
    if (required[i] == VK_TRUE && supported[i] == VK_FALSE)
      return false;
  }
  return true;
}

}  // namespace

Device::Device() = default;
Device::Device(VkDevice device) : device_(device), destroy_device_(false) {}
Device::~Device() = default;

void Device::Shutdown() {
  if (destroy_device_) {
    vkDestroyDevice(device_, nullptr);
    vkDestroyInstance(instance_, nullptr);
  }
}

Result Device::Initialize(const VkPhysicalDeviceFeatures& features) {
  if (device_ == VK_NULL_HANDLE) {
    Result r = CreateInstance();
    if (!r.IsSuccess())
      return r;

    r = ChoosePhysicalDevice(features);
    if (!r.IsSuccess())
      return r;

    r = CreateDevice(features);
    if (!r.IsSuccess())
      return r;
  }

  if (queue_ == VK_NULL_HANDLE) {
    vkGetDeviceQueue(device_, queue_family_index_, 0, &queue_);
    if (queue_ == VK_NULL_HANDLE)
      return Result("Vulkan::Calling vkGetDeviceQueue Fail");
  }
  return {};
}

bool Device::ChooseQueueFamilyIndex(const VkPhysicalDevice& physical_device) {
  uint32_t count;
  std::vector<VkQueueFamilyProperties> properties;

  vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &count, nullptr);
  properties.resize(count);
  vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &count,
                                           properties.data());

  for (uint32_t i = 0; i < count; ++i) {
    if (properties[i].queueFlags &
        (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT)) {
      queue_family_flags_ = properties[i].queueFlags;
      queue_family_index_ = i;
      return true;
    }
  }

  for (uint32_t i = 0; i < count; ++i) {
    if (properties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
      queue_family_flags_ = properties[i].queueFlags;
      queue_family_index_ = i;
      return true;
    }
  }

  return false;
}

Result Device::CreateInstance() {
  VkApplicationInfo appInfo = {};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);

  VkInstanceCreateInfo instInfo = {};
  instInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instInfo.pApplicationInfo = &appInfo;
  // TODO(jaebaek): Enable layers, extensions

  if (vkCreateInstance(&instInfo, nullptr, &instance_) != VK_SUCCESS)
    return Result("Vulkan::Calling vkCreateInstance Fail");

  return {};
}

Result Device::ChoosePhysicalDevice(
    const VkPhysicalDeviceFeatures& required_features) {
  uint32_t count;
  std::vector<VkPhysicalDevice> physical_devices;

  if (vkEnumeratePhysicalDevices(instance_, &count, nullptr) != VK_SUCCESS)
    return Result("Vulkan::Calling vkEnumeratePhysicalDevices Fail");
  physical_devices.resize(count);
  if (vkEnumeratePhysicalDevices(instance_, &count, physical_devices.data()) !=
      VK_SUCCESS)
    return Result("Vulkan::Calling vkEnumeratePhysicalDevices Fail");

  for (uint32_t i = 0; i < count; ++i) {
    VkPhysicalDeviceFeatures features = {};
    vkGetPhysicalDeviceFeatures(physical_devices[i], &features);

    if (!AreAllRequiredFeaturesSupported(features, required_features))
      continue;

    if (ChooseQueueFamilyIndex(physical_devices[i])) {
      physical_device_ = physical_devices[i];
      vkGetPhysicalDeviceMemoryProperties(physical_device_,
                                          &physical_memory_properties_);
      return {};
    }
  }

  return Result("Vulkan::No physical device supports Vulkan");
}

Result Device::CreateDevice(const VkPhysicalDeviceFeatures& features) {
  VkDeviceQueueCreateInfo queue_info;
  const float priorities[] = {1.0f};

  queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queue_info.queueFamilyIndex = queue_family_index_;
  queue_info.queueCount = 1;
  queue_info.pQueuePriorities = priorities;

  VkDeviceCreateInfo info = {};
  info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  info.pQueueCreateInfos = &queue_info;
  info.queueCreateInfoCount = 1;
  // TODO(jaebaek): Enable layers, extensions
  info.pEnabledFeatures = &features;

  if (vkCreateDevice(physical_device_, &info, nullptr, &device_) != VK_SUCCESS)
    return Result("Vulkan::Calling vkCreateDevice Fail");

  return {};
}

}  // namespace vulkan
}  // namespace amber
