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
#include <vector>

#include "src/buffer_data.h"
#include "src/datum_type.h"
#include "src/value.h"

namespace amber {

class Buffer {
 public:
  explicit Buffer(BufferType type);
  ~Buffer();

  BufferType GetBufferType() const { return buffer_type_; }

  void SetName(const std::string& name) { name_ = name; }
  std::string GetName() const { return name_; }

  void SetDatumType(const DatumType& type) { datum_type_ = type; }
  const DatumType& GetDatumType() const { return datum_type_; }

  void SetSize(size_t size) { size_ = size; }
  size_t GetSize() const { return size_; }
  size_t GetSizeInBytes() const { return size_ * datum_type_.SizeInBytes(); }

  void SetData(std::vector<Value>&& data) { data_ = std::move(data); }
  const std::vector<Value>& GetData() const { return data_; }

 private:
  BufferType buffer_type_;
  DatumType datum_type_;
  std::vector<Value> data_;
  std::string name_;
  size_t size_ = 0;
};

}  // namespace amber

#endif  // SRC_AMBERSCRIPT_BUFFER_H_
