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

#include "samples/config_helper.h"

#include <algorithm>
#include <set>
#include <string>
#include <vector>

#include "src/make_unique.h"

#if AMBER_ENGINE_DAWN
#include "samples/config_helper_dawn.h"
#endif  // AMBER_ENGINE_DAWN
#if AMBER_ENGINE_VULKAN
#include "samples/config_helper_vulkan.h"
#endif  // AMBER_ENGINE_VULKAN

namespace sample {

ConfigHelperImpl::~ConfigHelperImpl() = default;

ConfigHelper::ConfigHelper() = default;

ConfigHelper::~ConfigHelper() = default;

amber::Result ConfigHelper::CreateConfig(
    amber::EngineType engine,
    uint32_t engine_major,
    uint32_t engine_minor,
    const std::vector<std::string>& required_features,
    const std::vector<std::string>& required_extensions,
    bool disable_validation_layer,
    std::unique_ptr<amber::EngineConfig>* config) {
  switch (engine) {
    case amber::kEngineTypeVulkan:
#if AMBER_ENGINE_VULKAN
      impl_ = amber::MakeUnique<ConfigHelperVulkan>();
#else
      return amber::Result("Unable to create engine config for Vulkan");
#endif  // AMBER_ENGINE_VULKAN
      break;
    case amber::kEngineTypeDawn:
#if AMBER_ENGINE_DAWN
#else
      return amber::Result("Unable to create engine config for Dawn");
#endif  // AMBER_ENGINE_DAWN
      break;
  }

  if (!impl_)
    return amber::Result("Unable to create config helper");

  return impl_->CreateConfig(engine_major, engine_minor, required_features,
                             required_extensions, disable_validation_layer,
                             config);
}

amber::Result ConfigHelper::Shutdown() {
  if (!impl_)
    return {};
  return impl_->Shutdown();
}

}  // namespace sample
