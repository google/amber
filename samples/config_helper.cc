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
#include <cassert>
#include <set>
#include <string>
#include <vector>

#include "src/make_unique.h"

#if AMBER_ENGINE_VULKAN
#include "samples/config_helper_vulkan.h"
#endif  // AMBER_ENGINE_VULKAN

namespace sample {

ConfigHelper::ConfigHelper() = default;

ConfigHelper::~ConfigHelper() = default;

std::unique_ptr<amber::EngineConfig> ConfigHelper::CreateConfig(
    amber::EngineType engine,
    const std::vector<std::string>& required_features,
    const std::vector<std::string>& required_extensions) {
  if (engine == amber::EngineType::kDawn)
    return nullptr;

#if AMBER_ENGINE_VULKAN
  impl_ = amber::MakeUnique<ConfigHelperVulkan>();
#endif  // AMBER_ENGINE_VULKAN
  return impl_ ? impl_->CreateConfig(required_features, required_extensions)
               : nullptr;
}

void ConfigHelper::Shutdown() {
  if (!impl_)
    return;
  impl_->Shutdown();
}

}  // namespace sample
