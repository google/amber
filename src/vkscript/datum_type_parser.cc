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

#include "src/vkscript/datum_type_parser.h"

#include "src/make_unique.h"
#include "src/type_parser.h"

namespace amber {
namespace vkscript {

DatumTypeParser::DatumTypeParser() = default;

DatumTypeParser::~DatumTypeParser() = default;

std::unique_ptr<type::Type> DatumTypeParser::Parse(const std::string& data) {
  TypeParser tp;
  if (data == "int")
    return tp.Parse("R32_SINT");
  if (data == "uint")
    return tp.Parse("R32_UINT");
  if (data == "int8_t")
    return tp.Parse("R8_SINT");
  if (data == "uint8_t")
    return tp.Parse("R8_UINT");
  if (data == "int16_t")
    return tp.Parse("R16_SINT");
  if (data == "uint16_t")
    return tp.Parse("R16_UINT");
  if (data == "int64_t")
    return tp.Parse("R64_SINT");
  if (data == "uint64_t")
    return tp.Parse("R64_UINT");
  if (data == "float")
    return tp.Parse("R32_SFLOAT");
  if (data == "double")
    return tp.Parse("R64_SFLOAT");

  int row_count = 4;
  int column_count = 1;
  FormatMode mode = FormatMode::kSFloat;
  uint8_t num_bits = 32;
  size_t vec_pos = data.find("vec");
  if (vec_pos != std::string::npos) {
    if (data[0] == 'i') {
      mode = FormatMode::kSInt;
    } else if (data[0] == 'u') {
      mode = FormatMode::kUInt;
    } else if (data[0] == 'd') {
      num_bits = 64;
    }

    if (data[1] == '8')
      num_bits = 8;
    else if (data[1] == '1' && data[2] == '6')
      num_bits = 16;
    else if (data[1] == '6' && data[2] == '4')
      num_bits = 64;

    if ((vec_pos + 3) < data.length())
      row_count = data[vec_pos + 3] - '0';
  } else {
    size_t mat_pos = data.find("mat");
    if (mat_pos == std::string::npos)
      return nullptr;

    if (data[0] == 'd')
      num_bits = 64;

    if (mat_pos + 3 < data.length())
      column_count = data[mat_pos + 3] - '0';

    if (mat_pos + 5 < data.length())
      row_count = data[mat_pos + 5] - '0';
    else
      row_count = column_count;
  }

  std::unique_ptr<type::Type> type;
  if (mode == FormatMode::kSFloat)
    type = type::Number::Float(num_bits);
  else if (mode == FormatMode::kSInt)
    type = type::Number::Int(num_bits);
  else
    type = type::Number::Uint(num_bits);

  type->SetRowCount(static_cast<uint32_t>(row_count));
  type->SetColumnCount(static_cast<uint32_t>(column_count));
  return type;
}

}  // namespace vkscript
}  // namespace amber
