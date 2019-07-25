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

#include "src/tokenizer.h"

#include <cctype>
#include <cstdlib>
#include <limits>
#include <sstream>

#include "src/make_unique.h"

namespace amber {

Token::Token(TokenType type) : type_(type) {}

Token::~Token() = default;

Result Token::ConvertToDouble() {
  if (IsDouble())
    return {};

  if (IsString() || IsEOL() || IsEOS())
    return Result("Invalid conversion to double");

  if (IsInteger()) {
    if (is_negative_ ||
        uint_value_ <=
            static_cast<uint64_t>(std::numeric_limits<int64_t>::max())) {
      double_value_ = static_cast<double>(AsInt64());
    } else {
      return Result("uint64_t value too big to fit in double");
    }

    uint_value_ = 0;
  } else if (IsHex()) {
    double_value_ = static_cast<double>(AsHex());
    string_value_ = "";
  }
  type_ = TokenType::kDouble;
  return {};
}

Tokenizer::Tokenizer(const std::string& data) : data_(data) {}

Tokenizer::~Tokenizer() = default;

std::unique_ptr<Token> Tokenizer::NextToken() {
  SkipWhitespace();
  if (current_position_ >= data_.length())
    return MakeUnique<Token>(TokenType::kEOS);

  if (data_[current_position_] == '#') {
    SkipComment();
    SkipWhitespace();
  }
  if (current_position_ >= data_.length())
    return MakeUnique<Token>(TokenType::kEOS);

  if (data_[current_position_] == '\n') {
    ++current_line_;
    ++current_position_;
    return MakeUnique<Token>(TokenType::kEOL);
  }

  // If the current position is a , ( or ) then handle it specially as we don't
  // want to consume any other characters.
  if (data_[current_position_] == ',' || data_[current_position_] == '(' ||
      data_[current_position_] == ')') {
    auto tok = MakeUnique<Token>(TokenType::kString);
    std::string str(1, data_[current_position_]);
    tok->SetStringValue(str);
    ++current_position_;
    return tok;
  }

  size_t end_pos = current_position_;
  while (end_pos < data_.length()) {
    if (data_[end_pos] == ' ' || data_[end_pos] == '\r' ||
        data_[end_pos] == '\n' || data_[end_pos] == ')' ||
        data_[end_pos] == ',' || data_[end_pos] == '(') {
      break;
    }
    ++end_pos;
  }

  std::string tok_str =
      data_.substr(current_position_, end_pos - current_position_);
  current_position_ = end_pos;

  // Check for "NaN" explicitly.
  bool is_nan =
      (tok_str.size() == 3 && std::tolower(tok_str[0]) == 'n' &&
       std::tolower(tok_str[1]) == 'a' && std::tolower(tok_str[2]) == 'n');

  // Starts with an alpha is a string.
  if (!is_nan && !std::isdigit(tok_str[0]) &&
      !(tok_str[0] == '-' && tok_str.size() >= 2 && std::isdigit(tok_str[1])) &&
      !(tok_str[0] == '.' && tok_str.size() >= 2 && std::isdigit(tok_str[1]))) {
    // If we've got a continuation, skip over the end of line and get the next
    // token.
    if (tok_str == "\\") {
      if ((current_position_ < data_.length() &&
           data_[current_position_] == '\n')) {
        ++current_line_;
        ++current_position_;
        return NextToken();
      } else if (current_position_ + 1 < data_.length() &&
                 data_[current_position_] == '\r' &&
                 data_[current_position_ + 1] == '\n') {
        ++current_line_;
        current_position_ += 2;
        return NextToken();
      }
    }

    auto tok = MakeUnique<Token>(TokenType::kString);
    tok->SetStringValue(tok_str);
    return tok;
  }

  // Handle hex strings
  if (!is_nan && tok_str.size() > 2 && tok_str[0] == '0' && tok_str[1] == 'x') {
    auto tok = MakeUnique<Token>(TokenType::kHex);
    tok->SetStringValue(tok_str);
    return tok;
  }

  bool is_double = false;
  if (is_nan) {
    is_double = true;
  } else {
    for (const char ch : tok_str) {
      if (ch == '.') {
        is_double = true;
        break;
      }
    }
  }

  std::unique_ptr<Token> tok;

  char* final_pos = nullptr;
  if (is_double) {
    tok = MakeUnique<Token>(TokenType::kDouble);

    double val = strtod(tok_str.c_str(), &final_pos);
    tok->SetDoubleValue(val);
  } else {
    tok = MakeUnique<Token>(TokenType::kInteger);

    uint64_t val = uint64_t(std::strtoull(tok_str.c_str(), &final_pos, 10));
    tok->SetUint64Value(static_cast<uint64_t>(val));
  }
  if (tok_str.size() > 1 && tok_str[0] == '-')
    tok->SetNegative();

  tok->SetOriginalString(
      tok_str.substr(0, static_cast<size_t>(final_pos - tok_str.c_str())));

  // If the number isn't the whole token then move back so we can then parse
  // the string portion.
  auto diff = size_t(final_pos - tok_str.c_str());
  if (diff > 0)
    current_position_ -= tok_str.length() - diff;

  return tok;
}

std::string Tokenizer::ExtractToNext(const std::string& str) {
  size_t pos = data_.find(str, current_position_);
  std::string ret;
  if (pos == std::string::npos) {
    ret = data_.substr(current_position_);
    current_position_ = data_.length();
  } else {
    ret = data_.substr(current_position_, pos - current_position_);
    current_position_ = pos;
  }

  // Account for any new lines in the extracted text so our current line
  // number stays correct.
  for (const char c : ret) {
    if (c == '\n')
      ++current_line_;
  }

  return ret;
}

bool Tokenizer::IsWhitespace(char ch) {
  return ch == '\0' || ch == '\t' || ch == '\r' || ch == 0x0c /* ff */ ||
         ch == ' ';
}

void Tokenizer::SkipWhitespace() {
  while (current_position_ < data_.size() &&
         IsWhitespace(data_[current_position_])) {
    ++current_position_;
  }
}

void Tokenizer::SkipComment() {
  while (current_position_ < data_.length() &&
         data_[current_position_] != '\n') {
    ++current_position_;
  }
}

}  // namespace amber
