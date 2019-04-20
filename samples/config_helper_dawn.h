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

#ifndef SAMPLES_CONFIG_HELPER_DAWN_H_
#define SAMPLES_CONFIG_HELPER_DAWN_H_

#include <limits>
#include <memory>
#include <string>
#include <vector>

#include "amber/amber.h"
#include "amber/amber_dawn.h"
#include "dawn_native/DawnNative.h"
#include "samples/config_helper.h"

namespace sample {

/// Child class of ConfigHelperImpl for Dawn.
class ConfigHelperDawn : public ConfigHelperImpl {
 public:
  ConfigHelperDawn();
  ~ConfigHelperDawn() override;

  /// Create a Dawn instance and device and return them as
  /// amber::DawnEngineConfig.  Engine version number and features
  /// and extension lists are ignored.
  amber::Result CreateConfig(
      uint32_t engine_major,
      uint32_t engine_minor,
      const std::vector<std::string>& required_features,
      const std::vector<std::string>& required_instance_extensions,
      const std::vector<std::string>& required_device_extensions,
      bool disable_validation_layer,
      bool show_version_info,
      std::unique_ptr<amber::EngineConfig>* config) override;

 private:
  ::dawn_native::Instance dawn_instance_;
  ::dawn::Device dawn_device_;
};

}  // namespace sample

#endif  // SAMPLES_CONFIG_HELPER_DAWN_H_
