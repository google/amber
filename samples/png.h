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

#ifndef SAMPLES_PNG_H_
#define SAMPLES_PNG_H_

#include <string>
#include <utility>
#include <vector>

#include "amber/amber.h"

namespace png {

/// Converts the image of dimensions |width| and |height| and with pixels stored
/// in row-major order in |values| with format B8G8R8A8 into PNG format,
/// returning the PNG binary as a string.
amber::Result ConvertToPNG(uint32_t width,
                           uint32_t height,
                           const std::vector<amber::Value>& values,
                           std::vector<uint8_t>* buffer);

}  // namespace png

#endif  // SAMPLES_PNG_H_
