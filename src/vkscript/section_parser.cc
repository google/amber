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

#include <cassert>
#include <iostream>
#include <sstream>
#include <string>

#include "src/make_unique.h"
#include "src/shader_data.h"

namespace amber {
namespace vkscript {

// static
bool SectionParser::HasShader(const NodeType type) {
  return type == NodeType::kShader;
}

SectionParser::SectionParser() = default;

SectionParser::~SectionParser() = default;

Result SectionParser::Parse(const std::string& data) {
  Result result = SplitSections(data);
  if (!result.IsSuccess())
    return result;
  return {};
}

Result SectionParser::NameToNodeType(const std::string& data,
                                     NodeType* section_type,
                                     ShaderType* shader_type,
                                     ShaderFormat* fmt) const {
  assert(section_type);
  assert(shader_type);
  assert(fmt);

  *fmt = kShaderFormatText;

  std::string name;
  size_t pos = data.rfind(" spirv hex");
  if (pos != std::string::npos) {
    *fmt = kShaderFormatSpirvHex;
    name = data.substr(0, pos);
  } else {
    pos = data.rfind(" spirv");
    if (pos != std::string::npos) {
      *fmt = kShaderFormatSpirvAsm;
      name = data.substr(0, pos);
    } else {
      name = data;
    }
  }

  pos = data.rfind(" passthrough");
  if (pos != std::string::npos) {
    *fmt = kShaderFormatDefault;
    name = data.substr(0, pos);
  }

  if (name == "comment") {
    *section_type = NodeType::kComment;
  } else if (name == "indices") {
    *section_type = NodeType::kIndices;
  } else if (name == "require") {
    *section_type = NodeType::kRequire;
  } else if (name == "test") {
    *section_type = NodeType::kTest;
  } else if (name == "vertex data") {
    *section_type = NodeType::kVertexData;
  } else if (name == "compute shader") {
    *section_type = NodeType::kShader;
    *shader_type = kShaderTypeCompute;
    if (*fmt == kShaderFormatText)
      *fmt = kShaderFormatGlsl;
  } else if (name == "fragment shader") {
    *section_type = NodeType::kShader;
    *shader_type = kShaderTypeFragment;
    if (*fmt == kShaderFormatText)
      *fmt = kShaderFormatGlsl;
  } else if (name == "geometry shader") {
    *section_type = NodeType::kShader;
    *shader_type = kShaderTypeGeometry;
    if (*fmt == kShaderFormatText)
      *fmt = kShaderFormatGlsl;
  } else if (name == "tessellation control shader") {
    *section_type = NodeType::kShader;
    *shader_type = kShaderTypeTessellationControl;
    if (*fmt == kShaderFormatText)
      *fmt = kShaderFormatGlsl;
  } else if (name == "tessellation evaluation shader") {
    *section_type = NodeType::kShader;
    *shader_type = kShaderTypeTessellationEvaluation;
    if (*fmt == kShaderFormatText)
      *fmt = kShaderFormatGlsl;
  } else if (name == "vertex shader") {
    *section_type = NodeType::kShader;
    *shader_type = kShaderTypeVertex;
    if (*fmt == kShaderFormatText)
      *fmt = kShaderFormatGlsl;
  } else {
    return Result("Invalid name: " + data);
  }

  if (!SectionParser::HasShader(*section_type) &&
      (*fmt == kShaderFormatGlsl || *fmt == kShaderFormatSpirvAsm ||
       *fmt == kShaderFormatSpirvHex)) {
    return Result("Invalid source format: " + data);
  }

  return {};
}

void SectionParser::AddSection(NodeType section_type,
                               ShaderType shader_type,
                               ShaderFormat fmt,
                               size_t line_count,
                               const std::string& contents) {
  if (section_type == NodeType::kComment)
    return;

  if (fmt == kShaderFormatDefault) {
    sections_.push_back({section_type, shader_type, kShaderFormatSpirvAsm,
                         line_count, kPassThroughShader});
    return;
  }

  size_t size = contents.size();
  while (size > 0) {
    if (contents[size - 1] == '\n' || contents[size - 1] == '\r') {
      --size;
      continue;
    }
    break;
  }

  sections_.push_back(
      {section_type, shader_type, fmt, line_count, contents.substr(0, size)});
}

Result SectionParser::SplitSections(const std::string& data) {
  std::stringstream ss(data);
  size_t line_count = 0;
  size_t section_start = 0;
  bool in_section = false;

  NodeType current_type = NodeType::kComment;
  ShaderType current_shader = kShaderTypeVertex;
  ShaderFormat current_fmt = kShaderFormatText;
  std::string section_contents;

  for (std::string line; std::getline(ss, line);) {
    ++line_count;

    if (!in_section) {
      if (line.empty() || line[0] == '#' || line == "\r")
        continue;

      if (line[0] != '[')
        return Result(std::to_string(line_count) + ": Invalid character");

      section_start = line_count;
      in_section = true;
    }

    if (line.empty()) {
      section_contents += "\n";
      continue;
    }

    if (line[0] == '[') {
      AddSection(current_type, current_shader, current_fmt, section_start,
                 section_contents);
      section_start = line_count;
      section_contents = "";

      size_t name_end = line.rfind("]");
      if (name_end == std::string::npos)
        return Result(std::to_string(line_count) + ": Missing section close");

      std::string name = line.substr(1, name_end - 1);

      Result r =
          NameToNodeType(name, &current_type, &current_shader, &current_fmt);
      if (!r.IsSuccess())
        return Result(std::to_string(line_count) + ": " + r.Error());
    } else {
      section_contents += line + "\n";
    }
  }
  AddSection(current_type, current_shader, current_fmt, section_start,
             section_contents);

  return {};
}

}  // namespace vkscript
}  // namespace amber
