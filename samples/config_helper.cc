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

#include "samples/config_helper.h"

#include <string>

#include "src/feature.h"
#include "src/make_unique.h"
#include "src/script.h"
#include "src/vulkan/device.h"
#include "vulkan/vulkan.h"

namespace sample {

ConfigHelper::ConfigHelper() = default;

ConfigHelper::~ConfigHelper() = default;

std::pair<amber::Result, amber::VulkanEngineConfig>
ConfigHelper::CreateVulkanConfig(const std::vector<amber::Recipe>& recipes) {
  amber::VulkanEngineConfig config = {};

  if (device_) {
    return std::make_pair(amber::Result("Sample: Vulkan device already exists"),
                          config);
  }

  std::vector<amber::Feature> required_features;
  std::vector<std::string> required_extensions;
  for (const auto& recipe : recipes) {
    const amber::Script* script =
        static_cast<const amber::Script*>(recipe.GetImpl());
    if (!script) {
      return std::make_pair(
          amber::Result("Sample: Recipe must contain a parsed script"), config);
    }

    const auto& features = script->RequiredFeatures();
    const auto& extensions = script->RequiredExtensions();
    required_features.insert(required_features.end(), features.begin(),
                             features.end());
    required_extensions.insert(required_extensions.end(), extensions.begin(),
                               extensions.end());
  }

  device_ = amber::MakeUnique<amber::vulkan::Device>();
  amber::Result r = device_->Initialize(required_features, required_extensions);
  if (!r.IsSuccess())
    return std::make_pair(r, config);

  config.instance = device_->GetInstance();
  config.physical_device = device_->GetPhysicalDevice();
  config.device = device_->GetDevice();
  return std::make_pair(r, config);
}

void ConfigHelper::Shutdown() {
  device_->Shutdown();
}

}  // namespace sample
