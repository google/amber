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
                                const ShaderMap& shader_map) {
  for (auto& pipeline : script->GetPipelines()) {
    for (auto& shader_info : pipeline->GetShaders()) {
      ShaderCompiler sc(script->GetSpvTargetEnv());

      Result r;
      std::vector<uint32_t> data;
      std::tie(r, data) = sc.Compile(shader_info.GetShader(), shader_map);
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
                         ExecutionType executionType) {
  engine->SetEngineData(script->GetEngineData());

  if (script->GetPipelines().empty())
    return Result("no pipelines defined");

  Result r = CompileShaders(script, shader_map);
  if (!r.IsSuccess())
    return r;

  for (auto& pipeline : script->GetPipelines()) {
    r = engine->CreatePipeline(pipeline.get());
    if (!r.IsSuccess())
      return r;
  }

  if (executionType == ExecutionType::kPipelineCreateOnly)
    return {};

  // Process Commands
  for (const auto& cmd : script->GetCommands()) {
    if (cmd->IsProbe()) {
      assert(cmd->AsProbe()->GetBuffer()->IsFormatBuffer());

      auto* buffer = cmd->AsProbe()->GetBuffer()->AsFormatBuffer();
      assert(buffer);

      r = verifier_.Probe(cmd->AsProbe(), &buffer->GetFormat(),
                          buffer->GetTexelStride(), buffer->GetRowStride(),
                          buffer->GetWidth(), buffer->GetHeight(),
                          buffer->ValuePtr()->data());
    } else if (cmd->IsProbeSSBO()) {
      auto probe_ssbo = cmd->AsProbeSSBO();

      const auto* buffer = cmd->AsProbe()->GetBuffer();
      assert(buffer);

      r = verifier_.ProbeSSBO(probe_ssbo, buffer->GetSize(),
                              buffer->ValuePtr()->data());
    } else if (cmd->IsClear()) {
      r = engine->DoClear(cmd->AsClear());
    } else if (cmd->IsClearColor()) {
      r = engine->DoClearColor(cmd->AsClearColor());
    } else if (cmd->IsClearDepth()) {
      r = engine->DoClearDepth(cmd->AsClearDepth());
    } else if (cmd->IsClearStencil()) {
      r = engine->DoClearStencil(cmd->AsClearStencil());
    } else if (cmd->IsDrawRect()) {
      r = engine->DoDrawRect(cmd->AsDrawRect());
    } else if (cmd->IsDrawArrays()) {
      r = engine->DoDrawArrays(cmd->AsDrawArrays());
    } else if (cmd->IsCompute()) {
      r = engine->DoCompute(cmd->AsCompute());
    } else if (cmd->IsEntryPoint()) {
      r = engine->DoEntryPoint(cmd->AsEntryPoint());
    } else if (cmd->IsPatchParameterVertices()) {
      r = engine->DoPatchParameterVertices(cmd->AsPatchParameterVertices());
    } else if (cmd->IsBuffer()) {
      r = engine->DoBuffer(cmd->AsBuffer());
    } else {
      return Result("Unknown command type");
    }

    if (!r.IsSuccess())
      return r;
  }
  return {};
}

}  // namespace amber
