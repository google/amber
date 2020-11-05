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

#ifndef SRC_IMAGE_H_
#define SRC_IMAGE_H_

#include <cstdint>

namespace amber {

enum class ImageDimension : int8_t { kUnknown = -1, k1D = 0, k2D = 1, k3D = 2 };

}  // namespace amber

#endif  // SRC_IMAGE_H_
