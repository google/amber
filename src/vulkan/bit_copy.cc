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

#include "src/vulkan/bit_copy.h"

#include <cassert>
#include <cstring>

namespace amber {
namespace vulkan {

// static
void BitCopy::ShiftBufferBits(uint8_t* buffer,
                              uint8_t length_bytes,
                              uint8_t shift_bits) {
  if (shift_bits == 0)
    return;

  assert(shift_bits < 8);

  uint8_t carry = 0;
  for (uint32_t i = 0; i < length_bytes; ++i) {
    uint8_t new_value = static_cast<uint8_t>(buffer[i] << shift_bits) | carry;
    carry = buffer[i] >> (8 - shift_bits);
    buffer[i] = new_value;
  }
}

// static
void BitCopy::CopyValueToBuffer(uint8_t* dst,
                                const Value& src,
                                uint8_t bit_offset,
                                uint8_t bits) {
  uint8_t data[9] = {};
  if (src.IsInteger()) {
    if (bits <= 8) {
      uint8_t data_uint8 = src.AsUint8();
      data[0] = data_uint8;
    } else if (bits <= 16) {
      uint16_t data_uint16 = src.AsUint16();
      std::memcpy(data, &data_uint16, sizeof(uint16_t));
    } else if (bits <= 32) {
      uint32_t data_uint32 = src.AsUint32();
      std::memcpy(data, &data_uint32, sizeof(uint32_t));
    } else if (bits <= 64) {
      uint64_t data_uint64 = src.AsUint64();
      std::memcpy(data, &data_uint64, sizeof(uint64_t));
    } else {
      assert(false && "Invalid int bits for CopyBits");
    }
  } else {
    if (bits == 64) {
      double data_double = src.AsDouble();
      std::memcpy(data, &data_double, sizeof(double));
    } else {
      float data_float = src.AsFloat();
      uint16_t hex_float = 0;
      switch (bits) {
        case 32:
          std::memcpy(data, &data_float, sizeof(float));
          break;
        case 16:
        case 11:
        case 10:
          hex_float = FloatToHexFloat(data_float, bits);
          std::memcpy(data, &hex_float, sizeof(uint16_t));
          break;
        default:
          assert(false && "Invalid float bits for CopyBits");
          break;
      }
    }
  }

  while (bit_offset > 7) {
    ++dst;
    bit_offset -= 8;
  }

  ShiftBufferBits(data, ((bit_offset + bits - 1) / 8) + 1, bit_offset);
  CopyBits(dst, data, bit_offset, bits);
}

// static
void BitCopy::CopyBits(uint8_t* dst,
                       const uint8_t* src,
                       uint8_t bit_offset,
                       uint8_t bits) {
  while (bit_offset + bits > 0) {
    uint8_t target_bits = bits;
    if (bit_offset + target_bits > 8)
      target_bits = 8 - bit_offset;

    uint8_t bit_mask =
        static_cast<uint8_t>(((1U << target_bits) - 1U) << bit_offset);
    *dst = (*src & bit_mask) | (*dst & ~bit_mask);

    bit_offset -= bit_offset;
    bits -= target_bits;
    ++dst;
    ++src;
  }
}

// static
uint16_t BitCopy::FloatExponent(const uint32_t hex_float) {
  uint32_t exponent = ((hex_float >> 23U) & ((1U << 8U) - 1U)) - 127U + 15U;
  const uint32_t half_exponent_mask = (1U << 5U) - 1U;
  assert((exponent & ~half_exponent_mask) == 0U);
  return static_cast<uint16_t>(exponent & half_exponent_mask);
}

// static
uint16_t BitCopy::FloatToHexFloat(float value, uint8_t bits) {
  switch (bits) {
    case 10:
      return FloatToHexFloat10(value);
    case 11:
      return FloatToHexFloat11(value);
    case 16:
      return FloatToHexFloat16(value);
  }

  assert(false && "Invalid bits");
  return 0;
}

// static
uint16_t BitCopy::FloatToHexFloat16(const float value) {
  uint32_t hex;
  memcpy(&hex, &value, sizeof(float));
  return static_cast<uint16_t>(FloatSign(hex) << 15U) |
         static_cast<uint16_t>(FloatExponent(hex) << 10U) | FloatMantissa(hex);
}

// static
uint16_t BitCopy::FloatToHexFloat11(const float value) {
  uint32_t hex;
  memcpy(&hex, &value, sizeof(float));
  assert(FloatSign(hex) == 0);
  return static_cast<uint16_t>(FloatExponent(hex) << 6U) |
         static_cast<uint16_t>(FloatMantissa(hex) >> 4U);
}

// static
uint16_t BitCopy::FloatToHexFloat10(const float value) {
  uint32_t hex;
  memcpy(&hex, &value, sizeof(float));
  assert(FloatSign(hex) == 0);
  return static_cast<uint16_t>(FloatExponent(hex) << 5U) |
         static_cast<uint16_t>(FloatMantissa(hex) >> 5U);
}

}  // namespace vulkan
}  // namespace amber
