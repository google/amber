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

#include "src/sampler.h"

#include <cassert>
#include <cmath>
#include <cstring>

namespace amber {
namespace {}  // namespace

Sampler::Sampler()
    : min_filter_(FilterType::kNearest),
      mag_filter_(FilterType::kNearest),
      mipmap_mode_(FilterType::kNearest),
      address_mode_u_(AddressMode::kRepeat),
      address_mode_v_(AddressMode::kRepeat),
      border_color_(BorderColor::kFloatTransparentBlack) {}

Sampler::~Sampler() = default;

}  // namespace amber
