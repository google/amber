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
  if (buffer->element_count_ != element_count_)
    return Result("Buffer::CopyBaseFields() buffers have a different size");
  buffer->values_ = values_;
  return {};
}

Result Buffer::IsEqual(Buffer* buffer) const {
  if (buffer->buffer_type_ != buffer_type_)
    return Result{"Buffers have a different type"};
  if (buffer->element_count_ != element_count_)
    return Result{"Buffers have a different size"};
  if (buffer->width_ != width_)
    return Result{"Buffers have a different width"};
  if (buffer->height_ != height_)
    return Result{"Buffers have a different height"};
  if (buffer->values_.size() != values_.size())
    return Result{"Buffers have a different number of values"};

  uint32_t num_different = 0;
  uint32_t first_different_index = 0;
  uint8_t first_different_left = 0;
  uint8_t first_different_right = 0;
  for (uint32_t i = 0; i < values_.size(); ++i) {
    if (values_[i] != buffer->values_[i]) {
      if (num_different == 0) {
        first_different_index = i;
        first_different_left = values_[i];
        first_different_right = buffer->values_[i];
      }
      num_different++;
    }
  }

  if (num_different) {
    return Result{"Buffers have different values. " +
                  std::to_string(num_different) +
                  " values differed, first difference at byte " +
                  std::to_string(first_different_index) + " values " +
                  std::to_string(first_different_left) +
                  " != " + std::to_string(first_different_right)};
  }

  return {};
}

DataBuffer::DataBuffer() = default;

DataBuffer::DataBuffer(BufferType type) : Buffer(type) {}

DataBuffer::~DataBuffer() = default;

Result DataBuffer::SetData(const std::vector<Value>& data) {
  SetValueCount(static_cast<uint32_t>(data.size()));
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

Result FormatBuffer::SetData(const std::vector<Value>& data) {
  SetValueCount(static_cast<uint32_t>(data.size()));
  values_.resize(GetSizeInBytes());
  return CopyData(data);
}

Result FormatBuffer::CopyData(const std::vector<Value>& data) {
  uint8_t* ptr = values_.data();

  for (uint32_t i = 0; i < data.size();) {
    const auto pack_size = format_->GetPackSize();
    if (pack_size) {
      if (pack_size == 8) {
        *(ValuesAs<uint8_t>(ptr)) = data[i].AsUint8();
        ptr += sizeof(uint8_t);
      } else if (pack_size == 16) {
        *(ValuesAs<uint16_t>(ptr)) = data[i].AsUint16();
        ptr += sizeof(uint16_t);
      } else if (pack_size == 32) {
        *(ValuesAs<uint32_t>(ptr)) = data[i].AsUint32();
        ptr += sizeof(uint32_t);
      }
      ++i;
      continue;
    }

    for (const auto& comp : format_->GetComponents()) {
      if (comp.IsInt8()) {
        *(ValuesAs<int8_t>(ptr)) = data[i].AsInt8();
        ptr += sizeof(int8_t);
      } else if (comp.IsInt16()) {
        *(ValuesAs<int16_t>(ptr)) = data[i].AsInt16();
        ptr += sizeof(int16_t);
      } else if (comp.IsInt32()) {
        *(ValuesAs<int32_t>(ptr)) = data[i].AsInt32();
        ptr += sizeof(int32_t);
      } else if (comp.IsInt64()) {
        *(ValuesAs<int64_t>(ptr)) = data[i].AsInt64();
        ptr += sizeof(int64_t);
      } else if (comp.IsUint8()) {
        *(ValuesAs<uint8_t>(ptr)) = data[i].AsUint8();
        ptr += sizeof(uint8_t);
      } else if (comp.IsUint16()) {
        *(ValuesAs<uint16_t>(ptr)) = data[i].AsUint16();
        ptr += sizeof(uint16_t);
      } else if (comp.IsUint32()) {
        *(ValuesAs<uint32_t>(ptr)) = data[i].AsUint32();
        ptr += sizeof(uint32_t);
      } else if (comp.IsUint64()) {
        *(ValuesAs<uint64_t>(ptr)) = data[i].AsUint64();
        ptr += sizeof(uint64_t);
      } else if (comp.IsFloat()) {
        *(ValuesAs<float>(ptr)) = data[i].AsFloat();
        ptr += sizeof(float);
      } else if (comp.IsDouble()) {
        *(ValuesAs<double>(ptr)) = data[i].AsDouble();
        ptr += sizeof(double);
      } else if (comp.IsFloat16()) {
        *(ValuesAs<uint16_t>(ptr)) = FloatToHexFloat16(data[i].AsFloat());
        ptr += sizeof(uint16_t);
      } else {
        // The float 10 and float 11 sizes are only used in PACKED formats.
        assert(false && "Not reached");
      }
      ++i;
    }
    // Need to add an extra element if this is std140 and there are 3 elements.
    if (format_->IsStd140() && format_->RowCount() == 3)
      ptr += (format_->GetComponents()[0].num_bits / 8);
  }
  return {};
}

}  // namespace amber
