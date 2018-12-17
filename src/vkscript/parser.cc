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

#include "src/make_unique.h"
#include "src/shader.h"
#include "src/vkscript/command_parser.h"
#include "src/vkscript/format_parser.h"

namespace amber {
namespace vkscript {
namespace {

Feature NameToFeature(const std::string& name) {
  if (name == "robustBufferAccess")
    return Feature::kRobustBufferAccess;
  if (name == "fullDrawIndexUint32")
    return Feature::kFullDrawIndexUint32;
  if (name == "imageCubeArray")
    return Feature::kImageCubeArray;
  if (name == "independentBlend")
    return Feature::kIndependentBlend;
  if (name == "geometryShader")
    return Feature::kGeometryShader;
  if (name == "tessellationShader")
    return Feature::kTessellationShader;
  if (name == "sampleRateShading")
    return Feature::kSampleRateShading;
  if (name == "dualSrcBlend")
    return Feature::kDualSrcBlend;
  if (name == "logicOp")
    return Feature::kLogicOp;
  if (name == "multiDrawIndirect")
    return Feature::kMultiDrawIndirect;
  if (name == "drawIndirectFirstInstance")
    return Feature::kDrawIndirectFirstInstance;
  if (name == "depthClamp")
    return Feature::kDepthClamp;
  if (name == "depthBiasClamp")
    return Feature::kDepthBiasClamp;
  if (name == "fillModeNonSolid")
    return Feature::kFillModeNonSolid;
  if (name == "depthBounds")
    return Feature::kDepthBounds;
  if (name == "wideLines")
    return Feature::kWideLines;
  if (name == "largePoints")
    return Feature::kLargePoints;
  if (name == "alphaToOne")
    return Feature::kAlphaToOne;
  if (name == "multiViewport")
    return Feature::kMultiViewport;
  if (name == "samplerAnisotropy")
    return Feature::kSamplerAnisotropy;
  if (name == "textureCompressionETC2")
    return Feature::kTextureCompressionETC2;
  if (name == "textureCompressionASTC_LDR")
    return Feature::kTextureCompressionASTC_LDR;
  if (name == "textureCompressionBC")
    return Feature::kTextureCompressionBC;
  if (name == "occlusionQueryPrecise")
    return Feature::kOcclusionQueryPrecise;
  if (name == "pipelineStatisticsQuery")
    return Feature::kPipelineStatisticsQuery;
  if (name == "vertexPipelineStoresAndAtomics")
    return Feature::kVertexPipelineStoresAndAtomics;
  if (name == "fragmentStoresAndAtomics")
    return Feature::kFragmentStoresAndAtomics;
  if (name == "shaderTessellationAndGeometryPointSize")
    return Feature::kShaderTessellationAndGeometryPointSize;
  if (name == "shaderImageGatherExtended")
    return Feature::kShaderImageGatherExtended;
  if (name == "shaderStorageImageExtendedFormats")
    return Feature::kShaderStorageImageExtendedFormats;
  if (name == "shaderStorageImageMultisample")
    return Feature::kShaderStorageImageMultisample;
  if (name == "shaderStorageImageReadWithoutFormat")
    return Feature::kShaderStorageImageReadWithoutFormat;
  if (name == "shaderStorageImageWriteWithoutFormat")
    return Feature::kShaderStorageImageWriteWithoutFormat;
  if (name == "shaderUniformBufferArrayDynamicIndexing")
    return Feature::kShaderUniformBufferArrayDynamicIndexing;
  if (name == "shaderSampledImageArrayDynamicIndexing")
    return Feature::kShaderSampledImageArrayDynamicIndexing;
  if (name == "shaderStorageBufferArrayDynamicIndexing")
    return Feature::kShaderStorageBufferArrayDynamicIndexing;
  if (name == "shaderStorageImageArrayDynamicIndexing")
    return Feature::kShaderStorageImageArrayDynamicIndexing;
  if (name == "shaderClipDistance")
    return Feature::kShaderClipDistance;
  if (name == "shaderCullDistance")
    return Feature::kShaderCullDistance;
  if (name == "shaderFloat64")
    return Feature::kShaderFloat64;
  if (name == "shaderInt64")
    return Feature::kShaderInt64;
  if (name == "shaderInt16")
    return Feature::kShaderInt16;
  if (name == "shaderResourceResidency")
    return Feature::kShaderResourceResidency;
  if (name == "shaderResourceMinLod")
    return Feature::kShaderResourceMinLod;
  if (name == "sparseBinding")
    return Feature::kSparseBinding;
  if (name == "sparseResidencyBuffer")
    return Feature::kSparseResidencyBuffer;
  if (name == "sparseResidencyImage2D")
    return Feature::kSparseResidencyImage2D;
  if (name == "sparseResidencyImage3D")
    return Feature::kSparseResidencyImage3D;
  if (name == "sparseResidency2Samples")
    return Feature::kSparseResidency2Samples;
  if (name == "sparseResidency4Samples")
    return Feature::kSparseResidency4Samples;
  if (name == "sparseResidency8Samples")
    return Feature::kSparseResidency8Samples;
  if (name == "sparseResidency16Samples")
    return Feature::kSparseResidency16Samples;
  if (name == "sparseResidencyAliased")
    return Feature::kSparseResidencyAliased;
  if (name == "variableMultisampleRate")
    return Feature::kVariableMultisampleRate;
  if (name == "inheritedQueries")
    return Feature::kInheritedQueries;
  if (name == "framebuffer")
    return Feature::kFramebuffer;
  if (name == "depthstencil")
    return Feature::kDepthStencil;
  if (name == "fence_timeout")
    return Feature::kFenceTimeout;
  return Feature::kUnknown;
}

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

