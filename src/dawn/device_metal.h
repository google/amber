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

#ifndef SRC_DAWN_DEVICE_METAL_H_
#define SRC_DAWN_DEVICE_METAL_H_

#if AMBER_DAWN_METAL

#include "amber/result.h"
#include "dawn/dawncpp.h"
#include "dawn_native/DawnNative.h"

namespace amber {
namespace dawn {

Result CreateMetalDevice(::dawn::Device*,
                         std::unique_ptr<::dawn_native::Instance>*);

}  // namespace dawn
}  // namespace amber

#endif  // AMBER_DAWN_METAL

#endif  // SRC_DAWN_DEVICE_METAL_H_
