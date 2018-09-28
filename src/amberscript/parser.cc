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

#include "src/make_unique.h"
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
    if (tok == "SHADER") {
      r = ParseShaderBlock();
    } else if (tok == "PIPELINE") {
      r = ParsePipelineBlock();
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

    r = script_.AddShader(std::move(shader));
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

  r = script_.AddShader(std::move(shader));
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

  r = script_.AddPipeline(std::move(pipeline));
  if (!r.IsSuccess())
    return r;

  return ValidateEndOfStatement("END");
}

Result Parser::ParsePipelineAttach(Pipeline* pipeline) {
  auto token = tokenizer_->NextToken();
  if (!token->IsString())
    return Result("invalid token in ATTACH command");

  auto* shader = script_.GetShader(token->AsString());
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

  auto* shader = script_.GetShader(token->AsString());
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

  auto* shader = script_.GetShader(token->AsString());
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

}  // namespace amberscript
}  // namespace amber
