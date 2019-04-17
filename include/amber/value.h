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

#ifndef AMBER_VALUE_H_
#define AMBER_VALUE_H_

#include <stdint.h>

namespace amber {

/// Wrapper for a single value. The value will be either an integer or a
/// floating point value.
class Value {
 public:
  Value();
  Value(const Value&);
  ~Value();

  Value& operator=(const Value&);

  void SetIntValue(uint64_t val) {
    type_ = kValueTypeInteger;
    uint_value_ = val;
  }
  bool IsInteger() const { return type_ == kValueTypeInteger; }

  void SetDoubleValue(double val) {
    type_ = kValueTypeFloat;
    double_value_ = val;
  }
  bool IsFloat() const { return type_ == kValueTypeFloat; }

  uint8_t AsUint8() const { return static_cast<uint8_t>(uint_value_); }
  uint16_t AsUint16() const { return static_cast<uint16_t>(uint_value_); }
  uint32_t AsUint32() const { return static_cast<uint32_t>(uint_value_); }
  uint64_t AsUint64() const { return static_cast<uint64_t>(uint_value_); }

  int8_t AsInt8() const { return static_cast<int8_t>(uint_value_); }
  int16_t AsInt16() const { return static_cast<int16_t>(uint_value_); }
  int32_t AsInt32() const { return static_cast<int32_t>(uint_value_); }
  int64_t AsInt64() const { return static_cast<int64_t>(uint_value_); }

  float AsFloat() const { return static_cast<float>(double_value_); }
  double AsDouble() const { return double_value_; }

 private:
  enum Type { kValueTypeFloat, kValueTypeInteger };
  Type type_;
  uint64_t uint_value_;
  double double_value_;
};

}  // namespace amber

#endif  // AMBER_VALUE_H_
