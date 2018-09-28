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

#include "src/shader_compiler.h"
#include "src/tokenizer.h"
#include "src/vkscript/command_parser.h"
#include "src/vkscript/format_parser.h"
#include "src/vkscript/nodes.h"

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
  return Feature::kUnknown;
}

}  // namespace

Parser::Parser() : amber::Parser() {}

Parser::~Parser() = default;

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
    return ProcessRequireBlock(section.contents);
  if (section.section_type == NodeType::kIndices)
    return ProcessIndicesBlock(section.contents);
  if (section.section_type == NodeType::kVertexData)
    return ProcessVertexDataBlock(section.contents);
  if (section.section_type == NodeType::kTest)
    return ProcessTestBlock(section.contents);

  return Result("Unknown node type ....");
}

Result Parser::ProcessShaderBlock(const SectionParser::Section& section) {
  ShaderCompiler sc;

  assert(SectionParser::HasShader(section.section_type));

  Result r;
  std::vector<uint32_t> shader;
  std::tie(r, shader) =
      sc.Compile(section.shader_type, section.format, section.contents);
  if (!r.IsSuccess())
    return r;

  script_.AddShader(section.shader_type, std::move(shader));

  return {};
}

Result Parser::ProcessRequireBlock(const std::string& data) {
  auto node = MakeUnique<RequireNode>();

  Tokenizer tokenizer(data);
  for (auto token = tokenizer.NextToken(); !token->IsEOS();
       token = tokenizer.NextToken()) {
    if (token->IsEOL())
      continue;

    if (!token->IsString())
      return Result("Failed to parse requirements block.");

    std::string str = token->AsString();
    Feature feature = NameToFeature(str);
    if (feature == Feature::kUnknown) {
      auto it = std::find_if(str.begin(), str.end(),
                             [](char c) { return !(isalnum(c) || c == '_'); });
      if (it != str.end())
        return Result("Unknown feature or extension: " + str);

      node->AddExtension(str);
    } else if (feature == Feature::kFramebuffer) {
      token = tokenizer.NextToken();
      if (!token->IsString())
        return Result("Missing framebuffer format");

      FormatParser fmt_parser;
      auto fmt = fmt_parser.Parse(token->AsString());
      if (fmt == nullptr)
        return Result("Failed to parse framebuffer format");

      node->AddRequirement(feature, std::move(fmt));
    } else if (feature == Feature::kDepthStencil) {
      token = tokenizer.NextToken();
      if (!token->IsString())
        return Result("Missing depthStencil format");

      FormatParser fmt_parser;
      auto fmt = fmt_parser.Parse(token->AsString());
      if (fmt == nullptr)
        return Result("Failed to parse depthstencil format");

      node->AddRequirement(feature, std::move(fmt));
    } else {
      node->AddRequirement(feature);
    }

    token = tokenizer.NextToken();
    if (!token->IsEOS() && !token->IsEOL())
      return Result("Failed to parser requirements block: invalid token");
  }

  if (!node->Requirements().empty() || !node->Extensions().empty())
    script_.AddRequireNode(std::move(node));

  return {};
}

Result Parser::ProcessIndicesBlock(const std::string& data) {
  std::vector<uint16_t> indices;

  Tokenizer tokenizer(data);
  for (auto token = tokenizer.NextToken(); !token->IsEOS();
       token = tokenizer.NextToken()) {
    if (token->IsEOL())
      continue;

    if (!token->IsInteger())
      return Result("Invalid value in indices block");
    if (token->AsUint64() >
        static_cast<uint64_t>(std::numeric_limits<uint16_t>::max())) {
      return Result("Value too large in indices block");
    }

    indices.push_back(token->AsUint16());
  }

  if (!indices.empty())
    script_.AddIndices(indices);

  return {};
}

Result Parser::ProcessVertexDataBlock(const std::string& data) {
  Tokenizer tokenizer(data);

  // Skip blank and comment lines
  auto token = tokenizer.NextToken();
  while (token->IsEOL())
    token = tokenizer.NextToken();

  // Skip empty vertex data blocks
  if (token->IsEOS())
    return {};

  // Process the header line.
  std::vector<VertexDataNode::Header> headers;
  while (!token->IsEOL() && !token->IsEOS()) {
    // Because of the way the tokenizer works we'll see a number then a string
    // the string will start with a slash which we have to remove.
    if (!token->IsInteger())
      return Result("Unable to process vertex data header");

    uint8_t loc = token->AsUint8();

    token = tokenizer.NextToken();
    if (!token->IsString())
      return Result("Unable to process vertex data header");

    std::string fmt_name = token->AsString();
    if (fmt_name.size() < 2)
      return Result("Vertex data format too short");

    FormatParser parser;
    auto fmt = parser.Parse(fmt_name.substr(1, fmt_name.length()));
    if (!fmt)
      return Result("Invalid format in vertex data header");

    headers.push_back({loc, std::move(fmt)});

    token = tokenizer.NextToken();
  }

  auto node = MakeUnique<VertexDataNode>();

  // Process data lines
  for (; !token->IsEOS(); token = tokenizer.NextToken()) {
    if (token->IsEOL())
      continue;

    std::vector<VertexDataNode::Cell> row;
    for (const auto& header : headers) {
      row.emplace_back();

      if (header.format->GetPackSize() > 0) {
        if (!token->IsHex())
          return Result("Invalid packed value in Vertex Data");

        Value v;
        v.SetIntValue(token->AsHex());
        row.back().AppendValue(std::move(v));
      } else {
        auto& comps = header.format->GetComponents();
        for (size_t i = 0; i < comps.size();
             ++i, token = tokenizer.NextToken()) {
          if (token->IsEOS() || token->IsEOL())
            return Result("Too few cells in given vertex data row");

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
            return Result("Invalid vertex data value");
          }

          row.back().AppendValue(std::move(v));
        }
      }
    }
    node->AddRow(std::move(row));
  }

  node->SetHeaders(std::move(headers));
  script_.AddVertexData(std::move(node));

  return {};
}

Result Parser::ProcessTestBlock(const std::string& data) {
  CommandParser cp;
  Result r = cp.Parse(data);
  if (!r.IsSuccess())
    return r;

  script_.SetTestCommands(cp.TakeCommands());

  return {};
}

}  // namespace vkscript
}  // namespace amber
