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

#include "src/type_parser.h"

#include <cassert>
#include <cstdlib>
#include <string>

#include "src/make_unique.h"

namespace amber {

TypeParser::TypeParser() = default;

TypeParser::~TypeParser() = default;

std::unique_ptr<type::Type> TypeParser::Parse(const std::string& data) {
  if (data.empty())
    return nullptr;

  // See if this is a custom glsl string format.
  if (data.find('/', 0) != std::string::npos)
    return ParseGlslFormat(data);

  // Walk the string backwards. This means we'll know if it's pack and we'll
  // know the mode before we get to a given named component.
  size_t cur_pos = std::string::npos;
  for (;;) {
    size_t next_pos = data.rfind('_', cur_pos);
    if (next_pos == std::string::npos) {
      if (cur_pos != std::string::npos)
        ProcessChunk(data.substr(0, cur_pos + 1));
      break;
    }

    ProcessChunk(data.substr(next_pos + 1, cur_pos - next_pos));
    cur_pos = next_pos - 1;
  }

  if (pieces_.empty())
    return nullptr;

  std::unique_ptr<type::Type> type = nullptr;
  if (pack_size_ == 0 && pieces_.size() == 1 &&
      pieces_[0].type == FormatComponentType::kR) {
    type = MakeUnique<type::Number>(pieces_[0].mode, pieces_[0].num_bits);
  } else {
    type = MakeUnique<type::List>();
    type->SetRowCount(static_cast<uint32_t>(pieces_.size()));
    type->AsList()->SetPackSizeInBits(pack_size_);

    for (const auto& piece : pieces_)
      type->AsList()->AddMember(piece.type, piece.mode, piece.num_bits);
  }

  pack_size_ = 0;
  mode_ = FormatMode::kSInt;
  pieces_.clear();

  return type;
}

void TypeParser::AddPiece(FormatComponentType type,
                          FormatMode mode,
                          uint8_t bits) {
  pieces_.insert(pieces_.begin(), Pieces{type, mode, bits});
}

void TypeParser::ProcessChunk(const std::string& data) {
  assert(data.size() > 0);

  if (data[0] == 'P') {
    if (data == "PACK8")
      pack_size_ = 8;
    else if (data == "PACK16")
      pack_size_ = 16;
    else if (data == "PACK32")
      pack_size_ = 32;
    else
      assert(false);

    return;
  }

  if (data[0] == 'U') {
    if (data == "UINT")
      mode_ = FormatMode::kUInt;
    else if (data == "UNORM")
      mode_ = FormatMode::kUNorm;
    else if (data == "UFLOAT")
      mode_ = FormatMode::kUFloat;
    else if (data == "USCALED")
      mode_ = FormatMode::kUScaled;
    else
      assert(false);

    return;
  }

  if (data[0] == 'S') {
    if (data == "SINT")
      mode_ = FormatMode::kSInt;
    else if (data == "SNORM")
      mode_ = FormatMode::kSNorm;
    else if (data == "SSCALED")
      mode_ = FormatMode::kSScaled;
    else if (data == "SFLOAT")
      mode_ = FormatMode::kSFloat;
    else if (data == "SRGB")
      mode_ = FormatMode::kSRGB;
    else if (data == "S8")
      AddPiece(FormatComponentType::kS, mode_, 8);
    else
      assert(false);

    return;
  }

  int32_t cur_pos = static_cast<int32_t>(data.size()) - 1;
  for (;;) {
    FormatComponentType type = FormatComponentType::kA;
    while (cur_pos >= 0) {
      if (data[static_cast<size_t>(cur_pos)] == 'X') {
        type = FormatComponentType::kX;
        break;
      } else if (data[static_cast<size_t>(cur_pos)] == 'D') {
        type = FormatComponentType::kD;
        break;
      } else if (data[static_cast<size_t>(cur_pos)] == 'R') {
        type = FormatComponentType::kR;
        break;
      } else if (data[static_cast<size_t>(cur_pos)] == 'G') {
        type = FormatComponentType::kG;
        break;
      } else if (data[static_cast<size_t>(cur_pos)] == 'B') {
        type = FormatComponentType::kB;
        break;
      } else if (data[static_cast<size_t>(cur_pos)] == 'A') {
        type = FormatComponentType::kA;
        break;
      }
      --cur_pos;
    }
    assert(cur_pos >= 0);

    char* next_str;
    const char* str = data.c_str() + cur_pos + 1;

    uint64_t val = static_cast<uint64_t>(std::strtol(str, &next_str, 10));
    if (val > 0)
      AddPiece(type, mode_, static_cast<uint8_t>(val));

    if (cur_pos == 0)
      break;

    --cur_pos;
  }
}

// static
FormatType TypeParser::NameToFormatType(const std::string& data) {
  if (data == "A1R5G5B5_UNORM_PACK16")
    return FormatType::kA1R5G5B5_UNORM_PACK16;
  if (data == "A2B10G10R10_SINT_PACK32")
    return FormatType::kA2B10G10R10_SINT_PACK32;
  if (data == "A2B10G10R10_SNORM_PACK32")
    return FormatType::kA2B10G10R10_SNORM_PACK32;
  if (data == "A2B10G10R10_SSCALED_PACK32")
    return FormatType::kA2B10G10R10_SSCALED_PACK32;
  if (data == "A2B10G10R10_UINT_PACK32")
    return FormatType::kA2B10G10R10_UINT_PACK32;
  if (data == "A2B10G10R10_UNORM_PACK32")
    return FormatType::kA2B10G10R10_UNORM_PACK32;
  if (data == "A2B10G10R10_USCALED_PACK32")
    return FormatType::kA2B10G10R10_USCALED_PACK32;
  if (data == "A2R10G10B10_SINT_PACK32")
    return FormatType::kA2R10G10B10_SINT_PACK32;
  if (data == "A2R10G10B10_SNORM_PACK32")
    return FormatType::kA2R10G10B10_SNORM_PACK32;
  if (data == "A2R10G10B10_SSCALED_PACK32")
    return FormatType::kA2R10G10B10_SSCALED_PACK32;
  if (data == "A2R10G10B10_UINT_PACK32")
    return FormatType::kA2R10G10B10_UINT_PACK32;
  if (data == "A2R10G10B10_UNORM_PACK32")
    return FormatType::kA2R10G10B10_UNORM_PACK32;
  if (data == "A2R10G10B10_USCALED_PACK32")
    return FormatType::kA2R10G10B10_USCALED_PACK32;
  if (data == "A8B8G8R8_SINT_PACK32")
    return FormatType::kA8B8G8R8_SINT_PACK32;
  if (data == "A8B8G8R8_SNORM_PACK32")
    return FormatType::kA8B8G8R8_SNORM_PACK32;
  if (data == "A8B8G8R8_SRGB_PACK32")
    return FormatType::kA8B8G8R8_SRGB_PACK32;
  if (data == "A8B8G8R8_SSCALED_PACK32")
    return FormatType::kA8B8G8R8_SSCALED_PACK32;
  if (data == "A8B8G8R8_UINT_PACK32")
    return FormatType::kA8B8G8R8_UINT_PACK32;
  if (data == "A8B8G8R8_UNORM_PACK32")
    return FormatType::kA8B8G8R8_UNORM_PACK32;
  if (data == "A8B8G8R8_USCALED_PACK32")
    return FormatType::kA8B8G8R8_USCALED_PACK32;
  if (data == "B10G11R11_UFLOAT_PACK32")
    return FormatType::kB10G11R11_UFLOAT_PACK32;
  if (data == "B4G4R4A4_UNORM_PACK16")
    return FormatType::kB4G4R4A4_UNORM_PACK16;
  if (data == "B5G5R5A1_UNORM_PACK16")
    return FormatType::kB5G5R5A1_UNORM_PACK16;
  if (data == "B5G6R5_UNORM_PACK16")
    return FormatType::kB5G6R5_UNORM_PACK16;
  if (data == "B8G8R8A8_SINT")
    return FormatType::kB8G8R8A8_SINT;
  if (data == "B8G8R8A8_SNORM")
    return FormatType::kB8G8R8A8_SNORM;
  if (data == "B8G8R8A8_SRGB")
    return FormatType::kB8G8R8A8_SRGB;
  if (data == "B8G8R8A8_SSCALED")
    return FormatType::kB8G8R8A8_SSCALED;
  if (data == "B8G8R8A8_UINT")
    return FormatType::kB8G8R8A8_UINT;
  if (data == "B8G8R8A8_UNORM")
    return FormatType::kB8G8R8A8_UNORM;
  if (data == "B8G8R8A8_USCALED")
    return FormatType::kB8G8R8A8_USCALED;
  if (data == "B8G8R8_SINT")
    return FormatType::kB8G8R8_SINT;
  if (data == "B8G8R8_SNORM")
    return FormatType::kB8G8R8_SNORM;
  if (data == "B8G8R8_SRGB")
    return FormatType::kB8G8R8_SRGB;
  if (data == "B8G8R8_SSCALED")
    return FormatType::kB8G8R8_SSCALED;
  if (data == "B8G8R8_UINT")
    return FormatType::kB8G8R8_UINT;
  if (data == "B8G8R8_UNORM")
    return FormatType::kB8G8R8_UNORM;
  if (data == "B8G8R8_USCALED")
    return FormatType::kB8G8R8_USCALED;
  if (data == "D16_UNORM")
    return FormatType::kD16_UNORM;
  if (data == "D16_UNORM_S8_UINT")
    return FormatType::kD16_UNORM_S8_UINT;
  if (data == "D24_UNORM_S8_UINT")
    return FormatType::kD24_UNORM_S8_UINT;
  if (data == "D32_SFLOAT")
    return FormatType::kD32_SFLOAT;
  if (data == "D32_SFLOAT_S8_UINT")
    return FormatType::kD32_SFLOAT_S8_UINT;
  if (data == "R16G16B16A16_SFLOAT")
    return FormatType::kR16G16B16A16_SFLOAT;
  if (data == "R16G16B16A16_SINT")
    return FormatType::kR16G16B16A16_SINT;
  if (data == "R16G16B16A16_SNORM")
    return FormatType::kR16G16B16A16_SNORM;
  if (data == "R16G16B16A16_SSCALED")
    return FormatType::kR16G16B16A16_SSCALED;
  if (data == "R16G16B16A16_UINT")
    return FormatType::kR16G16B16A16_UINT;
  if (data == "R16G16B16A16_UNORM")
    return FormatType::kR16G16B16A16_UNORM;
  if (data == "R16G16B16A16_USCALED")
    return FormatType::kR16G16B16A16_USCALED;
  if (data == "R16G16B16_SFLOAT")
    return FormatType::kR16G16B16_SFLOAT;
  if (data == "R16G16B16_SINT")
    return FormatType::kR16G16B16_SINT;
  if (data == "R16G16B16_SNORM")
    return FormatType::kR16G16B16_SNORM;
  if (data == "R16G16B16_SSCALED")
    return FormatType::kR16G16B16_SSCALED;
  if (data == "R16G16B16_UINT")
    return FormatType::kR16G16B16_UINT;
  if (data == "R16G16B16_UNORM")
    return FormatType::kR16G16B16_UNORM;
  if (data == "R16G16B16_USCALED")
    return FormatType::kR16G16B16_USCALED;
  if (data == "R16G16_SFLOAT")
    return FormatType::kR16G16_SFLOAT;
  if (data == "R16G16_SINT")
    return FormatType::kR16G16_SINT;
  if (data == "R16G16_SNORM")
    return FormatType::kR16G16_SNORM;
  if (data == "R16G16_SSCALED")
    return FormatType::kR16G16_SSCALED;
  if (data == "R16G16_UINT")
    return FormatType::kR16G16_UINT;
  if (data == "R16G16_UNORM")
    return FormatType::kR16G16_UNORM;
  if (data == "R16G16_USCALED")
    return FormatType::kR16G16_USCALED;
  if (data == "R16_SFLOAT")
    return FormatType::kR16_SFLOAT;
  if (data == "R16_SINT")
    return FormatType::kR16_SINT;
  if (data == "R16_SNORM")
    return FormatType::kR16_SNORM;
  if (data == "R16_SSCALED")
    return FormatType::kR16_SSCALED;
  if (data == "R16_UINT")
    return FormatType::kR16_UINT;
  if (data == "R16_UNORM")
    return FormatType::kR16_UNORM;
  if (data == "R16_USCALED")
    return FormatType::kR16_USCALED;
  if (data == "R32G32B32A32_SFLOAT")
    return FormatType::kR32G32B32A32_SFLOAT;
  if (data == "R32G32B32A32_SINT")
    return FormatType::kR32G32B32A32_SINT;
  if (data == "R32G32B32A32_UINT")
    return FormatType::kR32G32B32A32_UINT;
  if (data == "R32G32B32_SFLOAT")
    return FormatType::kR32G32B32_SFLOAT;
  if (data == "R32G32B32_SINT")
    return FormatType::kR32G32B32_SINT;
  if (data == "R32G32B32_UINT")
    return FormatType::kR32G32B32_UINT;
  if (data == "R32G32_SFLOAT")
    return FormatType::kR32G32_SFLOAT;
  if (data == "R32G32_SINT")
    return FormatType::kR32G32_SINT;
  if (data == "R32G32_UINT")
    return FormatType::kR32G32_UINT;
  if (data == "R32_SFLOAT")
    return FormatType::kR32_SFLOAT;
  if (data == "R32_SINT")
    return FormatType::kR32_SINT;
  if (data == "R32_UINT")
    return FormatType::kR32_UINT;
  if (data == "R4G4B4A4_UNORM_PACK16")
    return FormatType::kR4G4B4A4_UNORM_PACK16;
  if (data == "R4G4_UNORM_PACK8")
    return FormatType::kR4G4_UNORM_PACK8;
  if (data == "R5G5B5A1_UNORM_PACK16")
    return FormatType::kR5G5B5A1_UNORM_PACK16;
  if (data == "R5G6B5_UNORM_PACK16")
    return FormatType::kR5G6B5_UNORM_PACK16;
  if (data == "R64G64B64A64_SFLOAT")
    return FormatType::kR64G64B64A64_SFLOAT;
  if (data == "R64G64B64A64_SINT")
    return FormatType::kR64G64B64A64_SINT;
  if (data == "R64G64B64A64_UINT")
    return FormatType::kR64G64B64A64_UINT;
  if (data == "R64G64B64_SFLOAT")
    return FormatType::kR64G64B64_SFLOAT;
  if (data == "R64G64B64_SINT")
    return FormatType::kR64G64B64_SINT;
  if (data == "R64G64B64_UINT")
    return FormatType::kR64G64B64_UINT;
  if (data == "R64G64_SFLOAT")
    return FormatType::kR64G64_SFLOAT;
  if (data == "R64G64_SINT")
    return FormatType::kR64G64_SINT;
  if (data == "R64G64_UINT")
    return FormatType::kR64G64_UINT;
  if (data == "R64_SFLOAT")
    return FormatType::kR64_SFLOAT;
  if (data == "R64_SINT")
    return FormatType::kR64_SINT;
  if (data == "R64_UINT")
    return FormatType::kR64_UINT;
  if (data == "R8G8B8A8_SINT")
    return FormatType::kR8G8B8A8_SINT;
  if (data == "R8G8B8A8_SNORM")
    return FormatType::kR8G8B8A8_SNORM;
  if (data == "R8G8B8A8_SRGB")
    return FormatType::kR8G8B8A8_SRGB;
  if (data == "R8G8B8A8_SSCALED")
    return FormatType::kR8G8B8A8_SSCALED;
  if (data == "R8G8B8A8_UINT")
    return FormatType::kR8G8B8A8_UINT;
  if (data == "R8G8B8A8_UNORM")
    return FormatType::kR8G8B8A8_UNORM;
  if (data == "R8G8B8A8_USCALED")
    return FormatType::kR8G8B8A8_USCALED;
  if (data == "R8G8B8_SINT")
    return FormatType::kR8G8B8_SINT;
  if (data == "R8G8B8_SNORM")
    return FormatType::kR8G8B8_SNORM;
  if (data == "R8G8B8_SRGB")
    return FormatType::kR8G8B8_SRGB;
  if (data == "R8G8B8_SSCALED")
    return FormatType::kR8G8B8_SSCALED;
  if (data == "R8G8B8_UINT")
    return FormatType::kR8G8B8_UINT;
  if (data == "R8G8B8_UNORM")
    return FormatType::kR8G8B8_UNORM;
  if (data == "R8G8B8_USCALED")
    return FormatType::kR8G8B8_USCALED;
  if (data == "R8G8_SINT")
    return FormatType::kR8G8_SINT;
  if (data == "R8G8_SNORM")
    return FormatType::kR8G8_SNORM;
  if (data == "R8G8_SRGB")
    return FormatType::kR8G8_SRGB;
  if (data == "R8G8_SSCALED")
    return FormatType::kR8G8_SSCALED;
  if (data == "R8G8_UINT")
    return FormatType::kR8G8_UINT;
  if (data == "R8G8_UNORM")
    return FormatType::kR8G8_UNORM;
  if (data == "R8G8_USCALED")
    return FormatType::kR8G8_USCALED;
  if (data == "R8_SINT")
    return FormatType::kR8_SINT;
  if (data == "R8_SNORM")
    return FormatType::kR8_SNORM;
  if (data == "R8_SRGB")
    return FormatType::kR8_SRGB;
  if (data == "R8_SSCALED")
    return FormatType::kR8_SSCALED;
  if (data == "R8_UINT")
    return FormatType::kR8_UINT;
  if (data == "R8_UNORM")
    return FormatType::kR8_UNORM;
  if (data == "R8_USCALED")
    return FormatType::kR8_USCALED;
  if (data == "S8_UINT")
    return FormatType::kS8_UINT;
  if (data == "X8_D24_UNORM_PACK32")
    return FormatType::kX8_D24_UNORM_PACK32;

  return FormatType::kUnknown;
}

std::unique_ptr<type::Type> TypeParser::ParseGlslFormat(
    const std::string& fmt) {
  size_t pos = fmt.find('/');
  std::string gl_type = fmt.substr(0, pos);
  std::string glsl_type = fmt.substr(pos + 1);

  uint8_t bits = 0;
  FormatMode mode = FormatMode::kUNorm;

  static const struct {
    const char* name;
    uint8_t bits;
    bool is_signed;
    bool is_int;
  } types[] = {
      {"byte", 8, true, true},     {"ubyte", 8, false, true},
      {"short", 16, true, true},   {"ushort", 16, false, true},
      {"int", 32, true, true},     {"uint", 32, false, true},
      {"half", 16, true, false},   {"float", 32, true, false},
      {"double", 64, true, false},
  };
  for (auto& type : types) {
    if (gl_type == std::string(type.name)) {
      if (type.is_int)
        mode = type.is_signed ? FormatMode::kSInt : FormatMode::kUInt;
      else
        mode = FormatMode::kSFloat;

      bits = type.bits;
    }
  }

  // Failed to find gl type.
  if (mode == FormatMode::kUNorm)
    return nullptr;

  if (fmt.length() < 4)
    return nullptr;

  int8_t num_components = 0;
  if (glsl_type == "float" || glsl_type == "double" || glsl_type == "int" ||
      glsl_type == "uint") {
    num_components = 1;
  } else if (glsl_type.substr(0, 3) == "vec") {
    num_components =
        static_cast<int8_t>(strtol(glsl_type.c_str() + 3, nullptr, 10));
    if (num_components < 2)
      return nullptr;
  } else if ((glsl_type[0] == 'd' || glsl_type[0] == 'i' ||
              glsl_type[0] == 'u') &&
             glsl_type.substr(1, 3) == "vec") {
    num_components =
        static_cast<int8_t>(strtol(glsl_type.c_str() + 4, nullptr, 10));
    if (num_components < 2)
      return nullptr;
  }
  if (num_components > 4)
    return nullptr;

  std::string new_name = "";
  static const char* prefix = "RGBA";
  for (int8_t i = 0; i < num_components; ++i)
    new_name += prefix[i] + std::to_string(bits);

  new_name += "_";
  if (mode == FormatMode::kSInt)
    new_name += "SINT";
  else if (mode == FormatMode::kUInt)
    new_name += "UINT";
  else if (mode == FormatMode::kSFloat)
    new_name += "SFLOAT";

  return Parse(new_name);
}

}  // namespace amber
