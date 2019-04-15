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

#ifndef AMBER_AMBER_DAWN_H_
#define AMBER_AMBER_DAWN_H_

#include "amber/amber.h"
#include "dawn/dawncpp.h"

namespace amber {

/// Configuration for the Dawn engine.
struct DawnEngineConfig : public EngineConfig {
  ~DawnEngineConfig() override;

  /// The Dawn Device to use for running tests.
  ::dawn::Device* device = nullptr;
};

}  // namespace amber

#endif  // AMBER_AMBER_DAWN_H_
