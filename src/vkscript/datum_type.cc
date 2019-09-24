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

#include "src/vkscript/datum_type.h"

#include <string>

#include "src/format_parser.h"

namespace amber {
namespace vkscript {
namespace {

uint32_t ElementSizeInBytes(DataType type) {
  if (type == DataType::kInt8 || type == DataType::kUint8)
    return 1;
  if (type == DataType::kInt16 || type == DataType::kUint16)
    return 2;
  if (type == DataType::kInt32 || type == DataType::kUint32)
    return 4;
  if (type == DataType::kInt64 || type == DataType::kUint64)
    return 8;
  if (type == DataType::kFloat)
    return sizeof(float);

  return sizeof(double);
}

}  // namespace

DatumType::DatumType() = default;

DatumType::DatumType(const DatumType&) = default;

DatumType::~DatumType() = default;

std::unique_ptr<Format> DatumType::AsFormat() const {
  uint32_t bits_per_element = ElementSizeInBytes(type_) * 8;
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

  return fmt;
}

}  // namespace vkscript
}  // namespace amber
