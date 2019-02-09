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

#include "samples/config_helper_dawn.h"
#include "samples/dawn_device_metal.h"

namespace sample {

ConfigHelperDawn::ConfigHelperDawn() = default;

ConfigHelperDawn::~ConfigHelperDawn() = default;

amber::Result ConfigHelperDawn::CreateConfig(
    uint32_t,
    uint32_t,
    const std::vector<std::string>&,
    const std::vector<std::string>&,
    bool,
    std::unique_ptr<amber::EngineConfig>* config) {
#if AMBER_DAWN_METAL
  auto r = dawn::CreateMetalDevice(&dawn_instance_, &dawn_device_);
  if (!r.IsSuccess())
    return r;
#else
  return amber::Result("Can't make Dawn engine config");
#endif

  auto* dawn_config = new amber::DawnEngineConfig;
  dawn_config->device = &dawn_device_;
  config->reset(dawn_config);
  return {};
}

amber::Result ConfigHelperDawn::Shutdown() {
  dawn_device_ = {};
  return {};
}

}  // namespace sample
