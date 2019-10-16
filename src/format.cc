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

#include <algorithm>
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

uint32_t CalculatePad(uint32_t val) {
  if ((val % 16) == 0)
    return 0;
  return 16 - (val % 16);
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
  if (layout == layout_)
    return;

  layout_ = layout;
  RebuildSegments();
}

void Format::RebuildSegments() {
  segments_.clear();
  AddSegmentsForType(type_);
}

void Format::AddPaddedSegment(uint32_t size) {
  // If the last item was already padding we just extend by the |size| bytes
  if (!segments_.empty() && segments_.back().IsPadding()) {
    segments_[segments_.size() - 1] =
        Segment{size + segments_.back().SizeInBytes()};
  } else {
    segments_.push_back(Segment{size});
  }
}

void Format::AddPaddedSegmentPackable(uint32_t size) {
  AddPaddedSegment(size);
  segments_.back().SetPackable(true);
}

bool Format::AddSegment(const Segment& seg) {
  if (!segments_.empty()) {
    auto last = segments_.back();

    if (last.IsPackable() && last.IsPadding() &&
        last.SizeInBytes() >= seg.SizeInBytes()) {
      segments_.back() = seg;
      auto pad = last.SizeInBytes() - seg.SizeInBytes();
      if (pad > 0)
        AddPaddedSegmentPackable(pad);

      return false;
    }
  }
  segments_.push_back(seg);
  return true;
}

bool Format::NeedsPadding(type::Type* t) const {
  if (layout_ == Layout::kStd140 && (t->IsMatrix() || t->IsArray()))
    return true;
  if (t->IsVec3() || (t->IsMatrix() && t->RowCount() == 3))
    return true;
  return false;
}

uint32_t Format::CalcVecBaseAlignmentInBytes(type::Number* n) const {
  // vec3 rounds up to a Vec4, so 4 * N
  if (n->IsVec3())
    return 4 * n->SizeInBytes();

  // vec2 and vec4 are 2 * N and 4 * N respectively
  return n->RowCount() * n->SizeInBytes();
}

uint32_t Format::CalcArrayBaseAlignmentInBytes(type::Type* t) const {
  uint32_t align = 0;
  if (t->IsStruct()) {
    align = CalcStructBaseAlignmentInBytes(t->AsStruct());
  } else if (t->IsMatrix()) {
    align = CalcMatrixBaseAlignmentInBytes(t->AsNumber());
  } else if (t->IsVec()) {
    align = CalcVecBaseAlignmentInBytes(t->AsNumber());
  } else if (t->IsList()) {
    align = CalcListBaseAlignmentInBytes(t->AsList());
  } else if (t->IsNumber()) {
    align = t->SizeInBytes();
  }

  // In std140 array elements round up to multiple of vec4.
  if (layout_ == Layout::kStd140)
    align += CalculatePad(align);

  return align;
}

uint32_t Format::CalcStructBaseAlignmentInBytes(type::Struct* s) const {
  uint32_t base_alignment = 0;
  for (const auto& member : s->Members()) {
    base_alignment =
        std::max(base_alignment, CalcTypeBaseAlignmentInBytes(member.type));
  }

  return base_alignment;
}

uint32_t Format::CalcMatrixBaseAlignmentInBytes(type::Number* m) const {
  // TODO(dsinclair): Deal with row major when needed. Currently this assumes
  // the matrix is column major.

  uint32_t align = 0;
  if (m->RowCount() == 3)
    align = 4 * m->SizeInBytes();
  else
    align = m->RowCount() * m->SizeInBytes();

  // STD140 rounds up to 16 byte alignment
  if (layout_ == Layout::kStd140)
    align += CalculatePad(align);

  return align;
}

uint32_t Format::CalcListBaseAlignmentInBytes(type::List* l) const {
  return l->SizeInBytes();
}

