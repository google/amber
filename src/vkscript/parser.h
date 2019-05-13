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

#ifndef SRC_VKSCRIPT_PARSER_H_
#define SRC_VKSCRIPT_PARSER_H_

#include <memory>
#include <string>
#include <utility>

#include "amber/result.h"
#include "src/parser.h"
#include "src/script.h"
#include "src/tokenizer.h"
#include "src/vkscript/section_parser.h"

namespace amber {
namespace vkscript {

/// Parser for the `VkScript` data format.
class Parser : public amber::Parser {
 public:
  Parser();
  ~Parser() override;

  // amber::Parser
  Result Parse(const std::string& data) override;

  void SkipValidationForTest() { skip_validation_for_test_ = true; }

 private:
  bool skip_validation_for_test_ = false;

  std::string make_error(const Tokenizer& tokenizer, const std::string& err);
  Result GenerateDefaultPipeline(const SectionParser& section_parser);
  Result ProcessSection(const SectionParser::Section& section);
  Result ProcessShaderBlock(const SectionParser::Section& section);
  Result ProcessRequireBlock(const SectionParser::Section& section);
  Result ProcessIndicesBlock(const SectionParser::Section& section);
  Result ProcessVertexDataBlock(const SectionParser::Section& section);
  Result ProcessTestBlock(const SectionParser::Section& section);
};

}  // namespace vkscript
}  // namespace amber

#endif  // SRC_VKSCRIPT_PARSER_H_
