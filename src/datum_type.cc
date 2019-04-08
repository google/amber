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

#include "src/datum_type.h"

#include <string>

#include "src/format_parser.h"

namespace amber {

DatumType::DatumType() = default;

DatumType::DatumType(const DatumType&) = default;

DatumType::~DatumType() = default;

DatumType& DatumType::operator=(const DatumType&) = default;

uint32_t DatumType::ElementSizeInBytes() const {
  uint32_t s;
  if (type_ == DataType::kInt8 || type_ == DataType::kUint8)
    s = 1;
  else if (type_ == DataType::kInt16 || type_ == DataType::kUint16)
    s = 2;
  else if (type_ == DataType::kInt32 || type_ == DataType::kUint32)
    s = 4;
  else if (type_ == DataType::kInt64 || type_ == DataType::kUint64)
    s = 8;
  else if (type_ == DataType::kFloat)
    s = sizeof(float);
  else
    s = sizeof(double);

  return s;
}

uint32_t DatumType::SizeInBytes() const {
  uint32_t s = ElementSizeInBytes();
  uint32_t bytes = s * column_count_ * row_count_;

  // For a vector of size 3 in std140 we need to have alignment of 4N. For a
  // matrix, we update each row to 4N.
  if (is_std140_ && row_count_ == 3)
    bytes += (s * column_count_);

  return bytes;
}

std::unique_ptr<Format> DatumType::AsFormat() const {
  uint32_t bits_per_element = ElementSizeInBytes() * 8;
  static const char* prefixes = "RGBA";
  std::string name = "";
  for (size_t i = 0; i < row_count_; ++i)
    name += prefixes[i] + std::to_string(bits_per_element);

  name += "_";

  if (IsFloat() || IsDouble())
    name += "SFLOAT";
  else if (IsInt8() || IsInt16() || IsInt32() || IsInt64())
    name += "SINT";
  else
    name += "UINT";

  FormatParser fp;

  auto fmt = fp.Parse(name);
  // There is no format string equivalent to a matrix ...
  if (column_count_ > 1) {
    fmt->SetFormatType(FormatType::kUnknown);
    fmt->SetColumnCount(column_count_);
  }
  // Always pretend to be std140 as that's what datum type does.
  fmt->SetIsStd140();

  return fmt;
}

}  // namespace amber
