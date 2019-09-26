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

#include <cassert>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "src/format_data.h"
#include "src/make_unique.h"

namespace amber {

/// The format class describes requested  data formats. (eg. R8G8B8A8_UINT).
///
/// There is a distinction between the input values needed and the values needed
/// for a given format. The input values is the number needed to be read to fill
/// out the format. The number of values is the number needed in memory to fill
/// out the format. These two numbers maybe different. The number of values will
/// always be equal or greater then the number of input values needed.
///
/// The place these differ is a) std140 layouts and b) vectors with 3 items. In
/// both those cases we inflate the to 4 elements. So the input data will be
/// smaller then the values per element.
class Format {
 public:
  enum Layout { kStd140 = 0, kStd430 };

  /// Describes an individual component of a format.
  struct Component {
    Component(FormatComponentType t, FormatMode m, uint8_t bits)
        : type(t), mode(m), num_bits(bits) {}

    FormatComponentType type;
    FormatMode mode;
    uint8_t num_bits;

    /// Returns the number of bytes used to store this component.
    size_t SizeInBytes() const { return num_bits / 8; }

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

  class Segment {
   public:
    explicit Segment(Component* component) : component_(component) {}
    Segment(const Segment&) = default;
    ~Segment() = default;

    Segment& operator=(const Segment&) = default;

    void SetIsPadding() { is_padding_ = true; }
    bool IsPadding() const { return is_padding_; }

    Component* GetComponent() const { return component_; }

   private:
    Component* component_;
    bool is_padding_ = false;
  };

  /// Creates a format of unknown type.
  Format();
  Format(const Format&);
  ~Format();

  Format& operator=(const Format&);

  /// Returns true if |b| describes the same format as this object.
  bool Equal(const Format* b) const;

  /// Sets the type of the format. For image types this maps closely to the
  /// list of Vulkan formats. For data types, this maybe Unknown if the data
  /// type can not be represented by the image format (e.g. matrix types)
  void SetFormatType(FormatType type) { type_ = type; }
  FormatType GetFormatType() const { return type_; }

  void SetLayout(Layout layout);
  Layout GetLayout() const { return layout_; }

  /// Set the number of bytes this format is packed into, if provided.
  void SetPackSize(uint8_t size_in_bytes) {
    pack_size_in_bytes_ = size_in_bytes;
  }
  /// Retrieves the number of bytes this format is packed into.
  uint8_t GetPackSize() const { return pack_size_in_bytes_; }

  void AddComponent(FormatComponentType type, FormatMode mode, uint8_t bits);

  /// Returns a pointer to the only component in this format. Only valid if
  /// there is only a single component, asserts otherwise.
  Component* GetOnlyComponent() const {
    assert(components_.size() == 1);
    return components_[0].get();
  }

  /// The segment is the individual pieces of the components including padding.
  const std::vector<Segment>& GetSegments() const { return segments_; }

  /// Returns the number of bytes this format requires.
  uint32_t SizeInBytes() const;

  bool IsFormatKnown() const { return type_ != FormatType::kUnknown; }
  bool HasStencilComponent() const {
    return type_ == FormatType::kD24_UNORM_S8_UINT ||
           type_ == FormatType::kD16_UNORM_S8_UINT ||
           type_ == FormatType::kD32_SFLOAT_S8_UINT ||
           type_ == FormatType::kS8_UINT;
  }

  /// Returns true if the format components are scaled
  bool IsScaled() const {
    FormatMode mode = components_.back()->mode;
    return mode == FormatMode::kUNorm || mode == FormatMode::kSNorm ||
           mode == FormatMode::kSRGB;
  }

  /// Returns the number of input values required for an item of this format.
  /// This differs from ValuesPerElement because it doesn't take padding into
  /// account.
  uint32_t InputNeededPerElement() const;

  /// Returns the number of values for a given row, including padding.
  uint32_t ValuesPerElement() const {
    return static_cast<uint32_t>(segments_.size());
  }

  /// Returns the number of values for a given row, excluding padding.
  uint32_t RowCount() const {
    return static_cast<uint32_t>(components_.size());
  }
  uint32_t ColumnCount() const { return column_count_; }
  void SetColumnCount(uint32_t c);

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

  /// Generates the image format name for this format if possible. Returns
  /// name if generated or "" otherwise.
  std::string GenerateName() const;

 private:
  bool AreAllComponents(FormatMode mode, uint32_t bits) const;
  void RebuildSegments();

  FormatType type_ = FormatType::kUnknown;
  Layout layout_ = Layout::kStd430;
  uint8_t pack_size_in_bytes_ = 0;
  uint32_t column_count_ = 1;
  std::vector<std::unique_ptr<Component>> components_;
  std::vector<Segment> segments_;
};

}  // namespace amber

#endif  // SRC_FORMAT_H_
