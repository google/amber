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

#ifndef SRC_BUFFER_H_
#define SRC_BUFFER_H_

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "src/buffer_data.h"
#include "src/datum_type.h"
#include "src/value.h"

namespace amber {

/// A buffer stores data. The buffer maybe provided from the input script, or
/// maybe created as needed. A buffer must have a unique name.
class Buffer {
 public:
  /// Create a buffer of |type|.
  explicit Buffer(BufferType type);
  ~Buffer();

  /// Returns the BufferType of this buffer.
  BufferType GetBufferType() const { return buffer_type_; }

  /// Sets the buffer |name|.
  void SetName(const std::string& name) { name_ = name; }
  /// Returns the name of the buffer.
  std::string GetName() const { return name_; }

  /// Sets the DatumType of the buffer to |type|.
  void SetDatumType(const DatumType& type) { datum_type_ = type; }
  /// Returns the DatumType describing the buffer data.
  const DatumType& GetDatumType() const { return datum_type_; }

  /// Sets the buffer to |size| items.
  void SetSize(size_t size) { size_ = size; }
  /// Returns the number of items in the buffer.
  size_t GetSize() const { return size_; }

  /// Returns the number of bytes needed for the data in the buffer.
  size_t GetSizeInBytes() const { return size_ * datum_type_.SizeInBytes(); }

  /// Sets the data into the buffer. The size will also be updated to be the size
  /// of the data provided.
  void SetData(std::vector<Value>&& data) {
    size_ = data.size() / datum_type_.ColumnCount() / datum_type_.RowCount();
    data_ = std::move(data);
  }
  /// Returns the vector of Values stored in the buffer.
  const std::vector<Value>& GetData() const { return data_; }

 private:
  BufferType buffer_type_;
  DatumType datum_type_;
  std::vector<Value> data_;
  std::string name_;
  size_t size_ = 0;
};

}  // namespace amber

#endif  // SRC_BUFFER_H_
