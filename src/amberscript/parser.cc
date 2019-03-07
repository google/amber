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

#include "src/amberscript/parser.h"

#include <cassert>
#include <limits>
#include <string>
#include <utility>
#include <vector>

#include "src/format_parser.h"
#include "src/make_unique.h"
#include "src/shader_data.h"
#include "src/tokenizer.h"

namespace amber {
namespace amberscript {

Parser::Parser() : amber::Parser() {}

Parser::~Parser() = default;

std::string Parser::make_error(const std::string& err) {
  return std::to_string(tokenizer_->GetCurrentLine()) + ": " + err;
}

Result Parser::Parse(const std::string& data) {
  tokenizer_ = MakeUnique<Tokenizer>(data);

  for (auto token = tokenizer_->NextToken(); !token->IsEOS();
       token = tokenizer_->NextToken()) {
    if (token->IsEOL())
      continue;
    if (!token->IsString())
      return Result(make_error("expected string"));

    Result r;
    std::string tok = token->AsString();
    if (tok == "BUFFER") {
      r = ParseBuffer();
    } else if (tok == "PIPELINE") {
      r = ParsePipelineBlock();
    } else if (tok == "SHADER") {
      r = ParseShaderBlock();
    } else if (tok == "RUN") {
      r = ParseRun();
    } else if (tok == "CLEAR") {
      r = ParseClear();
    } else {
      r = Result("unknown token: " + tok);
    }
    if (!r.IsSuccess())
      return Result(make_error(r.Error()));
  }

  // Generate any  needed color and depth attachments. This is done before
  // validating in case one of the pipelines specifies the framebuffer size
  // it needs to be verified against all other pipelines.
  for (const auto& pipeline : script_->GetPipelines()) {
    // Add a color attachment if needed
    if (pipeline->GetColorAttachments().empty()) {
      auto* buf = script_->GetBuffer(Pipeline::kGeneratedColorBuffer);
      if (!buf) {
        auto new_buf = pipeline->GenerateDefaultColorAttachmentBuffer();
        buf = new_buf.get();

        Result r = script_->AddBuffer(std::move(new_buf));
        if (!r.IsSuccess())
          return r;
      }
      Result r = pipeline->AddColorAttachment(buf, 0);
      if (!r.IsSuccess())
        return r;
    }

    // Add a depth buffer if needed
    if (pipeline->GetDepthBuffer().buffer == nullptr) {
      auto* buf = script_->GetBuffer(Pipeline::kGeneratedDepthBuffer);
      if (!buf) {
        auto new_buf = pipeline->GenerateDefaultDepthAttachmentBuffer();
        buf = new_buf.get();

        Result r = script_->AddBuffer(std::move(new_buf));
        if (!r.IsSuccess())
          return r;
      }

      Result r = pipeline->SetDepthBuffer(buf);
      if (!r.IsSuccess())
        return r;
    }
  }

  // Validate all the pipelines at the end. This allows us to verify the
  // framebuffer sizes are consistent over pipelines.
  for (const auto& pipeline : script_->GetPipelines()) {
    Result r = pipeline->Validate();
    if (!r.IsSuccess())
      return r;
  }

  return {};
}

Result Parser::ToShaderType(const std::string& str, ShaderType* type) {
  assert(type);

  if (str == "vertex")
    *type = kShaderTypeVertex;
  else if (str == "fragment")
    *type = kShaderTypeFragment;
  else if (str == "geometry")
    *type = kShaderTypeGeometry;
  else if (str == "tessellation_evaluation")
    *type = kShaderTypeTessellationEvaluation;
  else if (str == "tessellation_control")
    *type = kShaderTypeTessellationControl;
  else if (str == "compute")
    *type = kShaderTypeCompute;
  else if (str == "multi")
    *type = kShaderTypeMulti;
  else
    return Result("unknown shader type: " + str);
  return {};
}

Result Parser::ToShaderFormat(const std::string& str, ShaderFormat* fmt) {
  assert(fmt);

  if (str == "GLSL")
    *fmt = kShaderFormatGlsl;
  else if (str == "SPIRV-ASM")
    *fmt = kShaderFormatSpirvAsm;
  else if (str == "SPIRV-HEX")
    *fmt = kShaderFormatSpirvHex;
  else
    return Result("unknown shader format: " + str);
  return {};
}

Result Parser::ToPipelineType(const std::string& str, PipelineType* type) {
  assert(type);

  if (str == "compute")
    *type = PipelineType::kCompute;
  else if (str == "graphics")
    *type = PipelineType::kGraphics;
  else
    return Result("unknown pipeline type: " + str);
  return {};
}

Result Parser::ToDatumType(const std::string& str, DatumType* type) {
  assert(type);

  if (str == "int8") {
    type->SetType(DataType::kInt8);
  } else if (str == "int16") {
    type->SetType(DataType::kInt16);
  } else if (str == "int32") {
    type->SetType(DataType::kInt32);
  } else if (str == "int64") {
    type->SetType(DataType::kInt64);
  } else if (str == "uint8") {
    type->SetType(DataType::kUint8);
  } else if (str == "uint16") {
    type->SetType(DataType::kUint16);
  } else if (str == "uint32") {
    type->SetType(DataType::kUint32);
  } else if (str == "uint64") {
    type->SetType(DataType::kUint64);
  } else if (str == "float") {
    type->SetType(DataType::kFloat);
  } else if (str == "double") {
    type->SetType(DataType::kDouble);
  } else if (str.length() > 7 && str.substr(0, 3) == "vec") {
    if (str[4] != '<' || str[str.length() - 1] != '>')
      return Result("invalid data_type provided");

    if (str[3] == '2')
      type->SetRowCount(2);
    else if (str[3] == '3')
      type->SetRowCount(3);
    else if (str[3] == '4')
      type->SetRowCount(4);
    else
      return Result("invalid data_type provided");

    DatumType subtype;
    Result r = ToDatumType(str.substr(5, str.length() - 6), &subtype);
    if (!r.IsSuccess())
      return r;

    if (subtype.RowCount() > 1 || subtype.ColumnCount() > 1)
      return Result("invalid data_type provided");

    type->SetType(subtype.GetType());

  } else if (str.length() > 9 && str.substr(0, 3) == "mat") {
    if (str[4] != 'x' || str[6] != '<' || str[str.length() - 1] != '>')
      return Result("invalid data_type provided");

    if (str[3] == '2')
      type->SetRowCount(2);
    else if (str[3] == '3')
      type->SetRowCount(3);
    else if (str[3] == '4')
      type->SetRowCount(4);
    else
      return Result("invalid data_type provided");

    if (str[5] == '2')
      type->SetColumnCount(2);
    else if (str[5] == '3')
      type->SetColumnCount(3);
    else if (str[5] == '4')
      type->SetColumnCount(4);
    else
      return Result("invalid data_type provided");

    DatumType subtype;
    Result r = ToDatumType(str.substr(7, str.length() - 8), &subtype);
    if (!r.IsSuccess())
      return r;

    if (subtype.RowCount() > 1 || subtype.ColumnCount() > 1)
      return Result("invalid data_type provided");

    type->SetType(subtype.GetType());
  } else {
    return Result("invalid data_type provided");
  }

  return {};
}

Result Parser::ValidateEndOfStatement(const std::string& name) {
  auto token = tokenizer_->NextToken();
  if (token->IsEOL() || token->IsEOS())
    return {};
  return Result("extra parameters after " + name);
}

Result Parser::ParseShaderBlock() {
  auto token = tokenizer_->NextToken();
  if (!token->IsString())
    return Result("invalid token when looking for shader type");

  ShaderType type = kShaderTypeVertex;
  Result r = ToShaderType(token->AsString(), &type);
  if (!r.IsSuccess())
    return r;

  auto shader = MakeUnique<Shader>(type);

  token = tokenizer_->NextToken();
  if (!token->IsString())
    return Result("invalid token when looking for shader name");

  shader->SetName(token->AsString());

  token = tokenizer_->NextToken();
  if (!token->IsString())
    return Result("invalid token when looking for shader format");

  std::string fmt = token->AsString();
  if (fmt == "PASSTHROUGH") {
    if (type != kShaderTypeVertex) {
      return Result(
          "invalid shader type for PASSTHROUGH. Only vertex "
          "PASSTHROUGH allowed");
    }
    shader->SetFormat(kShaderFormatSpirvAsm);
    shader->SetData(kPassThroughShader);

    r = script_->AddShader(std::move(shader));
    if (!r.IsSuccess())
      return r;

    return ValidateEndOfStatement("SHADER PASSTHROUGH");
  }

  ShaderFormat format = kShaderFormatGlsl;
  r = ToShaderFormat(fmt, &format);
  if (!r.IsSuccess())
    return r;

  shader->SetFormat(format);

  r = ValidateEndOfStatement("SHADER command");
  if (!r.IsSuccess())
    return r;

  std::string data = tokenizer_->ExtractToNext("END");
  if (data.empty())
    return Result("SHADER must not be empty");

  shader->SetData(data);

  token = tokenizer_->NextToken();
  if (!token->IsString() || token->AsString() != "END")
    return Result("SHADER missing END command");

  r = script_->AddShader(std::move(shader));
  if (!r.IsSuccess())
    return r;

  return ValidateEndOfStatement("END");
}

Result Parser::ParsePipelineBlock() {
  auto token = tokenizer_->NextToken();
  if (!token->IsString())
    return Result("invalid token when looking for pipeline type");

  PipelineType type = PipelineType::kCompute;
  Result r = ToPipelineType(token->AsString(), &type);
  if (!r.IsSuccess())
    return r;

  auto pipeline = MakeUnique<Pipeline>(type);

  token = tokenizer_->NextToken();
  if (!token->IsString())
    return Result("invalid token when looking for pipeline name");

  pipeline->SetName(token->AsString());

  r = ValidateEndOfStatement("PIPELINE command");
  if (!r.IsSuccess())
    return r;

  for (token = tokenizer_->NextToken(); !token->IsEOS();
       token = tokenizer_->NextToken()) {
    if (token->IsEOL())
      continue;
    if (!token->IsString())
      return Result("expected string");

    std::string tok = token->AsString();
    if (tok == "END") {
      break;
    } else if (tok == "ATTACH") {
      r = ParsePipelineAttach(pipeline.get());
    } else if (tok == "SHADER_OPTIMIZATION") {
      r = ParsePipelineShaderOptimizations(pipeline.get());
    } else if (tok == "FRAMEBUFFER_SIZE") {
      r = ParsePipelineFramebufferSize(pipeline.get());
    } else if (tok == "BIND") {
      r = ParsePipelineBind(pipeline.get());
    } else if (tok == "VERTEX_DATA") {
      r = ParsePipelineVertexData(pipeline.get());
    } else if (tok == "INDEX_DATA") {
      r = ParsePipelineIndexData(pipeline.get());
    } else {
      r = Result("unknown token in pipeline block: " + tok);
    }
    if (!r.IsSuccess())
      return r;
  }

  if (!token->IsString() || token->AsString() != "END")
    return Result("PIPELINE missing END command");

  r = script_->AddPipeline(std::move(pipeline));
  if (!r.IsSuccess())
    return r;

  return ValidateEndOfStatement("END");
}

Result Parser::ParsePipelineAttach(Pipeline* pipeline) {
  auto token = tokenizer_->NextToken();
  if (!token->IsString())
    return Result("invalid token in ATTACH command");

  auto* shader = script_->GetShader(token->AsString());
  if (!shader)
    return Result("unknown shader in ATTACH command");

  token = tokenizer_->NextToken();
  if (token->IsEOL() || token->IsEOS()) {
    if (shader->GetType() == kShaderTypeMulti)
      return Result("multi shader ATTACH requires TYPE");

    Result r = pipeline->AddShader(shader, shader->GetType());
    if (!r.IsSuccess())
      return r;
    return {};
  }
  if (!token->IsString())
    return Result("Invalid token after ATTACH");

  bool set_shader_type = false;
  ShaderType shader_type = shader->GetType();
  auto type = token->AsString();
  if (type == "TYPE") {
    token = tokenizer_->NextToken();
    if (!token->IsString())
      return Result("invalid type in ATTACH");

    Result r = ToShaderType(token->AsString(), &shader_type);
    if (!r.IsSuccess())
      return r;

    set_shader_type = true;

    token = tokenizer_->NextToken();
    if (!token->IsString())
      return Result("ATTACH TYPE requires an ENTRY_POINT");

    type = token->AsString();
  }
  if (type != "ENTRY_POINT")
    return Result("Unknown ATTACH parameter: " + type);

  if (shader->GetType() == ShaderType::kShaderTypeMulti && !set_shader_type)
    return Result("ATTACH missing TYPE for multi shader");

  Result r = pipeline->AddShader(shader, shader_type);
  if (!r.IsSuccess())
    return r;

  token = tokenizer_->NextToken();
  if (!token->IsString())
    return Result("missing shader name in ATTACH ENTRY_POINT command");

  r = pipeline->SetShaderEntryPoint(shader, token->AsString());
  if (!r.IsSuccess())
    return r;

  return ValidateEndOfStatement("ATTACH command");
}

Result Parser::ParsePipelineShaderOptimizations(Pipeline* pipeline) {
  auto token = tokenizer_->NextToken();
  if (!token->IsString())
    return Result("missing shader name in SHADER_OPTIMIZATION command");

  auto* shader = script_->GetShader(token->AsString());
  if (!shader)
    return Result("unknown shader in SHADER_OPTIMIZATION command");

  token = tokenizer_->NextToken();
  if (!token->IsEOL())
    return Result("extra parameters after SHADER_OPTIMIZATION command");

  std::vector<std::string> optimizations;
  while (true) {
    token = tokenizer_->NextToken();
    if (token->IsEOL())
      continue;
    if (token->IsEOS())
      return Result("SHADER_OPTIMIZATION missing END command");
    if (!token->IsString())
      return Result("SHADER_OPTIMIZATION options must be strings");
    if (token->AsString() == "END")
      break;

    optimizations.push_back(token->AsString());
  }

  Result r = pipeline->SetShaderOptimizations(shader, optimizations);
  if (!r.IsSuccess())
    return r;

  return ValidateEndOfStatement("SHADER_OPTIMIZATION command");
}

Result Parser::ParsePipelineFramebufferSize(Pipeline* pipeline) {
  auto token = tokenizer_->NextToken();
  if (token->IsEOL() || token->IsEOS())
    return Result("missing size for FRAMEBUFFER_SIZE command");
  if (!token->IsInteger())
    return Result("invalid width for FRAMEBUFFER_SIZE command");

  pipeline->SetFramebufferWidth(token->AsUint32());

  token = tokenizer_->NextToken();
  if (token->IsEOL() || token->IsEOS())
    return Result("missing height for FRAMEBUFFER_SIZE command");
  if (!token->IsInteger())
    return Result("invalid height for FRAMEBUFFER_SIZE command");

  pipeline->SetFramebufferHeight(token->AsUint32());

  return ValidateEndOfStatement("FRAMEBUFFER_SIZE command");
}

Result Parser::ToBufferType(const std::string& name, BufferType* type) {
  assert(type);
  if (name == "push_constant")
    *type = BufferType::kPushConstant;
  else if (name == "uniform")
    *type = BufferType::kUniform;
  else if (name == "storage")
    *type = BufferType::kStorage;
  else
    return Result("unknown buffer_type: " + name);

  return {};
}

Result Parser::ParsePipelineBind(Pipeline* pipeline) {
  auto token = tokenizer_->NextToken();
  if (!token->IsString())
    return Result("missing BUFFER in BIND command");
  if (token->AsString() != "BUFFER")
    return Result("missing BUFFER in BIND command");

  token = tokenizer_->NextToken();
  if (!token->IsString())
    return Result("missing buffer name in BIND command");

  auto* buffer = script_->GetBuffer(token->AsString());
  if (!buffer)
    return Result("unknown buffer: " + token->AsString());

  token = tokenizer_->NextToken();
  if (!token->IsString() || token->AsString() != "AS")
    return Result("BUFFER command missing AS keyword");

  token = tokenizer_->NextToken();
  if (!token->IsString())
    return Result("invalid token for BUFFER type");

  if (token->AsString() == "color") {
    if (!buffer->IsFormatBuffer())
      return Result("color buffer must be a FORMAT buffer");

    token = tokenizer_->NextToken();
    if (!token->IsString() || token->AsString() != "LOCATION")
      return Result("BIND missing LOCATION");

    token = tokenizer_->NextToken();
    if (!token->IsInteger())
      return Result("invalid value for BIND LOCATION");

    Result r = pipeline->AddColorAttachment(buffer, token->AsUint32());
    if (!r.IsSuccess())
      return r;
  } else if (token->AsString() == "depth_stencil") {
    if (!buffer->IsFormatBuffer())
      return Result("depth buffer must be a FORMAT buffer");

    Result r = pipeline->SetDepthBuffer(buffer);
    if (!r.IsSuccess())
      return r;
  } else {
    BufferType type = BufferType::kColor;
    Result r = ToBufferType(token->AsString(), &type);
    if (!r.IsSuccess())
      return r;

    token = tokenizer_->NextToken();
    if (!token->IsString() || token->AsString() != "DESCRIPTOR_SET")
      return Result("missing DESCRIPTOR_SET for BIND command");

    token = tokenizer_->NextToken();
    if (!token->IsInteger())
      return Result("invalid value for DESCRIPTOR_SET in BIND command");
    uint32_t descriptor_set = token->AsUint32();

    token = tokenizer_->NextToken();
    if (!token->IsString() || token->AsString() != "BINDING")
      return Result("missing BINDING for BIND command");

    token = tokenizer_->NextToken();
    if (!token->IsInteger())
      return Result("invalid value for BINDING in BIND command");
    uint32_t binding = token->AsUint32();

    token = tokenizer_->NextToken();
    if (token->IsEOL() || token->IsEOS()) {
      pipeline->AddBuffer(buffer, type, descriptor_set, binding, 0);
      return {};
    }
    if (!token->IsString() || token->AsString() != "IDX")
      return Result("extra parameters after BIND command");

    token = tokenizer_->NextToken();
    if (!token->IsInteger())
      return Result("invalid value for IDX in BIND command");

    pipeline->AddBuffer(buffer, type, descriptor_set, binding,
                        token->AsUint32());
  }

  return ValidateEndOfStatement("BIND command");
}

Result Parser::ParsePipelineVertexData(Pipeline* pipeline) {
  auto token = tokenizer_->NextToken();
  if (!token->IsString())
    return Result("missing buffer name in VERTEX_DATA command");

  auto* buffer = script_->GetBuffer(token->AsString());
  if (!buffer)
    return Result("unknown buffer: " + token->AsString());

  token = tokenizer_->NextToken();
  if (!token->IsString() || token->AsString() != "LOCATION")
    return Result("VERTEX_DATA missing LOCATION");

  token = tokenizer_->NextToken();
  if (!token->IsInteger())
    return Result("invalid value for VERTEX_DATA LOCATION");

  Result r = pipeline->AddVertexBuffer(buffer, token->AsUint32());
  if (!r.IsSuccess())
    return r;

  return ValidateEndOfStatement("VERTEX_DATA command");
}

Result Parser::ParsePipelineIndexData(Pipeline* pipeline) {
  auto token = tokenizer_->NextToken();
  if (!token->IsString())
    return Result("missing buffer name in INDEX_DATA command");

  auto* buffer = script_->GetBuffer(token->AsString());
  if (!buffer)
    return Result("unknown buffer: " + token->AsString());

  Result r = pipeline->SetIndexBuffer(buffer);
  if (!r.IsSuccess())
    return r;

  return ValidateEndOfStatement("INDEX_DATA command");
}

Result Parser::ParseBuffer() {
  auto token = tokenizer_->NextToken();
  if (!token->IsString())
    return Result("invalid BUFFER name provided");

  auto name = token->AsString();
  if (name == "DATA_TYPE" || name == "FORMAT")
    return Result("missing BUFFER name");

  token = tokenizer_->NextToken();
  if (!token->IsString())
    return Result("invalid BUFFER command provided");

  std::unique_ptr<Buffer> buffer;
  auto& cmd = token->AsString();
  if (cmd == "DATA_TYPE") {
    buffer = MakeUnique<DataBuffer>();

    Result r = ParseBufferInitializer(buffer->AsDataBuffer());
    if (!r.IsSuccess())
      return r;
  } else if (cmd == "FORMAT") {
    token = tokenizer_->NextToken();
    if (!token->IsString())
      return Result("BUFFER FORMAT must be a string");

    buffer = MakeUnique<FormatBuffer>();

    FormatParser fmt_parser;
    auto fmt = fmt_parser.Parse(token->AsString());
    if (fmt == nullptr)
      return Result("invalid BUFFER FORMAT");

    buffer->AsFormatBuffer()->SetFormat(std::move(fmt));
  } else {
    return Result("unknown BUFFER command provided: " + cmd);
  }
  buffer->SetName(name);

  Result r = script_->AddBuffer(std::move(buffer));
  if (!r.IsSuccess())
    return r;

  return {};
}

Result Parser::ParseBufferInitializer(DataBuffer* buffer) {
  auto token = tokenizer_->NextToken();
  if (!token->IsString())
    return Result("BUFFER invalid data type");

  DatumType type;
  Result r = ToDatumType(token->AsString(), &type);
  if (!r.IsSuccess())
    return r;

  buffer->SetDatumType(type);

  token = tokenizer_->NextToken();
  if (!token->IsString())
    return Result("BUFFER missing initializer");

  if (token->AsString() == "SIZE")
    return ParseBufferInitializerSize(buffer);
  if (token->AsString() == "DATA")
    return ParseBufferInitializerData(buffer);

  return Result("unknown initializer for BUFFER");
}

Result Parser::ParseBufferInitializerSize(DataBuffer* buffer) {
  auto token = tokenizer_->NextToken();
  if (token->IsEOS() || token->IsEOL())
    return Result("BUFFER size missing");
  if (!token->IsInteger())
    return Result("BUFFER size invalid");

  uint32_t size_in_items = token->AsUint32();
  buffer->SetSize(size_in_items);

  token = tokenizer_->NextToken();
  if (!token->IsString())
    return Result("BUFFER invalid initializer");

  if (token->AsString() == "FILL")
    return ParseBufferInitializerFill(buffer, size_in_items);
  if (token->AsString() == "SERIES_FROM")
    return ParseBufferInitializerSeries(buffer, size_in_items);

  return Result("invalid BUFFER initializer provided");
}

Result Parser::ParseBufferInitializerFill(DataBuffer* buffer,
                                          uint32_t size_in_items) {
  auto token = tokenizer_->NextToken();
  if (token->IsEOS() || token->IsEOL())
    return Result("missing BUFFER fill value");
  if (!token->IsInteger() && !token->IsDouble())
    return Result("invalid BUFFER fill value");

  auto& type = buffer->GetDatumType();
  bool is_double_data = type.IsFloat() || type.IsDouble();

  // Inflate the size because our items are multi-dimensional.
  size_in_items = size_in_items * type.RowCount() * type.ColumnCount();

  std::vector<Value> values;
  values.resize(size_in_items);
  for (size_t i = 0; i < size_in_items; ++i) {
    if (is_double_data)
      values[i].SetDoubleValue(token->AsDouble());
    else
      values[i].SetIntValue(token->AsUint64());
  }
  buffer->SetData(std::move(values));
  return ValidateEndOfStatement("BUFFER fill command");
}

Result Parser::ParseBufferInitializerSeries(DataBuffer* buffer,
                                            uint32_t size_in_items) {
  auto token = tokenizer_->NextToken();
  if (token->IsEOS() || token->IsEOL())
    return Result("missing BUFFER series_from value");
  if (!token->IsInteger() && !token->IsDouble())
    return Result("invalid BUFFER series_from value");

  auto& type = buffer->GetDatumType();
  if (type.RowCount() > 1 || type.ColumnCount() > 1)
    return Result("BUFFER series_from must not be multi-row/column types");

  bool is_double_data = type.IsFloat() || type.IsDouble();

  Value counter;
  if (is_double_data)
    counter.SetDoubleValue(token->AsDouble());
  else
    counter.SetIntValue(token->AsUint64());

  token = tokenizer_->NextToken();
  if (!token->IsString())
    return Result("missing BUFFER series_from inc_by");
  if (token->AsString() != "INC_BY")
    return Result("BUFFER series_from invalid command");

  token = tokenizer_->NextToken();
  if (token->IsEOS() || token->IsEOL())
    return Result("missing BUFFER series_from inc_by value");
  if (!token->IsInteger() && !token->IsDouble())
    return Result("invalid BUFFER series_from inc_by value");

  std::vector<Value> values;
  values.resize(size_in_items);
  for (size_t i = 0; i < size_in_items; ++i) {
    if (is_double_data) {
      double value = counter.AsDouble();
      values[i].SetDoubleValue(value);
      counter.SetDoubleValue(value + token->AsDouble());
    } else {
      uint64_t value = counter.AsUint64();
      values[i].SetIntValue(value);
      counter.SetIntValue(value + token->AsUint64());
    }
  }
  buffer->SetData(std::move(values));
  return ValidateEndOfStatement("BUFFER series_from command");
}

Result Parser::ParseBufferInitializerData(DataBuffer* buffer) {
  auto token = tokenizer_->NextToken();
  if (!token->IsEOL())
    return Result("extra parameters after BUFFER data command");

  auto& type = buffer->GetDatumType();
  bool is_double_type = type.IsFloat() || type.IsDouble();

  std::vector<Value> values;
  for (token = tokenizer_->NextToken();; token = tokenizer_->NextToken()) {
    if (token->IsEOL())
      continue;
    if (token->IsEOS())
      return Result("missing BUFFER END command");
    if (token->IsString() && token->AsString() == "END")
      break;
    if (!token->IsInteger() && !token->IsDouble() && !token->IsHex())
      return Result("invalid BUFFER data value");

    if (!is_double_type && token->IsDouble())
      return Result("invalid BUFFER data value");

    Value v;
    if (is_double_type) {
      double val = token->IsHex() ? static_cast<double>(token->AsHex())
                                  : token->AsDouble();
      v.SetDoubleValue(val);
    } else {
      uint64_t val = token->IsHex() ? token->AsHex() : token->AsUint64();
      v.SetIntValue(val);
    }

    values.emplace_back(v);
  }

  size_t size_in_items = values.size() / type.RowCount() / type.ColumnCount();
  buffer->SetSize(size_in_items);

  buffer->SetData(std::move(values));
  return ValidateEndOfStatement("BUFFER data command");
}

Result Parser::ParseRun() {
  auto token = tokenizer_->NextToken();
  if (!token->IsString())
    return Result("missing pipeline name for RUN command");

  auto* pipeline = script_->GetPipeline(token->AsString());
  if (!pipeline)
    return Result("unknown pipeline for RUN command: " + token->AsString());

  token = tokenizer_->NextToken();
  if (token->IsEOL() || token->IsEOS())
    return Result("RUN command requires parameters");

  if (token->IsInteger()) {
    if (!pipeline->IsCompute())
      return Result("RUN command requires compute pipeline, got graphics");

    auto cmd = MakeUnique<ComputeCommand>(pipeline);
    cmd->SetX(token->AsUint32());

    token = tokenizer_->NextToken();
    if (!token->IsInteger()) {
      return Result("invalid parameter for RUN command: " +
                    token->ToOriginalString());
    }
    cmd->SetY(token->AsUint32());

    token = tokenizer_->NextToken();
    if (!token->IsInteger()) {
      return Result("invalid parameter for RUN command: " +
                    token->ToOriginalString());
    }
    cmd->SetZ(token->AsUint32());

    script_->AddCommand(std::move(cmd));
    return ValidateEndOfStatement("RUN command");
  }
  if (!token->IsString())
    return Result("invalid token in RUN command: " + token->ToOriginalString());

  if (token->AsString() == "DRAW_RECT") {
    if (!pipeline->IsGraphics())
      return Result("RUN command requires graphics pipeline, got compute");

    token = tokenizer_->NextToken();
    if (token->IsEOS() || token->IsEOL())
      return Result("RUN DRAW_RECT command requires parameters");

    if (!token->IsString() || token->AsString() != "POS") {
      return Result("invalid token in RUN command: " +
                    token->ToOriginalString() + "; expected POS");
    }

    token = tokenizer_->NextToken();
    if (!token->IsInteger())
      return Result("missing X position for RUN command");

    auto cmd = MakeUnique<DrawRectCommand>(pipeline, PipelineData{});
    Result r = token->ConvertToDouble();
    if (!r.IsSuccess())
      return r;
    cmd->SetX(token->AsFloat());

    token = tokenizer_->NextToken();
    if (!token->IsInteger())
      return Result("missing Y position for RUN command");

    r = token->ConvertToDouble();
    if (!r.IsSuccess())
      return r;
    cmd->SetY(token->AsFloat());

    token = tokenizer_->NextToken();
    if (!token->IsString() || token->AsString() != "SIZE") {
      return Result("invalid token in RUN command: " +
                    token->ToOriginalString() + "; expected SIZE");
    }

    token = tokenizer_->NextToken();
    if (!token->IsInteger())
      return Result("missing width value for RUN command");

    r = token->ConvertToDouble();
    if (!r.IsSuccess())
      return r;
    cmd->SetWidth(token->AsFloat());

    token = tokenizer_->NextToken();
    if (!token->IsInteger())
      return Result("missing height value for RUN command");

    r = token->ConvertToDouble();
    if (!r.IsSuccess())
      return r;
    cmd->SetHeight(token->AsFloat());

    script_->AddCommand(std::move(cmd));
    return ValidateEndOfStatement("RUN command");
  }

  if (token->AsString() == "DRAW_ARRAY") {
    if (!pipeline->IsGraphics())
      return Result("RUN command requires graphics pipeline, got compute");

    auto cmd = MakeUnique<DrawArraysCommand>(pipeline, PipelineData{});

    script_->AddCommand(std::move(cmd));
    return ValidateEndOfStatement("RUN command");
  }

  return Result("invalid token in RUN command: " + token->AsString());
}

Result Parser::ParseClear() {
  auto token = tokenizer_->NextToken();

  if (!token->IsString())
    return Result("missing pipeline name for CLEAR command");

  auto* pipeline = script_->GetPipeline(token->AsString());
  if (!pipeline)
    return Result("unknown pipeline for CLEAR command: " + token->AsString());
  if (!pipeline->IsGraphics())
    return Result("CLEAR command requires graphics pipeline, got compute");

  auto cmd = MakeUnique<ClearCommand>();
  script_->AddCommand(std::move(cmd));

  return ValidateEndOfStatement("CLEAR command");
}

}  // namespace amberscript
}  // namespace amber
