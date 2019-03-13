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

#include "src/datum_type.h"

namespace amber {

DatumType::DatumType() = default;

DatumType::~DatumType() = default;

DatumType& DatumType::operator=(const DatumType&) = default;

uint32_t DatumType::SizeInBytes() const {
  uint32_t s;
  if (type_ == DataType::kInt8 || type_ == DataType::kUint8)
    s = 1;
  else if (type_ == DataType::kInt16 || type_ == DataType::kUint16)
    s = 2;
  else if (type_ == DataType::kInt32 || type_ == DataType::kUint32)
    s = 4;
  else if (type_ == DataType::kInt64 || type_ == DataType::kUint64)
    s = 8;
  else if (type_ == DataType::kFloat)
    s = sizeof(float);
  else
    s = sizeof(double);

  return s * column_count_ * row_count_;
}

}  // namespace amber