  for (const auto& section : section_parser.Sections()) {
    r = ProcessSection(section);
    if (!r.IsSuccess())
      return r;
  }

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

  Result r = script_->AddShader(std::move(shader));
  if (!r.IsSuccess())
    return r;

  return {};
}

Result Parser::ProcessRequireBlock(const SectionParser::Section& section) {
  Tokenizer tokenizer(section.contents);
  tokenizer.SetCurrentLine(section.starting_line_number);
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
    Feature feature = NameToFeature(str);
    if (feature == Feature::kUnknown) {
      auto it = std::find_if(str.begin(), str.end(),
                             [](char c) { return !(isalnum(c) || c == '_'); });
      if (it != str.end()) {
        return Result(
            make_error(tokenizer, "Unknown feature or extension: " + str));
      }

      script_->AddRequiredExtension(str);
    } else if (feature == Feature::kFramebuffer) {
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

      auto framebuffer = MakeUnique<FormatBuffer>(BufferType::kColor);
      framebuffer->SetName("framebuffer");
      framebuffer->SetFormat(std::move(fmt));
      framebuffer->SetLocation(0);  // Only one image attachment in vkscript
      script_->AddBuffer(std::move(framebuffer));
    } else if (feature == Feature::kDepthStencil) {
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

      auto depthbuffer = MakeUnique<FormatBuffer>(BufferType::kDepth);
      depthbuffer->SetName("depth_stencil_buffer");
      depthbuffer->SetFormat(std::move(fmt));
      depthbuffer->SetLocation(0);
      script_->AddBuffer(std::move(depthbuffer));
    } else if (feature == Feature::kFenceTimeout) {
      token = tokenizer.NextToken();
      if (!token->IsInteger())
        return Result(make_error(tokenizer, "Missing fence_timeout value"));

      script_->GetEngineData().fence_timeout_ms = token->AsUint32();
    } else {
      script_->AddRequiredFeature(feature);
    }

    token = tokenizer.NextToken();
    if (!token->IsEOS() && !token->IsEOL()) {
      return Result(make_error(
          tokenizer, "Failed to parser requirements block: invalid token: " +
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
    type.SetType(DataType::kUint16);

    auto b = MakeUnique<DataBuffer>(BufferType::kIndex);
    b->SetName("indices");
    b->SetDatumType(type);
    b->SetData(std::move(indices));
    Result r = script_->AddBuffer(std::move(b));
    if (!r.IsSuccess())
      return r;
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
        if (!token->IsHex())
          return Result(
              make_error(tokenizer, "Invalid packed value in Vertex Data: " +
                                        token->ToOriginalString()));

        Value v;
        v.SetIntValue(token->AsHex());
        value_data.push_back(v);
      } else {
        auto& comps = header.format->GetComponents();
        for (size_t i = 0; i < comps.size();
             ++i, token = tokenizer.NextToken()) {
          if (token->IsEOS() || token->IsEOL())
            return Result(make_error(tokenizer,
                                     "Too few cells in given vertex data row"));

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

  for (size_t i = 0; i < headers.size(); ++i) {
    auto buffer = MakeUnique<FormatBuffer>(BufferType::kVertex);
    buffer->SetName("Vertices" + std::to_string(i));
    buffer->SetFormat(std::move(headers[i].format));
    buffer->SetLocation(headers[i].location);
    buffer->SetData(std::move(values[i]));
    script_->AddBuffer(std::move(buffer));
  }

  return {};
}

Result Parser::ProcessTestBlock(const SectionParser::Section& section) {
  CommandParser cp(section.starting_line_number, section.contents);
  Result r = cp.Parse();
  if (!r.IsSuccess())
    return r;

  script_->SetCommands(cp.TakeCommands());

  return {};
}

}  // namespace vkscript
}  // namespace amber
