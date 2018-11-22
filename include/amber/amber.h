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

#include <string>

#include "amber/result.h"

namespace amber {

enum class EngineType : uint8_t {
  kVulkan = 0,
  kDawn,
};

struct Options {
  EngineType engine = EngineType::kVulkan;
  void* default_device = nullptr;
  bool parse_only = false;
};

class Amber {
 public:
  Amber();
  ~Amber();

  amber::Result Execute(const std::string& data, const Options& opts);
};

}  // namespace amber

#endif  // AMBER_AMBER_H_
