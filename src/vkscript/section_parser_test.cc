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

#include "src/vkscript/section_parser.h"

#include "gtest/gtest.h"
#include "src/shader_data.h"

namespace amber {
namespace vkscript {

using SectionParserTest = testing::Test;

TEST_F(SectionParserTest, SectionParserCommentSection) {
  std::string input = "[comment]\nThis is the comment body\n.Lots of Text.";

  SectionParser p;
  Result r = p.SplitSectionsForTesting(input);
  ASSERT_TRUE(r.IsSuccess());

  auto sections = p.Sections();
  EXPECT_TRUE(sections.empty());
}

TEST_F(SectionParserTest, ParseShaderGlslVertex) {
  std::string shader = R"(#version 430
void main() {
})";
  std::string input = "[vertex shader]\n" + shader;

  SectionParser p;
  Result r = p.SplitSectionsForTesting(input);
  ASSERT_TRUE(r.IsSuccess());

  auto sections = p.Sections();
  ASSERT_EQ(1U, sections.size());
  EXPECT_EQ(NodeType::kShader, sections[0].section_type);
  EXPECT_EQ(kShaderTypeVertex, sections[0].shader_type);
  EXPECT_EQ(kShaderFormatGlsl, sections[0].format);
  EXPECT_EQ(shader, sections[0].contents);
}

TEST_F(SectionParserTest, ParseShaderGlslVertexPassthrough) {
  std::string input = "[vertex shader passthrough]";

  SectionParser p;
  Result r = p.SplitSectionsForTesting(input);
  ASSERT_TRUE(r.IsSuccess());

  auto sections = p.Sections();
  ASSERT_EQ(1U, sections.size());
  EXPECT_EQ(NodeType::kShader, sections[0].section_type);
  EXPECT_EQ(kShaderTypeVertex, sections[0].shader_type);
  EXPECT_EQ(kShaderFormatSpirvAsm, sections[0].format);
  EXPECT_EQ(kPassThroughShader, sections[0].contents);
}

TEST_F(SectionParserTest, SectionParserMultipleSections) {
  std::string input = R"(
[comment]
This is a test.

[vertex shader passthrough]
[fragment shader]
#version 430
void main() {}

[geometry shader]
float4 main() {}

[comment]
Another comment section.
Multi line.

[indices]
1 2 3 4
5 6 7 8
[test]
test body.)";

  SectionParser p;
  Result r = p.SplitSectionsForTesting(input);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto sections = p.Sections();
  ASSERT_EQ(5U, sections.size());

  // Passthrough vertext shader
  EXPECT_EQ(NodeType::kShader, sections[0].section_type);
  EXPECT_EQ(kShaderTypeVertex, sections[0].shader_type);
  EXPECT_EQ(kShaderFormatSpirvAsm, sections[0].format);
  EXPECT_EQ(kPassThroughShader, sections[0].contents);

  // fragment shader
  EXPECT_EQ(NodeType::kShader, sections[1].section_type);
  EXPECT_EQ(kShaderTypeFragment, sections[1].shader_type);
  EXPECT_EQ(kShaderFormatGlsl, sections[1].format);
  EXPECT_EQ("#version 430\nvoid main() {}", sections[1].contents);

  // geometry shader
  EXPECT_EQ(NodeType::kShader, sections[2].section_type);
  EXPECT_EQ(kShaderTypeGeometry, sections[2].shader_type);
  EXPECT_EQ(kShaderFormatGlsl, sections[2].format);
  EXPECT_EQ("float4 main() {}", sections[2].contents);

  // indices
  EXPECT_EQ(NodeType::kIndices, sections[3].section_type);
  EXPECT_EQ(kShaderFormatText, sections[3].format);
  EXPECT_EQ("1 2 3 4\n5 6 7 8", sections[3].contents);

  // test
  EXPECT_EQ(NodeType::kTest, sections[4].section_type);
  EXPECT_EQ(kShaderFormatText, sections[4].format);
  EXPECT_EQ("test body.", sections[4].contents);
}

