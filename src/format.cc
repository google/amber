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

#include "src/format.h"

namespace amber {

Format::Format() = default;

Format::Format(const Format&) = default;

Format::~Format() = default;

uint32_t Format::SizeInBytes() const {
  uint32_t bits = 0;
  for (const auto& comp : components_)
    bits += comp.num_bits;

  if (is_std140_ && components_.size() == 3)
    bits += components_[0].num_bits;

  uint32_t bytes_per_element = bits / 8;
  return bytes_per_element * column_count_;
}

bool Format::AreAllComponents(FormatMode mode, uint32_t bits) const {
  for (const auto& comp : components_) {
    if (comp.mode != mode || comp.num_bits != bits)
      return false;
  }
  return true;
}

}  // namespace amber
