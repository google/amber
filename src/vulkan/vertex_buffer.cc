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

#include "src/vulkan/vertex_buffer.h"

#include <cassert>
#include <cstring>

#include "src/make_unique.h"
#include "src/vulkan/device.h"
#include "src/vulkan/format_data.h"

namespace amber {
namespace vulkan {
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
  return static_cast<uint16_t>(FloatSign(*hex) << 15U) |
         static_cast<uint16_t>(FloatExponent(*hex) << 10U) |
         static_cast<uint16_t>(FloatMantissa(*hex) >> 13U);
}

// Convert 32 bits float |value| to 11 bits float based on IEEE-754.
uint16_t FloatToHexFloat11(const float value) {
  const uint32_t* hex = reinterpret_cast<const uint32_t*>(&value);
  assert(FloatSign(*hex) == 0);
  return static_cast<uint16_t>(FloatExponent(*hex) << 6U) |
         static_cast<uint16_t>(FloatMantissa(*hex) >> 17U);
}

// Convert 32 bits float |value| to 10 bits float based on IEEE-754.
uint16_t FloatToHexFloat10(const float value) {
  const uint32_t* hex = reinterpret_cast<const uint32_t*>(&value);
  assert(FloatSign(*hex) == 0);
  return static_cast<uint16_t>(FloatExponent(*hex) << 5U) |
         static_cast<uint16_t>(FloatMantissa(*hex) >> 18U);
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

// Copy [0, bits) bits of |src| to
// [dst_bit_offset, dst_bit_offset + bits) of |dst|. If |bits| is
// less than 32 and the type is float, this method uses
// FloatToHexFloat() to convert it into small bits float.
Result CopyBitsOfValueToBuffer(uint8_t* dst,
                               const Value& src,
                               uint8_t dst_bit_offset,
                               uint8_t bits) {
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
          float* float_ptr = nullptr;
          float_ptr = reinterpret_cast<float*>(&data);
          *float_ptr = src.AsFloat();
          break;
        }
        case 16:
        case 11:
        case 10: {
          uint16_t* uint16_ptr = nullptr;
          uint16_ptr = reinterpret_cast<uint16_t*>(&data);
          *uint16_ptr = FloatToHexFloat(src.AsFloat(), bits);
          break;
        }
        default: {
          return Result(
              "Vulkan: Invalid float bits for CopyBitsOfValueToBuffer");
        }
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

  uint64_t* dst64 = reinterpret_cast<uint64_t*>(dst);
  uint64_t dst_lower_bits = *dst64 & ((1UL << dst_bit_offset) - 1UL);
  uint64_t dst_upper_bits =
      *dst64 & ~(((1UL << (dst_bit_offset + bits)) - 1UL));

  *dst64 = dst_lower_bits | data | dst_upper_bits;
  return {};
}

}  // namespace

VertexBuffer::VertexBuffer(Device* device) : device_(device) {}

VertexBuffer::~VertexBuffer() = default;

void VertexBuffer::Shutdown() {
  if (buffer_)
    buffer_->Shutdown();
}

void VertexBuffer::SetData(uint8_t location,
                           const Format& format,
                           const std::vector<Value>& values) {
  vertex_attr_desc_.emplace_back();
  // TODO(jaebaek): Support multiple binding
  vertex_attr_desc_.back().binding = 0;
  vertex_attr_desc_.back().location = location;
  vertex_attr_desc_.back().format = ToVkFormat(format.GetFormatType());
  vertex_attr_desc_.back().offset = stride_in_bytes_;

  stride_in_bytes_ += format.GetByteSize();

  formats_.push_back(format);
  data_.push_back(values);
}

Result VertexBuffer::FillVertexBufferWithData(VkCommandBuffer command) {
  // Send vertex data from host to device.
  uint8_t* ptr_in_stride_begin =
      static_cast<uint8_t*>(buffer_->HostAccessibleMemoryPtr());
  for (uint32_t i = 0; i < GetVertexCount(); ++i) {
    uint8_t* ptr = ptr_in_stride_begin;
    for (uint32_t j = 0; j < formats_.size(); ++j) {
      const auto pack_size = formats_[j].GetPackSize();
      if (pack_size) {
        Result r = CopyBitsOfValueToBuffer(ptr, data_[j][i], 0, pack_size);
        if (!r.IsSuccess())
          return r;

        ptr += pack_size / 8;
        continue;
      }

      const auto& components = formats_[j].GetComponents();
      uint8_t bit_offset = 0;

      for (uint32_t k = 0; k < components.size(); ++k) {
        uint8_t bits = components[k].num_bits;
        Result r = CopyBitsOfValueToBuffer(
            ptr, data_[j][i * components.size() + k], bit_offset, bits);
        if (!r.IsSuccess())
          return r;

        if ((k != components.size() - 1) &&
            (static_cast<uint32_t>(bit_offset) + static_cast<uint32_t>(bits) >=
             256)) {
          return Result(
              "Vulkan: VertexBuffer::FillVertexBufferWithData bit_offset "
              "overflow");
        }
        bit_offset = static_cast<uint8_t>(bit_offset + bits);
      }

      ptr += formats_[j].GetByteSize();
    }
    ptr_in_stride_begin += Get4BytesAlignedStride();
  }

  return buffer_->CopyToDevice(command);
}

void VertexBuffer::BindToCommandBuffer(VkCommandBuffer command) {
  const VkDeviceSize offset = 0;
  const VkBuffer buffer = buffer_->GetVkBuffer();
  // TODO(jaebaek): Support multiple binding
  device_->GetPtrs()->vkCmdBindVertexBuffers(command, 0, 1, &buffer, &offset);
}

Result VertexBuffer::SendVertexData(
    VkCommandBuffer command,
    const VkPhysicalDeviceMemoryProperties& properties) {
  if (!is_vertex_data_pending_)
    return Result("Vulkan::Vertices data was already sent");

  const size_t n_vertices = GetVertexCount();
  if (n_vertices == 0)
    return Result("Vulkan::Data for VertexBuffer is empty");

  size_t bytes = static_cast<size_t>(Get4BytesAlignedStride()) * n_vertices;

  if (!buffer_) {
    buffer_ = MakeUnique<Buffer>(device_, bytes, properties);
    Result r = buffer_->Initialize(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                                   VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    if (!r.IsSuccess())
      return r;
  }

  if (formats_.empty() || formats_[0].GetComponents().empty())
    return Result("Vulkan::Formats for VertexBuffer is empty");

  FillVertexBufferWithData(command);

  is_vertex_data_pending_ = false;
  return {};
}

}  // namespace vulkan
}  // namespace amber
