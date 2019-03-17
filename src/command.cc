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

#include "src/command.h"

#include "src/pipeline.h"

namespace amber {

Command::Command(Type type)
    : command_type_(type) {}

Command::~Command() = default;

ClearCommand* Command::AsClear() {
  return static_cast<ClearCommand*>(this);
}

ClearColorCommand* Command::AsClearColor() {
  return static_cast<ClearColorCommand*>(this);
}

ClearDepthCommand* Command::AsClearDepth() {
  return static_cast<ClearDepthCommand*>(this);
}

ClearStencilCommand* Command::AsClearStencil() {
  return static_cast<ClearStencilCommand*>(this);
}

ComputeCommand* Command::AsCompute() {
  return static_cast<ComputeCommand*>(this);
}

DrawArraysCommand* Command::AsDrawArrays() {
  return static_cast<DrawArraysCommand*>(this);
}

DrawRectCommand* Command::AsDrawRect() {
  return static_cast<DrawRectCommand*>(this);
}

EntryPointCommand* Command::AsEntryPoint() {
  return static_cast<EntryPointCommand*>(this);
}

PatchParameterVerticesCommand* Command::AsPatchParameterVertices() {
  return static_cast<PatchParameterVerticesCommand*>(this);
}

ProbeCommand* Command::AsProbe() {
  return static_cast<ProbeCommand*>(this);
}

BufferCommand* Command::AsBuffer() {
  return static_cast<BufferCommand*>(this);
}

ProbeSSBOCommand* Command::AsProbeSSBO() {
  return static_cast<ProbeSSBOCommand*>(this);
}

PipelineCommand::PipelineCommand(Type type, Pipeline* pipeline)
    : Command(type), pipeline_(pipeline) {}

PipelineCommand::~PipelineCommand() = default;

DrawRectCommand::DrawRectCommand(Pipeline* pipeline, PipelineData data)
    : PipelineCommand(Type::kDrawRect, pipeline), data_(data) {}

DrawRectCommand::~DrawRectCommand() = default;

DrawArraysCommand::DrawArraysCommand(Pipeline* pipeline, PipelineData data)
    : PipelineCommand(Type::kDrawArrays, pipeline), data_(data) {}

DrawArraysCommand::~DrawArraysCommand() = default;

ComputeCommand::ComputeCommand(Pipeline* pipeline)
    : PipelineCommand(Type::kCompute, pipeline) {}

ComputeCommand::~ComputeCommand() = default;

Probe::Probe(Type type, Buffer* buffer) : Command(type), buffer_(buffer) {}

Probe::~Probe() = default;

ProbeCommand::ProbeCommand(Buffer* buffer) : Probe(Type::kProbe, buffer) {}

ProbeCommand::~ProbeCommand() = default;

ProbeSSBOCommand::ProbeSSBOCommand(Buffer* buffer)
    : Probe(Type::kProbeSSBO, buffer) {}

ProbeSSBOCommand::~ProbeSSBOCommand() = default;

BufferCommand::BufferCommand(BufferType type, Pipeline* pipeline)
    : PipelineCommand(Type::kBuffer, pipeline), buffer_type_(type) {}

BufferCommand::~BufferCommand() = default;

ClearCommand::ClearCommand(Pipeline* pipeline)
    : PipelineCommand(Type::kClear, pipeline) {}

ClearCommand::~ClearCommand() = default;

ClearColorCommand::ClearColorCommand(Pipeline* pipeline)
    : PipelineCommand(Type::kClearColor, pipeline) {}

ClearColorCommand::~ClearColorCommand() = default;

ClearDepthCommand::ClearDepthCommand(Pipeline* pipeline)
    : PipelineCommand(Type::kClearDepth, pipeline) {}

ClearDepthCommand::~ClearDepthCommand() = default;

ClearStencilCommand::ClearStencilCommand(Pipeline* pipeline)
    : PipelineCommand(Type::kClearStencil, pipeline) {}

ClearStencilCommand::~ClearStencilCommand() = default;

PatchParameterVerticesCommand::PatchParameterVerticesCommand(Pipeline* pipeline)
    : PipelineCommand(Type::kPatchParameterVertices, pipeline) {}

PatchParameterVerticesCommand::~PatchParameterVerticesCommand() = default;

EntryPointCommand::EntryPointCommand(Pipeline* pipeline)
    : PipelineCommand(Type::kEntryPoint, pipeline) {}

EntryPointCommand::~EntryPointCommand() = default;

}  // namespace amber