uint32_t Format::CalcTypeBaseAlignmentInBytes(type::Type* t) const {
  if (t->IsArray())
    return CalcArrayBaseAlignmentInBytes(t);
  if (t->IsVec())
    return CalcVecBaseAlignmentInBytes(t->AsNumber());
  if (t->IsMatrix())
    return CalcMatrixBaseAlignmentInBytes(t->AsNumber());
  if (t->IsNumber())
    return t->SizeInBytes();
  if (t->IsList())
    return CalcListBaseAlignmentInBytes(t->AsList());
  if (t->IsStruct()) {
    // Pad struct to 16 bytes in STD140
    uint32_t base = CalcStructBaseAlignmentInBytes(t->AsStruct());
    if (layout_ == Layout::kStd140)
      base += CalculatePad(base);
    return base;
  }

  assert(false && "Not reached");
  return 0;
}

uint32_t Format::AddSegmentsForType(type::Type* type) {
  if (type->IsList() && type->AsList()->IsPacked()) {
    auto l = type->AsList();
    if (AddSegment(Segment(FormatComponentType::kR, FormatMode::kUInt,
                           l->PackSizeInBits()))) {
      return l->SizeInBytes();
    }
    return 0;
  }

  // Remove packable from previous packing for types which can't pack back.
  if (type->IsStruct() || type->IsVec() || type->IsMatrix() ||
      type->IsArray()) {
    if (!segments_.empty() && segments_.back().IsPadding())
      segments_.back().SetPackable(false);
  }

  // TODO(dsinclair): How to handle matrix stride .... Stride comes from parent
  // member ....

  if (type->IsStruct()) {
    auto s = type->AsStruct();
    auto base_alignment_in_bytes = CalcStructBaseAlignmentInBytes(s);

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
        for (size_t i = 0; i < member.type->ArraySize(); ++i) {
          auto ary_seg_size = AddSegmentsForType(member.type);
          // Don't allow array members to pack together
          if (!segments_.empty() && segments_.back().IsPadding())
            segments_.back().SetPackable(false);

          if (member.HasArrayStride()) {
            uint32_t array_stride =
                static_cast<uint32_t>(member.array_stride_in_bytes);
            assert(ary_seg_size <= array_stride &&
                   "Array element larger than stride");

            seg_size += array_stride;
          } else {
            seg_size += ary_seg_size;
          }
        }
      } else {
        seg_size = AddSegmentsForType(member.type);
      }

      if (seg_size > 0 && seg_size < base_alignment_in_bytes) {
        AddPaddedSegmentPackable(base_alignment_in_bytes - seg_size);
        seg_size += base_alignment_in_bytes - seg_size;
      }

      cur_offset += seg_size;
    }
    if (s->HasStride()) {
      assert(cur_offset <= s->StrideInBytes() &&
             "Struct has more members then fit within stride");
      AddPaddedSegment(s->StrideInBytes() - cur_offset);
      cur_offset = s->StrideInBytes();
    } else if (layout_ == Layout::kStd140) {
      // Round struct up to 16 byte alignment in STD140.
      auto pad = CalculatePad(cur_offset);
      if (pad > 0) {
        AddPaddedSegment(pad);
        cur_offset += pad;
      }
    }
    return cur_offset;
  }

  // List members are only numbers and must not be vecs or matrices.
  if (type->IsList()) {
    uint32_t size = 0;
    auto l = type->AsList();
    for (uint32_t i = 0; i < type->ColumnCount(); ++i) {
      for (const auto& m : l->Members()) {
        if (AddSegment(Segment{m.name, m.mode, m.num_bits}))
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
      if (AddSegment(Segment{static_cast<FormatComponentType>(i),
                             n->GetFormatMode(), n->NumBits()})) {
        size += type->SizeInBytes();
      }
    }

    // In std140 a matrix (column count > 1) has each row stored like an array
    // which rounds up to a vec4.
    //
    // In std140 and std430 a vector of size 3N will round up to a vector of 4N.
    if (NeedsPadding(type)) {
      for (size_t k = 0; k < (4 - type->RowCount()); ++k) {
        AddPaddedSegmentPackable(type->SizeInBytes());
        size += type->SizeInBytes();
      }
    }

    // Make sure matrix rows don't accidentally pack together.
    if (type->IsMatrix() && segments_.back().IsPadding())
      segments_.back().SetPackable(false);
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
