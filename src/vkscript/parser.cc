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

#include "src/vkscript/parser.h"

#include <algorithm>
#include <cassert>
#include <limits>
#include <tuple>
#include <utility>
#include <vector>

#include "src/format_parser.h"
#include "src/make_unique.h"
#include "src/shader.h"
#include "src/vkscript/command_parser.h"

namespace amber {
namespace vkscript {
namespace {

uint32_t kDefaultFrameBufferSize = 250;
const char kDefaultPipelineName[] = "vk_pipeline";

}  // namespace

Parser::Parser() : amber::Parser() {}

Parser::~Parser() = default;

std::string Parser::make_error(const Tokenizer& tokenizer,
                               const std::string& err) {
  return std::to_string(tokenizer.GetCurrentLine()) + ": " + err;
}

Result Parser::Parse(const std::string& input) {
  SectionParser section_parser;
  Result r = section_parser.Parse(input);
  if (!r.IsSuccess())
    return r;

  r = GenerateDefaultPipeline(section_parser);
  if (!r.IsSuccess())
    return r;

  for (const auto& section : section_parser.Sections()) {
    r = ProcessSection(section);
    if (!r.IsSuccess())
      return r;
  }

  return {};
}

Result Parser::GenerateDefaultPipeline(const SectionParser& section_parser) {
  // Generate a pipeline for VkScript.
  PipelineType pipeline_type = PipelineType::kCompute;
  for (const auto& section : section_parser.Sections()) {
    if (!SectionParser::HasShader(section.section_type))
      continue;

    if (section.shader_type != kShaderTypeCompute) {
      pipeline_type = PipelineType::kGraphics;
      break;
    }
  }

  auto new_pipeline = MakeUnique<Pipeline>(pipeline_type);
  auto* pipeline = new_pipeline.get();
  pipeline->SetName(kDefaultPipelineName);
  pipeline->SetFramebufferWidth(kDefaultFrameBufferSize);
  pipeline->SetFramebufferHeight(kDefaultFrameBufferSize);

  Result r = script_->AddPipeline(std::move(new_pipeline));
  if (!r.IsSuccess())
    return r;

  // Generate and add a framebuffer
  auto color_buf = pipeline->GenerateDefaultColorAttachmentBuffer();
  r = pipeline->AddColorAttachment(color_buf.get(), 0);
  if (!r.IsSuccess())
    return r;

  r = script_->AddBuffer(std::move(color_buf));
  if (!r.IsSuccess())
    return r;

  return {};
}

Result Parser::ProcessSection(const SectionParser::Section& section) {
  // Should never get here, but skip it anyway.
  if (section.section_type == NodeType::kComment)
    return {};

  if (SectionParser::HasShader(section.section_type))
    return ProcessShaderBlock(section);
  if (section.section_type == NodeType::kRequire)
    return ProcessRequireBlock(section);
  if (section.section_type == NodeType::kIndices)
    return ProcessIndicesBlock(section);
  if (section.section_type == NodeType::kVertexData)
    return ProcessVertexDataBlock(section);
  if (section.section_type == NodeType::kTest)
    return ProcessTestBlock(section);

  return Result("Unknown node type ....");
}

Result Parser::ProcessShaderBlock(const SectionParser::Section& section) {
  assert(SectionParser::HasShader(section.section_type));

  auto shader = MakeUnique<Shader>(section.shader_type);
  // Generate a unique name for the shader.
  shader->SetName("vk_shader_" + std::to_string(script_->GetShaders().size()));
  shader->SetFormat(section.format);
  shader->SetData(section.contents);

  Result r = script_->GetPipeline(kDefaultPipelineName)
                 ->AddShader(shader.get(), shader->GetType());
  if (!r.IsSuccess())
    return r;

  r = script_->AddShader(std::move(shader));
  if (!r.IsSuccess())
    return r;

  return {};
}

Result Parser::ProcessRequireBlock(const SectionParser::Section& section) {
  Tokenizer tokenizer(section.contents);
  tokenizer.SetCurrentLine(section.starting_line_number + 1);

  for (auto token = tokenizer.NextToken(); !token->IsEOS();
       token = tokenizer.NextToken()) {
    if (token->IsEOL())
      continue;
    if (!token->IsString()) {
      return Result(make_error(
          tokenizer,
          "Invalid token in requirements block: " + token->ToOriginalString()));
    }

    std::string str = token->AsString();
    if (script_->IsKnownFeature(str)) {
      script_->AddRequiredFeature(str);
    } else if (str == Pipeline::kGeneratedColorBuffer) {
      token = tokenizer.NextToken();
      if (!token->IsString())
        return Result(make_error(tokenizer, "Missing framebuffer format"));

      FormatParser fmt_parser;
      auto fmt = fmt_parser.Parse(token->AsString());
      if (fmt == nullptr) {
        return Result(
            make_error(tokenizer, "Failed to parse framebuffer format: " +
                                      token->ToOriginalString()));
      }
      script_->GetPipeline(kDefaultPipelineName)
          ->GetColorAttachments()[0]
          .buffer->AsFormatBuffer()
          ->SetFormat(std::move(fmt));

    } else if (str == "depthstencil") {
      token = tokenizer.NextToken();
      if (!token->IsString())
        return Result(make_error(tokenizer, "Missing depthStencil format"));

      FormatParser fmt_parser;
      auto fmt = fmt_parser.Parse(token->AsString());
      if (fmt == nullptr) {
        return Result(
            make_error(tokenizer, "Failed to parse depthstencil format: " +
                                      token->ToOriginalString()));
      }

      auto* pipeline = script_->GetPipeline(kDefaultPipelineName);
      if (pipeline->GetDepthBuffer().buffer != nullptr)
        return Result("Only one depthstencil command allowed");

      // Generate and add a depth buffer
      auto depth_buf = pipeline->GenerateDefaultDepthAttachmentBuffer();
      depth_buf->AsFormatBuffer()->SetFormat(std::move(fmt));
      Result r = pipeline->SetDepthBuffer(depth_buf.get());
      if (!r.IsSuccess())
        return r;

      r = script_->AddBuffer(std::move(depth_buf));
      if (!r.IsSuccess())
        return r;

    } else if (str == "fence_timeout") {
      token = tokenizer.NextToken();
      if (!token->IsInteger())
        return Result(make_error(tokenizer, "Missing fence_timeout value"));

      script_->GetEngineData().fence_timeout_ms = token->AsUint32();

    } else if (str == "fbsize") {
      auto* pipeline = script_->GetPipeline(kDefaultPipelineName);

      token = tokenizer.NextToken();
      if (token->IsEOL() || token->IsEOS()) {
        return Result(make_error(
            tokenizer, "Missing width and height for fbsize command"));
      }
      if (!token->IsInteger()) {
        return Result(
            make_error(tokenizer, "Invalid width for fbsize command"));
      }

      pipeline->SetFramebufferWidth(token->AsUint32());

      token = tokenizer.NextToken();
      if (token->IsEOL() || token->IsEOS()) {
        return Result(
            make_error(tokenizer, "Missing height for fbsize command"));
      }
      if (!token->IsInteger()) {
        return Result(
            make_error(tokenizer, "Invalid height for fbsize command"));
      }

      pipeline->SetFramebufferHeight(token->AsUint32());

    } else {
      auto it = std::find_if(str.begin(), str.end(),
                             [](char c) { return !(isalnum(c) || c == '_'); });
      if (it != str.end()) {
        return Result(
            make_error(tokenizer, "Unknown feature or extension: " + str));
      }

      script_->AddRequiredExtension(str);
    }

    token = tokenizer.NextToken();
    if (!token->IsEOS() && !token->IsEOL()) {
      return Result(make_error(
          tokenizer, "Failed to parse requirements block: invalid token: " +
                         token->ToOriginalString()));
    }
  }
  return {};
}

Result Parser::ProcessIndicesBlock(const SectionParser::Section& section) {
  std::vector<Value> indices;

  Tokenizer tokenizer(section.contents);
  tokenizer.SetCurrentLine(section.starting_line_number);
  for (auto token = tokenizer.NextToken(); !token->IsEOS();
       token = tokenizer.NextToken()) {
    if (token->IsEOL())
      continue;

    if (!token->IsInteger())
      return Result(make_error(tokenizer, "Invalid value in indices block: " +
                                              token->ToOriginalString()));
    if (token->AsUint64() >
        static_cast<uint64_t>(std::numeric_limits<uint16_t>::max())) {
      return Result(make_error(tokenizer, "Value too large in indices block: " +
                                              token->ToOriginalString()));
    }

    indices.push_back(Value());
    indices.back().SetIntValue(token->AsUint16());
  }

  if (!indices.empty()) {
    DatumType type;
    type.SetType(DataType::kUint32);

    auto b = MakeUnique<DataBuffer>(BufferType::kIndex);
    auto* buf = b.get();
    b->SetName("indices");
    b->SetDatumType(type);
    b->SetData(std::move(indices));
    Result r = script_->AddBuffer(std::move(b));
    if (!r.IsSuccess())
      return r;

    script_->GetPipeline(kDefaultPipelineName)->SetIndexBuffer(buf);
  }

  return {};
}

Result Parser::ProcessVertexDataBlock(const SectionParser::Section& section) {
  Tokenizer tokenizer(section.contents);
  tokenizer.SetCurrentLine(section.starting_line_number);

  // Skip blank and comment lines
  auto token = tokenizer.NextToken();
  while (token->IsEOL())
    token = tokenizer.NextToken();

  // Skip empty vertex data blocks
  if (token->IsEOS())
    return {};

  // Process the header line.
  struct Header {
    uint8_t location;
    std::unique_ptr<Format> format;
  };
  std::vector<Header> headers;
  while (!token->IsEOL() && !token->IsEOS()) {
    // Because of the way the tokenizer works we'll see a number then a string
    // the string will start with a slash which we have to remove.
    if (!token->IsInteger()) {
      return Result(
          make_error(tokenizer, "Unable to process vertex data header: " +
                                    token->ToOriginalString()));
    }

    uint8_t loc = token->AsUint8();

    token = tokenizer.NextToken();
    if (!token->IsString()) {
      return Result(
          make_error(tokenizer, "Unable to process vertex data header: " +
                                    token->ToOriginalString()));
    }

    std::string fmt_name = token->AsString();
    if (fmt_name.size() < 2)
      return Result(make_error(tokenizer, "Vertex data format too short: " +
                                              token->ToOriginalString()));

    FormatParser parser;
    auto fmt = parser.Parse(fmt_name.substr(1, fmt_name.length()));
    if (!fmt) {
      return Result(
          make_error(tokenizer, "Invalid format in vertex data header: " +
                                    fmt_name.substr(1, fmt_name.length())));
    }

    headers.push_back({loc, std::move(fmt)});

    token = tokenizer.NextToken();
  }

  // Create a number of vectors equal to the number of headers.
  std::vector<std::vector<Value>> values;
  values.resize(headers.size());

  // Process data lines
  for (; !token->IsEOS(); token = tokenizer.NextToken()) {
    if (token->IsEOL())
      continue;

    for (size_t j = 0; j < headers.size(); ++j) {
      const auto& header = headers[j];
      auto& value_data = values[j];

      if (header.format->GetPackSize() > 0) {
        if (!token->IsHex()) {
          return Result(
              make_error(tokenizer, "Invalid packed value in Vertex Data: " +
                                        token->ToOriginalString()));
        }

        Value v;
        v.SetIntValue(token->AsHex());
        value_data.push_back(v);
      } else {
        auto& comps = header.format->GetComponents();
        for (size_t i = 0; i < comps.size();
             ++i, token = tokenizer.NextToken()) {
          if (token->IsEOS() || token->IsEOL()) {
            return Result(make_error(tokenizer,
                                     "Too few cells in given vertex data row"));
          }

          auto& comp = comps[i];

          Value v;
          if (comp.mode == FormatMode::kUFloat ||
              comp.mode == FormatMode::kSFloat) {
            Result r = token->ConvertToDouble();
            if (!r.IsSuccess())
              return r;

            v.SetDoubleValue(token->AsDouble());
          } else if (token->IsInteger()) {
            v.SetIntValue(token->AsUint64());
          } else {
            return Result(make_error(tokenizer, "Invalid vertex data value: " +
                                                    token->ToOriginalString()));
          }

          value_data.push_back(v);
        }
      }
    }
  }

  auto* pipeline = script_->GetPipeline(kDefaultPipelineName);
  for (size_t i = 0; i < headers.size(); ++i) {
    auto buffer = MakeUnique<FormatBuffer>(BufferType::kVertex);
    auto* buf = buffer.get();
    buffer->SetName("Vertices" + std::to_string(i));
    buffer->SetFormat(std::move(headers[i].format));
    buffer->SetLocation(headers[i].location);
    buffer->SetData(std::move(values[i]));
    script_->AddBuffer(std::move(buffer));

    pipeline->AddVertexBuffer(buf, headers[i].location);
  }

  return {};
}

Result Parser::ProcessTestBlock(const SectionParser::Section& section) {
  auto* pipeline = script_->GetPipeline(kDefaultPipelineName);
  CommandParser cp(script_.get(), pipeline, section.starting_line_number + 1,
                   section.contents);
  Result r = cp.Parse();
  if (!r.IsSuccess())
    return r;

  script_->SetCommands(cp.TakeCommands());

  return {};
}

}  // namespace vkscript
}  // namespace amber
