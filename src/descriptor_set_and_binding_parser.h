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

#ifndef SRC_DESCRIPTOR_SET_AND_BINDING_PARSER_H_
#define SRC_DESCRIPTOR_SET_AND_BINDING_PARSER_H_

#include <string>

#include "amber/result.h"

namespace amber {

/// Class for descriptor set and binding parser.
class DescriptorSetAndBindingParser {
 public:
  DescriptorSetAndBindingParser();
  ~DescriptorSetAndBindingParser();

  /// Run the parser against |buffer_id|. The result is success if the parse
  /// completes correctly. |buffer_id| must be non-empty string. It must
  /// be a single non-negative integer or two those integers separated by
  /// ':'. For example, ":0", "1", and "2:3" are valid strings for |buffer_id|,
  /// but "", "-4", ":-5", ":", "a", and "b:c" are invalid strings for
  /// |buffer_id|.
  Result Parse(const std::string& buffer_id);

  /// Return descriptor set that is the result of Parse().
  uint32_t GetDescriptorSet() const { return descriptor_set_; }

  /// Return binding that is the result of Parse().
  uint32_t GetBinding() const { return binding_; }

 private:
  uint32_t descriptor_set_ = 0;
  uint32_t binding_ = 0;
};

}  // namespace amber

#endif  // SRC_DESCRIPTOR_SET_AND_BINDING_PARSER_H_
