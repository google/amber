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

#include <iostream>

namespace sample {

ConfigHelperDawn::ConfigHelperDawn() = default;
ConfigHelperDawn::~ConfigHelperDawn() = default;

namespace {

// Callback which prints a message from a Dawn device operation.
void PrintDeviceError(DawnErrorType errorType, const char* message, void*) {
  switch (errorType) {
    case DAWN_ERROR_TYPE_VALIDATION:
      std::cout << "Validation ";
      break;
    case DAWN_ERROR_TYPE_OUT_OF_MEMORY:
      std::cout << "Out of memory ";
      break;
    case DAWN_ERROR_TYPE_UNKNOWN:
    case DAWN_ERROR_TYPE_FORCE32:
      std::cout << "Unknown ";
      break;
    case DAWN_ERROR_TYPE_DEVICE_LOST:
      std::cout << "Device lost ";
      break;
    default:
      std::cout << "Unreachable";
      return;
  }
  std::cout << "error: " << message << std::endl;
}

}  // namespace

amber::Result ConfigHelperDawn::CreateConfig(
    uint32_t,
    uint32_t,
    const std::vector<std::string>&,
    const std::vector<std::string>&,
    const std::vector<std::string>&,
    bool,
    bool,
    std::unique_ptr<amber::EngineConfig>* config) {
  // Set procedure table and error callback.
  DawnProcTable backendProcs = dawn_native::GetProcs();
  dawnSetProcs(&backendProcs);
  dawn_instance_.DiscoverDefaultAdapters();

  for (dawn_native::Adapter& adapter : dawn_instance_.GetAdapters()) {
#if AMBER_DAWN_METAL
    ::dawn_native::BackendType backendType = ::dawn_native::BackendType::Metal;
#else  // assuming VULKAN
    ::dawn_native::BackendType backendType = ::dawn_native::BackendType::Vulkan;
#endif

    if (adapter.GetBackendType() == backendType) {
      dawn_device_ = ::dawn::Device::Acquire(adapter.CreateDevice());
    }
  }

  if (!dawn_device_)
    return amber::Result("could not find Vulkan or Metal backend for Dawn");

  backendProcs.deviceSetUncapturedErrorCallback(dawn_device_.Get(),
                                                PrintDeviceError, nullptr);
  auto* dawn_config = new amber::DawnEngineConfig;
  dawn_config->device = &dawn_device_;
  config->reset(dawn_config);

  return {};
}

}  // namespace sample
