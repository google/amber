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

#include "src/vkscript/executor.h"

#include <cassert>
#include <vector>

#include "src/engine.h"
#include "src/vkscript/nodes.h"
#include "src/vkscript/script.h"

namespace amber {
namespace vkscript {

Executor::Executor() : amber::Executor() {}

Executor::~Executor() = default;

Result Executor::Execute(Engine* engine, const amber::Script* src_script) {
  if (!src_script->IsVkScript())
    return Result("VkScript Executor called with non-vkscript source");

  const Script* script = ToVkScript(src_script);

  // Process Requirement nodes
  for (const auto& node : script->Nodes()) {
    if (!node->IsRequire())
      continue;

    for (const auto& require : node->AsRequire()->Requirements()) {
      Result r =
          engine->AddRequirement(require.GetFeature(), require.GetFormat());
      if (!r.IsSuccess())
        return r;
    }
  }

  // Process Shader nodes
  PipelineType pipeline_type = PipelineType::kGraphics;
  for (const auto& node : script->Nodes()) {
    if (!node->IsShader())
      continue;

    const auto shader = node->AsShader();
    Result r = engine->SetShader(shader->GetShaderType(), shader->GetData());
    if (!r.IsSuccess())
      return r;

    if (shader->GetShaderType() == ShaderType::kCompute)
      pipeline_type = PipelineType::kCompute;
  }

  // TODO(jaebaek): Support multiple pipelines.
  Result r = engine->CreatePipeline(pipeline_type);
  if (!r.IsSuccess())
    return r;

  // Process VertexData nodes
  for (const auto& node : script->Nodes()) {
    if (!node->IsVertexData())
      continue;

    const auto data = node->AsVertexData();
    const auto& headers = data->GetHeaders();
    const auto& rows = data->GetRows();
    for (size_t i = 0; i < headers.size(); ++i) {
      std::vector<Value> values;
      for (const auto& row : rows) {
        const auto& cell = row[i];
        for (size_t z = 0; z < cell.size(); ++z)
          values.push_back(cell.GetValue(z));
      }

      r = engine->SetBuffer(BufferType::kVertexData, headers[i].location,
                            *(headers[i].format), values);
      if (!r.IsSuccess())
        return r;
    }
  }

  // Process Indices nodes
  for (const auto& node : script->Nodes()) {
    if (!node->IsIndices())
      continue;

    std::vector<Value> values;
    for (uint16_t index : node->AsIndices()->Indices()) {
      values.push_back(Value());
      values.back().SetIntValue(index);
    }

    r = engine->SetBuffer(BufferType::kIndices, 0, Format(), values);
    if (!r.IsSuccess())
      return r;
  }

  // Process Test nodes
  for (const auto& node : script->Nodes()) {
    if (!node->IsTest())
      continue;

    for (const auto& cmd : node->AsTest()->GetCommands()) {
      if (cmd->IsClear()) {
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
      } else if (cmd->IsProbe()) {
        r = engine->DoProbe(cmd->AsProbe());
      } else if (cmd->IsProbeSSBO()) {
        r = engine->DoProbeSSBO(cmd->AsProbeSSBO());
      } else if (cmd->IsBuffer()) {
        r = engine->DoBuffer(cmd->AsBuffer());
      } else if (cmd->IsTolerance()) {
        r = engine->DoTolerance(cmd->AsTolerance());
      } else {
        return Result("Unknown command type");
      }

      if (!r.IsSuccess())
        return r;
    }
  }
  return {};
}

}  // namespace vkscript
}  // namespace amber
