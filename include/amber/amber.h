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

#ifndef AMBER_AMBER_H_
#define AMBER_AMBER_H_

#include <memory>
#include <string>

#include "amber/result.h"

namespace amber {

enum class EngineType : uint8_t {
  /// Use the Vulkan backend, if available
  kVulkan = 0,
  /// Use the Dawn backend, if available
  kDawn,
};

/// Override point of engines to add their own configuration.
struct EngineConfig {
};

struct Options {
  /// Sets the engine to be created. Default Vulkan.
  EngineType engine = EngineType::kVulkan;
  /// Holds engine specific configuration.
  std::unique_ptr<EngineConfig> config;
  /// Set true to only parse the given script, does not execute the engine.
  bool parse_only = false;
};

/// Main interface to the Amber environment.
class Amber {
 public:
  Amber();
  ~Amber();

  /// Executes the given |data| script with the provided |opts|. Returns a
  /// |Result| which indicates if the execution succeded.
  amber::Result Execute(const std::string& data, const Options& opts);
};

}  // namespace amber

#endif  // AMBER_AMBER_H_
