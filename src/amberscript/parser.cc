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

#include "src/make_unique.h"
#include "src/shader_data.h"
#include "src/tokenizer.h"

namespace amber {
namespace amberscript {

Parser::Parser()
    : amber::Parser(), script_(MakeUnique<Script>()) {}

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
    } else {
      r = Result("unknown token: " + tok);
    }
    if (!r.IsSuccess())
      return Result(make_error(r.Error()));
  }
  return {};
}

Result Parser::ToShaderType(const std::string& str, ShaderType* type) {
  assert(type);

  if (str == "vertex")
    *type = ShaderType::kVertex;
  else if (str == "fragment")
    *type = ShaderType::kFragment;
  else if (str == "geometry")
    *type = ShaderType::kGeometry;
  else if (str == "tessellation_evaluation")
    *type = ShaderType::kTessellationEvaluation;
  else if (str == "tessellation_control")
    *type = ShaderType::kTessellationControl;
  else if (str == "compute")
    *type = ShaderType::kCompute;
  else
    return Result("unknown shader type: " + str);
  return {};
}

Result Parser::ToShaderFormat(const std::string& str, ShaderFormat* fmt) {
  assert(fmt);

  if (str == "GLSL")
    *fmt = ShaderFormat::kGlsl;
  else if (str == "SPIRV-ASM")
    *fmt = ShaderFormat::kSpirvAsm;
  else if (str == "SPIRV-HEX")
    *fmt = ShaderFormat::kSpirvHex;
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

  ShaderType type = ShaderType::kVertex;
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
    if (type != ShaderType::kVertex) {
      return Result(
          "invalid shader type for PASSTHROUGH. Only vertex "
          "PASSTHROUGH allowed");
    }
    shader->SetFormat(ShaderFormat::kSpirvAsm);
    shader->SetData(kPassThroughShader);

    r = script_->AddShader(std::move(shader));
    if (!r.IsSuccess())
      return r;

    return ValidateEndOfStatement("SHADER PASSTHROUGH");
  }

  ShaderFormat format = ShaderFormat::kGlsl;
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
    } else if (tok == "ENTRY_POINT") {
      r = ParsePipelineEntryPoint(pipeline.get());
    } else if (tok == "SHADER_OPTIMIZATION") {
      r = ParsePipelineShaderOptimizations(pipeline.get());
    } else {
      r = Result("unknown token in pipeline block: " + tok);
    }
    if (!r.IsSuccess())
      return r;
  }

  if (!token->IsString() || token->AsString() != "END")
    return Result("PIPELINE missing END command");

  r = pipeline->Validate();
  if (!r.IsSuccess())
    return r;

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

  Result r = pipeline->AddShader(shader);
  if (!r.IsSuccess())
    return r;

  return ValidateEndOfStatement("ATTACH command");
}

Result Parser::ParsePipelineEntryPoint(Pipeline* pipeline) {
  auto token = tokenizer_->NextToken();
  if (!token->IsString())
    return Result("missing shader name in ENTRY_POINT command");

  auto* shader = script_->GetShader(token->AsString());
  if (!shader)
    return Result("unknown shader in ENTRY_POINT command");

  token = tokenizer_->NextToken();
  if (!token->IsString())
    return Result("invalid value in ENTRY_POINT command");

  Result r = pipeline->SetShaderEntryPoint(shader, token->AsString());
  if (!r.IsSuccess())
    return r;

  return ValidateEndOfStatement("ENTRY_POINT command");
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

Result Parser::ToBufferType(const std::string& str, BufferType* type) {
  assert(type);

  if (str == "color")
    *type = BufferType::kColor;
  else if (str == "depth")
    *type = BufferType::kDepth;
  else if (str == "framebuffer")
    *type = BufferType::kFramebuffer;
  else if (str == "index")
    *type = BufferType::kIndex;
  else if (str == "sampled")
    *type = BufferType::kSampled;
  else if (str == "storage")
    *type = BufferType::kStorage;
  else if (str == "uniform")
    *type = BufferType::kUniform;
  else if (str == "vertex")
    *type = BufferType::kVertex;
  else
    return Result("unknown BUFFER type provided: " + str);

  return {};
}

Result Parser::ParseBuffer() {
  auto token = tokenizer_->NextToken();
  if (!token->IsString())
    return Result("invalid BUFFER type provided");

  BufferType type;
  Result r = ToBufferType(token->AsString(), &type);
  if (!r.IsSuccess())
    return r;

  auto buffer = MakeUnique<DataBuffer>(type);

  token = tokenizer_->NextToken();
  if (!token->IsString())
    return Result("invalid BUFFER name provided");

  auto& name = token->AsString();
  if (name == "DATA_TYPE" || name == "DIMS")
    return Result("missing BUFFER name");

  buffer->SetName(token->AsString());

  token = tokenizer_->NextToken();
  if (!token->IsString())
    return Result("invalid BUFFER command provided");

  auto& cmd = token->AsString();
  if (cmd == "DATA_TYPE") {
    if (type == BufferType::kFramebuffer)
      return Result("BUFFER framebuffer must be used with DIMS");

    r = ParseBufferInitializer(buffer.get());
    if (!r.IsSuccess())
      return r;
  } else if (cmd == "DIMS") {
    if (type != BufferType::kFramebuffer)
      return Result("BUFFER DIMS can only be used with a framebuffer");

    r = ParseBufferFramebuffer(buffer.get());
    if (!r.IsSuccess())
      return r;
  } else {
    return Result("unknown BUFFER command provided: " + cmd);
  }

  r = script_->AddBuffer(std::move(buffer));
  if (!r.IsSuccess())
    return r;

  return {};
}

Result Parser::ParseBufferFramebuffer(DataBuffer* buffer) {
  auto token = tokenizer_->NextToken();
  if (token->IsEOL() || token->IsEOS())
    return Result("BUFFER framebuffer missing DIMS values");
  if (!token->IsInteger())
    return Result("BUFFER framebuffer invalid width value");

  // TODO(dsinclair): Is uint32 rgba the right format to use here?
  DatumType datum;
  datum.SetType(DataType::kUint32);
  datum.SetColumnCount(4);
  buffer->SetDatumType(datum);

  auto w = token->AsUint32();

  token = tokenizer_->NextToken();
  if (token->IsEOS() || token->IsEOL())
    return Result("BUFFER framebuffer missing height value");
  if (!token->IsInteger())
    return Result("BUFFER framebuffer invalid height value");

  auto h = token->AsUint32();

  // TODO(dsinclair): Should this be a smaller maximum size?
  uint64_t size = static_cast<uint64_t>(w) * static_cast<uint64_t>(h);
  if (size >= std::numeric_limits<uint32_t>::max())
    return Result("BUFFER framebuffer size too large");

  buffer->SetSize(static_cast<size_t>(size));
  return ValidateEndOfStatement("BUFFER framebuffer command");
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

}  // namespace amberscript
}  // namespace amber
