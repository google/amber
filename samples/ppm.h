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

#ifndef SAMPLES_PPM_H_
#define SAMPLES_PPM_H_

#include <string>
#include <utility>
#include <vector>

#include "amber/amber.h"
#include "amber/result.h"

namespace ppm {

/// Converts the image of dimensions |width| and |height| and with pixels stored
/// in row-major order in |values| with format B8G8R8A8 into PPM format,
/// returning the PPM binary in |buffer|.
amber::Result ConvertToPPM(uint32_t width,
                           uint32_t height,
                           const std::vector<amber::Value>& values,
                           std::vector<uint8_t>* buffer);

}  // namespace ppm

#endif  // SAMPLES_PPM_H_
