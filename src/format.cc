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

#include "src/make_unique.h"

namespace amber {

Format::Format() = default;

Format::~Format() = default;

uint32_t Format::SizeInBytes() const {
  uint32_t size = 0;
  for (const auto& seg : segments_)
    size += static_cast<uint32_t>(seg.GetComponent()->SizeInBytes());

  return size;
}

bool Format::AreAllComponents(FormatMode mode, uint32_t bits) const {
  for (const auto& comp : components_) {
    if (comp->mode != mode || comp->num_bits != bits)
      return false;
  }
  return true;
}

bool Format::Equal(const Format* b) const {
  if (type_ != b->type_ || layout_ != b->layout_ ||
      pack_size_in_bytes_ != b->pack_size_in_bytes_ ||
      column_count_ != b->column_count_) {
    return false;
  }
  if (components_.size() != b->components_.size())
    return false;

  for (uint32_t i = 0; i < components_.size(); ++i) {
    if (components_[i]->type != b->components_[i]->type ||
        components_[i]->mode != b->components_[i]->mode ||
        components_[i]->num_bits != b->components_[i]->num_bits) {
      return false;
    }
  }
  return true;
}

uint32_t Format::InputNeededPerElement() const {
  uint32_t count = 0;
  for (const auto& seg : segments_) {
    if (seg.IsPadding())
      continue;

    count += 1;
  }
  return count;
}

void Format::AddComponent(FormatComponentType type,
                          FormatMode mode,
                          uint8_t bits) {
  components_.push_back(MakeUnique<Component>(type, mode, bits));
  RebuildSegments();
}

void Format::SetColumnCount(uint32_t c) {
  column_count_ = c;
  RebuildSegments();
}

void Format::SetLayout(Layout layout) {
  layout_ = layout;
  RebuildSegments();
}

void Format::RebuildSegments() {
  segments_.clear();

  for (size_t i = 0; i < column_count_; ++i) {
    for (size_t k = 0; k < components_.size(); ++k) {
      segments_.push_back(Segment{components_[k].get()});
    }

    // In std140 a matrix (column count > 1) has each row stored like an array
    // which rounds up to a vec4.
    //
    // In std140 and std430 a vector of size 3N will round up to a vector of 4N.
    if ((layout_ == Layout::kStd140 && column_count_ > 1) || RowCount() == 3) {
      for (size_t k = 0; k < (4 - RowCount()); ++k) {
        // TODO(dsinclair): This component will be wrong if all the components
        // aren't the same size. This will be the case when we have struct
        // support ....
        segments_.push_back(Segment{components_[0].get()});
        segments_.back().SetIsPadding();
      }
    }
  }
}

std::string Format::GenerateName() const {
  std::string name = "";
  std::string parts = "ARGB";
  for (const auto& comp : components_) {
    name += parts[static_cast<uint8_t>(comp->type)] +
            std::to_string(comp->num_bits);
  }
  name += "_";
  switch (components_[0]->mode) {
    case FormatMode::kUNorm:
    case FormatMode::kUFloat:
    case FormatMode::kUScaled:
    case FormatMode::kSNorm:
    case FormatMode::kSScaled:
    case FormatMode::kSRGB:
      return "";
    case FormatMode::kUInt:
      name += "UINT";
      break;
    case FormatMode::kSInt:
      name += "SINT";
      break;
    case FormatMode::kSFloat:
      name += "SFLOAT";
      break;
  }
  return name;
}

}  // namespace amber
