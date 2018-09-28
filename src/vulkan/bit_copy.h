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

#ifndef SRC_VULKAN_BIT_COPY_H_
#define SRC_VULKAN_BIT_COPY_H_

#include "src/value.h"

namespace amber {
namespace vulkan {

class BitCopy {
 public:
  // Copy [0, bits) bits of src to [bit_offset, bit_offset + bits) of dst.
  static void CopyValueToBuffer(uint8_t* dst,
                                const Value& src,
                                uint8_t bit_offset,
                                uint8_t bits);

 private:
  BitCopy() = delete;

  ~BitCopy() = delete;

  static void ShiftBufferBits(uint8_t* buffer,
                              uint8_t length_bytes,
                              uint8_t shift_bits);

  static void CopyBits(uint8_t* dst,
                       const uint8_t* src,
                       uint8_t bit_offset,
                       uint8_t bits);

  // Convert float to small float format.
  // See https://www.khronos.org/opengl/wiki/Small_Float_Formats
  // and https://en.wikipedia.org/wiki/IEEE_754.
  //
  //    Sign Exponent Mantissa Exponent-Bias
  // 16    1        5       10            15
  // 11    0        5        6            15
  // 10    0        5        5            15
  // 32    1        8       23           127
  // 64    1       11       52          1023
  //
  // 11 and 10 bits floats are always positive.
  // 14 bits float is used only RGB9_E5 format but it does not exist in Vulkan.
  //
  // For example, 1234 in 32 bits float = 1.0011010010 * 2^10 with base 2.
  //
  // 1.0011010010 * 2^10 --> 0 (sign) / 10 + 127 (exp) / 0011010010 (Mantissa)
  //                     --> 0x449a4000
  static uint16_t FloatToHexFloat(float value, uint8_t bits);
  static uint16_t FloatToHexFloat16(const float value);
  static uint16_t FloatToHexFloat11(const float value);
  static uint16_t FloatToHexFloat10(const float value);

  static uint16_t FloatSign(const uint32_t hex_float) {
    return static_cast<uint16_t>(hex_float >> 31U);
  }

  static uint16_t FloatExponent(const uint32_t hex_float);

  static uint16_t FloatMantissa(const uint32_t hex_float) {
    return static_cast<uint16_t>((hex_float & ((1U << 23U) - 1U)) >> 13U);
  }
};

}  // namespace vulkan
}  // namespace amber

#endif  // SRC_VULKAN_BIT_COPY_H_
