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

#ifndef SRC_AMBERSCRIPT_PARSER_H_
#define SRC_AMBERSCRIPT_PARSER_H_

#include <memory>
#include <string>
#include <utility>

#include "amber/result.h"
#include "src/parser.h"
#include "src/script.h"

namespace amber {

class Tokenizer;

namespace amberscript {

class Parser : public amber::Parser {
 public:
  Parser();
  ~Parser() override;

  // amber::Parser
  Result Parse(const std::string& data) override;

 private:
  std::string make_error(const std::string& err);
  Result ToShaderType(const std::string& str, ShaderType* type);
  Result ToBufferType(const std::string& str, BufferType* type);
  Result ToShaderFormat(const std::string& str, ShaderFormat* fmt);
  Result ToDatumType(const std::string& str, DatumType* type);
  Result ToPipelineType(const std::string& str, PipelineType* type);
  Result ValidateEndOfStatement(const std::string& name);

  Result ParseBuffer();
  Result ParseBufferInitializer(DataBuffer*);
  Result ParseBufferInitializerSize(DataBuffer*);
  Result ParseBufferInitializerFill(DataBuffer*, uint32_t);
  Result ParseBufferInitializerSeries(DataBuffer*, uint32_t);
  Result ParseBufferInitializerData(DataBuffer*);
  Result ParseShaderBlock();
  Result ParsePipelineBlock();
  Result ParsePipelineAttach(Pipeline*);
  Result ParsePipelineShaderOptimizations(Pipeline*);
  Result ParsePipelineFramebufferSize(Pipeline*);
  Result ParsePipelineBind(Pipeline*);
  Result ParsePipelineVertexData(Pipeline*);
  Result ParsePipelineIndexData(Pipeline*);

  std::unique_ptr<Tokenizer> tokenizer_;
};

}  // namespace amberscript
}  // namespace amber

#endif  // SRC_AMBERSCRIPT_PARSER_H_
