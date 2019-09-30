// Copyright 2019 The Amber Authors.
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

#ifndef SRC_TYPE_H_
#define SRC_TYPE_H_

#include <cassert>
#include <memory>
#include <string>
#include <vector>

#include "src/format_data.h"
#include "src/make_unique.h"

namespace amber {
namespace type {

class List;
class Number;
class Struct;

class Type {
 public:
  Type();
  virtual ~Type();

  static bool IsSignedInt(FormatMode mode) {
    return mode == FormatMode::kSInt || mode == FormatMode::kSNorm ||
           mode == FormatMode::kSScaled;
  }

  static bool IsUnsignedInt(FormatMode mode) {
    return mode == FormatMode::kUInt || mode == FormatMode::kUNorm ||
           mode == FormatMode::kUScaled || mode == FormatMode::kSRGB;
  }

  static bool IsInt(FormatMode mode) {
    return IsSignedInt(mode) || IsUnsignedInt(mode);
  }

  static bool IsFloat(FormatMode mode) {
    return mode == FormatMode::kSFloat || mode == FormatMode::kUFloat;
  }

  static bool IsInt8(FormatMode mode, uint32_t num_bits) {
    return IsSignedInt(mode) && num_bits == 8;
  }
  static bool IsInt16(FormatMode mode, uint32_t num_bits) {
    return IsSignedInt(mode) && num_bits == 16;
  }
  static bool IsInt32(FormatMode mode, uint32_t num_bits) {
    return IsSignedInt(mode) && num_bits == 32;
  }
  static bool IsInt64(FormatMode mode, uint32_t num_bits) {
    return IsSignedInt(mode) && num_bits == 64;
  }

  static bool IsUint8(FormatMode mode, uint32_t num_bits) {
    return IsUnsignedInt(mode) && num_bits == 8;
  }
  static bool IsUint16(FormatMode mode, uint32_t num_bits) {
    return IsUnsignedInt(mode) && num_bits == 16;
  }
  static bool IsUint32(FormatMode mode, uint32_t num_bits) {
    return IsUnsignedInt(mode) && num_bits == 32;
  }
  static bool IsUint64(FormatMode mode, uint32_t num_bits) {
    return IsUnsignedInt(mode) && num_bits == 64;
  }

  static bool IsFloat16(FormatMode mode, uint32_t num_bits) {
    return IsFloat(mode) && num_bits == 16;
  }
  static bool IsFloat32(FormatMode mode, uint32_t num_bits) {
    return IsFloat(mode) && num_bits == 32;
  }
  static bool IsFloat64(FormatMode mode, uint32_t num_bits) {
    return IsFloat(mode) && num_bits == 64;
  }

  // Returns the size in bytes of a single element of the type. This does not
  // include space for arrays, vectors, etc.
  virtual uint32_t SizeInBytes() const = 0;

  virtual bool Equal(const Type* b) const = 0;

  virtual bool IsList() const { return false; }
  virtual bool IsNumber() const { return false; }
  virtual bool IsStruct() const { return false; }

  List* AsList();
  Number* AsNumber();
  Struct* AsStruct();

  const List* AsList() const;
  const Number* AsNumber() const;
  const Struct* AsStruct() const;

  void SetRowCount(uint32_t size) { row_count_ = size; }
  uint32_t RowCount() const { return row_count_; }

  void SetColumnCount(uint32_t size) { column_count_ = size; }
  uint32_t ColumnCount() const { return column_count_; }

  void SetIsRuntimeArray() { is_array_ = true; }
  void SetIsSizedArray(uint32_t size) {
    is_array_ = true;
    array_size_ = size;
  }
  bool IsArray() const { return is_array_; }
  bool IsSizedArray() const { return is_array_ && array_size_ > 0; }
  bool IsRuntimeArray() const { return is_array_ && array_size_ == 0; }
  uint32_t ArraySize() const { return array_size_; }

  bool IsVec() const { return column_count_ == 1 && row_count_ > 1; }

  // Returns true if this type holds a vec3.
  bool IsVec3() const { return column_count_ == 1 && row_count_ == 3; }

  // Returns true if this type holds a matrix.
  bool IsMatrix() const { return column_count_ > 1 && row_count_ > 1; }

 private:
  uint32_t row_count_ = 1;
  uint32_t column_count_ = 1;
  uint32_t array_size_ = 0;
  bool is_array_ = false;
};

class Number : public Type {
 public:
  explicit Number(FormatMode mode);
  Number(FormatMode mode, uint32_t bits);
  ~Number() override;