TEST_F(SectionParserTest, SkipCommentLinesOutsideSections) {
  std::string input = "# comment 1\n#comment 2\r\n[vertex shader]";

  SectionParser p;
  Result r = p.SplitSectionsForTesting(input);
  ASSERT_TRUE(r.IsSuccess());

  auto sections = p.Sections();
  ASSERT_EQ(1U, sections.size());
  EXPECT_EQ(NodeType::kShader, sections[0].section_type);
  EXPECT_EQ(kShaderTypeVertex, sections[0].shader_type);
  EXPECT_EQ(kShaderFormatGlsl, sections[0].format);
  EXPECT_EQ("", sections[0].contents);
}

TEST_F(SectionParserTest, SkipBlankLinesOutsideSections) {
  std::string input = "\n\r\n[vertex shader]";

  SectionParser p;
  Result r = p.SplitSectionsForTesting(input);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto sections = p.Sections();
  ASSERT_EQ(1U, sections.size());
  EXPECT_EQ(NodeType::kShader, sections[0].section_type);
  EXPECT_EQ(kShaderTypeVertex, sections[0].shader_type);
  EXPECT_EQ(kShaderFormatGlsl, sections[0].format);
  EXPECT_EQ("", sections[0].contents);
}

TEST_F(SectionParserTest, UnknownTextOutsideSection) {
  std::string input = "Invalid Text";

  SectionParser p;
  Result r = p.SplitSectionsForTesting(input);
  EXPECT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Invalid character", r.Error());
}

TEST_F(SectionParserTest, UnknownSectionName) {
  std::string input = "[Invalid Section]";

  SectionParser p;
  Result r = p.SplitSectionsForTesting(input);
  EXPECT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Invalid name: Invalid Section", r.Error());
}

TEST_F(SectionParserTest, MissingSectionClose) {
  std::string input = "[vertex shader\nMore Content";

  SectionParser p;
  Result r = p.SplitSectionsForTesting(input);
  EXPECT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: Missing section close", r.Error());
}

