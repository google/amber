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
#include "src/shader_compiler.h"
#include "src/vkscript/script.h"

namespace amber {
namespace vkscript {

Executor::Executor() : amber::Executor() {}

Executor::~Executor() = default;

Result Executor::Execute(Engine* engine,
                         const amber::Script* script,
                         const ShaderMap& shader_map) {
  if (!script->IsVkScript())
    return Result("VkScript Executor called with non-vkscript source");

  engine->SetEngineData(script->GetEngineData());

  // Process Shader nodes
  PipelineType pipeline_type = PipelineType::kGraphics;
  for (const auto& shader : script->GetShaders()) {
    ShaderCompiler sc;

    Result r;
    std::vector<uint32_t> data;
    std::tie(r, data) = sc.Compile(shader.get(), shader_map);
    if (!r.IsSuccess())
      return r;

    r = engine->SetShader(shader->GetType(), data);
    if (!r.IsSuccess())
      return r;

    if (shader->GetType() == ShaderType::kCompute)
      pipeline_type = PipelineType::kCompute;
  }

  // Handle Image and Depth buffers early so they are available when we call
  // the CreatePipeline method.
  for (const auto& buf : script->GetBuffers()) {
    // Image and depth are handled earler. They will be moved to the pipeline
    // object when it exists.
    if (buf->GetBufferType() != BufferType::kColor &&
        buf->GetBufferType() != BufferType::kDepth) {
      continue;
    }

    Result r = engine->SetBuffer(
          buf->GetBufferType(),
          buf->GetLocation(),
          buf->IsFormatBuffer() ? buf->AsFormatBuffer()->GetFormat() : Format(),
          buf->GetData());
    if (!r.IsSuccess())
      return r;
  }

  // TODO(jaebaek): Support multiple pipelines.
  Result r = engine->CreatePipeline(pipeline_type);
  if (!r.IsSuccess())
    return r;

  // Process Buffers
  for (const auto& buf : script->GetBuffers()) {
    // Image and depth are handled earler. They will be moved to the pipeline
    // object when it exists.
    if (buf->GetBufferType() == BufferType::kColor ||
        buf->GetBufferType() == BufferType::kDepth) {
      continue;
    }

    r = engine->SetBuffer(
        buf->GetBufferType(),
        buf->GetLocation(),
        buf->IsFormatBuffer() ? buf->AsFormatBuffer()->GetFormat() : Format(),
        buf->GetData());
    if (!r.IsSuccess())
      return r;
  }

  // Process Commands
  for (const auto& cmd : script->GetCommands()) {
    if (cmd->IsProbe()) {
      ResourceInfo info;

      r = engine->DoProcessCommands();
      if (!r.IsSuccess())
        return r;

      // This must come after processing commands because we require
      // the frambuffer buffer to be mapped into host memory and have
      // a valid host-side pointer.
      r = engine->GetFrameBufferInfo(&info);
      if (!r.IsSuccess())
        return r;
      assert(info.cpu_memory != nullptr);

      r = verifier_.Probe(cmd->AsProbe(), info.image_info.texel_stride,
                          info.image_info.row_stride, info.image_info.width,
                          info.image_info.height, info.cpu_memory);
    } else if (cmd->IsProbeSSBO()) {
      auto probe_ssbo = cmd->AsProbeSSBO();
      ResourceInfo info;
      r = engine->GetDescriptorInfo(probe_ssbo->GetDescriptorSet(),
                                    probe_ssbo->GetBinding(), &info);
      if (!r.IsSuccess())
        return r;

      r = engine->DoProcessCommands();
      if (!r.IsSuccess())
        return r;

      r = verifier_.ProbeSSBO(probe_ssbo, info.size_in_bytes, info.cpu_memory);
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

}  // namespace vkscript
}  // namespace amber
