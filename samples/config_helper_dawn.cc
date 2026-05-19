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
#include <utility>

namespace sample {

ConfigHelperDawn::ConfigHelperDawn() = default;
ConfigHelperDawn::~ConfigHelperDawn() = default;

namespace {

// Callback which prints a message from a Dawn device operation.
void PrintDeviceError(wgpu::Device const& /* device */,
                      wgpu::ErrorType errorType,
                      wgpu::StringView message) {
  switch (errorType) {
    case wgpu::ErrorType::Validation:
      std::cout << "Validation ";
      break;
    case wgpu::ErrorType::OutOfMemory:
      std::cout << "Out of memory ";
      break;
    case wgpu::ErrorType::Internal:
      std::cout << "Internal ";
      break;
    case wgpu::ErrorType::Unknown:
      std::cout << "Unknown ";
      break;
    default:
      std::cout << "Unreachable";
      return;
  }
  std::cout << "error: " << (message.data ? message.data : "unknown error")
            << std::endl;
}

// Callback which prints a message when the Dawn device is lost.
void HandleDeviceLost(wgpu::Device const& /* device */,
                      wgpu::DeviceLostReason reason,
                      wgpu::StringView message) {
  std::cout << "Device lost ";
  switch (reason) {
    case wgpu::DeviceLostReason::Destroyed:
      std::cout << "(Destroyed) ";
      break;
    case wgpu::DeviceLostReason::CallbackCancelled:
      std::cout << "(Callback Cancelled) ";
      break;
    case wgpu::DeviceLostReason::FailedCreation:
      std::cout << "(Failed Creation) ";
      break;
    case wgpu::DeviceLostReason::Unknown:
    default:
      std::cout << "(Unknown) ";
      break;
  }
  std::cout << "error: " << (message.data ? message.data : "unknown error")
            << std::endl;
}

}  // namespace

amber::Result ConfigHelperDawn::CreateConfig(
    uint32_t,
    uint32_t,
    int32_t,
    const std::vector<std::string>&,
    const std::vector<std::string>&,
    const std::vector<std::string>&,
    bool,
    bool,
    bool,
    std::unique_ptr<amber::EngineConfig>* config) {
  const char* allow_unsafe_apis = "allow_unsafe_apis";
  wgpu::DawnTogglesDescriptor toggles;
  toggles.enabledToggleCount = 1;
  toggles.enabledToggles = &allow_unsafe_apis;

  wgpu::InstanceFeatureName required_features[] = {
      wgpu::InstanceFeatureName::ShaderSourceSPIRV};
  wgpu::InstanceDescriptor instance_desc;
  instance_desc.nextInChain = &toggles;
  instance_desc.requiredFeatureCount = 1;
  instance_desc.requiredFeatures = required_features;
  dawn_instance_ = wgpu::CreateInstance(&instance_desc);

  if (!dawn_instance_) {
    return amber::Result("could not create Dawn instance");
  }

  wgpu::RequestAdapterOptions adapter_options;
#if AMBER_DAWN_METAL
  adapter_options.backendType = wgpu::BackendType::Metal;
#else  // assuming VULKAN
  adapter_options.backendType = wgpu::BackendType::Vulkan;
#endif

  struct RequestAdapterData {
    wgpu::Adapter adapter = nullptr;
    bool completed = false;
  } adapter_data;

  dawn_instance_.RequestAdapter(
      &adapter_options, wgpu::CallbackMode::AllowProcessEvents,
      [](wgpu::RequestAdapterStatus status, wgpu::Adapter adapter,
         wgpu::StringView message, RequestAdapterData* data) {
        if (status == wgpu::RequestAdapterStatus::Success) {
          data->adapter = std::move(adapter);
        } else {
          std::cerr << "RequestAdapter failed: "
                    << (message.data ? message.data : "unknown error")
                    << std::endl;
        }
        data->completed = true;
      },
      &adapter_data);

  while (!adapter_data.completed) {
    dawn_instance_.ProcessEvents();
  }

  if (!adapter_data.adapter) {
    return amber::Result("could not find Vulkan or Metal backend for Dawn");
  }

  wgpu::DeviceDescriptor device_desc;
  device_desc.nextInChain = &toggles;
  device_desc.SetUncapturedErrorCallback(PrintDeviceError);
  device_desc.SetDeviceLostCallback(wgpu::CallbackMode::AllowProcessEvents,
                                    HandleDeviceLost);

  // Use synchronous CreateDevice extension in Dawn
  dawn_device_ = adapter_data.adapter.CreateDevice(&device_desc);

  if (!dawn_device_) {
    return amber::Result("could not create Dawn device");
  }

  auto* dawn_config = new amber::DawnEngineConfig;
  dawn_config->instance = dawn_instance_;
  dawn_config->device = dawn_device_;
  config->reset(dawn_config);

  return {};
}

}  // namespace sample
