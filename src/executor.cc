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

#include "src/executor.h"

#include <cassert>
#include <utility>
#include <vector>

#include "src/engine.h"
#include "src/make_unique.h"
#include "src/script.h"
#include "src/shader_compiler.h"

namespace amber {

Executor::Executor() = default;

Executor::~Executor() = default;

Result Executor::CompileShaders(const amber::Script* script,
                                const ShaderMap& shader_map,
                                Options* options) {
  for (auto& pipeline : script->GetPipelines()) {
    for (auto& shader_info : pipeline->GetShaders()) {
      ShaderCompiler sc(script->GetSpvTargetEnv(),
                        options->disable_spirv_validation);

      Result r;
      std::vector<uint32_t> data;
      std::tie(r, data) = sc.Compile(&shader_info, shader_map);
      if (!r.IsSuccess())
        return r;

      shader_info.SetData(std::move(data));
    }
  }
  return {};
}

Result Executor::Execute(Engine* engine,
                         const amber::Script* script,
                         const ShaderMap& shader_map,
                         Options* options) {
  engine->SetEngineData(script->GetEngineData());

  if (!script->GetPipelines().empty()) {
    Result r = CompileShaders(script, shader_map, options);
    if (!r.IsSuccess())
      return r;

    // OpenCL specific pipeline updates.
    for (auto& pipeline : script->GetPipelines()) {
      r = pipeline->UpdateOpenCLBufferBindings();
      if (!r.IsSuccess())
        return r;
      r = pipeline->GenerateOpenCLPodBuffers();
      if (!r.IsSuccess())
        return r;
    }

    for (auto& pipeline : script->GetPipelines()) {
      r = engine->CreatePipeline(pipeline.get());
      if (!r.IsSuccess())
        return r;
    }
  }

  if (options->execution_type == ExecutionType::kPipelineCreateOnly)
    return {};

  // Process Commands
  for (const auto& cmd : script->GetCommands()) {
    if (options->delegate && options->delegate->LogExecuteCalls()) {
      options->delegate->Log(std::to_string(cmd->GetLine()) + ": " +
                             cmd->ToString());
    }

    Result r = ExecuteCommand(engine, cmd.get());
    if (!r.IsSuccess())
      return r;
  }
  return {};
}

Result Executor::ExecuteCommand(Engine* engine, Command* cmd) {
  if (cmd->IsProbe()) {
    auto* buffer = cmd->AsProbe()->GetBuffer();
    assert(buffer);

    Format* fmt = buffer->GetFormat();
    return verifier_.Probe(cmd->AsProbe(), fmt, buffer->GetElementStride(),
                           buffer->GetRowStride(), buffer->GetWidth(),
                           buffer->GetHeight(), buffer->ValuePtr()->data());
  }
  if (cmd->IsProbeSSBO()) {
    auto probe_ssbo = cmd->AsProbeSSBO();

    const auto* buffer = cmd->AsProbe()->GetBuffer();
    assert(buffer);

    return verifier_.ProbeSSBO(probe_ssbo, buffer->ElementCount(),
                               buffer->ValuePtr()->data());
  }
  if (cmd->IsClear())
    return engine->DoClear(cmd->AsClear());
  if (cmd->IsClearColor())
    return engine->DoClearColor(cmd->AsClearColor());
  if (cmd->IsClearDepth())
    return engine->DoClearDepth(cmd->AsClearDepth());
  if (cmd->IsClearStencil())
    return engine->DoClearStencil(cmd->AsClearStencil());
  if (cmd->IsCompareBuffer()) {
    auto compare = cmd->AsCompareBuffer();
    auto buffer_1 = compare->GetBuffer1();
    auto buffer_2 = compare->GetBuffer2();
    switch (compare->GetComparator()) {
      case CompareBufferCommand::Comparator::kRmse:
        return buffer_1->CompareRMSE(buffer_2, compare->GetTolerance());
      case CompareBufferCommand::Comparator::kHistogramEmd:
        return buffer_1->CompareHistogramEMD(buffer_2, compare->GetTolerance());
      case CompareBufferCommand::Comparator::kEq:
        return buffer_1->IsEqual(buffer_2);
    }
  }
  if (cmd->IsCopy()) {
    auto copy = cmd->AsCopy();
    auto buffer_from = copy->GetBufferFrom();
    auto buffer_to = copy->GetBufferTo();
    return buffer_from->CopyTo(buffer_to);
  }
  if (cmd->IsDrawRect())
    return engine->DoDrawRect(cmd->AsDrawRect());
  if (cmd->IsDrawArrays())
    return engine->DoDrawArrays(cmd->AsDrawArrays());
  if (cmd->IsCompute())
    return engine->DoCompute(cmd->AsCompute());
  if (cmd->IsEntryPoint())
    return engine->DoEntryPoint(cmd->AsEntryPoint());
  if (cmd->IsPatchParameterVertices())
    return engine->DoPatchParameterVertices(cmd->AsPatchParameterVertices());
  if (cmd->IsBuffer())
    return engine->DoBuffer(cmd->AsBuffer());
  if (cmd->IsRepeat()) {
    for (uint32_t i = 0; i < cmd->AsRepeat()->GetCount(); ++i) {
      for (const auto& sub_cmd : cmd->AsRepeat()->GetCommands()) {
        Result r = ExecuteCommand(engine, sub_cmd.get());
        if (!r.IsSuccess())
          return r;
      }
    }
    return {};
  }
  return Result("Unknown command type: " +
                std::to_string(static_cast<uint32_t>(cmd->GetType())));
}

}  // namespace amber
