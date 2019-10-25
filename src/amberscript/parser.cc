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
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "src/make_unique.h"
#include "src/shader_data.h"
#include "src/tokenizer.h"
#include "src/type_parser.h"

namespace amber {
namespace amberscript {
namespace {

bool IsComparator(const std::string& in) {
  return in == "EQ" || in == "NE" || in == "GT" || in == "LT" || in == "GE" ||
         in == "LE";
}

ProbeSSBOCommand::Comparator ToComparator(const std::string& in) {
  if (in == "EQ")
    return ProbeSSBOCommand::Comparator::kEqual;
  if (in == "NE")
    return ProbeSSBOCommand::Comparator::kNotEqual;
  if (in == "GT")
    return ProbeSSBOCommand::Comparator::kGreater;
  if (in == "LT")
    return ProbeSSBOCommand::Comparator::kLess;
  if (in == "GE")
    return ProbeSSBOCommand::Comparator::kGreaterOrEqual;

  assert(in == "LE");
  return ProbeSSBOCommand::Comparator::kLessOrEqual;
}

std::unique_ptr<type::Type> ToType(const std::string& str) {
  TypeParser parser;
  if (str == "int8")
    return parser.Parse("R8_SINT");
  if (str == "int16")
    return parser.Parse("R16_SINT");
  if (str == "int32")
    return parser.Parse("R32_SINT");
  if (str == "int64")
    return parser.Parse("R64_SINT");
  if (str == "uint8")
    return parser.Parse("R8_UINT");
  if (str == "uint16")
    return parser.Parse("R16_UINT");
  if (str == "uint32")
    return parser.Parse("R32_UINT");
  if (str == "uint64")
    return parser.Parse("R64_UINT");
  if (str == "float")
    return parser.Parse("R32_SFLOAT");
  if (str == "double")
    return parser.Parse("R64_SFLOAT");

  if (str.length() > 7 && str.substr(0, 3) == "vec") {
    if (str[4] != '<' || str[str.length() - 1] != '>')
      return nullptr;

    int component_count = str[3] - '0';
    if (component_count < 2 || component_count > 4)
      return nullptr;

    auto type = ToType(str.substr(5, str.length() - 6));
    if (!type)
      return nullptr;

    if (!type->IsNumber() || type->IsArray() || type->IsVec() ||
        type->IsMatrix()) {
      return nullptr;
    }

    type->SetRowCount(static_cast<uint32_t>(component_count));
    return type;
  }

  if (str.length() > 9 && str.substr(0, 3) == "mat") {
    if (str[4] != 'x' || str[6] != '<' || str[str.length() - 1] != '>')
      return nullptr;

    int column_count = str[3] - '0';
    if (column_count < 2 || column_count > 4)
      return nullptr;

    int row_count = str[5] - '0';
    if (row_count < 2 || row_count > 4)
      return nullptr;

    auto type = ToType(str.substr(7, str.length() - 8));
    if (!type)
      return nullptr;
    if (!type->IsNumber() || type->IsArray() || type->IsVec() ||
        type->IsMatrix()) {
      return nullptr;
    }

    type->SetRowCount(static_cast<uint32_t>(row_count));
    type->SetColumnCount(static_cast<uint32_t>(column_count));
    return type;
  }
  return nullptr;
}

}  // namespace

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
    if (IsRepeatable(tok)) {
      r = ParseRepeatableCommand(tok);
    } else if (tok == "BUFFER") {
      r = ParseBuffer();
    } else if (tok == "DERIVE_PIPELINE") {
      r = ParseDerivePipelineBlock();
    } else if (tok == "DEVICE_FEATURE") {
      r = ParseDeviceFeature();
    } else if (tok == "DEVICE_EXTENSION") {
      r = ParseDeviceExtension();
    } else if (tok == "INSTANCE_EXTENSION") {
      r = ParseInstanceExtension();
    } else if (tok == "PIPELINE") {
      r = ParsePipelineBlock();
    } else if (tok == "REPEAT") {
      r = ParseRepeat();
    } else if (tok == "SET") {
      r = ParseSet();
    } else if (tok == "SHADER") {
      r = ParseShaderBlock();
    } else if (tok == "STRUCT") {
      r = ParseStruct();
    } else {
      r = Result("unknown token: " + tok);
    }
    if (!r.IsSuccess())
      return Result(make_error(r.Error()));
  }
  script_->SetCommands(std::move(command_list_));

