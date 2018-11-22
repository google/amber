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

#include <string>

#include "amber/result.h"
#include "src/datum_type.h"

namespace amber {
namespace vkscript {

class DatumTypeParser {
 public:
  DatumTypeParser();
  ~DatumTypeParser();

  Result Parse(const std::string& data);
  const DatumType& GetType() const { return type_; }

 private:
  DatumType type_;
};

}  // namespace vkscript
}  // namespace amber

#endif  // SRC_VKSCRIPT_DATUM_TYPE_PARSER_H_
