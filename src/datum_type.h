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

#ifndef SRC_DATUM_TYPE_H_
#define SRC_DATUM_TYPE_H_

#include <cstddef>
#include <cstdint>
#include <memory>

#include "src/format.h"

namespace amber {

enum class DataType {
  kInt8 = 0,
  kInt16,
  kInt32,
  kInt64,
  kUint8,
  kUint16,
  kUint32,
  kUint64,
  kFloat,
  kDouble,
};

/// Stores information on a given type of data. This class should only be used
/// as a simple way to create format objects. DatumType should not appear as a
/// member of any classes.
class DatumType {
 public:
  DatumType();
  DatumType(const DatumType&);
  ~DatumType();

  DatumType& operator=(const DatumType&);

  bool IsInt8() const { return type_ == DataType::kInt8; }
  bool IsInt16() const { return type_ == DataType::kInt16; }
  bool IsInt32() const { return type_ == DataType::kInt32; }
  bool IsInt64() const { return type_ == DataType::kInt64; }
  bool IsUint8() const { return type_ == DataType::kUint8; }
  bool IsUint16() const { return type_ == DataType::kUint16; }
  bool IsUint32() const { return type_ == DataType::kUint32; }
  bool IsUint64() const { return type_ == DataType::kUint64; }
  bool IsFloat() const { return type_ == DataType::kFloat; }
  bool IsDouble() const { return type_ == DataType::kDouble; }

  void SetType(DataType type) { type_ = type; }
  DataType GetType() const { return type_; }

  void SetColumnCount(uint32_t count) { column_count_ = count; }
  uint32_t ColumnCount() const { return column_count_; }

  void SetRowCount(uint32_t count) { row_count_ = count; }
  uint32_t RowCount() const { return row_count_; }

  uint32_t ElementSizeInBytes() const;
  uint32_t SizeInBytes() const;

  std::unique_ptr<Format> AsFormat() const;

 private:
  DataType type_ = DataType::kUint8;
  uint32_t column_count_ = 1;
  uint32_t row_count_ = 1;
  bool is_std140_ = true;
};

}  // namespace amber

#endif  // SRC_DATUM_TYPE_H_
