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

#include <utility>

#include "src/make_unique.h"
#include "src/type_parser.h"

namespace amber {
namespace {

std::string FormatModeToName(FormatMode mode) {
  switch (mode) {
    case FormatMode::kUNorm:
      return "UNORM";
    case FormatMode::kUInt:
      return "UINT";
    case FormatMode::kUScaled:
      return "USCALED";
    case FormatMode::kSInt:
      return "SINT";
    case FormatMode::kSNorm:
      return "SNORM";
    case FormatMode::kSScaled:
      return "SSCALED";
    case FormatMode::kSRGB:
      return "SRGB";
    case FormatMode::kSFloat:
      return "SFLOAT";
    case FormatMode::kUFloat:
      return "UFLOAT";
  }

  return "";
}

}  // namespace

Format::Format(type::Type* type) : type_(type) {
  auto name = GenerateName();
  if (name == "")
    format_type_ = FormatType::kUnknown;
  else
    format_type_ = TypeParser::NameToFormatType(name);
  RebuildSegments();
}

Format::~Format() = default;

uint32_t Format::SizeInBytes() const {
  uint32_t size = 0;
  for (const auto& seg : segments_) {
    if (seg.IsPadding()) {
      size += seg.PaddingBytes();
      continue;
    }
    size += static_cast<uint32_t>(seg.SizeInBytes());
  }

  return size;
}

bool Format::Equal(const Format* b) const {
  return format_type_ == b->format_type_ && layout_ == b->layout_ &&
         type_->Equal(b->type_);
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

void Format::SetLayout(Layout layout) {
  layout_ = layout;
  RebuildSegments();
}

void Format::RebuildSegments() {
  segments_.clear();
  AddSegmentsForType(type_);
}

void Format::AddPaddedSegment(uint32_t size) {
  segments_.push_back(Segment{size});
}

bool Format::NeedsPadding(type::Type* t) const {
  if (layout_ == Layout::kStd140 && (t->IsMatrix() || t->IsArray()))
    return true;
  if (t->IsVec3() || (t->IsMatrix() && t->RowCount() == 3))
    return true;
  return false;
}

uint32_t Format::AddSegmentsForType(type::Type* type) {
  if (type->IsList() && type->AsList()->IsPacked()) {
    auto l = type->AsList();
    segments_.push_back(Segment(FormatComponentType::kR, FormatMode::kUInt,
                                l->PackSizeInBits()));
    return l->SizeInBytes();
  }

  if (type->IsStruct()) {
    auto s = type->AsStruct();
    uint32_t cur_offset = 0;
    for (const auto& member : s->Members()) {
      if (member.HasOffset()) {
        assert(static_cast<uint32_t>(member.offset_in_bytes) >= cur_offset);

        AddPaddedSegment(static_cast<uint32_t>(member.offset_in_bytes) -
                         cur_offset);
        cur_offset = static_cast<uint32_t>(member.offset_in_bytes);
      }

      uint32_t seg_size = 0;
      if (member.type->IsSizedArray()) {
        for (size_t i = 0; i < member.type->ArraySize(); ++i)
          seg_size += AddSegmentsForType(member.type);
      } else {
        seg_size = AddSegmentsForType(member.type);
      }

      if (member.HasArrayStride()) {
        uint32_t array_stride =
            static_cast<uint32_t>(member.array_stride_in_bytes);
        assert(seg_size <= array_stride && "Array has more size then stride");
        seg_size = array_stride;
      } else if (member.HasMatrixStride()) {
        uint32_t matrix_stride =
            static_cast<uint32_t>(member.matrix_stride_in_bytes);
        assert(seg_size <= matrix_stride && "Matrix has more size then stride");
        seg_size = matrix_stride;
      }
      cur_offset += seg_size;
    }
    if (s->HasStride()) {
      assert(cur_offset <= s->StrideInBytes() &&
             "Struct has more members then fit within stride");
      AddPaddedSegment(s->StrideInBytes() - cur_offset);
      cur_offset = s->StrideInBytes();
    }
    return cur_offset;
  }

  // List members are only numbers and must not be vecs or matrices.
  if (type->IsList()) {
    uint32_t size = 0;
    auto l = type->AsList();
    for (uint32_t i = 0; i < type->ColumnCount(); ++i) {
      for (const auto& m : l->Members()) {
        segments_.push_back(Segment{m.name, m.mode, m.num_bits});
        size += m.SizeInBytes();
      }

      if (NeedsPadding(type)) {
        auto& seg = l->Members().back();
        for (size_t k = 0; k < (4 - type->RowCount()); ++k) {
          AddPaddedSegment(seg.SizeInBytes());
          size += seg.SizeInBytes();
        }
      }
    }
    return size;
  }

  auto n = type->AsNumber();
  uint32_t size = 0;
  for (uint32_t i = 0; i < type->ColumnCount(); ++i) {
    for (uint32_t k = 0; k < type->RowCount(); ++k) {
      segments_.push_back(Segment{static_cast<FormatComponentType>(i),
                                  n->GetFormatMode(), n->NumBits()});
      size += type->SizeInBytes();
    }

    // In std140 a matrix (column count > 1) has each row stored like an array
    // which rounds up to a vec4.
    //
    // In std140 and std430 a vector of size 3N will round up to a vector of 4N.
    if (NeedsPadding(type)) {
      for (size_t k = 0; k < (4 - type->RowCount()); ++k) {
        AddPaddedSegment(type->SizeInBytes());
        size += type->SizeInBytes();
      }
    }
  }
  return size;
}

std::string Format::GenerateName() const {
  if (type_->IsMatrix())
    return "";

  static const char NAME_PARTS[] = "RGBAXDS";
  if (type_->IsList()) {
    std::vector<std::pair<std::string, std::string>> parts;
    const auto& members = type_->AsList()->Members();
    for (size_t i = 0; i < members.size(); ++i) {
      const auto& member = members[i];
      std::string name(1, NAME_PARTS[static_cast<uint8_t>(member.name)]);
      name += std::to_string(member.num_bits);

      std::string type = FormatModeToName(member.mode);
      parts.push_back({name, type});
    }

    std::string name = "";
    for (size_t i = 0; i < parts.size(); ++i) {
      name += parts[i].first;

      if (i + 1 < parts.size() && parts[i].second != parts[i + 1].second)
        name += "_" + parts[i].second + "_";

      // Handle the X8_D24 underscore.
      if (parts[i].first[0] == 'X')
        name += "_";
    }
    name += "_" + parts.back().second;
    if (type_->AsList()->IsPacked())
      name += "_PACK" + std::to_string(type_->AsList()->PackSizeInBits());

    return name;
  }

  if (type_->IsNumber()) {
    std::string name = "";
    for (uint32_t i = 0; i < type_->RowCount(); ++i)
      name += NAME_PARTS[i] + std::to_string(type_->SizeInBytes() * 8);

    name += "_" + FormatModeToName(type_->AsNumber()->GetFormatMode());
    return name;
  }

  return "";
}

}  // namespace amber