TEST_F(SectionParserTest, NameToNodeType) {
  struct {
    const char* name;
    NodeType section_type;
    ShaderType shader_type;
    ShaderFormat fmt;
  } name_cases[] = {
      {"comment", NodeType::kComment, kShaderTypeVertex, kShaderFormatText},
      {"indices", NodeType::kIndices, kShaderTypeVertex, kShaderFormatText},
      {"require", NodeType::kRequire, kShaderTypeVertex, kShaderFormatText},
      {"test", NodeType::kTest, kShaderTypeVertex, kShaderFormatText},
      {"vertex data", NodeType::kVertexData, kShaderTypeVertex,
       kShaderFormatText},

      {"compute shader", NodeType::kShader, kShaderTypeCompute,
       kShaderFormatGlsl},
      {"fragment shader", NodeType::kShader, kShaderTypeFragment,
       kShaderFormatGlsl},
      {"geometry shader", NodeType::kShader, kShaderTypeGeometry,
       kShaderFormatGlsl},
      {"tessellation control shader", NodeType::kShader,
       kShaderTypeTessellationControl, kShaderFormatGlsl},
      {"tessellation evaluation shader", NodeType::kShader,
       kShaderTypeTessellationEvaluation, kShaderFormatGlsl},
      {"vertex shader", NodeType::kShader, kShaderTypeVertex,
       kShaderFormatGlsl},
      {"compute shader spirv", NodeType::kShader, kShaderTypeCompute,
       kShaderFormatSpirvAsm},
      {"fragment shader spirv", NodeType::kShader, kShaderTypeFragment,
       kShaderFormatSpirvAsm},
      {"geometry shader spirv", NodeType::kShader, kShaderTypeGeometry,
       kShaderFormatSpirvAsm},
      {"tessellation control shader spirv", NodeType::kShader,
       kShaderTypeTessellationControl, kShaderFormatSpirvAsm},
      {"tessellation evaluation shader spirv", NodeType::kShader,
       kShaderTypeTessellationEvaluation, kShaderFormatSpirvAsm},
      {"vertex shader spirv", NodeType::kShader, kShaderTypeVertex,
       kShaderFormatSpirvAsm},
      {"compute shader spirv hex", NodeType::kShader, kShaderTypeCompute,
       kShaderFormatSpirvHex},
      {"fragment shader spirv hex", NodeType::kShader, kShaderTypeFragment,
       kShaderFormatSpirvHex},
      {"geometry shader spirv hex", NodeType::kShader, kShaderTypeGeometry,
       kShaderFormatSpirvHex},
      {"tessellation control shader spirv hex", NodeType::kShader,
       kShaderTypeTessellationControl, kShaderFormatSpirvHex},
      {"tessellation evaluation shader spirv hex", NodeType::kShader,
       kShaderTypeTessellationEvaluation, kShaderFormatSpirvHex},
      {"vertex shader spirv hex", NodeType::kShader, kShaderTypeVertex,
       kShaderFormatSpirvHex},
      {"vertex shader passthrough", NodeType::kShader, kShaderTypeVertex,
       kShaderFormatDefault}};

  for (auto name_case : name_cases) {
    NodeType section_type = NodeType::kTest;
    ShaderType shader_type = kShaderTypeVertex;
    ShaderFormat fmt = kShaderFormatText;
    SectionParser p;
    Result r = p.NameToNodeTypeForTesting(name_case.name, &section_type,
                                          &shader_type, &fmt);

    ASSERT_TRUE(r.IsSuccess()) << r.Error();
    EXPECT_EQ(name_case.section_type, section_type) << name_case.name;
    EXPECT_EQ(name_case.shader_type, shader_type) << name_case.name;
    EXPECT_EQ(name_case.fmt, fmt) << name_case.name;
  }
}

TEST_F(SectionParserTest, NameToNodeTypeInvalidName) {
  NodeType section_type = NodeType::kTest;
  ShaderType shader_type = kShaderTypeVertex;
  ShaderFormat fmt = kShaderFormatText;
  SectionParser p;
  Result r = p.NameToNodeTypeForTesting("InvalidName", &section_type,
                                        &shader_type, &fmt);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("Invalid name: InvalidName", r.Error());
}

TEST_F(SectionParserTest, NameToSectionInvalidSuffix) {
  struct {
    const char* name;
  } cases[] = {{"comment spirv"},     {"indices spirv"},
               {"require spirv"},     {"test spirv"},
               {"vertex data spirv"}, {"comment spirv hex"},
               {"indices spirv hex"}, {"require spirv hex"},
               {"test spirv hex"},    {"vertex data spirv hex"}};

  for (auto name_case : cases) {
    NodeType section_type = NodeType::kTest;
    ShaderType shader_type = kShaderTypeVertex;
    ShaderFormat fmt = kShaderFormatText;
    SectionParser p;

    Result r = p.NameToNodeTypeForTesting(name_case.name, &section_type,
                                          &shader_type, &fmt);
    ASSERT_FALSE(r.IsSuccess()) << name_case.name;
    EXPECT_EQ("Invalid source format: " + std::string(name_case.name),
              r.Error());
  }
}

TEST_F(SectionParserTest, HasShader) {
  EXPECT_TRUE(SectionParser::HasShader(NodeType::kShader));
}

TEST_F(SectionParserTest, HasNoShader) {
  const NodeType false_types[] = {NodeType::kComment, NodeType::kTest,
                                  NodeType::kIndices, NodeType::kVertexData,
                                  NodeType::kRequire};
  for (auto type : false_types) {
    EXPECT_FALSE(SectionParser::HasShader(type));
  }
}

}  // namespace vkscript
}  // namespace amber
