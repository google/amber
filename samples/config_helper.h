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

#ifndef SAMPLES_CONFIG_HELPER_H_
#define SAMPLES_CONFIG_HELPER_H_

#include <memory>
#include <string>
#include <vector>

#include "amber/amber.h"

namespace sample {

// Proof of concept implementation showing how to provide and use
// EngineConfig within sample amber program.
class ConfigHelperImpl {
 public:
  virtual ~ConfigHelperImpl();

  // Create instance and device and return them as amber::EngineConfig.
  // |required_features| and |required_extensions| contain lists of
  // required features and required extensions, respectively.
  virtual amber::Result CreateConfig(
      const std::vector<std::string>& required_features,
      const std::vector<std::string>& required_extensions,
      std::unique_ptr<amber::EngineConfig>* config) = 0;

  // Destroy instance and device.
  virtual amber::Result Shutdown() = 0;
};

// Wrapper of ConfigHelperImpl.
class ConfigHelper {
 public:
  ConfigHelper();
  ~ConfigHelper();

  // Create instance and device and return them as amber::EngineConfig.
  // |required_features| and |required_extensions| contain lists of
  // required features and required extensions, respectively. |engine|
  // indicates whether the caller required VulkanEngineConfig or
  // DawnEngineConfig.
  amber::Result CreateConfig(
      amber::EngineType engine,
      const std::vector<std::string>& required_features,
      const std::vector<std::string>& required_extensions,
      std::unique_ptr<amber::EngineConfig>* config);

  // Destroy instance and device.
  amber::Result Shutdown();

 private:
  std::unique_ptr<ConfigHelperImpl> impl_;
};

}  // namespace sample

#endif  // SAMPLES_CONFIG_HELPER_H_
