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

#include "src/bit_copy.h"

#include <cassert>

namespace amber {

// static
void BitCopy::LeftShiftBufferBits(uint8_t* buffer,
                                  uint8_t length_bytes,
                                  uint8_t shift_bits) {
  if (shift_bits == 0)
    return;

  assert(shift_bits < 8);

  uint8_t carry = 0;
  for (uint32_t i = 0; i < length_bytes; ++i) {
    uint8_t new_value = (buffer[i] << shift_bits) | carry;
    carry = buffer[i] >> (8 - shift_bits);
    buffer[i] = new_value;
  }
}

// static
void BitCopy::CopyValueToBuffer(uint8_t* dst,
                                const Value& src,
                                uint8_t dst_bit_offset,
                                uint8_t bits) {
  uint8_t data[9] = {};
  if (src.IsInteger()) {
    if (bits <= 8)
      data[0] = src.AsUint8();
    else if (bits <= 16)
      *(reinterpret_cast<uint16_t*>(data)) = src.AsUint16();
    else if (bits <= 32)
      *(reinterpret_cast<uint32_t*>(data)) = src.AsUint32();
    else if (bits <= 64)
      *(reinterpret_cast<uint64_t*>(data)) = src.AsUint64();
    else
      assert(false && "Invalid int bits for CopyBits");
  } else {
    if (bits == 64) {
      *(reinterpret_cast<double*>(data)) = src.AsDouble();
    } else {
      switch (bits) {
        case 32:
          *(reinterpret_cast<float*>(data)) = src.AsFloat();
          break;
        case 16:
        case 11:
        case 10:
          *(reinterpret_cast<uint16_t*>(data)) =
              FloatToHexFloat(src.AsFloat(), bits);
          break;
        default:
          assert(false && "Invalid float bits for CopyBits");
          break;
      }
    }
  }

  while (dst_bit_offset > 7) {
    ++dst;
    dst_bit_offset -= 8;
  }

  LeftShiftBufferBits(data, ((dst_bit_offset + bits - 1) / 8) + 1,
                      dst_bit_offset);
  CopyBits(dst, data, dst_bit_offset, bits);
}

// static
void BitCopy::RightShiftBufferBits(uint8_t* buffer,
                                   uint8_t length_bytes,
                                   uint8_t shift_bits) {
  if (shift_bits == 0)
    return;

  assert(shift_bits < 8);

  uint8_t carry = 0;
  for (uint32_t i = 0; i < length_bytes; ++i) {
    uint8_t old_value = buffer[i];
    buffer[i] = (buffer[i] >> shift_bits) | (carry << (8 - shift_bits));
    carry = old_value & ((1 << shift_bits) - 1);
  }
}

// static
void BitCopy::CopyMemoryToBuffer(uint8_t* dst,
                                 const uint8_t* src,
                                 uint8_t src_bit_offset,
                                 uint8_t bits) {
  while (src_bit_offset > 7) {
    ++src;
    src_bit_offset -= 8;
  }

  const uint8_t size_in_bytes = src_bit_offset + bits / 8;
  assert(size_in_bytes <= 9);

  uint8_t data[9] = {};
  for (uint8_t i = 0; i < size_in_bytes; ++i)
    data[i] = src[i];

  RightShiftBufferBits(data, size_in_bytes, src_bit_offset);
  CopyBits(dst, data, 0, bits);
}

// static
float BitCopy::HexFloatToFloat(const uint8_t* value, uint8_t bits) {
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

// static
void BitCopy::CopyBits(uint8_t* dst,
                       const uint8_t* src,
                       uint8_t bit_offset,
                       uint8_t bits) {
  assert(bit_offset < 8);

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
  const uint32_t& hex = *(reinterpret_cast<const uint32_t*>(&value));
  return static_cast<uint16_t>(FloatSign(hex) << 15U) |
         static_cast<uint16_t>(FloatExponent(hex) << 10U) | FloatMantissa(hex);
}

// static
uint16_t BitCopy::FloatToHexFloat11(const float value) {
  const uint32_t& hex = *(reinterpret_cast<const uint32_t*>(&value));
  assert(FloatSign(hex) == 0);
  return static_cast<uint16_t>(FloatExponent(hex) << 6U) |
         static_cast<uint16_t>(FloatMantissa(hex) >> 4U);
}

// static
uint16_t BitCopy::FloatToHexFloat10(const float value) {
  const uint32_t& hex = *(reinterpret_cast<const uint32_t*>(&value));
  assert(FloatSign(hex) == 0);
  return static_cast<uint16_t>(FloatExponent(hex) << 5U) |
         static_cast<uint16_t>(FloatMantissa(hex) >> 5U);
}

// static
float BitCopy::HexFloat16ToFloat(const uint8_t* value) {
  uint32_t sign = (static_cast<uint32_t>(value[1]) & 0x80) << 16U;
  uint32_t exponent = (static_cast<uint32_t>(value[1]) & 0x7c) << 13U;
  uint32_t mantissa = (static_cast<uint32_t>(value[1]) & 0x3) << 8U |
                      static_cast<uint32_t>(value[0]);

  uint32_t hex = sign | exponent | mantissa;
  return *(reinterpret_cast<float*>(&hex));
}

// static
float BitCopy::HexFloat11ToFloat(const uint8_t* value) {
  uint32_t exponent = (static_cast<uint32_t>(value[1]) << 25U) |
                      ((static_cast<uint32_t>(value[0]) & 0xc0) << 17U);
  uint32_t mantissa = static_cast<uint32_t>(value[0]) & 0x3f;

  uint32_t hex = exponent | mantissa;
  return *(reinterpret_cast<float*>(&hex));
}

// static
float BitCopy::HexFloat10ToFloat(const uint8_t* value) {
  uint32_t exponent = (static_cast<uint32_t>(value[1]) << 26U) |
                      ((static_cast<uint32_t>(value[0]) & 0xe0) << 18U);
  uint32_t mantissa = static_cast<uint32_t>(value[0]) & 0x1f;

  uint32_t hex = exponent | mantissa;
  return *(reinterpret_cast<float*>(&hex));
}

}  // namespace amber
