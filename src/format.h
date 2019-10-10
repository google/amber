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
#include <string>
#include <vector>

#include "src/format_data.h"
#include "src/make_unique.h"
#include "src/type.h"

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

  class Segment {
   public:
    explicit Segment(uint32_t num_bytes)
        : is_padding_(true), num_bits_(num_bytes * 8) {}
    Segment(FormatComponentType name, FormatMode mode, uint32_t num_bits)
        : name_(name), mode_(mode), num_bits_(num_bits) {}

    bool IsPadding() const { return is_padding_; }
    uint32_t PaddingBytes() const { return num_bits_ / 8; }

    FormatComponentType GetName() const { return name_; }
    FormatMode GetFormatMode() const { return mode_; }
    uint32_t GetNumBits() const { return num_bits_; }

    uint32_t SizeInBytes() const { return num_bits_ / 8; }

    // The packable flag can be set on padding segments. This means, the next
    // byte, if it's the same type as this packing, can be inserted before
    // this packing segment as long as it fits within the pack size, removing
    // that much pack space.
    bool IsPackable() const { return is_packable_; }
    void SetPackable(bool packable) { is_packable_ = packable; }

   private:
    bool is_padding_ = false;
    bool is_packable_ = false;
    FormatComponentType name_ = FormatComponentType::kR;
    FormatMode mode_ = FormatMode::kSInt;
    uint32_t num_bits_ = 0;
  };

  /// Creates a format of unknown type.
  explicit Format(type::Type* type);
  ~Format();

  static bool IsNormalized(FormatMode mode) {
    return mode == FormatMode::kUNorm || mode == FormatMode::kSNorm ||
           mode == FormatMode::kSRGB;
  }

  /// Returns true if |b| describes the same format as this object.
  bool Equal(const Format* b) const;

  /// Sets the type of the format. For image types this maps closely to the
  /// list of Vulkan formats. For data types, this maybe Unknown if the data
  /// type can not be represented by the image format (e.g. matrix types)
  void SetFormatType(FormatType type) { format_type_ = type; }
  FormatType GetFormatType() const { return format_type_; }

  void SetLayout(Layout layout);
  Layout GetLayout() const { return layout_; }

  type::Type* GetType() const { return type_; }

  /// Returns a pointer to the only type in this format. Only valid if
  /// there is only an int or float type, nullptr otherwise.
  type::Type* GetOnlyType() const {
    if (type_->IsNumber())
      return type_;
    return nullptr;
  }

  bool IsPacked() const {
    return type_->IsList() && type_->AsList()->IsPacked();
  }

  /// The segment is the individual pieces of the components including padding.
  const std::vector<Segment>& GetSegments() const { return segments_; }

  /// Returns the number of bytes this format requires.
  uint32_t SizeInBytes() const;

  bool IsFormatKnown() const { return format_type_ != FormatType::kUnknown; }
  bool HasStencilComponent() const {
    return format_type_ == FormatType::kD24_UNORM_S8_UINT ||
           format_type_ == FormatType::kD16_UNORM_S8_UINT ||
           format_type_ == FormatType::kD32_SFLOAT_S8_UINT ||
           format_type_ == FormatType::kS8_UINT;
  }

  /// Returns true if the format components are normalized.
  bool IsNormalized() const {
    if (type_->IsNumber() && IsNormalized(type_->AsNumber()->GetFormatMode()))
      return true;

    if (type_->IsList()) {
      for (auto& member : type_->AsList()->Members()) {
        if (!IsNormalized(member.mode)) {
          return false;
        }
      }
      return true;
    }
    return false;
  }

  /// Returns the number of input values required for an item of this format.
  /// This differs from ValuesPerElement because it doesn't take padding into
  /// account.
  uint32_t InputNeededPerElement() const;

  /// Returns true if all components of this format are an 8 bit signed int.
  bool IsInt8() const {
    return type_->IsNumber() &&
           type::Type::IsInt8(type_->AsNumber()->GetFormatMode(),
                              type_->AsNumber()->NumBits());
  }
  /// Returns true if all components of this format are a 16 bit signed int.
  bool IsInt16() const {
    return type_->IsNumber() &&
           type::Type::IsInt16(type_->AsNumber()->GetFormatMode(),
                               type_->AsNumber()->NumBits());
  }
  /// Returns true if all components of this format are a 32 bit signed int.
  bool IsInt32() const {
    return type_->IsNumber() &&
           type::Type::IsInt32(type_->AsNumber()->GetFormatMode(),
                               type_->AsNumber()->NumBits());
  }
  /// Returns true if all components of this format are a 64 bit signed int.
  bool IsInt64() const {
    return type_->IsNumber() &&
           type::Type::IsInt64(type_->AsNumber()->GetFormatMode(),
                               type_->AsNumber()->NumBits());
  }
  /// Returns true if all components of this format are a 8 bit unsigned int.
  bool IsUint8() const {
    return type_->IsNumber() &&
           type::Type::IsUint8(type_->AsNumber()->GetFormatMode(),
                               type_->AsNumber()->NumBits());
  }
  /// Returns true if all components of this format are a 16 bit unsigned int.
  bool IsUint16() const {
    return type_->IsNumber() &&
           type::Type::IsUint16(type_->AsNumber()->GetFormatMode(),
                                type_->AsNumber()->NumBits());
  }
  /// Returns true if all components of this format are a 32 bit unsigned int.
  bool IsUint32() const {
    return type_->IsNumber() &&
           type::Type::IsUint32(type_->AsNumber()->GetFormatMode(),
                                type_->AsNumber()->NumBits());
  }
  /// Returns true if all components of this format are a 64 bit unsigned int.
  bool IsUint64() const {
    return type_->IsNumber() &&
           type::Type::IsUint64(type_->AsNumber()->GetFormatMode(),
                                type_->AsNumber()->NumBits());
  }
  /// Returns true if all components of this format are a 32 bit float.
  bool IsFloat32() const {
    return type_->IsNumber() &&
           type::Type::IsFloat32(type_->AsNumber()->GetFormatMode(),
                                 type_->AsNumber()->NumBits());
  }
  /// Returns true if all components of this format are a 64 bit float.
  bool IsFloat64() const {
    return type_->IsNumber() &&
           type::Type::IsFloat64(type_->AsNumber()->GetFormatMode(),
                                 type_->AsNumber()->NumBits());
  }

  std::string GenerateNameForTesting() const { return GenerateName(); }

 private:
  void RebuildSegments();
  uint32_t AddSegmentsForType(type::Type* type);
  bool NeedsPadding(type::Type* t) const;
  // Returns true if a segment was added, false if we packed the requested
  // segment into previously allocated space.
  bool AddSegment(const Segment& seg);
  void AddPaddedSegment(uint32_t size);
  void AddPaddedSegmentPackable(uint32_t size);
  uint32_t CalcTypeBaseAlignmentInBytes(type::Type* s) const;
  uint32_t CalcStructBaseAlignmentInBytes(type::Struct* s) const;
  uint32_t CalcVecBaseAlignmentInBytes(type::Number* n) const;
  uint32_t CalcArrayBaseAlignmentInBytes(type::Type* t) const;
  uint32_t CalcMatrixBaseAlignmentInBytes(type::Number* m) const;
  uint32_t CalcListBaseAlignmentInBytes(type::List* l) const;

  /// Generates the image format name for this format if possible. Returns
  /// the name if generated or "" otherwise.
  std::string GenerateName() const;

  FormatType format_type_ = FormatType::kUnknown;
  Layout layout_ = Layout::kStd430;
  type::Type* type_;
  std::vector<FormatComponentType> type_names_;
  std::vector<Segment> segments_;
};

}  // namespace amber

#endif  // SRC_FORMAT_H_
