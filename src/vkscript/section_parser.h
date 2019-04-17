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

#ifndef SRC_VKSCRIPT_SECTION_PARSER_H_
#define SRC_VKSCRIPT_SECTION_PARSER_H_

#include <memory>
#include <string>
#include <vector>

#include "amber/result.h"
#include "amber/shader_info.h"

namespace amber {
namespace vkscript {

enum class NodeType : uint8_t {
  kComment = 0,
  kShader,
  kIndices,
  kVertexData,
  kRequire,
  kTest,
};

/// Parses the VkScript into the general sections. This includes things like
/// the [test], [indices], [vertex data], etc.
class SectionParser {
 public:
  /// Structure describing a single section of the VkScript document.
  struct Section {
    NodeType section_type;
    ShaderType shader_type;  // Only valid when section_type == kShader
    ShaderFormat format;
    size_t starting_line_number;
    std::string contents;
  };

  static bool HasShader(const NodeType type);

  SectionParser();
  ~SectionParser();

  Result Parse(const std::string& data);
  const std::vector<Section>& Sections() const { return sections_; }

  Result SplitSectionsForTesting(const std::string& data) {
    return SplitSections(data);
  }

  Result NameToNodeTypeForTesting(const std::string& name,
                                  NodeType* section_type,
                                  ShaderType* shader_type,
                                  ShaderFormat* fmt) const {
    return NameToNodeType(name, section_type, shader_type, fmt);
  }

 private:
  Result SplitSections(const std::string& data);
  void AddSection(NodeType section_type,
                  ShaderType shader_type,
                  ShaderFormat fmt,
                  size_t starting_line_number,
                  const std::string& contents);
  Result NameToNodeType(const std::string& name,
                        NodeType* section_type,
                        ShaderType* shader_type,
                        ShaderFormat* fmt) const;

  std::vector<Section> sections_;
};

}  // namespace vkscript
}  // namespace amber

#endif  // SRC_VKSCRIPT_SECTION_PARSER_H_
