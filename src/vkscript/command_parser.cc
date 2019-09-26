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

#include "src/vkscript/command_parser.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <limits>
#include <string>
#include <utility>

#include "src/command_data.h"
#include "src/make_unique.h"
#include "src/tokenizer.h"
#include "src/type_parser.h"
#include "src/vkscript/datum_type_parser.h"

namespace amber {
namespace vkscript {
namespace {

ShaderType ShaderNameToType(const std::string& name) {
  if (name == "fragment")
    return kShaderTypeFragment;
  if (name == "compute")
    return kShaderTypeCompute;
  if (name == "geometry")
    return kShaderTypeGeometry;
  if (name == "tessellation evaluation")
    return kShaderTypeTessellationEvaluation;
  if (name == "tessellation control")
    return kShaderTypeTessellationControl;

  return kShaderTypeVertex;
}

}  // namespace

CommandParser::CommandParser(Script* script,
                             Pipeline* pipeline,
                             size_t current_line,
                             const std::string& data)
    : script_(script),
      pipeline_(pipeline),
      tokenizer_(MakeUnique<Tokenizer>(data)) {
  tokenizer_->SetCurrentLine(current_line);
}

CommandParser::~CommandParser() = default;

std::string CommandParser::make_error(const std::string& err) {
  return std::to_string(tokenizer_->GetCurrentLine()) + ": " + err;
}

Result CommandParser::ParseBoolean(const std::string& str, bool* result) {
  assert(result);

  std::string tmp;
  tmp.resize(str.size());
  std::transform(str.begin(), str.end(), tmp.begin(),
                 [](unsigned char c) { return std::tolower(c); });

  if (tmp == "true") {
    *result = true;
    return {};
  }
  if (tmp == "false") {
    *result = false;
    return {};
  }
  return Result("Invalid value passed as a boolean string: " + str);
}

Result CommandParser::Parse() {
  for (auto token = tokenizer_->NextToken(); !token->IsEOS();
       token = tokenizer_->NextToken()) {
    if (token->IsEOL())
      continue;

    if (!token->IsString()) {
      return Result(make_error(
          "Command not recognized. Received something other then a string: " +
          token->ToOriginalString()));
    }

    std::string cmd_name = token->AsString();
    Result r;
    if (cmd_name == "draw") {
      token = tokenizer_->NextToken();
      if (!token->IsString())
        return Result(make_error("Invalid draw command in test: " +
                                 token->ToOriginalString()));

      cmd_name = token->AsString();
      if (cmd_name == "rect")
        r = ProcessDrawRect();
      else if (cmd_name == "arrays")
        r = ProcessDrawArrays();
      else
        r = Result("Unknown draw command: " + cmd_name);

    } else if (cmd_name == "clear") {
      r = ProcessClear();
    } else if (cmd_name == "ssbo") {
      r = ProcessSSBO();
    } else if (cmd_name == "uniform") {
      r = ProcessUniform();
    } else if (cmd_name == "patch") {
      r = ProcessPatch();
    } else if (cmd_name == "probe") {
      r = ProcessProbe(false);
    } else if (cmd_name == "tolerance") {
      r = ProcessTolerance();
    } else if (cmd_name == "relative") {
      token = tokenizer_->NextToken();
      if (!token->IsString() || token->AsString() != "probe")
        return Result(make_error("relative must be used with probe: " +
                                 token->ToOriginalString()));

      r = ProcessProbe(true);
    } else if (cmd_name == "compute") {
      r = ProcessCompute();
    } else if (cmd_name == "vertex" || cmd_name == "fragment" ||
               cmd_name == "geometry" || cmd_name == "tessellation") {
      std::string shader_name = cmd_name;
      if (cmd_name == "tessellation") {
        token = tokenizer_->NextToken();
        if (!token->IsString() || (token->AsString() != "control" &&
                                   token->AsString() != "evaluation")) {
          return Result(
              make_error("Tessellation entrypoint must have "
                         "<evaluation|control> in name: " +
                         token->ToOriginalString()));
        }
        shader_name += " " + token->AsString();
      }

      token = tokenizer_->NextToken();
      if (!token->IsString() || token->AsString() != "entrypoint")
        return Result(make_error("Unknown command: " + shader_name));

      r = ProcessEntryPoint(shader_name);

      // Pipeline Commands
    } else if (cmd_name == "primitiveRestartEnable") {
      r = ProcessPrimitiveRestartEnable();
    } else if (cmd_name == "depthClampEnable") {
      r = ProcessDepthClampEnable();
    } else if (cmd_name == "rasterizerDiscardEnable") {
      r = ProcessRasterizerDiscardEnable();
    } else if (cmd_name == "depthBiasEnable") {
      r = ProcessDepthBiasEnable();
    } else if (cmd_name == "logicOpEnable") {
      r = ProcessLogicOpEnable();
    } else if (cmd_name == "blendEnable") {
      r = ProcessBlendEnable();
    } else if (cmd_name == "depthTestEnable") {
      r = ProcessDepthTestEnable();
    } else if (cmd_name == "depthWriteEnable") {
      r = ProcessDepthWriteEnable();
    } else if (cmd_name == "depthBoundsTestEnable") {
      r = ProcessDepthBoundsTestEnable();
    } else if (cmd_name == "stencilTestEnable") {
      r = ProcessStencilTestEnable();
    } else if (cmd_name == "topology") {
      r = ProcessTopology();
    } else if (cmd_name == "polygonMode") {
      r = ProcessPolygonMode();
    } else if (cmd_name == "logicOp") {
      r = ProcessLogicOp();
    } else if (cmd_name == "frontFace") {
      r = ProcessFrontFace();
    } else if (cmd_name == "cullMode") {
      r = ProcessCullMode();
    } else if (cmd_name == "depthBiasConstantFactor") {
      r = ProcessDepthBiasConstantFactor();
    } else if (cmd_name == "depthBiasClamp") {
      r = ProcessDepthBiasClamp();
    } else if (cmd_name == "depthBiasSlopeFactor") {
      r = ProcessDepthBiasSlopeFactor();
    } else if (cmd_name == "lineWidth") {
      r = ProcessLineWidth();
    } else if (cmd_name == "minDepthBounds") {
      r = ProcessMinDepthBounds();
    } else if (cmd_name == "maxDepthBounds") {
      r = ProcessMaxDepthBounds();
    } else if (cmd_name == "srcColorBlendFactor") {
      r = ProcessSrcColorBlendFactor();
    } else if (cmd_name == "dstColorBlendFactor") {
      r = ProcessDstColorBlendFactor();
    } else if (cmd_name == "srcAlphaBlendFactor") {
      r = ProcessSrcAlphaBlendFactor();
    } else if (cmd_name == "dstAlphaBlendFactor") {
      r = ProcessDstAlphaBlendFactor();
    } else if (cmd_name == "colorBlendOp") {
      r = ProcessColorBlendOp();
    } else if (cmd_name == "alphaBlendOp") {
      r = ProcessAlphaBlendOp();
    } else if (cmd_name == "depthCompareOp") {
      r = ProcessDepthCompareOp();
    } else if (cmd_name == "front.compareOp") {
      r = ProcessFrontCompareOp();
    } else if (cmd_name == "back.compareOp") {
      r = ProcessBackCompareOp();
    } else if (cmd_name == "front.failOp") {
      r = ProcessFrontFailOp();
    } else if (cmd_name == "front.passOp") {
      r = ProcessFrontPassOp();
    } else if (cmd_name == "front.depthFailOp") {
      r = ProcessFrontDepthFailOp();
    } else if (cmd_name == "back.failOp") {
      r = ProcessBackFailOp();
    } else if (cmd_name == "back.passOp") {
      r = ProcessBackPassOp();
    } else if (cmd_name == "back.depthFailOp") {
      r = ProcessBackDepthFailOp();
    } else if (cmd_name == "front.compareMask") {
      r = ProcessFrontCompareMask();
    } else if (cmd_name == "front.writeMask") {
      r = ProcessFrontWriteMask();
    } else if (cmd_name == "back.compareMask") {
      r = ProcessBackCompareMask();
    } else if (cmd_name == "back.writeMask") {
      r = ProcessBackWriteMask();
    } else if (cmd_name == "front.reference") {
      r = ProcessFrontReference();
    } else if (cmd_name == "back.reference") {
      r = ProcessBackReference();
    } else if (cmd_name == "colorWriteMask") {
      r = ProcessColorWriteMask();
    } else {
      r = Result("Unknown command: " + cmd_name);
    }

    if (!r.IsSuccess())
      return Result(make_error(r.Error()));
  }

  return {};
}

Result CommandParser::ProcessDrawRect() {
  auto cmd = MakeUnique<DrawRectCommand>(pipeline_, pipeline_data_);
  cmd->SetLine(tokenizer_->GetCurrentLine());

  if (pipeline_->GetVertexBuffers().size() > 1) {
    return Result(
        "draw rect command is not supported in a pipeline with more than one "
        "vertex buffer attached");
  }

  auto token = tokenizer_->NextToken();
  while (token->IsString()) {
    std::string str = token->AsString();
    if (str != "ortho" && str != "patch")
      return Result("Unknown parameter to draw rect: " + str);

    if (str == "ortho") {
      cmd->EnableOrtho();
    } else {
      cmd->EnablePatch();
    }
    token = tokenizer_->NextToken();
  }

  Result r = token->ConvertToDouble();
  if (!r.IsSuccess())
    return r;
  cmd->SetX(token->AsFloat());

  token = tokenizer_->NextToken();
  r = token->ConvertToDouble();
  if (!r.IsSuccess())
    return r;
  cmd->SetY(token->AsFloat());

  token = tokenizer_->NextToken();
  r = token->ConvertToDouble();
  if (!r.IsSuccess())
    return r;
  cmd->SetWidth(token->AsFloat());

  token = tokenizer_->NextToken();
  r = token->ConvertToDouble();
  if (!r.IsSuccess())
    return r;
  cmd->SetHeight(token->AsFloat());

  token = tokenizer_->NextToken();
  if (!token->IsEOS() && !token->IsEOL())
    return Result("Extra parameter to draw rect command: " +
                  token->ToOriginalString());

  commands_.push_back(std::move(cmd));
  return {};
}

Result CommandParser::ProcessDrawArrays() {
  auto cmd = MakeUnique<DrawArraysCommand>(pipeline_, pipeline_data_);
  cmd->SetLine(tokenizer_->GetCurrentLine());

  auto token = tokenizer_->NextToken();
  while (token->IsString()) {
    std::string str = token->AsString();
    if (str != "indexed" && str != "instanced") {
      Topology topo = NameToTopology(token->AsString());
      if (topo != Topology::kUnknown) {
        cmd->SetTopology(topo);

        // Advance token here so we're consistent with the non-topology case.
        token = tokenizer_->NextToken();
        break;
      }
      return Result("Unknown parameter to draw arrays: " + str);
    }

    if (str == "indexed") {
      cmd->EnableIndexed();
    } else {
      cmd->EnableInstanced();
    }
    token = tokenizer_->NextToken();
  }

  if (cmd->GetTopology() == Topology::kUnknown)
    return Result("Missing draw arrays topology");

  if (!token->IsInteger())
    return Result("Missing integer first vertex value for draw arrays: " +
                  token->ToOriginalString());
  cmd->SetFirstVertexIndex(token->AsUint32());

  token = tokenizer_->NextToken();
  if (!token->IsInteger())
    return Result("Missing integer vertex count value for draw arrays: " +
                  token->ToOriginalString());
  cmd->SetVertexCount(token->AsUint32());

  token = tokenizer_->NextToken();
  if (cmd->IsInstanced()) {
    if (!token->IsEOL() && !token->IsEOS()) {
      if (!token->IsInteger())
        return Result("Invalid instance count for draw arrays: " +
                      token->ToOriginalString());

      cmd->SetInstanceCount(token->AsUint32());
    }
    token = tokenizer_->NextToken();
  }

  if (!token->IsEOL() && !token->IsEOS())
    return Result("Extra parameter to draw arrays command: " +
                  token->ToOriginalString());

  commands_.push_back(std::move(cmd));
  return {};
}

Result CommandParser::ProcessCompute() {
  auto cmd = MakeUnique<ComputeCommand>(pipeline_);
  cmd->SetLine(tokenizer_->GetCurrentLine());

  auto token = tokenizer_->NextToken();

  // Compute can start a compute line or an entryp oint line ...
  if (token->IsString() && token->AsString() == "entrypoint")
    return ProcessEntryPoint("compute");

  if (!token->IsInteger())
    return Result("Missing integer value for compute X entry: " +
                  token->ToOriginalString());
  cmd->SetX(token->AsUint32());

  token = tokenizer_->NextToken();
  if (!token->IsInteger())
    return Result("Missing integer value for compute Y entry: " +
                  token->ToOriginalString());
  cmd->SetY(token->AsUint32());

  token = tokenizer_->NextToken();
  if (!token->IsInteger())
    return Result("Missing integer value for compute Z entry: " +
                  token->ToOriginalString());
  cmd->SetZ(token->AsUint32());

  token = tokenizer_->NextToken();
  if (!token->IsEOS() && !token->IsEOL())
    return Result("Extra parameter to compute command: " +
                  token->ToOriginalString());

  commands_.push_back(std::move(cmd));
  return {};
}

Result CommandParser::ProcessClear() {
  std::unique_ptr<Command> cmd;

  auto token = tokenizer_->NextToken();
  std::string cmd_suffix = "";
  if (token->IsString()) {
    std::string str = token->AsString();
    cmd_suffix = str + " ";
    if (str == "depth") {
      cmd = MakeUnique<ClearDepthCommand>(pipeline_);
      cmd->SetLine(tokenizer_->GetCurrentLine());

      token = tokenizer_->NextToken();
      Result r = token->ConvertToDouble();
      if (!r.IsSuccess())
        return r;

      cmd->AsClearDepth()->SetValue(token->AsFloat());
    } else if (str == "stencil") {
      cmd = MakeUnique<ClearStencilCommand>(pipeline_);
      cmd->SetLine(tokenizer_->GetCurrentLine());

      token = tokenizer_->NextToken();
      if (token->IsEOL() || token->IsEOS())
        return Result("Missing stencil value for clear stencil command: " +
                      token->ToOriginalString());
      if (!token->IsInteger())
        return Result("Invalid stencil value for clear stencil command: " +
                      token->ToOriginalString());

      cmd->AsClearStencil()->SetValue(token->AsUint32());
    } else if (str == "color") {
      cmd = MakeUnique<ClearColorCommand>(pipeline_);
      cmd->SetLine(tokenizer_->GetCurrentLine());

      token = tokenizer_->NextToken();
      Result r = token->ConvertToDouble();
      if (!r.IsSuccess())
        return r;
      cmd->AsClearColor()->SetR(token->AsFloat());

      token = tokenizer_->NextToken();
      r = token->ConvertToDouble();
      if (!r.IsSuccess())
        return r;
      cmd->AsClearColor()->SetG(token->AsFloat());

      token = tokenizer_->NextToken();
      r = token->ConvertToDouble();
      if (!r.IsSuccess())
        return r;
      cmd->AsClearColor()->SetB(token->AsFloat());

      token = tokenizer_->NextToken();
      r = token->ConvertToDouble();
      if (!r.IsSuccess())
        return r;
      cmd->AsClearColor()->SetA(token->AsFloat());
    } else {
      return Result("Extra parameter to clear command: " +
                    token->ToOriginalString());
    }

    token = tokenizer_->NextToken();
  } else {
    cmd = MakeUnique<ClearCommand>(pipeline_);
    cmd->SetLine(tokenizer_->GetCurrentLine());
  }
  if (!token->IsEOS() && !token->IsEOL())
    return Result("Extra parameter to clear " + cmd_suffix +
                  "command: " + token->ToOriginalString());

  commands_.push_back(std::move(cmd));
  return {};
}

Result CommandParser::ParseValues(const std::string& name,
                                  Format* fmt,
                                  std::vector<Value>* values) {
  assert(values);

  uint32_t row_index = 0;
  auto token = tokenizer_->NextToken();
  size_t seen = 0;
  while (!token->IsEOL() && !token->IsEOS()) {
    Value v;

    if ((fmt->IsFloat32() || fmt->IsFloat64())) {
      if (!token->IsInteger() && !token->IsDouble()) {
        return Result(std::string("Invalid value provided to ") + name +
                      " command: " + token->ToOriginalString());
      }

      Result r = token->ConvertToDouble();
      if (!r.IsSuccess())
        return r;

      v.SetDoubleValue(token->AsDouble());
    } else {
      if (!token->IsInteger()) {
        return Result(std::string("Invalid value provided to ") + name +
                      " command: " + token->ToOriginalString());
      }

      v.SetIntValue(token->AsUint64());
    }

    values->push_back(v);
    token = tokenizer_->NextToken();

    ++row_index;
    ++seen;
  }

  // This could overflow, but I don't really expect us to get command files
  // that big ....
  size_t num_per_row = fmt->GetType()->RowCount();
  if (seen == 0 || (seen % num_per_row) != 0) {
    return Result(std::string("Incorrect number of values provided to ") +
                  name + " command");
  }

  return {};
}

Result CommandParser::ProcessSSBO() {
  auto cmd =
      MakeUnique<BufferCommand>(BufferCommand::BufferType::kSSBO, pipeline_);
  cmd->SetLine(tokenizer_->GetCurrentLine());

  auto token = tokenizer_->NextToken();
  if (token->IsEOL() || token->IsEOS())
    return Result("Missing binding and size values for ssbo command");
  if (!token->IsInteger())
    return Result("Invalid binding value for ssbo command");

  uint32_t val = token->AsUint32();

  token = tokenizer_->NextToken();
  if (token->IsString() && token->AsString() != "subdata") {
    auto& str = token->AsString();
    if (str.size() >= 2 && str[0] == ':') {
      cmd->SetDescriptorSet(val);

      auto substr = str.substr(1, str.size());
      uint64_t binding_val = strtoul(substr.c_str(), nullptr, 10);
      if (binding_val > std::numeric_limits<uint32_t>::max())
        return Result("binding value too large in ssbo command");

      cmd->SetBinding(static_cast<uint32_t>(binding_val));
    } else {
      return Result("Invalid value for ssbo command: " +
                    token->ToOriginalString());
    }

    token = tokenizer_->NextToken();
  } else {
    cmd->SetBinding(val);
  }

  {
    // Generate an internal buffer for this binding if needed.
    auto set = cmd->GetDescriptorSet();
    auto binding = cmd->GetBinding();

    auto* buffer = pipeline_->GetBufferForBinding(set, binding);
    if (!buffer) {
      auto b = MakeUnique<Buffer>(BufferType::kStorage);
      b->SetName("AutoBuf-" + std::to_string(script_->GetBuffers().size()));
      buffer = b.get();
      script_->AddBuffer(std::move(b));
      pipeline_->AddBuffer(buffer, set, binding);
    }
    cmd->SetBuffer(buffer);
  }

  if (token->IsString() && token->AsString() == "subdata") {
    cmd->SetIsSubdata();

    token = tokenizer_->NextToken();
    if (!token->IsString())
      return Result("Invalid type for ssbo command: " +
                    token->ToOriginalString());

    DatumTypeParser tp;
    auto type = tp.Parse(token->AsString());
    if (!type)
      return Result("Invalid type provided: " + token->AsString());

    auto fmt = MakeUnique<Format>(type.get());
    auto* buf = cmd->GetBuffer();
    if (buf->FormatIsDefault() || !buf->GetFormat()) {
      buf->SetFormat(fmt.get());
      script_->RegisterFormat(std::move(fmt));
      script_->RegisterType(std::move(type));
    } else if (!buf->GetFormat()->Equal(fmt.get())) {
      return Result("probe ssbo format does not match buffer format");
    }

    token = tokenizer_->NextToken();
    if (!token->IsInteger()) {
      return Result("Invalid offset for ssbo command: " +
                    token->ToOriginalString());
    }
    if (token->AsInt32() < 0) {
      return Result("offset for SSBO must be positive, got: " +
                    std::to_string(token->AsInt32()));
    }
    if ((token->AsUint32() % buf->GetFormat()->SizeInBytes()) != 0) {
      return Result(
          "offset for SSBO must be a multiple of the data size expected " +
          std::to_string(buf->GetFormat()->SizeInBytes()));
    }

    cmd->SetOffset(token->AsUint32());

    std::vector<Value> values;
    Result r = ParseValues("ssbo", buf->GetFormat(), &values);
    if (!r.IsSuccess())
      return r;

    buf->RecalculateMaxSizeInBytes(values, cmd->GetOffset());

    cmd->SetValues(std::move(values));

  } else {
    if (token->IsEOL() || token->IsEOS())
      return Result("Missing size value for ssbo command: " +
                    token->ToOriginalString());
    if (!token->IsInteger())
      return Result("Invalid size value for ssbo command: " +
                    token->ToOriginalString());

    // Resize the buffer so we'll correctly create the descriptor sets.
    auto* buf = cmd->GetBuffer();
    buf->SetElementCount(token->AsUint32());

    // Set a default format into the buffer if needed.
    if (!buf->GetFormat()) {
      TypeParser parser;
      auto type = parser.Parse("R8_SINT");
      auto fmt = MakeUnique<Format>(type.get());
      buf->SetFormat(fmt.get());
      script_->RegisterFormat(std::move(fmt));
      script_->RegisterType(std::move(type));

      // This has to come after the SetFormat() call because SetFormat() resets
      // the value back to false.
      buf->SetFormatIsDefault(true);
    }

    token = tokenizer_->NextToken();
    if (!token->IsEOS() && !token->IsEOL())
      return Result("Extra parameter for ssbo command: " +
                    token->ToOriginalString());
  }

  commands_.push_back(std::move(cmd));
  return {};
}

Result CommandParser::ProcessUniform() {
  auto token = tokenizer_->NextToken();
  if (token->IsEOL() || token->IsEOS())
    return Result("Missing binding and size values for uniform command: " +
                  token->ToOriginalString());
  if (!token->IsString())
    return Result("Invalid type value for uniform command: " +
                  token->ToOriginalString());

  std::unique_ptr<BufferCommand> cmd;
  bool is_ubo = false;
  if (token->AsString() == "ubo") {
    cmd = MakeUnique<BufferCommand>(BufferCommand::BufferType::kUniform,
                                    pipeline_);
    cmd->SetLine(tokenizer_->GetCurrentLine());

    token = tokenizer_->NextToken();
    if (!token->IsInteger()) {
      return Result("Invalid binding value for uniform ubo command: " +
                    token->ToOriginalString());
    }

    uint32_t val = token->AsUint32();

    token = tokenizer_->NextToken();
    if (!token->IsString()) {
      return Result("Invalid type value for uniform ubo command: " +
                    token->ToOriginalString());
    }

    auto& str = token->AsString();
    if (str.size() >= 2 && str[0] == ':') {
      cmd->SetDescriptorSet(val);

      auto substr = str.substr(1, str.size());
      uint64_t binding_val = strtoul(substr.c_str(), nullptr, 10);
      if (binding_val > std::numeric_limits<uint32_t>::max())
        return Result("binding value too large in uniform ubo command: " +
                      token->ToOriginalString());

      cmd->SetBinding(static_cast<uint32_t>(binding_val));

      token = tokenizer_->NextToken();
      if (!token->IsString()) {
        return Result("Invalid type value for uniform ubo command: " +
                      token->ToOriginalString());
      }
    } else {
      cmd->SetBinding(val);
    }
    is_ubo = true;

    auto set = cmd->GetDescriptorSet();
    auto binding = cmd->GetBinding();

    auto* buffer = pipeline_->GetBufferForBinding(set, binding);
    if (!buffer) {
      auto b = MakeUnique<Buffer>(BufferType::kUniform);
      b->SetName("AutoBuf-" + std::to_string(script_->GetBuffers().size()));
      buffer = b.get();
      script_->AddBuffer(std::move(b));
      pipeline_->AddBuffer(buffer, set, binding);
    }
    cmd->SetBuffer(buffer);

  } else {
    cmd = MakeUnique<BufferCommand>(BufferCommand::BufferType::kPushConstant,
                                    pipeline_);
    cmd->SetLine(tokenizer_->GetCurrentLine());

    // Push constants don't have descriptor set and binding values. So, we do
    // not want to try to lookup the buffer or we'll accidentally get whatever
    // is bound at 0:0.
    auto b = MakeUnique<Buffer>(BufferType::kUniform);
    b->SetName("AutoBuf-" + std::to_string(script_->GetBuffers().size()));
    cmd->SetBuffer(b.get());
    script_->AddBuffer(std::move(b));
  }

  DatumTypeParser tp;
  auto type = tp.Parse(token->AsString());
  if (!type)
    return Result("Invalid type provided: " + token->AsString());

  auto fmt = MakeUnique<Format>(type.get());

  // uniform is always std140.
  if (is_ubo)
    fmt->SetLayout(Format::Layout::kStd140);

  auto* buf = cmd->GetBuffer();
  if (buf->FormatIsDefault() || !buf->GetFormat()) {
    buf->SetFormat(fmt.get());
    script_->RegisterFormat(std::move(fmt));
    script_->RegisterType(std::move(type));
  } else if (!buf->GetFormat()->Equal(fmt.get())) {
    return Result("probe ssbo format does not match buffer format");
  }

  token = tokenizer_->NextToken();
  if (!token->IsInteger()) {
    return Result("Invalid offset value for uniform command: " +
                  token->ToOriginalString());
  }
  if (token->AsInt32() < 0) {
    return Result("offset for uniform must be positive, got: " +
                  std::to_string(token->AsInt32()));
  }

  auto buf_size = static_cast<int32_t>(buf->GetFormat()->SizeInBytes());
  if (token->AsInt32() % buf_size != 0)
    return Result("offset for uniform must be multiple of data size");

  cmd->SetOffset(token->AsUint32());

  std::vector<Value> values;
  Result r = ParseValues("uniform", buf->GetFormat(), &values);
  if (!r.IsSuccess())
    return r;

  buf->RecalculateMaxSizeInBytes(values, cmd->GetOffset());

  if (cmd->IsPushConstant())
    buf->SetData(values);
  else
    cmd->SetValues(std::move(values));

  commands_.push_back(std::move(cmd));
  return {};
}

Result CommandParser::ProcessTolerance() {
  current_tolerances_.clear();

  auto token = tokenizer_->NextToken();
  size_t found_tokens = 0;
  while (!token->IsEOL() && !token->IsEOS() && found_tokens < 4) {
    if (token->IsString() && token->AsString() == ",") {
      token = tokenizer_->NextToken();
      continue;
    }

    if (token->IsInteger() || token->IsDouble()) {
      Result r = token->ConvertToDouble();
      if (!r.IsSuccess())
        return r;
      double value = token->AsDouble();

      token = tokenizer_->NextToken();
      if (token->IsString() && token->AsString() != ",") {
        if (token->AsString() != "%")
          return Result("Invalid value for tolerance command: " +
                        token->ToOriginalString());

        current_tolerances_.push_back(Probe::Tolerance{true, value});
        token = tokenizer_->NextToken();
      } else {
        current_tolerances_.push_back(Probe::Tolerance{false, value});
      }
    } else {
      return Result("Invalid value for tolerance command: " +
                    token->ToOriginalString());
    }

    ++found_tokens;
  }
  if (found_tokens == 0)
    return Result("Missing value for tolerance command");
  if (found_tokens != 1 && found_tokens != 4)
    return Result("Invalid number of tolerance parameters provided");

  if (!token->IsEOS() && !token->IsEOL())
    return Result("Extra parameter for tolerance command: " +
                  token->ToOriginalString());

  return {};
}

Result CommandParser::ProcessPatch() {
  auto cmd = MakeUnique<PatchParameterVerticesCommand>(pipeline_);
  cmd->SetLine(tokenizer_->GetCurrentLine());

  auto token = tokenizer_->NextToken();
  if (!token->IsString() || token->AsString() != "parameter")
    return Result("Missing parameter flag to patch command: " +
                  token->ToOriginalString());

  token = tokenizer_->NextToken();
  if (!token->IsString() || token->AsString() != "vertices")
    return Result("Missing vertices flag to patch command: " +
                  token->ToOriginalString());

  token = tokenizer_->NextToken();
  if (!token->IsInteger())
    return Result("Invalid count parameter for patch parameter vertices: " +
                  token->ToOriginalString());
  cmd->SetControlPointCount(token->AsUint32());

  token = tokenizer_->NextToken();
  if (!token->IsEOS() && !token->IsEOL())
    return Result("Extra parameter for patch parameter vertices command: " +
                  token->ToOriginalString());

  commands_.push_back(std::move(cmd));
  return {};
}

Result CommandParser::ProcessEntryPoint(const std::string& name) {
  auto cmd = MakeUnique<EntryPointCommand>(pipeline_);
  cmd->SetLine(tokenizer_->GetCurrentLine());

  auto token = tokenizer_->NextToken();
  if (token->IsEOL() || token->IsEOS())
    return Result("Missing entrypoint name");

  if (!token->IsString())
    return Result("Entrypoint name must be a string: " +
                  token->ToOriginalString());

  cmd->SetShaderType(ShaderNameToType(name));
  cmd->SetEntryPointName(token->AsString());

  token = tokenizer_->NextToken();
  if (!token->IsEOS() && !token->IsEOL())
    return Result("Extra parameter for entrypoint command: " +
                  token->ToOriginalString());

  commands_.push_back(std::move(cmd));

  return {};
}

Result CommandParser::ProcessProbe(bool relative) {
  auto token = tokenizer_->NextToken();
  if (!token->IsString())
    return Result("Invalid token in probe command: " +
                  token->ToOriginalString());

  // The SSBO syntax is different from probe or probe all so handle specially.
  if (token->AsString() == "ssbo")
    return ProcessProbeSSBO();

  if (pipeline_->GetColorAttachments().empty())
    return Result("Pipeline missing color buffers. Something went wrong.");

  // VkScript has a single generated colour buffer which should always be
  // available.
  auto* buffer = pipeline_->GetColorAttachments()[0].buffer;
  if (!buffer)
    return Result("Pipeline missing color buffers, something went wrong.");

  auto cmd = MakeUnique<ProbeCommand>(buffer);
  cmd->SetLine(tokenizer_->GetCurrentLine());

  cmd->SetTolerances(current_tolerances_);
  if (relative)
    cmd->SetRelative();

  bool is_rect = false;
  if (token->AsString() == "rect") {
    is_rect = true;
    cmd->SetProbeRect();

    token = tokenizer_->NextToken();
    if (!token->IsString())
      return Result("Invalid token in probe command: " +
                    token->ToOriginalString());
  } else if (token->AsString() == "all") {
    cmd->SetWholeWindow();
    cmd->SetProbeRect();

    token = tokenizer_->NextToken();
    if (!token->IsString())
      return Result("Invalid token in probe command: " +
                    token->ToOriginalString());
  }

  std::string format = token->AsString();
  if (format != "rgba" && format != "rgb")
    return Result("Invalid format specified to probe command: " +
                  token->ToOriginalString());

  if (format == "rgba")
    cmd->SetIsRGBA();

  token = tokenizer_->NextToken();
  if (!cmd->IsWholeWindow()) {
    bool got_rect_open_bracket = false;
    if (token->IsOpenBracket()) {
      got_rect_open_bracket = true;
      token = tokenizer_->NextToken();
    }

    Result r = token->ConvertToDouble();
    if (!r.IsSuccess())
      return r;
    cmd->SetX(token->AsFloat());

    token = tokenizer_->NextToken();
    if (token->IsComma())
      token = tokenizer_->NextToken();

    r = token->ConvertToDouble();
    if (!r.IsSuccess())
      return r;
    cmd->SetY(token->AsFloat());

    if (is_rect) {
      token = tokenizer_->NextToken();
      if (token->IsComma())
        token = tokenizer_->NextToken();

      r = token->ConvertToDouble();
      if (!r.IsSuccess())
        return r;
      cmd->SetWidth(token->AsFloat());

      token = tokenizer_->NextToken();
      if (token->IsComma())
        token = tokenizer_->NextToken();

      r = token->ConvertToDouble();
      if (!r.IsSuccess())
        return r;
      cmd->SetHeight(token->AsFloat());
    }

    token = tokenizer_->NextToken();
    if (token->IsCloseBracket()) {
      // Close bracket without an open
      if (!got_rect_open_bracket)
        return Result("Missing open bracket for probe command");

      token = tokenizer_->NextToken();
    } else if (got_rect_open_bracket) {
      // An open bracket without a close bracket.
      return Result("Missing close bracket for probe command");
    }
  }

  bool got_color_open_bracket = false;
  if (token->IsOpenBracket()) {
    got_color_open_bracket = true;
    token = tokenizer_->NextToken();
  }

  Result r = token->ConvertToDouble();
  if (!r.IsSuccess())
    return r;
  cmd->SetR(token->AsFloat());

  token = tokenizer_->NextToken();
  if (token->IsComma())
    token = tokenizer_->NextToken();

  r = token->ConvertToDouble();
  if (!r.IsSuccess())
    return r;
  cmd->SetG(token->AsFloat());

  token = tokenizer_->NextToken();
  if (token->IsComma())
    token = tokenizer_->NextToken();

  r = token->ConvertToDouble();
  if (!r.IsSuccess())
    return r;
  cmd->SetB(token->AsFloat());

  if (format == "rgba") {
    token = tokenizer_->NextToken();
    if (token->IsComma())
      token = tokenizer_->NextToken();

    r = token->ConvertToDouble();
    if (!r.IsSuccess())
      return r;
    cmd->SetA(token->AsFloat());
  }

  token = tokenizer_->NextToken();
  if (token->IsCloseBracket()) {
    if (!got_color_open_bracket) {
      // Close without an open.
      return Result("Missing open bracket for probe command");
    }
    token = tokenizer_->NextToken();
  } else if (got_color_open_bracket) {
    // Open bracket without a close.
    return Result("Missing close bracket for probe command");
  }

  if (!token->IsEOS() && !token->IsEOL())
    return Result("Extra parameter to probe command: " +
                  token->ToOriginalString());

  commands_.push_back(std::move(cmd));
  return {};
}

Result CommandParser::ProcessTopology() {
  auto token = tokenizer_->NextToken();
  if (token->IsEOS() || token->IsEOL())
    return Result("Missing value for topology command");
  if (!token->IsString())
    return Result("Invalid value for topology command: " +
                  token->ToOriginalString());

  Topology topology = Topology::kPatchList;
  std::string topo = token->AsString();

  if (topo == "VK_PRIMITIVE_TOPOLOGY_PATCH_LIST")
    topology = Topology::kPatchList;
  else if (topo == "VK_PRIMITIVE_TOPOLOGY_POINT_LIST")
    topology = Topology::kPointList;
  else if (topo == "VK_PRIMITIVE_TOPOLOGY_LINE_LIST")
    topology = Topology::kLineList;
  else if (topo == "VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY")
    topology = Topology::kLineListWithAdjacency;
  else if (topo == "VK_PRIMITIVE_TOPOLOGY_LINE_STRIP")
    topology = Topology::kLineStrip;
  else if (topo == "VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY")
    topology = Topology::kLineStripWithAdjacency;
  else if (topo == "VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN")
    topology = Topology::kTriangleFan;
  else if (topo == "VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST")
    topology = Topology::kTriangleList;
  else if (topo == "VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY")
    topology = Topology::kTriangleListWithAdjacency;
  else if (topo == "VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP")
    topology = Topology::kTriangleStrip;
  else if (topo == "VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY")
    topology = Topology::kTriangleStripWithAdjacency;
  else
    return Result("Unknown value for topology command: " +
                  token->ToOriginalString());

  token = tokenizer_->NextToken();
  if (!token->IsEOS() && !token->IsEOL())
    return Result("Extra parameter for topology command: " +
                  token->ToOriginalString());

  pipeline_data_.SetTopology(topology);
  return {};
}

Result CommandParser::ProcessPolygonMode() {
  auto token = tokenizer_->NextToken();
  if (token->IsEOS() || token->IsEOL())
    return Result("Missing value for polygonMode command");
  if (!token->IsString())
    return Result("Invalid value for polygonMode command: " +
                  token->ToOriginalString());

  PolygonMode mode = PolygonMode::kFill;
  std::string m = token->AsString();
  if (m == "VK_POLYGON_MODE_FILL")
    mode = PolygonMode::kFill;
  else if (m == "VK_POLYGON_MODE_LINE")
    mode = PolygonMode::kLine;
  else if (m == "VK_POLYGON_MODE_POINT")
    mode = PolygonMode::kPoint;
  else
    return Result("Unknown value for polygonMode command: " +
                  token->ToOriginalString());

  token = tokenizer_->NextToken();
  if (!token->IsEOS() && !token->IsEOL())
    return Result("Extra parameter for polygonMode command: " +
                  token->ToOriginalString());

  pipeline_data_.SetPolygonMode(mode);
  return {};
}

Result CommandParser::ProcessLogicOp() {
  auto token = tokenizer_->NextToken();
  if (token->IsEOS() || token->IsEOL())
    return Result("Missing value for logicOp command");
  if (!token->IsString())
    return Result("Invalid value for logicOp command: " +
                  token->ToOriginalString());

  LogicOp op = LogicOp::kClear;
  std::string name = token->AsString();
  if (name == "VK_LOGIC_OP_CLEAR")
    op = LogicOp::kClear;
  else if (name == "VK_LOGIC_OP_AND")
    op = LogicOp::kAnd;
  else if (name == "VK_LOGIC_OP_AND_REVERSE")
    op = LogicOp::kAndReverse;
  else if (name == "VK_LOGIC_OP_COPY")
    op = LogicOp::kCopy;
  else if (name == "VK_LOGIC_OP_AND_INVERTED")
    op = LogicOp::kAndInverted;
  else if (name == "VK_LOGIC_OP_NO_OP")
    op = LogicOp::kNoOp;
  else if (name == "VK_LOGIC_OP_XOR")
    op = LogicOp::kXor;
  else if (name == "VK_LOGIC_OP_OR")
    op = LogicOp::kOr;
  else if (name == "VK_LOGIC_OP_NOR")
    op = LogicOp::kNor;
  else if (name == "VK_LOGIC_OP_EQUIVALENT")
    op = LogicOp::kEquivalent;
  else if (name == "VK_LOGIC_OP_INVERT")
    op = LogicOp::kInvert;
  else if (name == "VK_LOGIC_OP_OR_REVERSE")
    op = LogicOp::kOrReverse;
  else if (name == "VK_LOGIC_OP_COPY_INVERTED")
    op = LogicOp::kCopyInverted;
  else if (name == "VK_LOGIC_OP_OR_INVERTED")
    op = LogicOp::kOrInverted;
  else if (name == "VK_LOGIC_OP_NAND")
    op = LogicOp::kNand;
  else if (name == "VK_LOGIC_OP_SET")
    op = LogicOp::kSet;
  else
    return Result("Unknown value for logicOp command: " +
                  token->ToOriginalString());

  token = tokenizer_->NextToken();
  if (!token->IsEOS() && !token->IsEOL())
    return Result("Extra parameter for logicOp command: " +
                  token->ToOriginalString());

  pipeline_data_.SetLogicOp(op);
  return {};
}

Result CommandParser::ProcessCullMode() {
  auto token = tokenizer_->NextToken();
  if (token->IsEOS() || token->IsEOL())
    return Result("Missing value for cullMode command");
  if (!token->IsString())
    return Result("Invalid value for cullMode command: " +
                  token->ToOriginalString());

  CullMode mode = CullMode::kNone;
  while (!token->IsEOS() && !token->IsEOL()) {
    std::string name = token->AsString();

    if (name == "|") {
      // We treat everything as an |.
    } else if (name == "VK_CULL_MODE_FRONT_BIT") {
      if (mode == CullMode::kNone)
        mode = CullMode::kFront;
      else if (mode == CullMode::kBack)
        mode = CullMode::kFrontAndBack;
    } else if (name == "VK_CULL_MODE_BACK_BIT") {
      if (mode == CullMode::kNone)
        mode = CullMode::kBack;
      else if (mode == CullMode::kFront)
        mode = CullMode::kFrontAndBack;
    } else if (name == "VK_CULL_MODE_FRONT_AND_BACK") {
      mode = CullMode::kFrontAndBack;
    } else if (name == "VK_CULL_MODE_NONE") {
      // Do nothing ...
    } else {
      return Result("Unknown value for cullMode command: " +
                    token->ToOriginalString());
    }

    token = tokenizer_->NextToken();
  }

  pipeline_data_.SetCullMode(mode);
  return {};
}

Result CommandParser::ProcessFrontFace() {
  auto token = tokenizer_->NextToken();
  if (token->IsEOS() || token->IsEOL())
    return Result("Missing value for frontFace command");
  if (!token->IsString())
    return Result("Invalid value for frontFace command: " +
                  token->ToOriginalString());

  FrontFace face = FrontFace::kCounterClockwise;
  std::string f = token->AsString();
  if (f == "VK_FRONT_FACE_COUNTER_CLOCKWISE")
    face = FrontFace::kCounterClockwise;
  else if (f == "VK_FRONT_FACE_CLOCKWISE")
    face = FrontFace::kClockwise;
  else
    return Result("Unknown value for frontFace command: " +
                  token->ToOriginalString());

  token = tokenizer_->NextToken();
  if (!token->IsEOS() && !token->IsEOL())
    return Result("Extra parameter for frontFace command: " +
                  token->ToOriginalString());

  pipeline_data_.SetFrontFace(face);
  return {};
}

Result CommandParser::ProcessBooleanPipelineData(const std::string& name,
                                                 bool* value) {
  auto token = tokenizer_->NextToken();
  if (token->IsEOS() || token->IsEOL())
    return Result("Missing value for " + name + " command");
  if (!token->IsString())
    return Result("Invalid value for " + name +
                  " command: " + token->ToOriginalString());

  Result r = ParseBoolean(token->AsString(), value);
  if (!r.IsSuccess())
    return r;

  token = tokenizer_->NextToken();
  if (!token->IsEOS() && !token->IsEOL())
    return Result("Extra parameter for " + name +
                  " command: " + token->ToOriginalString());

  return {};
}

Result CommandParser::ProcessPrimitiveRestartEnable() {
  bool value = false;
  Result r = ProcessBooleanPipelineData("primitiveRestartEnable", &value);
  if (!r.IsSuccess())
    return r;

  pipeline_data_.SetEnablePrimitiveRestart(value);
  return {};
}

Result CommandParser::ProcessDepthClampEnable() {
  bool value = false;
  Result r = ProcessBooleanPipelineData("depthClampEnable", &value);
  if (!r.IsSuccess())
    return r;

  pipeline_data_.SetEnableDepthClamp(value);
  return {};
}

Result CommandParser::ProcessRasterizerDiscardEnable() {
  bool value = false;
  Result r = ProcessBooleanPipelineData("rasterizerDiscardEnable", &value);
  if (!r.IsSuccess())
    return r;

  pipeline_data_.SetEnableRasterizerDiscard(value);
  return {};
}

Result CommandParser::ProcessDepthBiasEnable() {
  bool value = false;
  Result r = ProcessBooleanPipelineData("depthBiasEnable", &value);
  if (!r.IsSuccess())
    return r;

  pipeline_data_.SetEnableDepthBias(value);
  return {};
}

Result CommandParser::ProcessLogicOpEnable() {
  bool value = false;
  Result r = ProcessBooleanPipelineData("logicOpEnable", &value);
  if (!r.IsSuccess())
    return r;

  pipeline_data_.SetEnableLogicOp(value);
  return {};
}

Result CommandParser::ProcessBlendEnable() {
  bool value = false;
  Result r = ProcessBooleanPipelineData("blendEnable", &value);
  if (!r.IsSuccess())
    return r;

  pipeline_data_.SetEnableBlend(value);
  return {};
}

Result CommandParser::ProcessDepthTestEnable() {
  bool value = false;
  Result r = ProcessBooleanPipelineData("depthTestEnable", &value);
  if (!r.IsSuccess())
    return r;

  pipeline_data_.SetEnableDepthTest(value);
  return {};
}

Result CommandParser::ProcessDepthWriteEnable() {
  bool value = false;
  Result r = ProcessBooleanPipelineData("depthWriteEnable", &value);
  if (!r.IsSuccess())
    return r;

  pipeline_data_.SetEnableDepthWrite(value);
  return {};
}

Result CommandParser::ProcessDepthBoundsTestEnable() {
  bool value = false;
  Result r = ProcessBooleanPipelineData("depthBoundsTestEnable", &value);
  if (!r.IsSuccess())
    return r;

  pipeline_data_.SetEnableDepthBoundsTest(value);
  return {};
}

Result CommandParser::ProcessStencilTestEnable() {
  bool value = false;
  Result r = ProcessBooleanPipelineData("stencilTestEnable", &value);
  if (!r.IsSuccess())
    return r;

  pipeline_data_.SetEnableStencilTest(value);
  return {};
}

Result CommandParser::ProcessFloatPipelineData(const std::string& name,
                                               float* value) {
  assert(value);

  auto token = tokenizer_->NextToken();
  if (token->IsEOS() || token->IsEOL())
    return Result("Missing value for " + name + " command");

  Result r = token->ConvertToDouble();
  if (!r.IsSuccess())
    return r;

  *value = token->AsFloat();

  token = tokenizer_->NextToken();
  if (!token->IsEOS() && !token->IsEOL())
    return Result("Extra parameter for " + name +
                  " command: " + token->ToOriginalString());

  return {};
}

Result CommandParser::ProcessDepthBiasConstantFactor() {
  float value = 0.0;
  Result r = ProcessFloatPipelineData("depthBiasConstantFactor", &value);
  if (!r.IsSuccess())
    return r;

  pipeline_data_.SetDepthBiasConstantFactor(value);
  return {};
}

Result CommandParser::ProcessDepthBiasClamp() {
  float value = 0.0;
  Result r = ProcessFloatPipelineData("depthBiasClamp", &value);
  if (!r.IsSuccess())
    return r;

  pipeline_data_.SetDepthBiasClamp(value);
  return {};
}

Result CommandParser::ProcessDepthBiasSlopeFactor() {
  float value = 0.0;
  Result r = ProcessFloatPipelineData("depthBiasSlopeFactor", &value);
  if (!r.IsSuccess())
    return r;

  pipeline_data_.SetDepthBiasSlopeFactor(value);
  return {};
}

Result CommandParser::ProcessLineWidth() {
  float value = 0.0;
  Result r = ProcessFloatPipelineData("lineWidth", &value);
  if (!r.IsSuccess())
    return r;

  pipeline_data_.SetLineWidth(value);
  return {};
}

Result CommandParser::ProcessMinDepthBounds() {
  float value = 0.0;
  Result r = ProcessFloatPipelineData("minDepthBounds", &value);
  if (!r.IsSuccess())
    return r;

  pipeline_data_.SetMinDepthBounds(value);
  return {};
}

Result CommandParser::ProcessMaxDepthBounds() {
  float value = 0.0;
  Result r = ProcessFloatPipelineData("maxDepthBounds", &value);
  if (!r.IsSuccess())
    return r;

  pipeline_data_.SetMaxDepthBounds(value);
  return {};
}

Result CommandParser::ParseBlendFactorName(const std::string& name,
                                           BlendFactor* factor) {
  assert(factor);

  if (name == "VK_BLEND_FACTOR_ZERO")
    *factor = BlendFactor::kZero;
  else if (name == "VK_BLEND_FACTOR_ONE")
    *factor = BlendFactor::kOne;
  else if (name == "VK_BLEND_FACTOR_SRC_COLOR")
    *factor = BlendFactor::kSrcColor;
  else if (name == "VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR")
    *factor = BlendFactor::kOneMinusSrcColor;
  else if (name == "VK_BLEND_FACTOR_DST_COLOR")
    *factor = BlendFactor::kDstColor;
  else if (name == "VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR")
    *factor = BlendFactor::kOneMinusDstColor;
  else if (name == "VK_BLEND_FACTOR_SRC_ALPHA")
    *factor = BlendFactor::kSrcAlpha;
  else if (name == "VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA")
    *factor = BlendFactor::kOneMinusSrcAlpha;
  else if (name == "VK_BLEND_FACTOR_DST_ALPHA")
    *factor = BlendFactor::kDstAlpha;
  else if (name == "VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA")
    *factor = BlendFactor::kOneMinusDstAlpha;
  else if (name == "VK_BLEND_FACTOR_CONSTANT_COLOR")
    *factor = BlendFactor::kConstantColor;
  else if (name == "VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR")
    *factor = BlendFactor::kOneMinusConstantColor;
  else if (name == "VK_BLEND_FACTOR_CONSTANT_ALPHA")
    *factor = BlendFactor::kConstantAlpha;
  else if (name == "VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA")
    *factor = BlendFactor::kOneMinusConstantAlpha;
  else if (name == "VK_BLEND_FACTOR_SRC_ALPHA_SATURATE")
    *factor = BlendFactor::kSrcAlphaSaturate;
  else if (name == "VK_BLEND_FACTOR_SRC1_COLOR")
    *factor = BlendFactor::kSrc1Color;
  else if (name == "VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR")
    *factor = BlendFactor::kOneMinusSrc1Color;
  else if (name == "VK_BLEND_FACTOR_SRC1_ALPHA")
    *factor = BlendFactor::kSrc1Alpha;
  else if (name == "VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA")
    *factor = BlendFactor::kOneMinusSrc1Alpha;
  else
    return Result("Unknown BlendFactor provided: " + name);

  return {};
}

Result CommandParser::ParseBlendFactor(const std::string& name,
                                       BlendFactor* factor) {
  auto token = tokenizer_->NextToken();
  if (token->IsEOL() || token->IsEOS())
    return Result(std::string("Missing parameter for ") + name + " command");
  if (!token->IsString())
    return Result(std::string("Invalid parameter for ") + name +
                  " command: " + token->ToOriginalString());

  Result r = ParseBlendFactorName(token->AsString(), factor);
  if (!r.IsSuccess())
    return r;

  token = tokenizer_->NextToken();
  if (!token->IsEOS() && !token->IsEOL())
    return Result(std::string("Extra parameter for ") + name +
                  " command: " + token->ToOriginalString());

  return {};
}

Result CommandParser::ProcessSrcAlphaBlendFactor() {
  BlendFactor factor = BlendFactor::kZero;
  Result r = ParseBlendFactor("srcAlphaBlendFactor", &factor);
  if (!r.IsSuccess())
    return r;

  pipeline_data_.SetSrcAlphaBlendFactor(factor);
  return {};
}

Result CommandParser::ProcessDstAlphaBlendFactor() {
  BlendFactor factor = BlendFactor::kZero;
  Result r = ParseBlendFactor("dstAlphaBlendFactor", &factor);
  if (!r.IsSuccess())
    return r;

  pipeline_data_.SetDstAlphaBlendFactor(factor);
  return {};
}

Result CommandParser::ProcessSrcColorBlendFactor() {
  BlendFactor factor = BlendFactor::kZero;
  Result r = ParseBlendFactor("srcColorBlendFactor", &factor);
  if (!r.IsSuccess())
    return r;

  pipeline_data_.SetSrcColorBlendFactor(factor);
  return {};
}

Result CommandParser::ProcessDstColorBlendFactor() {
  BlendFactor factor = BlendFactor::kZero;
  Result r = ParseBlendFactor("dstColorBlendFactor", &factor);
  if (!r.IsSuccess())
    return r;

  pipeline_data_.SetDstColorBlendFactor(factor);
  return {};
}

Result CommandParser::ParseBlendOpName(const std::string& name, BlendOp* op) {
  assert(op);

  if (name == "VK_BLEND_OP_ADD")
    *op = BlendOp::kAdd;
  else if (name == "VK_BLEND_OP_ADD")
    *op = BlendOp::kAdd;
  else if (name == "VK_BLEND_OP_SUBTRACT")
    *op = BlendOp::kSubtract;
  else if (name == "VK_BLEND_OP_REVERSE_SUBTRACT")
    *op = BlendOp::kReverseSubtract;
  else if (name == "VK_BLEND_OP_MIN")
    *op = BlendOp::kMin;
  else if (name == "VK_BLEND_OP_MAX")
    *op = BlendOp::kMax;
  else if (name == "VK_BLEND_OP_ZERO_EXT")
    *op = BlendOp::kZero;
  else if (name == "VK_BLEND_OP_SRC_EXT")
    *op = BlendOp::kSrc;
  else if (name == "VK_BLEND_OP_DST_EXT")
    *op = BlendOp::kDst;
  else if (name == "VK_BLEND_OP_SRC_OVER_EXT")
    *op = BlendOp::kSrcOver;
  else if (name == "VK_BLEND_OP_DST_OVER_EXT")
    *op = BlendOp::kDstOver;
  else if (name == "VK_BLEND_OP_SRC_IN_EXT")
    *op = BlendOp::kSrcIn;
  else if (name == "VK_BLEND_OP_DST_IN_EXT")
    *op = BlendOp::kDstIn;
  else if (name == "VK_BLEND_OP_SRC_OUT_EXT")
    *op = BlendOp::kSrcOut;
  else if (name == "VK_BLEND_OP_DST_OUT_EXT")
    *op = BlendOp::kDstOut;
  else if (name == "VK_BLEND_OP_SRC_ATOP_EXT")
    *op = BlendOp::kSrcAtop;
  else if (name == "VK_BLEND_OP_DST_ATOP_EXT")
    *op = BlendOp::kDstAtop;
  else if (name == "VK_BLEND_OP_XOR_EXT")
    *op = BlendOp::kXor;
  else if (name == "VK_BLEND_OP_MULTIPLY_EXT")
    *op = BlendOp::kMultiply;
  else if (name == "VK_BLEND_OP_SCREEN_EXT")
    *op = BlendOp::kScreen;
  else if (name == "VK_BLEND_OP_OVERLAY_EXT")
    *op = BlendOp::kOverlay;
  else if (name == "VK_BLEND_OP_DARKEN_EXT")
    *op = BlendOp::kDarken;
  else if (name == "VK_BLEND_OP_LIGHTEN_EXT")
    *op = BlendOp::kLighten;
  else if (name == "VK_BLEND_OP_COLORDODGE_EXT")
    *op = BlendOp::kColorDodge;
  else if (name == "VK_BLEND_OP_COLORBURN_EXT")
    *op = BlendOp::kColorBurn;
  else if (name == "VK_BLEND_OP_HARDLIGHT_EXT")
    *op = BlendOp::kHardLight;
  else if (name == "VK_BLEND_OP_SOFTLIGHT_EXT")
    *op = BlendOp::kSoftLight;
  else if (name == "VK_BLEND_OP_DIFFERENCE_EXT")
    *op = BlendOp::kDifference;
  else if (name == "VK_BLEND_OP_EXCLUSION_EXT")
    *op = BlendOp::kExclusion;
  else if (name == "VK_BLEND_OP_INVERT_EXT")
    *op = BlendOp::kInvert;
  else if (name == "VK_BLEND_OP_INVERT_RGB_EXT")
    *op = BlendOp::kInvertRGB;
  else if (name == "VK_BLEND_OP_LINEARDODGE_EXT")
    *op = BlendOp::kLinearDodge;
  else if (name == "VK_BLEND_OP_LINEARBURN_EXT")
    *op = BlendOp::kLinearBurn;
  else if (name == "VK_BLEND_OP_VIVIDLIGHT_EXT")
    *op = BlendOp::kVividLight;
  else if (name == "VK_BLEND_OP_LINEARLIGHT_EXT")
    *op = BlendOp::kLinearLight;
  else if (name == "VK_BLEND_OP_PINLIGHT_EXT")
    *op = BlendOp::kPinLight;
  else if (name == "VK_BLEND_OP_HARDMIX_EXT")
    *op = BlendOp::kHardMix;
  else if (name == "VK_BLEND_OP_HSL_HUE_EXT")
    *op = BlendOp::kHslHue;
  else if (name == "VK_BLEND_OP_HSL_SATURATION_EXT")
    *op = BlendOp::kHslSaturation;
  else if (name == "VK_BLEND_OP_HSL_COLOR_EXT")
    *op = BlendOp::kHslColor;
  else if (name == "VK_BLEND_OP_HSL_LUMINOSITY_EXT")
    *op = BlendOp::kHslLuminosity;
  else if (name == "VK_BLEND_OP_PLUS_EXT")
    *op = BlendOp::kPlus;
  else if (name == "VK_BLEND_OP_PLUS_CLAMPED_EXT")
    *op = BlendOp::kPlusClamped;
  else if (name == "VK_BLEND_OP_PLUS_CLAMPED_ALPHA_EXT")
    *op = BlendOp::kPlusClampedAlpha;
  else if (name == "VK_BLEND_OP_PLUS_DARKER_EXT")
    *op = BlendOp::kPlusDarker;
  else if (name == "VK_BLEND_OP_MINUS_EXT")
    *op = BlendOp::kMinus;
  else if (name == "VK_BLEND_OP_MINUS_CLAMPED_EXT")
    *op = BlendOp::kMinusClamped;
  else if (name == "VK_BLEND_OP_CONTRAST_EXT")
    *op = BlendOp::kContrast;
  else if (name == "VK_BLEND_OP_INVERT_OVG_EXT")
    *op = BlendOp::kInvertOvg;
  else if (name == "VK_BLEND_OP_RED_EXT")
    *op = BlendOp::kRed;
  else if (name == "VK_BLEND_OP_GREEN_EXT")
    *op = BlendOp::kGreen;
  else if (name == "VK_BLEND_OP_BLUE_EXT")
    *op = BlendOp::kBlue;
  else
    return Result("Unknown BlendOp provided: " + name);

  return {};
}

Result CommandParser::ParseBlendOp(const std::string& name, BlendOp* op) {
  auto token = tokenizer_->NextToken();
  if (token->IsEOL() || token->IsEOS())
    return Result(std::string("Missing parameter for ") + name + " command");
  if (!token->IsString())
    return Result(std::string("Invalid parameter for ") + name +
                  " command: " + token->ToOriginalString());

  Result r = ParseBlendOpName(token->AsString(), op);
  if (!r.IsSuccess())
    return r;

  token = tokenizer_->NextToken();
  if (!token->IsEOS() && !token->IsEOL())
    return Result(std::string("Extra parameter for ") + name +
                  " command: " + token->ToOriginalString());

  return {};
}

Result CommandParser::ProcessColorBlendOp() {
  BlendOp op = BlendOp::kAdd;
  Result r = ParseBlendOp("colorBlendOp", &op);
  if (!r.IsSuccess())
    return r;

  pipeline_data_.SetColorBlendOp(op);
  return {};
}

Result CommandParser::ProcessAlphaBlendOp() {
  BlendOp op = BlendOp::kAdd;
  Result r = ParseBlendOp("alphaBlendOp", &op);
  if (!r.IsSuccess())
    return r;

  pipeline_data_.SetAlphaBlendOp(op);
  return {};
}

Result CommandParser::ParseCompareOp(const std::string& name, CompareOp* op) {
  auto token = tokenizer_->NextToken();
  if (token->IsEOL() || token->IsEOS())
    return Result(std::string("Missing parameter for ") + name + " command");
  if (!token->IsString())
    return Result(std::string("Invalid parameter for ") + name +
                  " command: " + token->ToOriginalString());

  Result r = ParseCompareOpName(token->AsString(), op);
  if (!r.IsSuccess())
    return r;

  token = tokenizer_->NextToken();
  if (!token->IsEOS() && !token->IsEOL())
    return Result(std::string("Extra parameter for ") + name +
                  " command: " + token->ToOriginalString());

  return {};
}

Result CommandParser::ParseCompareOpName(const std::string& name,
                                         CompareOp* op) {
  assert(op);

  if (name == "VK_COMPARE_OP_NEVER")
    *op = CompareOp::kNever;
  else if (name == "VK_COMPARE_OP_LESS")
    *op = CompareOp::kLess;
  else if (name == "VK_COMPARE_OP_EQUAL")
    *op = CompareOp::kEqual;
  else if (name == "VK_COMPARE_OP_LESS_OR_EQUAL")
    *op = CompareOp::kLessOrEqual;
  else if (name == "VK_COMPARE_OP_GREATER")
    *op = CompareOp::kGreater;
  else if (name == "VK_COMPARE_OP_NOT_EQUAL")
    *op = CompareOp::kNotEqual;
  else if (name == "VK_COMPARE_OP_GREATER_OR_EQUAL")
    *op = CompareOp::kGreaterOrEqual;
  else if (name == "VK_COMPARE_OP_ALWAYS")
    *op = CompareOp::kAlways;
  else
    return Result("Unknown CompareOp provided: " + name);

  return {};
}

Result CommandParser::ProcessDepthCompareOp() {
  CompareOp op = CompareOp::kNever;
  Result r = ParseCompareOp("depthCompareOp", &op);
  if (!r.IsSuccess())
    return r;

  pipeline_data_.SetDepthCompareOp(op);
  return {};
}

Result CommandParser::ProcessFrontCompareOp() {
  CompareOp op = CompareOp::kNever;
  Result r = ParseCompareOp("front.compareOp", &op);
  if (!r.IsSuccess())
    return r;

  pipeline_data_.SetFrontCompareOp(op);
  return {};
}

Result CommandParser::ProcessBackCompareOp() {
  CompareOp op = CompareOp::kNever;
  Result r = ParseCompareOp("back.compareOp", &op);
  if (!r.IsSuccess())
    return r;

  pipeline_data_.SetBackCompareOp(op);
  return {};
}

Result CommandParser::ParseStencilOp(const std::string& name, StencilOp* op) {
  auto token = tokenizer_->NextToken();
  if (token->IsEOL() || token->IsEOS())
    return Result(std::string("Missing parameter for ") + name + " command");
  if (!token->IsString())
    return Result(std::string("Invalid parameter for ") + name +
                  " command: " + token->ToOriginalString());

  Result r = ParseStencilOpName(token->AsString(), op);
  if (!r.IsSuccess())
    return r;

  token = tokenizer_->NextToken();
  if (!token->IsEOS() && !token->IsEOL())
    return Result(std::string("Extra parameter for ") + name +
                  " command: " + token->ToOriginalString());

  return {};
}

Result CommandParser::ParseStencilOpName(const std::string& name,
                                         StencilOp* op) {
  assert(op);

  if (name == "VK_STENCIL_OP_KEEP")
    *op = StencilOp::kKeep;
  else if (name == "VK_STENCIL_OP_ZERO")
    *op = StencilOp::kZero;
  else if (name == "VK_STENCIL_OP_REPLACE")
    *op = StencilOp::kReplace;
  else if (name == "VK_STENCIL_OP_INCREMENT_AND_CLAMP")
    *op = StencilOp::kIncrementAndClamp;
  else if (name == "VK_STENCIL_OP_DECREMENT_AND_CLAMP")
    *op = StencilOp::kDecrementAndClamp;
  else if (name == "VK_STENCIL_OP_INVERT")
    *op = StencilOp::kInvert;
  else if (name == "VK_STENCIL_OP_INCREMENT_AND_WRAP")
    *op = StencilOp::kIncrementAndWrap;
  else if (name == "VK_STENCIL_OP_DECREMENT_AND_WRAP")
    *op = StencilOp::kDecrementAndWrap;
  else
    return Result("Unknown StencilOp provided: " + name);

  return {};
}

Result CommandParser::ProcessFrontFailOp() {
  StencilOp op = StencilOp::kKeep;
  Result r = ParseStencilOp("front.failOp", &op);
  if (!r.IsSuccess())
    return r;

  pipeline_data_.SetFrontFailOp(op);
  return {};
}

Result CommandParser::ProcessFrontPassOp() {
  StencilOp op = StencilOp::kKeep;
  Result r = ParseStencilOp("front.passOp", &op);
  if (!r.IsSuccess())
    return r;

  pipeline_data_.SetFrontPassOp(op);
  return {};
}

Result CommandParser::ProcessFrontDepthFailOp() {
  StencilOp op = StencilOp::kKeep;
  Result r = ParseStencilOp("front.depthFailOp", &op);
  if (!r.IsSuccess())
    return r;

  pipeline_data_.SetFrontDepthFailOp(op);
  return {};
}

Result CommandParser::ProcessBackFailOp() {
  StencilOp op = StencilOp::kKeep;
  Result r = ParseStencilOp("back.failOp", &op);
  if (!r.IsSuccess())
    return r;

  pipeline_data_.SetBackFailOp(op);
  return {};
}

Result CommandParser::ProcessBackPassOp() {
  StencilOp op = StencilOp::kKeep;
  Result r = ParseStencilOp("back.passOp", &op);
  if (!r.IsSuccess())
    return r;

  pipeline_data_.SetBackPassOp(op);
  return {};
}

Result CommandParser::ProcessBackDepthFailOp() {
  StencilOp op = StencilOp::kKeep;
  Result r = ParseStencilOp("back.depthFailOp", &op);
  if (!r.IsSuccess())
    return r;

  pipeline_data_.SetBackDepthFailOp(op);
  return {};
}

Result CommandParser::ProcessFrontCompareMask() {
  return Result("front.compareMask not implemented");
}

Result CommandParser::ProcessFrontWriteMask() {
  return Result("front.writeMask not implemented");
}

Result CommandParser::ProcessBackCompareMask() {
  return Result("back.compareMask not implemented");
}

Result CommandParser::ProcessBackWriteMask() {
  return Result("back.writeMask not implemented");
}

Result CommandParser::ProcessFrontReference() {
  auto token = tokenizer_->NextToken();
  if (token->IsEOL() || token->IsEOS())
    return Result("Missing parameter for front.reference command");
  if (!token->IsInteger())
    return Result("Invalid parameter for front.reference command: " +
                  token->ToOriginalString());

  pipeline_data_.SetFrontReference(token->AsUint32());

  token = tokenizer_->NextToken();
  if (!token->IsEOS() && !token->IsEOL())
    return Result("Extra parameter for front.reference command: " +
                  token->ToOriginalString());

  return {};
}

Result CommandParser::ProcessBackReference() {
  auto token = tokenizer_->NextToken();
  if (token->IsEOL() || token->IsEOS())
    return Result("Missing parameter for back.reference command");
  if (!token->IsInteger())
    return Result("Invalid parameter for back.reference command: " +
                  token->ToOriginalString());

  pipeline_data_.SetBackReference(token->AsUint32());

  token = tokenizer_->NextToken();
  if (!token->IsEOS() && !token->IsEOL())
    return Result("Extra parameter for back.reference command: " +
                  token->ToOriginalString());

  return {};
}

Result CommandParser::ProcessColorWriteMask() {
  auto token = tokenizer_->NextToken();
  if (token->IsEOS() || token->IsEOL())
    return Result("Missing parameter for colorWriteMask command");
  if (!token->IsString())
    return Result("Invalid parameter for colorWriteMask command: " +
                  token->ToOriginalString());

  uint8_t mask = 0;
  while (!token->IsEOS() && !token->IsEOL()) {
    std::string name = token->AsString();

    if (name == "|") {
      // We treat everything as an |.
    } else if (name == "VK_COLOR_COMPONENT_R_BIT") {
      mask |= kColorMaskR;
    } else if (name == "VK_COLOR_COMPONENT_G_BIT") {
      mask |= kColorMaskG;
    } else if (name == "VK_COLOR_COMPONENT_B_BIT") {
      mask |= kColorMaskB;
    } else if (name == "VK_COLOR_COMPONENT_A_BIT") {
      mask |= kColorMaskA;
    } else {
      return Result("Unknown parameter for colorWriteMask command: " + name);
    }

    token = tokenizer_->NextToken();
  }

  pipeline_data_.SetColorWriteMask(mask);
  return {};
}

Result CommandParser::ParseComparator(const std::string& name,
                                      ProbeSSBOCommand::Comparator* op) {
  if (name == "==")
    *op = ProbeSSBOCommand::Comparator::kEqual;
  else if (name == "!=")
    *op = ProbeSSBOCommand::Comparator::kNotEqual;
  else if (name == "~=")
    *op = ProbeSSBOCommand::Comparator::kFuzzyEqual;
  else if (name == "<")
    *op = ProbeSSBOCommand::Comparator::kLess;
  else if (name == "<=")
    *op = ProbeSSBOCommand::Comparator::kLessOrEqual;
  else if (name == ">")
    *op = ProbeSSBOCommand::Comparator::kGreater;
  else if (name == ">=")
    *op = ProbeSSBOCommand::Comparator::kGreaterOrEqual;
  else
    return Result("Invalid comparator: " + name);
  return {};
}

Result CommandParser::ProcessProbeSSBO() {
  size_t cur_line = tokenizer_->GetCurrentLine();

  auto token = tokenizer_->NextToken();
  if (token->IsEOL() || token->IsEOS())
    return Result("Missing values for probe ssbo command");
  if (!token->IsString())
    return Result("Invalid type for probe ssbo command: " +
                  token->ToOriginalString());

  DatumTypeParser tp;
  auto type = tp.Parse(token->AsString());
  if (!type)
    return Result("Invalid type provided: " + token->AsString());

  token = tokenizer_->NextToken();
  if (!token->IsInteger())
    return Result("Invalid binding value for probe ssbo command: " +
                  token->ToOriginalString());

  uint32_t val = token->AsUint32();

  uint32_t set = 0;
  uint32_t binding = 0;
  token = tokenizer_->NextToken();
  if (token->IsString()) {
    auto& str = token->AsString();
    if (str.size() >= 2 && str[0] == ':') {
      set = val;

      auto substr = str.substr(1, str.size());
      uint64_t binding_val = strtoul(substr.c_str(), nullptr, 10);
      if (binding_val > std::numeric_limits<uint32_t>::max())
        return Result("binding value too large in probe ssbo command: " +
                      token->ToOriginalString());

      binding = static_cast<uint32_t>(binding_val);
    } else {
      return Result("Invalid value for probe ssbo command: " +
                    token->ToOriginalString());
    }

    token = tokenizer_->NextToken();
  } else {
    binding = val;
  }

  auto* buffer = pipeline_->GetBufferForBinding(set, binding);
  if (!buffer) {
    return Result("unable to find buffer at descriptor set " +
                  std::to_string(set) + " and binding " +
                  std::to_string(binding));
  }

  auto fmt = MakeUnique<Format>(type.get());
  if (buffer->FormatIsDefault() || !buffer->GetFormat()) {
    buffer->SetFormat(fmt.get());
  } else if (buffer->GetFormat() && !buffer->GetFormat()->Equal(fmt.get())) {
    return Result("probe format does not match buffer format");
  }

  auto cmd = MakeUnique<ProbeSSBOCommand>(buffer);
  cmd->SetLine(cur_line);
  cmd->SetTolerances(current_tolerances_);
  cmd->SetFormat(fmt.get());
  cmd->SetDescriptorSet(set);
  cmd->SetBinding(binding);

  script_->RegisterFormat(std::move(fmt));
  script_->RegisterType(std::move(type));

  if (!token->IsInteger())
    return Result("Invalid offset for probe ssbo command: " +
                  token->ToOriginalString());

  cmd->SetOffset(token->AsUint32());

  token = tokenizer_->NextToken();
  if (!token->IsString())
    return Result("Invalid comparator for probe ssbo command: " +
                  token->ToOriginalString());

  ProbeSSBOCommand::Comparator comp = ProbeSSBOCommand::Comparator::kEqual;
  Result r = ParseComparator(token->AsString(), &comp);
  if (!r.IsSuccess())
    return r;

  cmd->SetComparator(comp);

  std::vector<Value> values;
  r = ParseValues("probe ssbo", cmd->GetFormat(), &values);
  if (!r.IsSuccess())
    return r;

  cmd->SetValues(std::move(values));

  commands_.push_back(std::move(cmd));
  return {};
}

}  // namespace vkscript
}  // namespace amber
