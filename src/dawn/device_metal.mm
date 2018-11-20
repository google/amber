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

#include <Metal/Metal.h>
#include <iostream>
#include "amber/result.h"
#include "dawn/dawncpp.h"
#include "dawn_native/DawnNative.h"
#include "dawn_native/MetalBackend.h"

namespace {
void PrintDeviceError(const char* message, dawn::CallbackUserdata) {
  std::cout << "Dawn device error: " << message << std::endl;
}
}  // namespace

namespace amber {
namespace dawn {

Result CreateMetalDevice(::dawn::Device* device_ptr) {
  if (!device_ptr) {
    return Result("::amber::dawn::CreateMetalDevice: invalid device parameter");
  }
  dawnDevice cDevice =
      ::dawn_native::metal::CreateDevice(MTLCreateSystemDefaultDevice());
  if (!cDevice) {
    return Result(
        "::amber::dawn::CreateMetalDevice: Failed to create metal device");
  }
  dawnProcTable procs = ::dawn_native::GetProcs();
  dawnSetProcs(&procs);
  procs.deviceSetErrorCallback(cDevice, PrintDeviceError, 0);
  *device_ptr = ::dawn::Device::Acquire(cDevice);

  return {};
}

}  // namespace dawn
}  // namespace amber
