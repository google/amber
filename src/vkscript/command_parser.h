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

#ifndef SRC_VKSCRIPT_COMMAND_PARSER_H_
#define SRC_VKSCRIPT_COMMAND_PARSER_H_

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "amber/result.h"
#include "src/command.h"
#include "src/pipeline.h"
#include "src/pipeline_data.h"
#include "src/script.h"

namespace amber {

class Tokenizer;
class Token;

namespace vkscript {

/// Parses the contents of the [test] section of a VkScript file into individual
/// commands.
class CommandParser {
 public:
  CommandParser(Script* script,
                Pipeline* pipeline,
                size_t current_line,
                const std::string& data);
  ~CommandParser();

  Result Parse();

  void AddCommand(std::unique_ptr<Command> command) {
    commands_.push_back(std::move(command));
  }

  const std::vector<std::unique_ptr<Command>>& Commands() const {
    return commands_;
  }

  std::vector<std::unique_ptr<Command>>&& TakeCommands() {
    return std::move(commands_);
  }

  const PipelineData* PipelineDataForTesting() const { return &pipeline_data_; }

  Result ParseBooleanForTesting(const std::string& str, bool* result) {
    return ParseBoolean(str, result);
  }

  Result ParseBlendFactorNameForTesting(const std::string& name,
                                        BlendFactor* factor) {
    return ParseBlendFactorName(name, factor);
  }
  Result ParseBlendOpNameForTesting(const std::string& name, BlendOp* op) {
    return ParseBlendOpName(name, op);
  }
  Result ParseCompareOpNameForTesting(const std::string& name, CompareOp* op) {
    return ParseCompareOpName(name, op);
  }
  Result ParseStencilOpNameForTesting(const std::string& name, StencilOp* op) {
    return ParseStencilOpName(name, op);
  }
  Result ParseComparatorForTesting(const std::string& name,
                                   ProbeSSBOCommand::Comparator* op) {
    return ParseComparator(name, op);
  }
  const std::vector<Probe::Tolerance>& TolerancesForTesting() const {
    return current_tolerances_;
  }

 private:
  std::string make_error(const std::string& err);
  Result TokenToFloat(Token* token, float* val) const;
  Result TokenToDouble(Token* token, double* val) const;
  Result ParseBoolean(const std::string& str, bool* result);
  Result ParseValues(const std::string& name,
                     Format* fmt,
                     std::vector<Value>* values);

  Result ProcessDrawRect();
  Result ProcessDrawArrays();
  Result ProcessCompute();
  Result ProcessClear();
  Result ProcessPatch();
  Result ProcessSSBO();
  Result ProcessUniform();
  Result ProcessTolerance();
  Result ProcessEntryPoint(const std::string& name);
  Result ProcessProbe(bool relative);
  Result ProcessProbeSSBO();
  Result ProcessTopology();
  Result ProcessPolygonMode();
  Result ProcessLogicOp();
  Result ProcessCullMode();
  Result ProcessFrontFace();
  Result ProcessFloatPipelineData(const std::string& name, float* value);
  Result ProcessDepthBiasConstantFactor();
  Result ProcessDepthBiasClamp();
  Result ProcessDepthBiasSlopeFactor();
  Result ProcessLineWidth();
  Result ProcessMinDepthBounds();
  Result ProcessMaxDepthBounds();
  Result ProcessBooleanPipelineData(const std::string& name, bool* value);
  Result ProcessPrimitiveRestartEnable();
  Result ProcessDepthClampEnable();
  Result ProcessRasterizerDiscardEnable();
  Result ProcessDepthBiasEnable();
  Result ProcessLogicOpEnable();
  Result ProcessBlendEnable();
  Result ProcessDepthTestEnable();
  Result ProcessDepthWriteEnable();
  Result ProcessDepthBoundsTestEnable();
  Result ProcessStencilTestEnable();
  Result ParseBlendFactor(const std::string& name, BlendFactor* factor);
  Result ParseBlendFactorName(const std::string& name, BlendFactor* factor);
  Result ProcessSrcAlphaBlendFactor();
  Result ProcessDstAlphaBlendFactor();
  Result ProcessSrcColorBlendFactor();
  Result ProcessDstColorBlendFactor();
  Result ParseBlendOp(const std::string& name, BlendOp* op);
  Result ParseBlendOpName(const std::string& name, BlendOp* op);
  Result ProcessColorBlendOp();
  Result ProcessAlphaBlendOp();
  Result ParseCompareOp(const std::string& name, CompareOp* op);
  Result ParseCompareOpName(const std::string& name, CompareOp* op);
  Result ProcessDepthCompareOp();
  Result ProcessFrontCompareOp();
  Result ProcessBackCompareOp();
  Result ParseStencilOp(const std::string& name, StencilOp* op);
  Result ParseStencilOpName(const std::string& name, StencilOp* op);
  Result ProcessFrontFailOp();
  Result ProcessFrontPassOp();
  Result ProcessFrontDepthFailOp();
  Result ProcessBackFailOp();
  Result ProcessBackPassOp();
  Result ProcessBackDepthFailOp();
  Result ProcessFrontCompareMask();
  Result ProcessFrontWriteMask();
  Result ProcessBackCompareMask();
  Result ProcessBackWriteMask();
  Result ProcessFrontReference();
  Result ProcessBackReference();
  Result ProcessColorWriteMask();
  Result ParseComparator(const std::string& name,
                         ProbeSSBOCommand::Comparator* op);

  Script* script_;
  Pipeline* pipeline_;
  PipelineData pipeline_data_;
  std::unique_ptr<Tokenizer> tokenizer_;
  std::vector<std::unique_ptr<Command>> commands_;
  std::vector<Probe::Tolerance> current_tolerances_;
};

}  // namespace vkscript
}  // namespace amber

#endif  // SRC_VKSCRIPT_COMMAND_PARSER_H_
