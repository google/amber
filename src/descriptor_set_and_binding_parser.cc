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

#include "src/descriptor_set_and_binding_parser.h"

#include <iostream>
#include <limits>

#include "src/tokenizer.h"

namespace amber {

DescriptorSetAndBindingParser::DescriptorSetAndBindingParser() = default;

DescriptorSetAndBindingParser::~DescriptorSetAndBindingParser() = default;

Result DescriptorSetAndBindingParser::Parse(const std::string& buffer_id) {
  Tokenizer t(buffer_id);
  auto token = t.NextToken();
  if (token->IsInteger()) {
    if (token->AsInt32() < 0) {
      return Result(
          "Descriptor set and binding for a buffer must be non-negative "
          "integer, but you gave: " +
          token->ToOriginalString());
    }

    uint32_t val = token->AsUint32();
    token = t.NextToken();
    if (token->IsEOS() || token->IsEOL()) {
      descriptor_set_ = 0;
      binding_ = val;
      return {};
    }

    descriptor_set_ = val;
  } else {
    descriptor_set_ = 0;
  }

  if (!token->IsString())
    return Result("Invalid buffer id: " + buffer_id);

  auto& str = token->AsString();
  if (str.size() < 2 || str[0] != ':')
    return Result("Invalid buffer id: " + buffer_id);

  auto substr = str.substr(1, str.size());
  // Validate all characters are integers.
  for (size_t i = 0; i < substr.size(); ++i) {
    if (substr[i] < '0' || substr[i] > '9') {
      return Result(
          "Binding for a buffer must be non-negative integer, "
          "but you gave: " +
          substr);
    }
  }

  uint64_t binding_val = strtoul(substr.c_str(), nullptr, 10);
  if (binding_val > std::numeric_limits<uint32_t>::max())
    return Result("binding value too large in probe ssbo command: " +
                  token->ToOriginalString());
  if (static_cast<int32_t>(binding_val) < 0) {
    return Result(
        "Binding for a buffer must be non-negative integer, but you gave: " +
        token->ToOriginalString());
  }

  binding_ = static_cast<uint32_t>(binding_val);
  return {};
}

}  // namespace amber