  // Generate any needed color attachments. This is done before
  // validating in case one of the pipelines specifies the framebuffer size
  // it needs to be verified against all other pipelines.
  for (const auto& pipeline : script_->GetPipelines()) {
    // Add a color attachment if needed
    if (pipeline->GetColorAttachments().empty()) {
      auto* buf = script_->GetBuffer(Pipeline::kGeneratedColorBuffer);
      if (!buf) {
        auto color_buf = pipeline->GenerateDefaultColorAttachmentBuffer();
        buf = color_buf.get();

        Result r = script_->AddBuffer(std::move(color_buf));
        if (!r.IsSuccess())
          return r;
      }
      Result r = pipeline->AddColorAttachment(buf, 0);
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

bool Parser::IsRepeatable(const std::string& name) const {
  return name == "CLEAR" || name == "CLEAR_COLOR" || name == "COPY" ||
         name == "EXPECT" || name == "RUN";
}

// The given |name| must be one of the repeatable commands or this method
// returns an error result.
Result Parser::ParseRepeatableCommand(const std::string& name) {
  if (name == "CLEAR")
    return ParseClear();
  if (name == "CLEAR_COLOR")
    return ParseClearColor();
  if (name == "COPY")
    return ParseCopy();
  if (name == "EXPECT")
    return ParseExpect();
  if (name == "RUN")
    return ParseRun();

  return Result("invalid repeatable command: " + name);
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
  else if (str == "HLSL")
    *fmt = kShaderFormatHlsl;
  else if (str == "SPIRV-ASM")
    *fmt = kShaderFormatSpirvAsm;
  else if (str == "SPIRV-HEX")
    *fmt = kShaderFormatSpirvHex;
  else if (str == "OPENCL-C")
    *fmt = kShaderFormatOpenCLC;
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

  return ParsePipelineBody("PIPELINE", std::move(pipeline));
}

Result Parser::ParsePipelineBody(const std::string& cmd_name,
                                 std::unique_ptr<Pipeline> pipeline) {
  std::unique_ptr<Token> token;
  for (token = tokenizer_->NextToken(); !token->IsEOS();
       token = tokenizer_->NextToken()) {
    if (token->IsEOL())
      continue;
    if (!token->IsString())
      return Result("expected string");

    Result r;
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
    } else if (tok == "SET") {
      r = ParsePipelineSet(pipeline.get());
    } else if (tok == "COMPILE_OPTIONS") {
      r = ParsePipelineShaderCompileOptions(pipeline.get());
    } else {
      r = Result("unknown token in pipeline block: " + tok);
    }
    if (!r.IsSuccess())
      return r;
  }

  if (!token->IsString() || token->AsString() != "END")
    return Result(cmd_name + " missing END command");

  Result r = script_->AddPipeline(std::move(pipeline));
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
    return Result("invalid token after ATTACH");

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
  if (set_shader_type && type != "ENTRY_POINT")
    return Result("unknown ATTACH parameter: " + type);

  if (shader->GetType() == ShaderType::kShaderTypeMulti && !set_shader_type)
    return Result("ATTACH missing TYPE for multi shader");

  Result r = pipeline->AddShader(shader, shader_type);
  if (!r.IsSuccess())
    return r;

  if (type == "ENTRY_POINT") {
    token = tokenizer_->NextToken();
    if (!token->IsString())
      return Result("missing shader name in ATTACH ENTRY_POINT command");

    r = pipeline->SetShaderEntryPoint(shader, token->AsString());
    if (!r.IsSuccess())
      return r;

    token = tokenizer_->NextToken();
  }

  while (true) {
    if (token->IsString() && token->AsString() == "SPECIALIZE") {
      r = ParseShaderSpecialization(pipeline);
      if (!r.IsSuccess())
        return r;

      token = tokenizer_->NextToken();
    } else {
      if (token->IsEOL() || token->IsEOS())
        return {};
      if (token->IsString())
        return Result("unknown ATTACH parameter: " + token->AsString());
      return Result("extra parameters after ATTACH command");
    }
  }
}

Result Parser::ParseShaderSpecialization(Pipeline* pipeline) {
  auto token = tokenizer_->NextToken();
  if (!token->IsInteger())
    return Result("specialization ID must be an integer");

  auto spec_id = token->AsUint32();

  token = tokenizer_->NextToken();
  if (!token->IsString() || token->AsString() != "AS")
    return Result("expected AS as next token");

  token = tokenizer_->NextToken();
  if (!token->IsString())
    return Result("expected data type in SPECIALIZE subcommand");

  auto type = ToType(token->AsString());
  if (!type)
    return Result("invalid data type '" + token->AsString() + "' provided");
  if (!type->IsNumber())
    return Result("only numeric types are accepted for specialization values");

  auto num = type->AsNumber();

  token = tokenizer_->NextToken();
  uint32_t value = 0;
  if (type::Type::IsUint32(num->GetFormatMode(), num->NumBits()) ||
      type::Type::IsInt32(num->GetFormatMode(), num->NumBits())) {
    value = token->AsUint32();
  } else if (type::Type::IsFloat32(num->GetFormatMode(), num->NumBits())) {
    Result r = token->ConvertToDouble();
    if (!r.IsSuccess())
      return Result("value is not a floating point value");

    union {
      uint32_t u;
      float f;
    } u;
    u.f = token->AsFloat();
    value = u.u;
  } else {
    return Result(
        "only 32-bit types are currently accepted for specialization values");
  }

  auto& shader = pipeline->GetShaders()[pipeline->GetShaders().size() - 1];
  shader.AddSpecialization(spec_id, value);
  return {};
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

Result Parser::ParsePipelineShaderCompileOptions(Pipeline* pipeline) {
  auto token = tokenizer_->NextToken();
  if (!token->IsString())
    return Result("missing shader name in COMPILE_OPTIONS command");

  auto* shader = script_->GetShader(token->AsString());
  if (!shader)
    return Result("unknown shader in COMPILE_OPTIONS command");

  if (shader->GetFormat() != kShaderFormatOpenCLC) {
    return Result("COMPILE_OPTIONS currently only supports OPENCL-C shaders");
  }

  token = tokenizer_->NextToken();
  if (!token->IsEOL())
    return Result("extra parameters after COMPILE_OPTIONS command");

  std::vector<std::string> options;
  while (true) {
    token = tokenizer_->NextToken();
    if (token->IsEOL())
      continue;
    if (token->IsEOS())
      return Result("COMPILE_OPTIONS missing END command");
    if (token->AsString() == "END")
      break;

    options.push_back(token->AsString());
  }

  Result r = pipeline->SetShaderCompileOptions(shader, options);
  if (!r.IsSuccess())
    return r;

  return ValidateEndOfStatement("COMPILE_OPTIONS command");
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
  if (name == "uniform")
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
  if (token->IsString() && token->AsString() == "AS") {
    token = tokenizer_->NextToken();
    if (!token->IsString())
      return Result("invalid token for BUFFER type");

    if (token->AsString() == "color") {
      token = tokenizer_->NextToken();
      if (!token->IsString() || token->AsString() != "LOCATION")
        return Result("BIND missing LOCATION");

      token = tokenizer_->NextToken();
      if (!token->IsInteger())
        return Result("invalid value for BIND LOCATION");

      buffer->SetBufferType(BufferType::kColor);

      Result r = pipeline->AddColorAttachment(buffer, token->AsUint32());
      if (!r.IsSuccess())
        return r;
    } else if (token->AsString() == "depth_stencil") {
      buffer->SetBufferType(BufferType::kDepth);
      Result r = pipeline->SetDepthBuffer(buffer);
      if (!r.IsSuccess())
        return r;
    } else if (token->AsString() == "push_constant") {
      buffer->SetBufferType(BufferType::kPushConstant);
      Result r = pipeline->SetPushConstantBuffer(buffer);
      if (!r.IsSuccess())
        return r;
    } else {
      BufferType type = BufferType::kColor;
      Result r = ToBufferType(token->AsString(), &type);
      if (!r.IsSuccess())
        return r;

      if (buffer->GetBufferType() == BufferType::kUnknown)
        buffer->SetBufferType(type);
      else if (buffer->GetBufferType() != type)
        return Result("buffer type does not match intended usage");
    }
  }

  if (buffer->GetBufferType() == BufferType::kUnknown ||
      buffer->GetBufferType() == BufferType::kStorage ||
      buffer->GetBufferType() == BufferType::kUniform) {
    // If AS was parsed above consume the next token.
    if (buffer->GetBufferType() != BufferType::kUnknown)
      token = tokenizer_->NextToken();
    // DESCRIPTOR_SET requires a buffer type to have been specified.
    if (buffer->GetBufferType() != BufferType::kUnknown && token->IsString() &&
        token->AsString() == "DESCRIPTOR_SET") {
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
      pipeline->AddBuffer(buffer, descriptor_set, token->AsUint32());
    } else if (token->IsString() && token->AsString() == "KERNEL") {
      token = tokenizer_->NextToken();
      if (!token->IsString())
        return Result("missing kernel arg identifier");

      if (token->AsString() == "ARG_NAME") {
        token = tokenizer_->NextToken();
        if (!token->IsString())
          return Result("expected argument identifier");

        pipeline->AddBuffer(buffer, token->AsString());
      } else if (token->AsString() == "ARG_NUMBER") {
        token = tokenizer_->NextToken();
        if (!token->IsInteger())
          return Result("expected argument number");

        pipeline->AddBuffer(buffer, token->AsUint32());
      } else {
        return Result("missing ARG_NAME or ARG_NUMBER keyword");
      }
    } else {
      return Result("missing DESCRIPTOR_SET or KERNEL for BIND command");
    }
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

  buffer->SetBufferType(BufferType::kVertex);
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

  buffer->SetBufferType(BufferType::kIndex);
  Result r = pipeline->SetIndexBuffer(buffer);
  if (!r.IsSuccess())
    return r;

  return ValidateEndOfStatement("INDEX_DATA command");
}

Result Parser::ParsePipelineSet(Pipeline* pipeline) {
  if (pipeline->GetShaders().empty() ||
      pipeline->GetShaders()[0].GetShader()->GetFormat() !=
          kShaderFormatOpenCLC) {
    return Result("SET can only be used with OPENCL-C shaders");
  }

  auto token = tokenizer_->NextToken();
  if (!token->IsString() || token->AsString() != "KERNEL")
    return Result("missing KERNEL in SET command");

  token = tokenizer_->NextToken();
  if (!token->IsString())
    return Result("expected ARG_NAME or ARG_NUMBER");

  std::string arg_name = "";
  uint32_t arg_no = std::numeric_limits<uint32_t>::max();
  if (token->AsString() == "ARG_NAME") {
    token = tokenizer_->NextToken();
    if (!token->IsString())
      return Result("expected argument identifier");

    arg_name = token->AsString();
  } else if (token->AsString() == "ARG_NUMBER") {
    token = tokenizer_->NextToken();
    if (!token->IsInteger())
      return Result("expected argument number");

    arg_no = token->AsUint32();
  } else {
    return Result("expected ARG_NAME or ARG_NUMBER");
  }

  token = tokenizer_->NextToken();
  if (!token->IsString() || token->AsString() != "AS")
    return Result("missing AS in SET command");

  token = tokenizer_->NextToken();
  if (!token->IsString())
    return Result("expected data type");

  auto type = ToType(token->AsString());
  if (!type)
    return Result("invalid data type '" + token->AsString() + "' provided");

  token = tokenizer_->NextToken();
  if (!token->IsInteger() && !token->IsDouble())
    return Result("expected data value");

  auto fmt = MakeUnique<Format>(type.get());
  Value value;
  if (fmt->IsFloat32() || fmt->IsFloat64())
    value.SetDoubleValue(token->AsDouble());
  else
    value.SetIntValue(token->AsUint64());

  Pipeline::ArgSetInfo info;
  info.name = arg_name;
  info.ordinal = arg_no;
  info.fmt = fmt.get();
  info.value = value;
  pipeline->SetArg(std::move(info));
  script_->RegisterFormat(std::move(fmt));
  script_->RegisterType(std::move(type));

  return ValidateEndOfStatement("SET command");
}

Result Parser::ParseStruct() {
  auto token = tokenizer_->NextToken();
  if (!token->IsString())
    return Result("invalid STRUCT name provided");

  auto struct_name = token->AsString();
  if (struct_name == "STRIDE")
    return Result("missing STRUCT name");

  auto s = MakeUnique<type::Struct>();
  auto type = s.get();

  Result r = script_->AddType(struct_name, std::move(s));
  if (!r.IsSuccess())
    return r;

  token = tokenizer_->NextToken();
  if (token->IsString()) {
    if (token->AsString() != "STRIDE")
      return Result("invalid token in STRUCT definition");

    token = tokenizer_->NextToken();
    if (token->IsEOL() || token->IsEOS())
      return Result("missing value for STRIDE");
    if (!token->IsInteger())
      return Result("invalid value for STRIDE");

    type->SetStrideInBytes(token->AsUint32());
    token = tokenizer_->NextToken();
  }
  if (!token->IsEOL()) {
    return Result("extra token " + token->ToOriginalString() +
                  " after STRUCT header");
  }

  std::map<std::string, bool> seen;
  for (;;) {
    token = tokenizer_->NextToken();
    if (!token->IsString())
      return Result("invalid type for STRUCT member");
    if (token->AsString() == "END")
      break;

    if (token->AsString() == struct_name)
      return Result("recursive types are not allowed");

    type::Type* member_type = script_->GetType(token->AsString());
    if (!member_type) {
      auto t = ToType(token->AsString());
      if (!t) {
        return Result("unknown type '" + token->AsString() +
                      "' for STRUCT member");
      }

      member_type = t.get();
      script_->RegisterType(std::move(t));
    }

    token = tokenizer_->NextToken();
    if (token->IsEOL())
      return Result("missing name for STRUCT member");
    if (!token->IsString())
      return Result("invalid name for STRUCT member");

    auto member_name = token->AsString();
    if (seen.find(member_name) != seen.end())
      return Result("duplicate name for STRUCT member");

    seen[member_name] = true;

    auto m = type->AddMember(member_type);
    m->name = member_name;

    token = tokenizer_->NextToken();
    while (token->IsString()) {
      if (token->AsString() == "OFFSET") {
        token = tokenizer_->NextToken();
        if (token->IsEOL())
          return Result("missing value for STRUCT member OFFSET");
        if (!token->IsInteger())
          return Result("invalid value for STRUCT member OFFSET");

        m->offset_in_bytes = token->AsInt32();
      } else if (token->AsString() == "ARRAY_STRIDE") {
        token = tokenizer_->NextToken();
        if (token->IsEOL())
          return Result("missing value for STRUCT member ARRAY_STRIDE");
        if (!token->IsInteger())
          return Result("invalid value for STRUCT member ARRAY_STRIDE");
        if (!member_type->IsArray())
          return Result("ARRAY_STRIDE only valid on array members");

        m->array_stride_in_bytes = token->AsInt32();
      } else if (token->AsString() == "MATRIX_STRIDE") {
        token = tokenizer_->NextToken();
        if (token->IsEOL())
          return Result("missing value for STRUCT member MATRIX_STRIDE");
        if (!token->IsInteger())
          return Result("invalid value for STRUCT member MATRIX_STRIDE");
        if (!member_type->IsMatrix())
          return Result("MATRIX_STRIDE only valid on matrix members");

        m->matrix_stride_in_bytes = token->AsInt32();
      } else {
        return Result("unknown param '" + token->AsString() +
                      "' for STRUCT member");
      }

      token = tokenizer_->NextToken();
    }

    if (!token->IsEOL())
      return Result("extra param for STRUCT member");
  }

  return {};
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
    buffer = MakeUnique<Buffer>();

    Result r = ParseBufferInitializer(buffer.get());
    if (!r.IsSuccess())
      return r;
  } else if (cmd == "FORMAT") {
    token = tokenizer_->NextToken();
    if (!token->IsString())
      return Result("BUFFER FORMAT must be a string");

    buffer = MakeUnique<Buffer>();

    auto type = script_->ParseType(token->AsString());
    if (!type)
      return Result("invalid BUFFER FORMAT");

    auto fmt = MakeUnique<Format>(type);
    buffer->SetFormat(fmt.get());
    script_->RegisterFormat(std::move(fmt));
  } else {
    return Result("unknown BUFFER command provided: " + cmd);
  }
  buffer->SetName(name);

  Result r = script_->AddBuffer(std::move(buffer));
  if (!r.IsSuccess())
    return r;

  return {};
}

Result Parser::ParseBufferInitializer(Buffer* buffer) {
  auto token = tokenizer_->NextToken();
  if (!token->IsString())
    return Result("BUFFER invalid data type");

  auto type = script_->ParseType(token->AsString());
  std::unique_ptr<Format> fmt;
  if (type != nullptr) {
    fmt = MakeUnique<Format>(type);
    buffer->SetFormat(fmt.get());
  } else {
    auto new_type = ToType(token->AsString());
    if (!new_type)
      return Result("invalid data type '" + token->AsString() + "' provided");

    fmt = MakeUnique<Format>(new_type.get());
    buffer->SetFormat(fmt.get());
    type = new_type.get();
    script_->RegisterType(std::move(new_type));
  }
  script_->RegisterFormat(std::move(fmt));

  token = tokenizer_->NextToken();
  if (!token->IsString())
    return Result("BUFFER missing initializer");

  if (token->AsString() == "STD140") {
    buffer->GetFormat()->SetLayout(Format::Layout::kStd140);
    token = tokenizer_->NextToken();
  } else if (token->AsString() == "STD430") {
    buffer->GetFormat()->SetLayout(Format::Layout::kStd430);
    token = tokenizer_->NextToken();
  }

  if (!token->IsString())
    return Result("BUFFER missing initializer");

  if (token->AsString() == "SIZE")
    return ParseBufferInitializerSize(buffer);
  if (token->AsString() == "DATA")
    return ParseBufferInitializerData(buffer);

  return Result("unknown initializer for BUFFER");
}

Result Parser::ParseBufferInitializerSize(Buffer* buffer) {
  auto token = tokenizer_->NextToken();
  if (token->IsEOS() || token->IsEOL())
    return Result("BUFFER size missing");
  if (!token->IsInteger())
    return Result("BUFFER size invalid");

  uint32_t size_in_items = token->AsUint32();
  buffer->SetElementCount(size_in_items);

  token = tokenizer_->NextToken();
  if (!token->IsString())
    return Result("BUFFER invalid initializer");

  if (token->AsString() == "FILL")
    return ParseBufferInitializerFill(buffer, size_in_items);
  if (token->AsString() == "SERIES_FROM")
    return ParseBufferInitializerSeries(buffer, size_in_items);

  return Result("invalid BUFFER initializer provided");
}

Result Parser::ParseBufferInitializerFill(Buffer* buffer,
                                          uint32_t size_in_items) {
  auto token = tokenizer_->NextToken();
  if (token->IsEOS() || token->IsEOL())
    return Result("missing BUFFER fill value");
  if (!token->IsInteger() && !token->IsDouble())
    return Result("invalid BUFFER fill value");

  auto fmt = buffer->GetFormat();
  bool is_double_data = fmt->IsFloat32() || fmt->IsFloat64();

  // Inflate the size because our items are multi-dimensional.
  size_in_items = size_in_items * fmt->InputNeededPerElement();

  std::vector<Value> values;
  values.resize(size_in_items);
  for (size_t i = 0; i < size_in_items; ++i) {
    if (is_double_data)
      values[i].SetDoubleValue(token->AsDouble());
    else
      values[i].SetIntValue(token->AsUint64());
  }
  Result r = buffer->SetData(std::move(values));
  if (!r.IsSuccess())
    return r;

  return ValidateEndOfStatement("BUFFER fill command");
}

Result Parser::ParseBufferInitializerSeries(Buffer* buffer,
                                            uint32_t size_in_items) {
  auto token = tokenizer_->NextToken();
  if (token->IsEOS() || token->IsEOL())
    return Result("missing BUFFER series_from value");
  if (!token->IsInteger() && !token->IsDouble())
    return Result("invalid BUFFER series_from value");

  auto type = buffer->GetFormat()->GetType();
  if (type->IsMatrix() || type->IsVec())
    return Result("BUFFER series_from must not be multi-row/column types");

  Value counter;

  auto n = type->AsNumber();
  FormatMode mode = n->GetFormatMode();
  uint32_t num_bits = n->NumBits();
  if (type::Type::IsFloat32(mode, num_bits) ||
      type::Type::IsFloat64(mode, num_bits)) {
    counter.SetDoubleValue(token->AsDouble());
  } else {
    counter.SetIntValue(token->AsUint64());
  }

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
    if (type::Type::IsFloat32(mode, num_bits) ||
        type::Type::IsFloat64(mode, num_bits)) {
      double value = counter.AsDouble();
      values[i].SetDoubleValue(value);
      counter.SetDoubleValue(value + token->AsDouble());
    } else {
      uint64_t value = counter.AsUint64();
      values[i].SetIntValue(value);
      counter.SetIntValue(value + token->AsUint64());
    }
  }
  Result r = buffer->SetData(std::move(values));
  if (!r.IsSuccess())
    return r;

  return ValidateEndOfStatement("BUFFER series_from command");
}

Result Parser::ParseBufferInitializerData(Buffer* buffer) {
  auto fmt = buffer->GetFormat();
  const auto& segs = fmt->GetSegments();
  size_t seg_idx = 0;
  uint32_t value_count = 0;

  std::vector<Value> values;
  for (auto token = tokenizer_->NextToken();; token = tokenizer_->NextToken()) {
    if (token->IsEOL())
      continue;
    if (token->IsEOS())
      return Result("missing BUFFER END command");
    if (token->IsString() && token->AsString() == "END")
      break;
    if (!token->IsInteger() && !token->IsDouble() && !token->IsHex())
      return Result("invalid BUFFER data value: " + token->ToOriginalString());

    while (segs[seg_idx].IsPadding()) {
      ++seg_idx;
      if (seg_idx >= segs.size())
        seg_idx = 0;
    }

    Value v;
    if (type::Type::IsFloat(segs[seg_idx].GetFormatMode())) {
      token->ConvertToDouble();

      double val = token->IsHex() ? static_cast<double>(token->AsHex())
                                  : token->AsDouble();
      v.SetDoubleValue(val);
      ++value_count;
    } else {
      if (token->IsDouble()) {
        return Result("invalid BUFFER data value: " +
                      token->ToOriginalString());
      }

      uint64_t val = token->IsHex() ? token->AsHex() : token->AsUint64();
      v.SetIntValue(val);
      ++value_count;
    }
    ++seg_idx;
    if (seg_idx >= segs.size())
      seg_idx = 0;

    values.emplace_back(v);
  }
  // Write final padding bytes
  while (segs[seg_idx].IsPadding()) {
    ++seg_idx;
    if (seg_idx >= segs.size())
      break;
  }

  buffer->SetValueCount(value_count);
  Result r = buffer->SetData(std::move(values));
  if (!r.IsSuccess())
    return r;

  return ValidateEndOfStatement("BUFFER data command");
}

Result Parser::ParseRun() {
  auto token = tokenizer_->NextToken();
  if (!token->IsString())
    return Result("missing pipeline name for RUN command");

  size_t line = tokenizer_->GetCurrentLine();

  auto* pipeline = script_->GetPipeline(token->AsString());
  if (!pipeline)
    return Result("unknown pipeline for RUN command: " + token->AsString());

  token = tokenizer_->NextToken();
  if (token->IsEOL() || token->IsEOS())
    return Result("RUN command requires parameters");

  if (token->IsInteger()) {
    if (!pipeline->IsCompute())
      return Result("RUN command requires compute pipeline");

    auto cmd = MakeUnique<ComputeCommand>(pipeline);
    cmd->SetLine(line);
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

    command_list_.push_back(std::move(cmd));
    return ValidateEndOfStatement("RUN command");
  }
  if (!token->IsString())
    return Result("invalid token in RUN command: " + token->ToOriginalString());

  if (token->AsString() == "DRAW_RECT") {
    if (!pipeline->IsGraphics())
      return Result("RUN command requires graphics pipeline");

    if (pipeline->GetVertexBuffers().size() > 1) {
      return Result(
          "RUN DRAW_RECT is not supported in a pipeline with more than one "
          "vertex buffer attached");
    }

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
    cmd->SetLine(line);
    cmd->EnableOrtho();

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

    command_list_.push_back(std::move(cmd));
    return ValidateEndOfStatement("RUN command");
  }

  if (token->AsString() == "DRAW_ARRAY") {
    if (!pipeline->IsGraphics())
      return Result("RUN command requires graphics pipeline");

    if (pipeline->GetVertexBuffers().empty())
      return Result("RUN DRAW_ARRAY requires attached vertex buffer");

    token = tokenizer_->NextToken();
    if (!token->IsString() || token->AsString() != "AS")
      return Result("missing AS for RUN command");

    token = tokenizer_->NextToken();
    if (!token->IsString()) {
      return Result("invalid topology for RUN command: " +
                    token->ToOriginalString());
    }

    Topology topo = NameToTopology(token->AsString());
    if (topo == Topology::kUnknown)
      return Result("invalid topology for RUN command: " + token->AsString());

    token = tokenizer_->NextToken();
    bool indexed = false;
    if (token->IsString() && token->AsString() == "INDEXED") {
      if (!pipeline->GetIndexBuffer())
        return Result("RUN DRAW_ARRAYS INDEXED requires attached index buffer");

      indexed = true;
      token = tokenizer_->NextToken();
    }

    uint32_t start_idx = 0;
    uint32_t count = 0;
    if (!token->IsEOS() && !token->IsEOL()) {
      if (!token->IsString() || token->AsString() != "START_IDX")
        return Result("missing START_IDX for RUN command");

      token = tokenizer_->NextToken();
      if (!token->IsInteger()) {
        return Result("invalid START_IDX value for RUN command: " +
                      token->ToOriginalString());
      }
      if (token->AsInt32() < 0)
        return Result("START_IDX value must be >= 0 for RUN command");
      start_idx = token->AsUint32();

      token = tokenizer_->NextToken();

      if (!token->IsEOS() && !token->IsEOL()) {
        if (!token->IsString() || token->AsString() != "COUNT")
          return Result("missing COUNT for RUN command");

        token = tokenizer_->NextToken();
        if (!token->IsInteger()) {
          return Result("invalid COUNT value for RUN command: " +
                        token->ToOriginalString());
        }
        if (token->AsInt32() <= 0)
          return Result("COUNT value must be > 0 for RUN command");

        count = token->AsUint32();
      }
    }
    // If we get here then we never set count, as if count was set it must
    // be > 0.
    if (count == 0) {
      count =
          pipeline->GetVertexBuffers()[0].buffer->ElementCount() - start_idx;
    }

    if (start_idx + count >
        pipeline->GetVertexBuffers()[0].buffer->ElementCount()) {
      return Result("START_IDX plus COUNT exceeds vertex buffer data size");
    }

    auto cmd = MakeUnique<DrawArraysCommand>(pipeline, PipelineData{});
    cmd->SetLine(line);
    cmd->SetTopology(topo);
    cmd->SetFirstVertexIndex(start_idx);
    cmd->SetVertexCount(count);

    if (indexed)
      cmd->EnableIndexed();

    command_list_.push_back(std::move(cmd));
    return ValidateEndOfStatement("RUN command");
  }

  return Result("invalid token in RUN command: " + token->AsString());
}

Result Parser::ParseClear() {
  auto token = tokenizer_->NextToken();
  if (!token->IsString())
    return Result("missing pipeline name for CLEAR command");

  size_t line = tokenizer_->GetCurrentLine();

  auto* pipeline = script_->GetPipeline(token->AsString());
  if (!pipeline)
    return Result("unknown pipeline for CLEAR command: " + token->AsString());
  if (!pipeline->IsGraphics())
    return Result("CLEAR command requires graphics pipeline");

  auto cmd = MakeUnique<ClearCommand>(pipeline);
  cmd->SetLine(line);
  command_list_.push_back(std::move(cmd));

  return ValidateEndOfStatement("CLEAR command");
}

Result Parser::ParseValues(const std::string& name,
                           Format* fmt,
                           std::vector<Value>* values) {
  assert(values);

  auto token = tokenizer_->NextToken();
  const auto& segs = fmt->GetSegments();
  size_t seg_idx = 0;
  while (!token->IsEOL() && !token->IsEOS()) {
    Value v;

    while (segs[seg_idx].IsPadding()) {
      ++seg_idx;
      if (seg_idx >= segs.size())
        seg_idx = 0;
    }

    if (type::Type::IsFloat(segs[seg_idx].GetFormatMode())) {
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
    ++seg_idx;
    if (seg_idx >= segs.size())
      seg_idx = 0;

    values->push_back(v);
    token = tokenizer_->NextToken();
  }
  return {};
}

Result Parser::ParseExpect() {
  auto token = tokenizer_->NextToken();
  if (!token->IsString())
    return Result("invalid buffer name in EXPECT command");

  if (token->AsString() == "IDX")
    return Result("missing buffer name between EXPECT and IDX");
  if (token->AsString() == "EQ_BUFFER")
    return Result("missing buffer name between EXPECT and EQ_BUFFER");
  if (token->AsString() == "RMSE_BUFFER")
    return Result("missing buffer name between EXPECT and RMSE_BUFFER");
  if (token->AsString() == "EQ_HISTOGRAM_EMD_BUFFER") {
    return Result(
        "missing buffer name between EXPECT and EQ_HISTOGRAM_EMD_BUFFER");
  }

  size_t line = tokenizer_->GetCurrentLine();
  auto* buffer = script_->GetBuffer(token->AsString());
  if (!buffer)
    return Result("unknown buffer name for EXPECT command: " +
                  token->AsString());

  token = tokenizer_->NextToken();

  if (!token->IsString())
    return Result("invalid comparator in EXPECT command");

  if (token->AsString() == "EQ_BUFFER" || token->AsString() == "RMSE_BUFFER" ||
      token->AsString() == "EQ_HISTOGRAM_EMD_BUFFER") {
    auto type = token->AsString();

    token = tokenizer_->NextToken();
    if (!token->IsString())
      return Result("invalid buffer name in EXPECT " + type + " command");

    auto* buffer_2 = script_->GetBuffer(token->AsString());
    if (!buffer_2) {
      return Result("unknown buffer name for EXPECT " + type +
                    " command: " + token->AsString());
    }

    if (!buffer->GetFormat()->Equal(buffer_2->GetFormat())) {
      return Result("EXPECT " + type +
                    " command cannot compare buffers of differing format");
    }
    if (buffer->ElementCount() != buffer_2->ElementCount()) {
      return Result("EXPECT " + type +
                    " command cannot compare buffers of different size: " +
                    std::to_string(buffer->ElementCount()) + " vs " +
                    std::to_string(buffer_2->ElementCount()));
    }
    if (buffer->GetWidth() != buffer_2->GetWidth()) {
      return Result("EXPECT " + type +
                    " command cannot compare buffers of different width");
    }
    if (buffer->GetHeight() != buffer_2->GetHeight()) {
      return Result("EXPECT " + type +
                    " command cannot compare buffers of different height");
    }

    auto cmd = MakeUnique<CompareBufferCommand>(buffer, buffer_2);
    if (type == "RMSE_BUFFER") {
      cmd->SetComparator(CompareBufferCommand::Comparator::kRmse);

      token = tokenizer_->NextToken();
      if (!token->IsString() && token->AsString() == "TOLERANCE")
        return Result("missing TOLERANCE for EXPECT RMSE_BUFFER");

      token = tokenizer_->NextToken();
      if (!token->IsInteger() && !token->IsDouble())
        return Result("invalid TOLERANCE for EXPECT RMSE_BUFFER");

      Result r = token->ConvertToDouble();
      if (!r.IsSuccess())
        return r;

      cmd->SetTolerance(token->AsFloat());
    } else if (type == "EQ_HISTOGRAM_EMD_BUFFER") {
      cmd->SetComparator(CompareBufferCommand::Comparator::kHistogramEmd);

      token = tokenizer_->NextToken();
      if (!token->IsString() && token->AsString() == "TOLERANCE")
        return Result("missing TOLERANCE for EXPECT EQ_HISTOGRAM_EMD_BUFFER");

      token = tokenizer_->NextToken();
      if (!token->IsInteger() && !token->IsDouble())
        return Result("invalid TOLERANCE for EXPECT EQ_HISTOGRAM_EMD_BUFFER");

      Result r = token->ConvertToDouble();
      if (!r.IsSuccess())
        return r;

      cmd->SetTolerance(token->AsFloat());
    }

    command_list_.push_back(std::move(cmd));

    // Early return
    return ValidateEndOfStatement("EXPECT " + type + " command");
  }

  if (token->AsString() != "IDX")
    return Result("missing IDX in EXPECT command");

  token = tokenizer_->NextToken();
  if (!token->IsInteger() || token->AsInt32() < 0)
    return Result("invalid X value in EXPECT command");
  token->ConvertToDouble();
  float x = token->AsFloat();

  bool has_y_val = false;
  float y = 0;
  token = tokenizer_->NextToken();
  if (token->IsInteger()) {
    has_y_val = true;

    if (token->AsInt32() < 0)
      return Result("invalid Y value in EXPECT command");
    token->ConvertToDouble();
    y = token->AsFloat();

    token = tokenizer_->NextToken();
  }

  if (token->IsString() && token->AsString() == "SIZE") {
    if (!has_y_val)
      return Result("invalid Y value in EXPECT command");

    auto probe = MakeUnique<ProbeCommand>(buffer);
    probe->SetLine(line);
    probe->SetX(x);
    probe->SetY(y);
    probe->SetProbeRect();

    token = tokenizer_->NextToken();
    if (!token->IsInteger() || token->AsInt32() <= 0)
      return Result("invalid width in EXPECT command");
    token->ConvertToDouble();
    probe->SetWidth(token->AsFloat());

    token = tokenizer_->NextToken();
    if (!token->IsInteger() || token->AsInt32() <= 0)
      return Result("invalid height in EXPECT command");
    token->ConvertToDouble();
    probe->SetHeight(token->AsFloat());

    token = tokenizer_->NextToken();
    if (!token->IsString()) {
      return Result("invalid token in EXPECT command:" +
                    token->ToOriginalString());
    }

    if (token->AsString() == "EQ_RGBA") {
      probe->SetIsRGBA();
    } else if (token->AsString() != "EQ_RGB") {
      return Result("unknown comparator type in EXPECT: " +
                    token->ToOriginalString());
    }

    token = tokenizer_->NextToken();
    if (!token->IsInteger() || token->AsInt32() < 0 || token->AsInt32() > 255)
      return Result("invalid R value in EXPECT command");
    token->ConvertToDouble();
    probe->SetR(token->AsFloat() / 255.f);

    token = tokenizer_->NextToken();
    if (!token->IsInteger() || token->AsInt32() < 0 || token->AsInt32() > 255)
      return Result("invalid G value in EXPECT command");
    token->ConvertToDouble();
    probe->SetG(token->AsFloat() / 255.f);

    token = tokenizer_->NextToken();
    if (!token->IsInteger() || token->AsInt32() < 0 || token->AsInt32() > 255)
      return Result("invalid B value in EXPECT command");
    token->ConvertToDouble();
    probe->SetB(token->AsFloat() / 255.f);

    if (probe->IsRGBA()) {
      token = tokenizer_->NextToken();
      if (!token->IsInteger() || token->AsInt32() < 0 || token->AsInt32() > 255)
        return Result("invalid A value in EXPECT command");
      token->ConvertToDouble();
      probe->SetA(token->AsFloat() / 255.f);
    }

    command_list_.push_back(std::move(probe));
    return ValidateEndOfStatement("EXPECT command");
  }

  auto probe = MakeUnique<ProbeSSBOCommand>(buffer);
  probe->SetLine(line);

  if (token->IsString() && token->AsString() == "TOLERANCE") {
    std::vector<Probe::Tolerance> tolerances;

    token = tokenizer_->NextToken();
    while (!token->IsEOL() && !token->IsEOS()) {
      if (!token->IsInteger() && !token->IsDouble())
        break;

      Result r = token->ConvertToDouble();
      if (!r.IsSuccess())
        return r;

      double value = token->AsDouble();
      token = tokenizer_->NextToken();
      if (token->IsString() && token->AsString() == "%") {
        tolerances.push_back(Probe::Tolerance{true, value});
        token = tokenizer_->NextToken();
      } else {
        tolerances.push_back(Probe::Tolerance{false, value});
      }
    }
    if (tolerances.empty())
      return Result("TOLERANCE specified but no tolerances provided");
    if (tolerances.size() > 4)
      return Result("TOLERANCE has a maximum of 4 values");

    probe->SetTolerances(std::move(tolerances));
  }

  if (!token->IsString() || !IsComparator(token->AsString())) {
    return Result("unexpected token in EXPECT command: " +
                  token->ToOriginalString());
  }

  if (has_y_val)
    return Result("Y value not needed for non-color comparator");

  auto cmp = ToComparator(token->AsString());
  if (probe->HasTolerances()) {
    if (cmp != ProbeSSBOCommand::Comparator::kEqual)
      return Result("TOLERANCE only available with EQ probes");

    cmp = ProbeSSBOCommand::Comparator::kFuzzyEqual;
  }

  probe->SetComparator(cmp);
  probe->SetFormat(buffer->GetFormat());
  probe->SetOffset(static_cast<uint32_t>(x));

  std::vector<Value> values;
  Result r = ParseValues("EXPECT", buffer->GetFormat(), &values);
  if (!r.IsSuccess())
    return r;

  if (values.empty())
    return Result("missing comparison values for EXPECT command");

  probe->SetValues(std::move(values));
  command_list_.push_back(std::move(probe));

  return {};
}

Result Parser::ParseCopy() {
  auto token = tokenizer_->NextToken();
  if (token->IsEOL() || token->IsEOS())
    return Result("missing buffer name after COPY");
  if (!token->IsString())
    return Result("invalid buffer name after COPY");

  size_t line = tokenizer_->GetCurrentLine();

  auto name = token->AsString();
  if (name == "TO")
    return Result("missing buffer name between COPY and TO");

  Buffer* buffer_from = script_->GetBuffer(name);
  if (!buffer_from)
    return Result("COPY origin buffer was not declared");

  token = tokenizer_->NextToken();
  if (token->IsEOL() || token->IsEOS())
    return Result("missing 'TO' after COPY and buffer name");
  if (!token->IsString())
    return Result("expected 'TO' after COPY and buffer name");

  name = token->AsString();
  if (name != "TO")
    return Result("expected 'TO' after COPY and buffer name");

  token = tokenizer_->NextToken();
  if (token->IsEOL() || token->IsEOS())
    return Result("missing buffer name after TO");
  if (!token->IsString())
    return Result("invalid buffer name after TO");

  name = token->AsString();
  Buffer* buffer_to = script_->GetBuffer(name);
  if (!buffer_to)
    return Result("COPY destination buffer was not declared");

  if (buffer_to->GetBufferType() == amber::BufferType::kUnknown) {
    // Set destination buffer to mirror origin buffer
    buffer_to->SetBufferType(buffer_from->GetBufferType());
    buffer_to->SetWidth(buffer_from->GetWidth());
    buffer_to->SetHeight(buffer_from->GetHeight());
    buffer_to->SetElementCount(buffer_from->ElementCount());
  }

  if (buffer_from->GetBufferType() != buffer_to->GetBufferType())
    return Result("cannot COPY between buffers of different types");
  if (buffer_from == buffer_to)
    return Result("COPY origin and destination buffers are identical");

  auto cmd = MakeUnique<CopyCommand>(buffer_from, buffer_to);
  cmd->SetLine(line);
  command_list_.push_back(std::move(cmd));

  return ValidateEndOfStatement("COPY command");
}

Result Parser::ParseClearColor() {
  auto token = tokenizer_->NextToken();
  if (!token->IsString())
    return Result("missing pipeline name for CLEAR_COLOR command");

  size_t line = tokenizer_->GetCurrentLine();

  auto* pipeline = script_->GetPipeline(token->AsString());
  if (!pipeline) {
    return Result("unknown pipeline for CLEAR_COLOR command: " +
                  token->AsString());
  }
  if (!pipeline->IsGraphics()) {
    return Result("CLEAR_COLOR command requires graphics pipeline");
  }

  auto cmd = MakeUnique<ClearColorCommand>(pipeline);
  cmd->SetLine(line);

  token = tokenizer_->NextToken();
  if (token->IsEOL() || token->IsEOS())
    return Result("missing R value for CLEAR_COLOR command");
  if (!token->IsInteger() || token->AsInt32() < 0 || token->AsInt32() > 255) {
    return Result("invalid R value for CLEAR_COLOR command: " +
                  token->ToOriginalString());
  }
  token->ConvertToDouble();
  cmd->SetR(token->AsFloat() / 255.f);

  token = tokenizer_->NextToken();
  if (token->IsEOL() || token->IsEOS())
    return Result("missing G value for CLEAR_COLOR command");
  if (!token->IsInteger() || token->AsInt32() < 0 || token->AsInt32() > 255) {
    return Result("invalid G value for CLEAR_COLOR command: " +
                  token->ToOriginalString());
  }
  token->ConvertToDouble();
  cmd->SetG(token->AsFloat() / 255.f);

  token = tokenizer_->NextToken();
  if (token->IsEOL() || token->IsEOS())
    return Result("missing B value for CLEAR_COLOR command");
  if (!token->IsInteger() || token->AsInt32() < 0 || token->AsInt32() > 255) {
    return Result("invalid B value for CLEAR_COLOR command: " +
                  token->ToOriginalString());
  }
  token->ConvertToDouble();
  cmd->SetB(token->AsFloat() / 255.f);

  token = tokenizer_->NextToken();
  if (token->IsEOL() || token->IsEOS())
    return Result("missing A value for CLEAR_COLOR command");
  if (!token->IsInteger() || token->AsInt32() < 0 || token->AsInt32() > 255) {
    return Result("invalid A value for CLEAR_COLOR command: " +
                  token->ToOriginalString());
  }
  token->ConvertToDouble();
  cmd->SetA(token->AsFloat() / 255.f);

  command_list_.push_back(std::move(cmd));
  return ValidateEndOfStatement("CLEAR_COLOR command");
}

Result Parser::ParseDeviceFeature() {
  auto token = tokenizer_->NextToken();
  if (token->IsEOS() || token->IsEOL())
    return Result("missing feature name for DEVICE_FEATURE command");
  if (!token->IsString())
    return Result("invalid feature name for DEVICE_FEATURE command");
  if (!script_->IsKnownFeature(token->AsString()))
    return Result("unknown feature name for DEVICE_FEATURE command");

  script_->AddRequiredFeature(token->AsString());

  return ValidateEndOfStatement("DEVICE_FEATURE command");
}

Result Parser::ParseRepeat() {
  auto token = tokenizer_->NextToken();
  if (token->IsEOL() || token->IsEOL())
    return Result("missing count parameter for REPEAT command");
  if (!token->IsInteger()) {
    return Result("invalid count parameter for REPEAT command: " +
                  token->ToOriginalString());
  }
  if (token->AsInt32() <= 0)
    return Result("count parameter must be > 0 for REPEAT command");

  uint32_t count = token->AsUint32();

  std::vector<std::unique_ptr<Command>> cur_commands;
  std::swap(cur_commands, command_list_);

  for (token = tokenizer_->NextToken(); !token->IsEOS();
       token = tokenizer_->NextToken()) {
    if (token->IsEOL())
      continue;
    if (!token->IsString())
      return Result("expected string");

    std::string tok = token->AsString();
    if (tok == "END")
      break;
    if (!IsRepeatable(tok))
      return Result("unknown token: " + tok);

    Result r = ParseRepeatableCommand(tok);
    if (!r.IsSuccess())
      return r;
  }
  if (!token->IsString() || token->AsString() != "END")
    return Result("missing END for REPEAT command");

  auto cmd = MakeUnique<RepeatCommand>(count);
  cmd->SetCommands(std::move(command_list_));

  std::swap(cur_commands, command_list_);
  command_list_.push_back(std::move(cmd));

  return ValidateEndOfStatement("REPEAT command");
}

Result Parser::ParseDerivePipelineBlock() {
  auto token = tokenizer_->NextToken();
  if (!token->IsString() || token->AsString() == "FROM")
    return Result("missing pipeline name for DERIVE_PIPELINE command");

  std::string name = token->AsString();
  if (script_->GetPipeline(name) != nullptr)
    return Result("duplicate pipeline name for DERIVE_PIPELINE command");

  token = tokenizer_->NextToken();
  if (!token->IsString() || token->AsString() != "FROM")
    return Result("missing FROM in DERIVE_PIPELINE command");

  token = tokenizer_->NextToken();
  if (!token->IsString())
    return Result("missing parent pipeline name in DERIVE_PIPELINE command");

  Pipeline* parent = script_->GetPipeline(token->AsString());
  if (!parent)
    return Result("unknown parent pipeline in DERIVE_PIPELINE command");

  Result r = ValidateEndOfStatement("DERIVE_PIPELINE command");
  if (!r.IsSuccess())
    return r;

  auto pipeline = parent->Clone();
  pipeline->SetName(name);

  return ParsePipelineBody("DERIVE_PIPELINE", std::move(pipeline));
}

Result Parser::ParseDeviceExtension() {
  auto token = tokenizer_->NextToken();
  if (token->IsEOL() || token->IsEOS())
    return Result("DEVICE_EXTENSION missing name");
  if (!token->IsString()) {
    return Result("DEVICE_EXTENSION invalid name: " +
                  token->ToOriginalString());
  }

  script_->AddRequiredDeviceExtension(token->AsString());

  return ValidateEndOfStatement("DEVICE_EXTENSION command");
}

Result Parser::ParseInstanceExtension() {
  auto token = tokenizer_->NextToken();
  if (token->IsEOL() || token->IsEOS())
    return Result("INSTANCE_EXTENSION missing name");
  if (!token->IsString()) {
    return Result("INSTANCE_EXTENSION invalid name: " +
                  token->ToOriginalString());
  }

  script_->AddRequiredInstanceExtension(token->AsString());

  return ValidateEndOfStatement("INSTANCE_EXTENSION command");
}

Result Parser::ParseSet() {
  auto token = tokenizer_->NextToken();
  if (!token->IsString() || token->AsString() != "ENGINE_DATA")
    return Result("SET missing ENGINE_DATA");

  token = tokenizer_->NextToken();
  if (token->IsEOS() || token->IsEOL())
    return Result("SET missing variable to be set");

  if (!token->IsString())
    return Result("SET invalid variable to set: " + token->ToOriginalString());

  if (token->AsString() != "fence_timeout_ms")
    return Result("SET unknown variable provided: " + token->AsString());

  token = tokenizer_->NextToken();
  if (token->IsEOS() || token->IsEOL())
    return Result("SET missing value for fence_timeout_ms");
  if (!token->IsInteger())
    return Result("SET invalid value for fence_timeout_ms, must be uint32");

  script_->GetEngineData().fence_timeout_ms = token->AsUint32();

  return ValidateEndOfStatement("SET command");
}

}  // namespace amberscript
}  // namespace amber
