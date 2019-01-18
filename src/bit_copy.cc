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
Result BitCopy::CopyValueToBuffer(uint8_t* dst,
                                  const Value& src,
                                  uint8_t dst_bit_offset,
                                  uint8_t bits) {
  uint64_t data = 0;
  if (src.IsInteger()) {
    if (bits <= 8) {
      uint8_t* ptr = reinterpret_cast<uint8_t*>(&data);
      *ptr = src.AsUint8();
    } else if (bits <= 16) {
      uint16_t* ptr = reinterpret_cast<uint16_t*>(&data);
      *ptr = src.AsUint16();
    } else if (bits <= 32) {
      uint32_t* ptr = reinterpret_cast<uint32_t*>(&data);
      *ptr = src.AsUint32();
    } else if (bits <= 64) {
      data = src.AsUint64();
    } else {
      return Result("Invalid int bits for CopyValueToBuffer");
    }
  } else {
    if (bits == 64) {
      double* ptr = reinterpret_cast<double*>(&data);
      *ptr = src.AsDouble();
    } else {
      float* float_ptr = nullptr;
      uint16_t* uint16_ptr = nullptr;
      switch (bits) {
        case 32:
          float_ptr = reinterpret_cast<float*>(&data);
          *float_ptr = src.AsFloat();
          break;
        case 16:
        case 11:
        case 10:
          uint16_ptr = reinterpret_cast<uint16_t*>(&data);
          *uint16_ptr = FloatToHexFloat(src.AsFloat(), bits);
          break;
        default:
          return Result("Invalid float bits for CopyValueToBuffer");
      }
    }
  }

  while (dst_bit_offset > 7) {
    ++dst;
    dst_bit_offset -= 8;
  }

  // No overflow will happen. |dst_bit_offset| is based on VkFormat
  // and if |bits| is 64, |dst_bit_offset| must be 0. No component
  // has |bits| bigger than 64.
  data <<= dst_bit_offset;

  CopyBits(dst, data, dst_bit_offset, bits);
  return {};
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

  uint64_t data = 0;
  for (uint8_t i = 0; i < size_in_bytes; ++i) {
    uint8_t* ptr = reinterpret_cast<uint8_t*>(&data);
    ptr[i] = src[i];
  }

  data >>= src_bit_offset;

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
                       const uint64_t& src,
                       uint8_t bit_offset,
                       uint8_t bits) {
  assert(bit_offset < 8);

  const uint8_t* ptr = reinterpret_cast<const uint8_t*>(&src);
  while (bit_offset + bits > 0) {
    uint8_t target_bits = bits;
    if (bit_offset + target_bits > 8)
      target_bits = 8 - bit_offset;

    uint8_t bit_mask =
        static_cast<uint8_t>(((1U << target_bits) - 1U) << bit_offset);
    *dst = (*ptr & bit_mask) | (*dst & ~bit_mask);

    bit_offset -= bit_offset;
    bits -= target_bits;
    ++dst;
    ++ptr;
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
  const uint32_t* hex = reinterpret_cast<const uint32_t*>(&value);
  return static_cast<uint16_t>(FloatSign(*hex) << 15U) |
         static_cast<uint16_t>(FloatExponent(*hex) << 10U) |
         FloatMantissa(*hex);
}

// static
uint16_t BitCopy::FloatToHexFloat11(const float value) {
  const uint32_t* hex = reinterpret_cast<const uint32_t*>(&value);
  assert(FloatSign(*hex) == 0);
  return static_cast<uint16_t>(FloatExponent(*hex) << 6U) |
         static_cast<uint16_t>(FloatMantissa(*hex) >> 4U);
}

// static
uint16_t BitCopy::FloatToHexFloat10(const float value) {
  const uint32_t* hex = reinterpret_cast<const uint32_t*>(&value);
  assert(FloatSign(*hex) == 0);
  return static_cast<uint16_t>(FloatExponent(*hex) << 5U) |
         static_cast<uint16_t>(FloatMantissa(*hex) >> 5U);
}

// static
float BitCopy::HexFloat16ToFloat(const uint8_t* value) {
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

// static
float BitCopy::HexFloat11ToFloat(const uint8_t* value) {
  uint32_t exponent = (((static_cast<uint32_t>(value[1]) << 2U) |
                        ((static_cast<uint32_t>(value[0]) & 0xc0) >> 6U)) +
                       112U)
                      << 23U;
  uint32_t mantissa = (static_cast<uint32_t>(value[0]) & 0x3f) << 17U;

  uint32_t hex = exponent | mantissa;
  float* hex_float = reinterpret_cast<float*>(&hex);
  return *hex_float;
}

// static
float BitCopy::HexFloat10ToFloat(const uint8_t* value) {
  uint32_t exponent = (((static_cast<uint32_t>(value[1]) << 3U) |
                        ((static_cast<uint32_t>(value[0]) & 0xe0) >> 5U)) +
                       112U)
                      << 23U;
  uint32_t mantissa = (static_cast<uint32_t>(value[0]) & 0x1f) << 18U;

  uint32_t hex = exponent | mantissa;
  float* hex_float = reinterpret_cast<float*>(&hex);
  return *hex_float;
}

}  // namespace amber
