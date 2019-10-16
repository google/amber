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
#include <vector>

#include "amber/result.h"
#include "src/parser.h"
#include "src/script.h"

namespace amber {

class Tokenizer;

namespace amberscript {

/// Parser for the `AmberScript` format.
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
  Result ToPipelineType(const std::string& str, PipelineType* type);
  Result ValidateEndOfStatement(const std::string& name);

  Result ParseStruct();
  Result ParseBuffer();
  Result ParseBufferInitializer(Buffer*);
  Result ParseBufferInitializerSize(Buffer*);
  Result ParseBufferInitializerFill(Buffer*, uint32_t);
  Result ParseBufferInitializerSeries(Buffer*, uint32_t);
  Result ParseBufferInitializerData(Buffer*);
  Result ParseShaderBlock();
  Result ParsePipelineBlock();
  Result ParsePipelineAttach(Pipeline*);
  Result ParsePipelineShaderOptimizations(Pipeline*);
  Result ParsePipelineShaderCompileOptions(Pipeline*);
  Result ParsePipelineFramebufferSize(Pipeline*);
  Result ParsePipelineBind(Pipeline*);
  Result ParsePipelineVertexData(Pipeline*);
  Result ParsePipelineIndexData(Pipeline*);
  Result ParsePipelineSet(Pipeline*);
  Result ParseRun();
  Result ParseClear();
  Result ParseClearColor();
  Result ParseExpect();
  Result ParseCopy();
  Result ParseDeviceFeature();
  Result ParseDeviceExtension();
  Result ParseInstanceExtension();
  Result ParseRepeat();
  Result ParseSet();
  bool IsRepeatable(const std::string& name) const;
  Result ParseRepeatableCommand(const std::string& name);
  Result ParseDerivePipelineBlock();
  Result ParsePipelineBody(const std::string& cmd_name,
                           std::unique_ptr<Pipeline> pipeline);
  Result ParseShaderSpecialization(Pipeline* pipeline);

  /// Parses a set of values out of the token stream. |name| is the name of the
  /// current command we're parsing for error purposes. The |type| is the type
  /// of data we expect for the current buffer. |values| will be appended to
  /// with the parsed values.
  Result ParseValues(const std::string& name,
                     Format* fmt,
                     std::vector<Value>* values);

  std::unique_ptr<Tokenizer> tokenizer_;
  std::vector<std::unique_ptr<Command>> command_list_;
};

}  // namespace amberscript
}  // namespace amber

#endif  // SRC_AMBERSCRIPT_PARSER_H_
