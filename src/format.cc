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
  // Odd number of bits, inflate or byte count to accommodate
  if ((bits % 8) != 0)
    bytes_per_element += 1;

  return bytes_per_element * column_count_;
}

bool Format::AreAllComponents(FormatMode mode, uint32_t bits) const {
  for (const auto& comp : components_) {
    if (comp.mode != mode || comp.num_bits != bits)
      return false;
  }
  return true;
}

bool Format::Equal(const Format* b) const {
  if (type_ != b->type_ || is_std140_ != b->is_std140_ ||
      pack_size_in_bytes_ != b->pack_size_in_bytes_ ||
      column_count_ != b->column_count_) {
    return false;
  }
  if (components_.size() != b->components_.size())
    return false;

  for (uint32_t i = 0; i < components_.size(); ++i) {
    if (components_[i].type != b->components_[i].type ||
        components_[i].mode != b->components_[i].mode ||
        components_[i].num_bits != b->components_[i].num_bits) {
      return false;
    }
  }
  return true;
}

}  // namespace amber
