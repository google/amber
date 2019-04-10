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

#ifndef SRC_FORMAT_H_
#define SRC_FORMAT_H_

#include <cstdint>
#include <memory>
#include <vector>

#include "src/format_data.h"

namespace amber {

/// The format class describes requested image formats. (eg. R8G8B8A8_UINT).
class Format {
 public:
  /// Describes an individual component of a format.
  struct Component {
    Component(FormatComponentType t, FormatMode m, uint8_t bits)
        : type(t), mode(m), num_bits(bits) {}

    FormatComponentType type;
    FormatMode mode;
    uint8_t num_bits;

    /// Is this component represented by an 8 bit signed integer. (This includes
    /// int, scaled, rgb and norm values).
    bool IsInt8() const {
      return (mode == FormatMode::kSInt || mode == FormatMode::kSNorm ||
              mode == FormatMode::kSScaled || mode == FormatMode::kSRGB) &&
             num_bits == 8;
    }
    /// Is this component represented by a 16 bit signed integer. (This includes
    /// int and norm values)
    bool IsInt16() const {
      return (mode == FormatMode::kSInt || mode == FormatMode::kSNorm) &&
             num_bits == 16;
    }
    /// Is this component represented by a 32 bit signed integer.
    bool IsInt32() const { return mode == FormatMode::kSInt && num_bits == 32; }
    /// Is this component represented by a 64 bit signed integer.
    bool IsInt64() const { return mode == FormatMode::kSInt && num_bits == 64; }
    /// Is this component represented by an 8 bit unsigned integer. (This
    /// includes uint, unorm and uscaled values).
    bool IsUint8() const {
      return (mode == FormatMode::kUInt || mode == FormatMode::kUNorm ||
              mode == FormatMode::kUScaled) &&
             num_bits == 8;
    }
    /// Is this component represented by a 16 bit unsigned integer.
    bool IsUint16() const {
      return mode == FormatMode::kUInt && num_bits == 16;
    }
    /// Is this component represented by a 32 bit unsigned integer.
    bool IsUint32() const {
      return mode == FormatMode::kUInt && num_bits == 32;
    }
    /// Is this component represented by a 64 bit unsigned integer.
    bool IsUint64() const {
      return mode == FormatMode::kUInt && num_bits == 64;
    }
    /// Is this component represented by a 16 bit floating point value.
    bool IsFloat16() const {
      return mode == FormatMode::kSFloat && num_bits == 16;
    }
    /// Is this component represented by a 32 bit floating point value
    bool IsFloat() const {
      return mode == FormatMode::kSFloat && num_bits == 32;
    }
    /// Is this component represented by a 64 bit floating point value
    bool IsDouble() const {
      return mode == FormatMode::kSFloat && num_bits == 64;
    }
  };

  /// Creates a format of unknown type.
  Format();
  Format(const Format&);
  ~Format();

  Format& operator=(const Format&) = default;

  /// Returns true if |b| describes the same format as this object.
  bool Equal(const Format* b) const;

  /// Sets the type of the format. For image types this maps closely to the
  /// list of Vulkan formats. For data types, this maybe Unknown if the data
  /// type can not be represented by the image format (i.e. matrix types)
  void SetFormatType(FormatType type) { type_ = type; }
  /// Returns the type for this format object.
  FormatType GetFormatType() const { return type_; }

  /// Sets this format to represent data in std140 mode.
  void SetIsStd140() { is_std140_ = true; }
  /// Returns true if this format is a std140 representation.
  bool IsStd140() const { return is_std140_; }

  /// Set the number of bytes this format is packed into, if provided.
  void SetPackSize(uint8_t size_in_bytes) {
    pack_size_in_bytes_ = size_in_bytes;
  }
  /// Retrieves the number of bytes this format is packed into.
  uint8_t GetPackSize() const { return pack_size_in_bytes_; }

  /// Adds a component to this format.
  void AddComponent(FormatComponentType type, FormatMode mode, uint8_t bits) {
    components_.emplace_back(type, mode, bits);
  }
  /// Retrieves the components this format describes
  const std::vector<Component>& GetComponents() const { return components_; }

  /// Returns the number of bytes this format requires.
  uint32_t SizeInBytes() const;
  /// Returns the number of bytes per single row this format requires.
  uint32_t SizeInBytesPerRow() const;

  /// Returns true if the format is not unknown.
  bool IsFormatKnown() const { return type_ != FormatType::kUnknown; }
  /// Returns true if this format has a stencil component.
  bool HasStencilComponent() const {
    return type_ == FormatType::kD24_UNORM_S8_UINT ||
           type_ == FormatType::kD16_UNORM_S8_UINT ||
           type_ == FormatType::kD32_SFLOAT_S8_UINT ||
           type_ == FormatType::kS8_UINT;
  }

  /// Returns the number of values for each instance of this format.
  uint32_t ValuesPerElement() const { return RowCount() * column_count_; }

  /// Returns the number of rows this format describes
  uint32_t RowCount() const {
    return static_cast<uint32_t>(components_.size());
  }
  /// Returns the number of columns this format describes.
  uint32_t ColumnCount() const { return column_count_; }
  /// Sets the number of columns this format describes.
  void SetColumnCount(uint32_t c) { column_count_ = c; }

  /// Returns true if all components of this format are an 8 bit signed int.
  bool IsInt8() const { return AreAllComponents(FormatMode::kSInt, 8); }
  /// Returns true if all components of this format are a 16 bit signed int.
  bool IsInt16() const { return AreAllComponents(FormatMode::kSInt, 16); }
  /// Returns true if all components of this format are a 32 bit signed int.
  bool IsInt32() const { return AreAllComponents(FormatMode::kSInt, 32); }
  /// Returns true if all components of this format are a 64 bit signed int.
  bool IsInt64() const { return AreAllComponents(FormatMode::kSInt, 64); }
  /// Returns true if all components of this format are a 8 bit unsigned int.
  bool IsUint8() const { return AreAllComponents(FormatMode::kUInt, 8); }
  /// Returns true if all components of this format are a 16 bit unsigned int.
  bool IsUint16() const { return AreAllComponents(FormatMode::kUInt, 16); }
  /// Returns true if all components of this format are a 32 bit unsigned int.
  bool IsUint32() const { return AreAllComponents(FormatMode::kUInt, 32); }
  /// Returns true if all components of this format are a 64 bit unsigned int.
  bool IsUint64() const { return AreAllComponents(FormatMode::kUInt, 64); }
  /// Returns true if all components of this format are a 32 bit float.
  bool IsFloat() const { return AreAllComponents(FormatMode::kSFloat, 32); }
  /// Returns true if all components of this format are a 64 bit float.
  bool IsDouble() const { return AreAllComponents(FormatMode::kSFloat, 64); }

 private:
  bool AreAllComponents(FormatMode mode, uint32_t bits) const;

  FormatType type_ = FormatType::kUnknown;
  bool is_std140_ = false;
  uint8_t pack_size_in_bytes_ = 0;
  uint32_t column_count_ = 1;
  std::vector<Component> components_;
};

}  // namespace amber

#endif  // SRC_FORMAT_H_
