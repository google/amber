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

#include "device_metal.h"

#include "amber/result.h"
#include "dawn/dawncpp.h"
#include "dawn_native/DawnNative.h"
#include "dawn_native/MetalBackend.h"

namespace amber {
namespace dawn {

Result CreateMetalDevice(
    ::dawn::Device* device_ptr,
    std::unique_ptr<::dawn_native::Instance>* dawn_instance_ptr) {
  if (!device_ptr) {
    return Result("::amber::dawn::CreateMetalDevice: invalid device parameter");
  }
  if (!dawn_instance_ptr) {
    return Result(
        "::amber::dawn::CreateMetalDevice: invalid dawn instance parameter");
  }
  *device_ptr = nullptr;
  dawn_instance_ptr->reset(new ::dawn_native::Instance);
  (*dawn_instance_ptr)->DiscoverDefaultAdapters();
  for (dawn_native::Adapter adapter : (*dawn_instance_ptr)->GetAdapters()) {
    if (adapter.GetBackendType() == ::dawn_native::BackendType::Metal) {
      *device_ptr = ::dawn::Device::Acquire(adapter.CreateDevice());
      return {};
    }
  }

  return Result(
      "::amber::dawn::CreateMetalDevice: Failed to create metal device");
}

}  // namespace dawn
}  // namespace amber