  static std::unique_ptr<Number> Int(uint32_t bits);
  static std::unique_ptr<Number> Uint(uint32_t bits);
  static std::unique_ptr<Number> Float(uint32_t bits);

  bool IsNumber() const override { return true; }

  uint32_t NumBits() const { return bits_; }
  uint32_t SizeInBytes() const override { return bits_ / 8; }

  bool Equal(const Type* b) const override {
    if (!b->IsNumber())
      return false;

    auto n = b->AsNumber();
    return format_mode_ == n->format_mode_ && bits_ == n->bits_;
  }

  FormatMode GetFormatMode() const { return format_mode_; }

 private:
  FormatMode format_mode_ = FormatMode::kSInt;
  uint32_t bits_ = 32;
};

// The list type only holds lists of scalar float and int values.
class List : public Type {
 public:
  struct Member {
    Member(FormatComponentType t, FormatMode m, uint32_t b)
        : name(t), mode(m), num_bits(b) {}

    uint32_t SizeInBytes() const { return num_bits / 8; }

    FormatComponentType name = FormatComponentType::kR;
    FormatMode mode = FormatMode::kSInt;
    uint32_t num_bits = 0;
  };

  List();
  ~List() override;

  bool IsList() const override { return true; }

  bool Equal(const Type* b) const override {
    if (!b->IsList())
      return false;

    auto l = b->AsList();
    if (pack_size_in_bits_ != l->pack_size_in_bits_)
      return false;
    if (members_.size() != l->members_.size())
      return false;

    auto& lm = l->Members();
    for (size_t i = 0; i < members_.size(); ++i) {
      if (members_[i].name != lm[i].name)
        return false;
      if (members_[i].mode != lm[i].mode)
        return false;
      if (members_[i].num_bits != lm[i].num_bits)
        return false;
    }
    return true;
  }

  void SetPackSizeInBits(uint32_t size) { pack_size_in_bits_ = size; }
  uint32_t PackSizeInBits() const { return pack_size_in_bits_; }
  bool IsPacked() const { return pack_size_in_bits_ > 0; }

  void AddMember(FormatComponentType name, FormatMode mode, uint32_t num_bits) {
    members_.push_back({name, mode, num_bits});
  }

  const std::vector<Member>& Members() const { return members_; }
  std::vector<Member>& Members() { return members_; }

  uint32_t SizeInBytes() const override;

 private:
  std::vector<Member> members_;
  uint32_t pack_size_in_bits_ = 0;
};

class Struct : public Type {
 public:
  struct Member {
    std::string name;
    Type* type;
    int32_t offset_in_bytes = -1;
    int32_t array_stride_in_bytes = -1;
    int32_t matrix_stride_in_bytes = -1;

    bool HasOffset() const { return offset_in_bytes >= 0; }
    bool HasArrayStride() const { return array_stride_in_bytes > 0; }
    bool HasMatrixStride() const { return matrix_stride_in_bytes > 0; }
  };

  Struct();
  ~Struct() override;

  uint32_t SizeInBytes() const override;
  bool IsStruct() const override { return true; }

  bool Equal(const Type* b) const override {
    if (!b->IsStruct())
      return false;

    auto s = b->AsStruct();
    if (is_stride_specified_ != s->is_stride_specified_)
      return false;
    if (stride_in_bytes_ != s->stride_in_bytes_)
      return false;
    if (members_.size() != s->members_.size())
      return false;

    auto& sm = s->Members();
    for (size_t i = 0; i < members_.size(); ++i) {
      if (members_[i].offset_in_bytes != sm[i].offset_in_bytes)
        return false;
      if (members_[i].array_stride_in_bytes != sm[i].array_stride_in_bytes)
        return false;
      if (members_[i].matrix_stride_in_bytes != sm[i].matrix_stride_in_bytes)
        return false;
      if (!members_[i].type->Equal(sm[i].type))
        return false;
    }
    return true;
  }

  bool HasStride() const { return is_stride_specified_; }
  uint32_t StrideInBytes() const { return stride_in_bytes_; }
  void SetStrideInBytes(uint32_t stride) {
    is_stride_specified_ = true;
    stride_in_bytes_ = stride;
  }

  Member* AddMember(Type* type) {
    members_.push_back({});
    members_.back().type = type;
    return &members_.back();
  }

  const std::vector<Member>& Members() const { return members_; }

 private:
  std::vector<Member> members_;
  bool is_stride_specified_ = false;
  uint32_t stride_in_bytes_ = 0;
};

}  // namespace type
}  // namespace amber

#endif  // SRC_TYPE_H_
