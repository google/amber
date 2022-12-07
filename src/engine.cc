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

#include "src/engine.h"

#include "src/make_unique.h"

#if AMBER_ENGINE_VULKAN
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"
#include "src/vulkan/engine_vulkan.h"
#pragma clang diagnostic pop
#endif  // AMBER_ENGINE_VULKAN

#if AMBER_ENGINE_DAWN
#include "src/dawn/engine_dawn.h"
#endif  // AMBER_ENGINE_DAWN

namespace amber {

// static
std::unique_ptr<Engine> Engine::Create(EngineType type) {
  std::unique_ptr<Engine> engine;
  switch (type) {
    case kEngineTypeVulkan:
#if AMBER_ENGINE_VULKAN
      engine = MakeUnique<vulkan::EngineVulkan>();
#endif  // AMBER_ENGINE_VULKAN
      break;
    case kEngineTypeDawn:
#if AMBER_ENGINE_DAWN
      engine = MakeUnique<dawn::EngineDawn>();
#endif  // AMBER_ENGINE_DAWN
      break;
  }
  return engine;
}

Engine::Engine() = default;

Engine::~Engine() = default;

}  // namespace amber
