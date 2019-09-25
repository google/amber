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

#include "src/float16_helper.h"

#include <cassert>
#include <iostream>

// Float10
// | 9 8 7 6 5 | 4 3 2 1 0 |
// | exponent  | mantissa  |
//
// Float11
// | 10 9 8 7 6 | 5 4 3 2 1 0 |
// | exponent   |  mantissa   |
//
// Float16
// | 15 | 14 13 12 11 10 | 9 8 7 6 5 4 3 2 1 0 |
// | s  |     exponent   |  mantissa           |
//
// Float32
// | 31 | 30 ... 23 | 22 ... 0 |
// | s  |  exponent | mantissa |

namespace amber {
namespace float16 {
namespace {

// Return sign value of 32 bits float.
uint16_t FloatSign(const uint32_t hex_float) {
  return static_cast<uint16_t>(hex_float >> 31U);
}

// Return exponent value of 32 bits float.
uint16_t FloatExponent(const uint32_t hex_float) {
  uint32_t exponent = ((hex_float >> 23U) & ((1U << 8U) - 1U)) - 112U;
  const uint32_t half_exponent_mask = (1U << 5U) - 1U;
  assert(((exponent & ~half_exponent_mask) == 0U) && "Float exponent overflow");
  return static_cast<uint16_t>(exponent & half_exponent_mask);
}

// Return mantissa value of 32 bits float. Note that mantissa for 32
// bits float is 23 bits and this method must return uint32_t.
uint32_t FloatMantissa(const uint32_t hex_float) {
  return static_cast<uint32_t>(hex_float & ((1U << 23U) - 1U));
}

// Convert float |value| whose size is 16 bits to 32 bits float
// based on IEEE-754.
float HexFloat16ToFloat(const uint8_t* value) {
  uint32_t sign = (static_cast<uint32_t>(value[1]) & 0x80) << 24U;
  uint32_t exponent = (((static_cast<uint32_t>(value[1]) & 0x7c) >> 2U) + 112U)
                      << 23U;
  uint32_t mantissa = ((static_cast<uint32_t>(value[1]) & 0x3) << 8U |
                       static_cast<uint32_t>(value[0]))
                      << 13U;

  uint32_t hex = sign | exponent | mantissa;
  float* hex_float = reinterpret_cast<float*>(&hex);
  return *hex_float;
}

// Convert float |value| whose size is 11 bits to 32 bits float
// based on IEEE-754.
float HexFloat11ToFloat(const uint8_t* value) {
  uint32_t exponent = (((static_cast<uint32_t>(value[1]) << 2U) |
                        ((static_cast<uint32_t>(value[0]) & 0xc0) >> 6U)) +
                       112U)
                      << 23U;
  uint32_t mantissa = (static_cast<uint32_t>(value[0]) & 0x3f) << 17U;

  uint32_t hex = exponent | mantissa;
  float* hex_float = reinterpret_cast<float*>(&hex);
  return *hex_float;
}

// Convert float |value| whose size is 10 bits to 32 bits float
// based on IEEE-754.
float HexFloat10ToFloat(const uint8_t* value) {
  uint32_t exponent = (((static_cast<uint32_t>(value[1]) << 3U) |
                        ((static_cast<uint32_t>(value[0]) & 0xe0) >> 5U)) +
                       112U)
                      << 23U;
  uint32_t mantissa = (static_cast<uint32_t>(value[0]) & 0x1f) << 18U;

  uint32_t hex = exponent | mantissa;
  float* hex_float = reinterpret_cast<float*>(&hex);
  return *hex_float;
}

}  // namespace

float HexFloatToFloat(const uint8_t* value, uint8_t bits) {
  switch (bits) {
    case 10:
      return HexFloat10ToFloat(value);
    case 11:
      return HexFloat11ToFloat(value);
    case 16:
      return HexFloat16ToFloat(value);
  }

  assert(false && "Invalid bits");
  return 0;
}

uint16_t FloatToHexFloat16(const float value) {
  const uint32_t* hex = reinterpret_cast<const uint32_t*>(&value);
  return static_cast<uint16_t>(
      static_cast<uint16_t>(FloatSign(*hex) << 15U) |
      static_cast<uint16_t>(FloatExponent(*hex) << 10U) |
      static_cast<uint16_t>(FloatMantissa(*hex) >> 13U));
}

}  // namespace float16
}  // namespace amber
