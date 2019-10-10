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

#ifndef SRC_VKSCRIPT_DATUM_TYPE_PARSER_H_
#define SRC_VKSCRIPT_DATUM_TYPE_PARSER_H_

#include <memory>
#include <string>

#include "amber/result.h"
#include "src/type.h"

namespace amber {
namespace vkscript {

/// Parses a data description on the VkScript format.
class DatumTypeParser {
 public:
  DatumTypeParser();
  ~DatumTypeParser();

  std::unique_ptr<type::Type> Parse(const std::string& data);
};

}  // namespace vkscript
}  // namespace amber

#endif  // SRC_VKSCRIPT_DATUM_TYPE_PARSER_H_
