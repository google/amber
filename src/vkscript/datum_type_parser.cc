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

#include "src/format_parser.h"
#include "src/make_unique.h"

namespace amber {
namespace vkscript {
namespace {

FormatComponentType FORMAT_TYPES[] = {
    FormatComponentType::kR, FormatComponentType::kG, FormatComponentType::kB,
    FormatComponentType::kA};

}  // namespace

DatumTypeParser::DatumTypeParser() = default;

DatumTypeParser::~DatumTypeParser() = default;

std::unique_ptr<Format> DatumTypeParser::Parse(const std::string& data) {
  auto fmt = MakeUnique<Format>();

  bool matrix = false;
  if (data == "int") {
    fmt->AddComponent(FormatComponentType::kR, FormatMode::kSInt, 32);
  } else if (data == "uint") {
    fmt->AddComponent(FormatComponentType::kR, FormatMode::kUInt, 32);
  } else if (data == "int8_t") {
    fmt->AddComponent(FormatComponentType::kR, FormatMode::kSInt, 8);
  } else if (data == "uint8_t") {
    fmt->AddComponent(FormatComponentType::kR, FormatMode::kUInt, 8);
  } else if (data == "int16_t") {
    fmt->AddComponent(FormatComponentType::kR, FormatMode::kSInt, 16);
  } else if (data == "uint16_t") {
    fmt->AddComponent(FormatComponentType::kR, FormatMode::kUInt, 16);
  } else if (data == "int64_t") {
    fmt->AddComponent(FormatComponentType::kR, FormatMode::kSInt, 64);
  } else if (data == "uint64_t") {
    fmt->AddComponent(FormatComponentType::kR, FormatMode::kUInt, 64);
  } else if (data == "float") {
    fmt->AddComponent(FormatComponentType::kR, FormatMode::kSFloat, 32);
  } else if (data == "double") {
    fmt->AddComponent(FormatComponentType::kR, FormatMode::kSFloat, 64);
  } else {
    size_t row_count = 4;
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

      matrix = true;

      if (data[0] == 'd')
        num_bits = 64;

      size_t column_count = 1;
      if (mat_pos + 3 < data.length())
        column_count = data[mat_pos + 3] - '0';

      if (mat_pos + 5 < data.length())
        row_count = data[mat_pos + 5] - '0';
      else
        row_count = column_count;

      fmt->SetColumnCount(column_count);
    }

    for (size_t i = 0; i < row_count; ++i)
      fmt->AddComponent(FORMAT_TYPES[i], mode, num_bits);
  }

  // Convert the name back into a FormatType so we can use it in the buffer
  // later Otherwise, we end up with a type of Unknown.
  //
  // There is no equivalent type for a matrix.
  if (!matrix) {
    std::string name = "";
    std::string parts = "ARGB";
    const auto& comps = fmt->GetComponents();
    for (const auto& comp : comps) {
      name += parts[static_cast<uint8_t>(comp.type)] +
              std::to_string(comp.num_bits);
    }
    name += "_";
    switch (comps[0].mode) {
      case FormatMode::kUNorm:
      case FormatMode::kUFloat:
      case FormatMode::kUScaled:
      case FormatMode::kSNorm:
      case FormatMode::kSScaled:
      case FormatMode::kSRGB:
        return nullptr;
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

    fmt->SetFormatType(FormatParser::NameToType(name));
  }
  return fmt;
}

}  // namespace vkscript
}  // namespace amber
