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

namespace amber {
namespace vkscript {

DatumTypeParser::DatumTypeParser() = default;

DatumTypeParser::~DatumTypeParser() = default;

Result DatumTypeParser::Parse(const std::string& data) {
  // TODO(dsinclair): Might want to make this nicer in the future, but this
  // works and is easy for now.
  if (data == "int") {
    type_.SetType(DataType::kInt32);
  } else if (data == "uint") {
    type_.SetType(DataType::kUint32);
  } else if (data == "int8_t") {
    type_.SetType(DataType::kInt8);
  } else if (data == "uint8_t") {
    type_.SetType(DataType::kUint8);
  } else if (data == "int16_t") {
    type_.SetType(DataType::kInt16);
  } else if (data == "uint16_t") {
    type_.SetType(DataType::kUint16);
  } else if (data == "int64_t") {
    type_.SetType(DataType::kInt64);
  } else if (data == "uint64_t") {
    type_.SetType(DataType::kUint64);
  } else if (data == "float") {
    type_.SetType(DataType::kFloat);
  } else if (data == "double") {
    type_.SetType(DataType::kDouble);
  } else if (data == "vec2") {
    type_.SetType(DataType::kFloat);
    type_.SetRowCount(2);
  } else if (data == "vec3") {
    type_.SetType(DataType::kFloat);
    type_.SetRowCount(3);
  } else if (data == "vec4") {
    type_.SetType(DataType::kFloat);
    type_.SetRowCount(4);
  } else if (data == "dvec2") {
    type_.SetType(DataType::kDouble);
    type_.SetRowCount(2);
  } else if (data == "dvec3") {
    type_.SetType(DataType::kDouble);
    type_.SetRowCount(3);
  } else if (data == "dvec4") {
    type_.SetType(DataType::kDouble);
    type_.SetRowCount(4);
  } else if (data == "ivec2") {
    type_.SetType(DataType::kInt32);
    type_.SetRowCount(2);
  } else if (data == "ivec3") {
    type_.SetType(DataType::kInt32);
    type_.SetRowCount(3);
  } else if (data == "ivec4") {
    type_.SetType(DataType::kInt32);
    type_.SetRowCount(4);
  } else if (data == "uvec2") {
    type_.SetType(DataType::kUint32);
    type_.SetRowCount(2);
  } else if (data == "uvec3") {
    type_.SetType(DataType::kUint32);
    type_.SetRowCount(3);
  } else if (data == "uvec4") {
    type_.SetType(DataType::kUint32);
    type_.SetRowCount(4);
  } else if (data == "i8vec2") {
    type_.SetType(DataType::kInt8);
    type_.SetRowCount(2);
  } else if (data == "i8vec3") {
    type_.SetType(DataType::kInt8);
    type_.SetRowCount(3);
  } else if (data == "i8vec4") {
    type_.SetType(DataType::kInt8);
    type_.SetRowCount(4);
  } else if (data == "u8vec2") {
    type_.SetType(DataType::kUint8);
    type_.SetRowCount(2);
  } else if (data == "u8vec3") {
    type_.SetType(DataType::kUint8);
    type_.SetRowCount(3);
  } else if (data == "u8vec4") {
    type_.SetType(DataType::kUint8);
    type_.SetRowCount(4);
  } else if (data == "i16vec2") {
    type_.SetType(DataType::kInt16);
    type_.SetRowCount(2);
  } else if (data == "i16vec3") {
    type_.SetType(DataType::kInt16);
    type_.SetRowCount(3);
  } else if (data == "i16vec4") {
    type_.SetType(DataType::kInt16);
    type_.SetRowCount(4);
  } else if (data == "u16vec2") {
    type_.SetType(DataType::kUint16);
    type_.SetRowCount(2);
  } else if (data == "u16vec3") {
    type_.SetType(DataType::kUint16);
    type_.SetRowCount(3);
  } else if (data == "u16vec4") {
    type_.SetType(DataType::kUint16);
    type_.SetRowCount(4);
  } else if (data == "i64vec2") {
    type_.SetType(DataType::kInt64);
    type_.SetRowCount(2);
  } else if (data == "i64vec3") {
    type_.SetType(DataType::kInt64);
    type_.SetRowCount(3);
  } else if (data == "i64vec4") {
    type_.SetType(DataType::kInt64);
    type_.SetRowCount(4);
  } else if (data == "u64vec2") {
    type_.SetType(DataType::kUint64);
    type_.SetRowCount(2);
  } else if (data == "u64vec3") {
    type_.SetType(DataType::kUint64);
    type_.SetRowCount(3);
  } else if (data == "u64vec4") {
    type_.SetType(DataType::kUint64);
    type_.SetRowCount(4);
  } else if (data == "mat2") {
    type_.SetType(DataType::kFloat);
    type_.SetColumnCount(2);
    type_.SetRowCount(2);
  } else if (data == "mat2x2") {
    type_.SetType(DataType::kFloat);
    type_.SetColumnCount(2);
    type_.SetRowCount(2);
  } else if (data == "mat2x3") {
    type_.SetType(DataType::kFloat);
    type_.SetColumnCount(2);
    type_.SetRowCount(3);
  } else if (data == "mat2x4") {
    type_.SetType(DataType::kFloat);
    type_.SetColumnCount(2);
    type_.SetRowCount(4);
  } else if (data == "mat3") {
    type_.SetType(DataType::kFloat);
    type_.SetColumnCount(3);
    type_.SetRowCount(3);
  } else if (data == "mat3x2") {
    type_.SetType(DataType::kFloat);
    type_.SetColumnCount(3);
    type_.SetRowCount(2);
  } else if (data == "mat3x3") {
    type_.SetType(DataType::kFloat);
    type_.SetColumnCount(3);
    type_.SetRowCount(3);
  } else if (data == "mat3x4") {
    type_.SetType(DataType::kFloat);
    type_.SetColumnCount(3);
    type_.SetRowCount(4);
  } else if (data == "mat4") {
    type_.SetType(DataType::kFloat);
    type_.SetColumnCount(4);
    type_.SetRowCount(4);
  } else if (data == "mat4x2") {
    type_.SetType(DataType::kFloat);
    type_.SetColumnCount(4);
    type_.SetRowCount(2);
  } else if (data == "mat4x3") {
    type_.SetType(DataType::kFloat);
    type_.SetColumnCount(4);
    type_.SetRowCount(3);
  } else if (data == "mat4x4") {
    type_.SetType(DataType::kFloat);
    type_.SetColumnCount(4);
    type_.SetRowCount(4);
  } else if (data == "dmat2") {
    type_.SetType(DataType::kDouble);
    type_.SetColumnCount(2);
    type_.SetRowCount(2);
  } else if (data == "dmat2x2") {
    type_.SetType(DataType::kDouble);
    type_.SetColumnCount(2);
    type_.SetRowCount(2);
  } else if (data == "dmat2x3") {
    type_.SetType(DataType::kDouble);
    type_.SetColumnCount(2);
    type_.SetRowCount(3);
  } else if (data == "dmat2x4") {
    type_.SetType(DataType::kDouble);
    type_.SetColumnCount(2);
    type_.SetRowCount(4);
  } else if (data == "dmat3") {
    type_.SetType(DataType::kDouble);
    type_.SetColumnCount(3);
    type_.SetRowCount(3);
  } else if (data == "dmat3x2") {
    type_.SetType(DataType::kDouble);
    type_.SetColumnCount(3);
    type_.SetRowCount(2);
  } else if (data == "dmat3x3") {
    type_.SetType(DataType::kDouble);
    type_.SetColumnCount(3);
    type_.SetRowCount(3);
  } else if (data == "dmat3x4") {
    type_.SetType(DataType::kDouble);
    type_.SetColumnCount(3);
    type_.SetRowCount(4);
  } else if (data == "dmat4") {
    type_.SetType(DataType::kDouble);
    type_.SetColumnCount(4);
    type_.SetRowCount(4);
  } else if (data == "dmat4x2") {
    type_.SetType(DataType::kDouble);
    type_.SetColumnCount(4);
    type_.SetRowCount(2);
  } else if (data == "dmat4x3") {
    type_.SetType(DataType::kDouble);
    type_.SetColumnCount(4);
    type_.SetRowCount(3);
  } else if (data == "dmat4x4") {
    type_.SetType(DataType::kDouble);
    type_.SetColumnCount(4);
    type_.SetRowCount(4);
  } else {
    return Result("Invalid type provided: " + data);
  }
  return {};
}

}  // namespace vkscript
}  // namespace amber
