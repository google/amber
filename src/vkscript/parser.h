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

#include <string>

#include "amber/result.h"
#include "src/parser.h"
#include "src/vkscript/script.h"
#include "src/vkscript/section_parser.h"

namespace amber {
namespace vkscript {

class Parser : public amber::Parser {
 public:
  Parser();
  ~Parser() override;

  // amber::Parser
  Result Parse(const std::string& data) override;
  const amber::Script* GetScript() const override { return &script_; }

  Result ProcessRequireBlockForTesting(const std::string& block) {
    return ProcessRequireBlock(block);
  }
  Result ProcessIndicesBlockForTesting(const std::string& block) {
    return ProcessIndicesBlock(block);
  }
  Result ProcessVertexDataBlockForTesting(const std::string& block) {
    return ProcessVertexDataBlock(block);
  }
  Result ProcessTestBlockForTesting(const std::string& block) {
    return ProcessTestBlock(block);
  }

 private:
  Result ProcessSection(const SectionParser::Section& section);
  Result ProcessShaderBlock(const SectionParser::Section& section);
  Result ProcessRequireBlock(const std::string&);
  Result ProcessIndicesBlock(const std::string&);
  Result ProcessVertexDataBlock(const std::string&);
  Result ProcessTestBlock(const std::string&);

  vkscript::Script script_;
};

}  // namespace vkscript
}  // namespace amber

#endif  // SRC_VKSCRIPT_PARSER_H_
