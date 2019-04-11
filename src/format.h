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

    bool IsInt8() const {
      return (mode == FormatMode::kSInt || mode == FormatMode::kSNorm ||
              mode == FormatMode::kSScaled || mode == FormatMode::kSRGB) &&
             num_bits == 8;
    }
    bool IsInt16() const {
      return (mode == FormatMode::kSInt || mode == FormatMode::kSNorm) &&
             num_bits == 16;
    }
    bool IsInt32() const { return mode == FormatMode::kSInt && num_bits == 32; }
    bool IsInt64() const { return mode == FormatMode::kSInt && num_bits == 64; }
    bool IsUint8() const {
      return (mode == FormatMode::kUInt || mode == FormatMode::kUNorm ||
              mode == FormatMode::kUScaled) &&
             num_bits == 8;
    }
    bool IsUint16() const {
      return mode == FormatMode::kUInt && num_bits == 16;
    }
    bool IsUint32() const {
      return mode == FormatMode::kUInt && num_bits == 32;
    }
    bool IsUint64() const {
      return mode == FormatMode::kUInt && num_bits == 64;
    }
    bool IsFloat16() const {
      return mode == FormatMode::kSFloat && num_bits == 16;
    }
    bool IsFloat() const {
      return mode == FormatMode::kSFloat && num_bits == 32;
    }
    bool IsDouble() const {
      return mode == FormatMode::kSFloat && num_bits == 64;
    }
  };

  Format();
  Format(const Format&);
  ~Format();

  Format& operator=(const Format&) = default;

  bool Equal(const Format* b) const;

  void SetFormatType(FormatType type) { type_ = type; }
  FormatType GetFormatType() const { return type_; }

  void SetIsStd140() { is_std140_ = true; }
  bool IsStd140() const { return is_std140_; }

  /// Set the number of bytes this format is packed into, if provided.
  void SetPackSize(uint8_t size_in_bytes) {
    pack_size_in_bytes_ = size_in_bytes;
  }
  /// Retrieves the number of bytes this format is packed into.
  uint8_t GetPackSize() const { return pack_size_in_bytes_; }

  void AddComponent(FormatComponentType type, FormatMode mode, uint8_t bits) {
    components_.emplace_back(type, mode, bits);
  }
  const std::vector<Component>& GetComponents() const { return components_; }

  uint32_t SizeInBytes() const;
  bool IsFormatKnown() const { return type_ != FormatType::kUnknown; }
  bool HasStencilComponent() const {
    return type_ == FormatType::kD24_UNORM_S8_UINT ||
           type_ == FormatType::kD16_UNORM_S8_UINT ||
           type_ == FormatType::kD32_SFLOAT_S8_UINT ||
           type_ == FormatType::kS8_UINT;
  }

  /// Returns the number of unique numbers for each instance of this format.
  uint32_t ValuesPerElement() const { return RowCount() * column_count_; }

  uint32_t RowCount() const {
    return static_cast<uint32_t>(components_.size());
  }
  uint32_t ColumnCount() const { return column_count_; }
  void SetColumnCount(uint32_t c) { column_count_ = c; }

  bool IsInt8() const { return AreAllComponents(FormatMode::kSInt, 8); }
  bool IsInt16() const { return AreAllComponents(FormatMode::kSInt, 16); }
  bool IsInt32() const { return AreAllComponents(FormatMode::kSInt, 32); }
  bool IsInt64() const { return AreAllComponents(FormatMode::kSInt, 64); }
  bool IsUint8() const { return AreAllComponents(FormatMode::kUInt, 8); }
  bool IsUint16() const { return AreAllComponents(FormatMode::kUInt, 16); }
  bool IsUint32() const { return AreAllComponents(FormatMode::kUInt, 32); }
  bool IsUint64() const { return AreAllComponents(FormatMode::kUInt, 64); }
  bool IsFloat() const { return AreAllComponents(FormatMode::kSFloat, 32); }
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
