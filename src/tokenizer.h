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

#ifndef SRC_TOKENIZER_H_
#define SRC_TOKENIZER_H_

#include <cstdlib>
#include <memory>
#include <string>

#include "amber/result.h"

namespace amber {

enum class TokenType : uint8_t {
  kEOS = 0,
  kEOL,
  kString,
  kInteger,
  kDouble,
  kHex,
};

/// A token read from the input source.
class Token {
 public:
  explicit Token(TokenType type);
  ~Token();

  bool IsHex() const { return type_ == TokenType::kHex; }
  bool IsInteger() const { return type_ == TokenType::kInteger; }
  bool IsDouble() const { return type_ == TokenType::kDouble; }
  bool IsString() const { return type_ == TokenType::kString; }
  bool IsEOS() const { return type_ == TokenType::kEOS; }
  bool IsEOL() const { return type_ == TokenType::kEOL; }

  bool IsComma() const {
    return type_ == TokenType::kString && string_value_ == ",";
  }
  bool IsOpenBracket() const {
    return type_ == TokenType::kString && string_value_ == "(";
  }
  bool IsCloseBracket() const {
    return type_ == TokenType::kString && string_value_ == ")";
  }

  void SetNegative() { is_negative_ = true; }
  void SetStringValue(const std::string& val) { string_value_ = val; }
  void SetUint64Value(uint64_t val) { uint_value_ = val; }
  void SetDoubleValue(double val) { double_value_ = val; }

  const std::string& AsString() const { return string_value_; }

  uint8_t AsUint8() const { return static_cast<uint8_t>(uint_value_); }
  uint16_t AsUint16() const { return static_cast<uint16_t>(uint_value_); }
  uint32_t AsUint32() const { return static_cast<uint32_t>(uint_value_); }
  uint64_t AsUint64() const { return static_cast<uint64_t>(uint_value_); }

  int8_t AsInt8() const { return static_cast<int8_t>(uint_value_); }
  int16_t AsInt16() const { return static_cast<int16_t>(uint_value_); }
  int32_t AsInt32() const { return static_cast<int32_t>(uint_value_); }
  int64_t AsInt64() const { return static_cast<int64_t>(uint_value_); }

  Result ConvertToDouble();

  float AsFloat() const { return static_cast<float>(double_value_); }
  double AsDouble() const { return double_value_; }

  uint64_t AsHex() const {
    return uint64_t(std::strtoull(string_value_.c_str(), nullptr, 16));
  }

  /// The OriginalString is set for integer and double values to store the
  /// unparsed number which we can return in error messages.
  void SetOriginalString(const std::string& orig_string) {
    string_value_ = orig_string;
  }
  std::string ToOriginalString() const { return string_value_; }

 private:
  TokenType type_;
  std::string string_value_;
  uint64_t uint_value_ = 0;
  double double_value_ = 0.0;
  bool is_negative_ = false;
};

/// Splits the provided input into a stream of tokens.
class Tokenizer {
 public:
  explicit Tokenizer(const std::string& data);
  ~Tokenizer();

  std::unique_ptr<Token> NextToken();
  std::string ExtractToNext(const std::string& str);

  void SetCurrentLine(size_t line) { current_line_ = line; }
  size_t GetCurrentLine() const { return current_line_; }

 private:
  bool IsWhitespace(char ch);
  void SkipWhitespace();
  void SkipComment();

  std::string data_;
  size_t current_position_ = 0;
  size_t current_line_ = 1;
};

}  // namespace amber

#endif  // SRC_TOKENIZER_H_
