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

namespace amber {

Command::Command(Type type) : command_type_(type) {}

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

DrawRectCommand::DrawRectCommand(PipelineData data)
    : Command(Type::kDrawRect), data_(data) {}

DrawRectCommand::~DrawRectCommand() = default;

DrawArraysCommand::DrawArraysCommand(PipelineData data)
    : Command(Type::kDrawArrays), data_(data) {}

DrawArraysCommand::~DrawArraysCommand() = default;

ComputeCommand::ComputeCommand() : Command(Type::kCompute) {}

ComputeCommand::~ComputeCommand() = default;

Probe::Probe(Type type) : Command(type) {}

Probe::~Probe() = default;

ProbeCommand::ProbeCommand() : Probe(Type::kProbe) {}

ProbeCommand::~ProbeCommand() = default;

BufferCommand::BufferCommand(BufferType type)
    : Command(Type::kBuffer), buffer_type_(type) {}

BufferCommand::~BufferCommand() = default;

ProbeSSBOCommand::ProbeSSBOCommand() : Probe(Type::kProbeSSBO) {}

ProbeSSBOCommand::~ProbeSSBOCommand() = default;

ClearCommand::ClearCommand() : Command(Type::kClear) {}

ClearCommand::~ClearCommand() = default;

ClearColorCommand::ClearColorCommand() : Command(Type::kClearColor) {}

ClearColorCommand::~ClearColorCommand() = default;

ClearDepthCommand::ClearDepthCommand() : Command(Type::kClearDepth) {}

ClearDepthCommand::~ClearDepthCommand() = default;

ClearStencilCommand::ClearStencilCommand() : Command(Type::kClearStencil) {}

ClearStencilCommand::~ClearStencilCommand() = default;

PatchParameterVerticesCommand::PatchParameterVerticesCommand()
    : Command(Type::kPatchParameterVertices) {}

PatchParameterVerticesCommand::~PatchParameterVerticesCommand() = default;

EntryPointCommand::EntryPointCommand() : Command(Type::kEntryPoint) {}

EntryPointCommand::~EntryPointCommand() = default;

}  // namespace amber
