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

#include "src/buffer.h"

#include <cassert>

namespace amber {
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

// Convert 32 bits float |value| to 16 bits float based on IEEE-754.
uint16_t FloatToHexFloat16(const float value) {
  const uint32_t* hex = reinterpret_cast<const uint32_t*>(&value);
  return static_cast<uint16_t>(
      static_cast<uint16_t>(FloatSign(*hex) << 15U) |
      static_cast<uint16_t>(FloatExponent(*hex) << 10U) |
      static_cast<uint16_t>(FloatMantissa(*hex) >> 13U));
}

// Convert 32 bits float |value| to 11 bits float based on IEEE-754.
uint16_t FloatToHexFloat11(const float value) {
  const uint32_t* hex = reinterpret_cast<const uint32_t*>(&value);
  assert(FloatSign(*hex) == 0);
  return static_cast<uint16_t>(
      static_cast<uint16_t>(FloatExponent(*hex) << 6U) |
      static_cast<uint16_t>(FloatMantissa(*hex) >> 17U));
}

// Convert 32 bits float |value| to 10 bits float based on IEEE-754.
uint16_t FloatToHexFloat10(const float value) {
  const uint32_t* hex = reinterpret_cast<const uint32_t*>(&value);
  assert(FloatSign(*hex) == 0);
  return static_cast<uint16_t>(
      static_cast<uint16_t>(FloatExponent(*hex) << 5U) |
      static_cast<uint16_t>(FloatMantissa(*hex) >> 18U));
}

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
// 14 bits float is used only RGB9_E5 format in OpenGL but it does not exist
// in Vulkan.
//
// For example, 1234 in 32 bits float = 1.0011010010 * 2^10 with base 2.
//
// 1.0011010010 * 2^10 --> 0 (sign) / 10 + 127 (exp) / 0011010010 (Mantissa)
//                     --> 0x449a4000
uint16_t FloatToHexFloat(float value, uint8_t bits) {
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

Result ValueToUint64(const Value& src, uint8_t bits, uint64_t* out) {
  uint64_t data = 0;
  if (src.IsInteger()) {
    switch (bits) {
      case 8: {
        uint8_t* ptr = reinterpret_cast<uint8_t*>(&data);
        *ptr = src.AsUint8();
        break;
      }
      case 16: {
        uint16_t* ptr = reinterpret_cast<uint16_t*>(&data);
        *ptr = src.AsUint16();
        break;
      }
      case 32: {
        uint32_t* ptr = reinterpret_cast<uint32_t*>(&data);
        *ptr = src.AsUint32();
        break;
      }
      case 64: {
        uint64_t* ptr = reinterpret_cast<uint64_t*>(&data);
        *ptr = src.AsUint64();
        break;
      }
      default: {
        return Result("Vulkan: Invalid int bits for CopyBitsOfValueToBuffer");
      }
    }
  } else {
    if (bits == 64) {
      double* ptr = reinterpret_cast<double*>(&data);
      *ptr = src.AsDouble();
    } else {
      switch (bits) {
        case 32: {
          float* float_ptr = reinterpret_cast<float*>(&data);
          *float_ptr = src.AsFloat();
          break;
        }
        case 16:
        case 11:
        case 10: {
          uint16_t* uint16_ptr = reinterpret_cast<uint16_t*>(&data);
          *uint16_ptr =
              static_cast<uint16_t>(FloatToHexFloat(src.AsFloat(), bits));
          break;
        }
        default: {
          return Result(
              "Vulkan: Invalid float bits for CopyBitsOfValueToBuffer");
        }
      }
    }
  }
  *out = data;
  return {};
}

// Copy [0, bits) bits of |src| to
// [dst_bit_offset, dst_bit_offset + bits) of |dst|. If |bits| is
// less than 32 and the type is float, this method uses
// FloatToHexFloat() to convert it into small bits float.
Result CopyBitsOfValueToBuffer(uint8_t* dst,
                               const Value& src,
                               uint32_t dst_bit_offset,
                               uint8_t bits) {
  uint64_t data = 0;
  Result r = ValueToUint64(src, bits, &data);
  if (!r.IsSuccess())
    return r;

  // Shift memory pointer to the start of the byte to write into.
  while (dst_bit_offset > 7) {
    ++dst;
    dst_bit_offset -= 8;
  }

  // No overflow will happen. |dst_bit_offset| is based on VkFormat
  // and if |bits| is 64, |dst_bit_offset| must be 0. No component
  // has |bits| bigger than 64.
  data <<= dst_bit_offset;

  uint64_t* dst64 = reinterpret_cast<uint64_t*>(dst);
  uint64_t dst_lower_bits = *dst64 & ((1UL << dst_bit_offset) - 1UL);
  uint64_t dst_upper_bits =
      *dst64 & ~(((1ULL << (dst_bit_offset + bits)) - 1ULL));

  *dst64 = dst_lower_bits | data | dst_upper_bits;
  return {};
}

template <typename T>
T* ValuesAs(uint8_t* values) {
  return reinterpret_cast<T*>(values);
}

}  // namespace

Buffer::Buffer() = default;

Buffer::Buffer(BufferType type) : buffer_type_(type) {}

Buffer::~Buffer() = default;

DataBuffer* Buffer::AsDataBuffer() {
  return static_cast<DataBuffer*>(this);
}

FormatBuffer* Buffer::AsFormatBuffer() {
  return static_cast<FormatBuffer*>(this);
}

Result Buffer::CopyTo(Buffer* buffer) const {
  if (buffer->width_ != width_)
    return Result("Buffer::CopyBaseFields() buffers have a different width");
  if (buffer->height_ != height_)
    return Result("Buffer::CopyBaseFields() buffers have a different height");
  if (buffer->size_ != size_)
    return Result("Buffer::CopyBaseFields() buffers have a different size");
  buffer->values_.clear();
  buffer->values_ = values_;
  return {};
}

DataBuffer::DataBuffer() = default;

DataBuffer::DataBuffer(BufferType type) : Buffer(type) {}

DataBuffer::~DataBuffer() = default;

Result DataBuffer::SetData(std::vector<Value>&& data) {
  uint32_t size = static_cast<uint32_t>(data.size()) /
                  datum_type_.ColumnCount() / datum_type_.RowCount();
  SetSize(size);
  values_.resize(GetSizeInBytes());
  return CopyData(data);
}

Result DataBuffer::CopyData(const std::vector<Value>& data) {
  uint8_t* values = values_.data();

  for (const auto& val : data) {
    if (datum_type_.IsInt8()) {
      *(ValuesAs<int8_t>(values)) = val.AsInt8();
      values += sizeof(int8_t);
    } else if (datum_type_.IsInt16()) {
      *(ValuesAs<int16_t>(values)) = val.AsInt16();
      values += sizeof(int16_t);
    } else if (datum_type_.IsInt32()) {
      *(ValuesAs<int32_t>(values)) = val.AsInt32();
      values += sizeof(int32_t);
    } else if (datum_type_.IsInt64()) {
      *(ValuesAs<int64_t>(values)) = val.AsInt64();
      values += sizeof(int64_t);
    } else if (datum_type_.IsUint8()) {
      *(ValuesAs<uint8_t>(values)) = val.AsUint8();
      values += sizeof(uint8_t);
    } else if (datum_type_.IsUint16()) {
      *(ValuesAs<uint16_t>(values)) = val.AsUint16();
      values += sizeof(uint16_t);
    } else if (datum_type_.IsUint32()) {
      *(ValuesAs<uint32_t>(values)) = val.AsUint32();
      values += sizeof(uint32_t);
    } else if (datum_type_.IsUint64()) {
      *(ValuesAs<uint64_t>(values)) = val.AsUint64();
      values += sizeof(uint64_t);
    } else if (datum_type_.IsFloat()) {
      *(ValuesAs<float>(values)) = val.AsFloat();
      values += sizeof(float);
    } else if (datum_type_.IsDouble()) {
      *(ValuesAs<double>(values)) = val.AsDouble();
      values += sizeof(double);
    }
  }
  return {};
}

FormatBuffer::FormatBuffer() = default;

FormatBuffer::FormatBuffer(BufferType type) : Buffer(type) {}

FormatBuffer::~FormatBuffer() = default;

Result FormatBuffer::SetData(std::vector<Value>&& data) {
  SetSize(static_cast<uint32_t>(data.size()));
  values_.resize(GetSizeInBytes());
  return CopyData(data);
}

Result FormatBuffer::CopyData(const std::vector<Value>& data) {
  uint8_t* ptr = values_.data();

  for (uint32_t i = 0; i < data.size();) {
    const auto pack_size = format_->GetPackSize();
    if (pack_size) {
      Result r = CopyBitsOfValueToBuffer(ptr, data[i], 0, pack_size);
      if (!r.IsSuccess())
        return r;

      ptr += pack_size / 8;
      ++i;
      continue;
    }

    const auto& components = format_->GetComponents();
    uint32_t bit_offset = 0;

    for (uint32_t k = 0; k < components.size(); ++k) {
      uint8_t bits = components[k].num_bits;
      Result r = CopyBitsOfValueToBuffer(ptr, data[i + k], bit_offset, bits);
      if (!r.IsSuccess())
        return r;

      bit_offset += bits;
    }

    i += static_cast<uint32_t>(components.size());
    ptr += format_->GetByteSize();
  }
  return {};
}

}  // namespace amber
