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

#include "src/dawn/engine_dawn.h"

#include "dawn/dawncpp.h"

namespace amber {
namespace dawn {

EngineDawn::EngineDawn() : Engine() {}

EngineDawn::~EngineDawn() = default;

Result EngineDawn::Initialize() {
  if (device_)
    return Result("Dawn:Initialize device_ already exists");

  // The constructor already gives us a default device.  So there is nothing
  // left to do.

  return {};
}

Result EngineDawn::InitializeWithDevice(void* default_device) {
  if (device_)
    return Result("Dawn:InitializeWithDevice device_ already exists");

  auto* dawn_device_ptr = static_cast<::dawn::Device*>(default_device);
  if (dawn_device_ptr == nullptr)
    return Result("Dawn:InitializeWithDevice given a null pointer");

  device_ = *dawn_device_ptr;
  return {};
}

Result EngineDawn::Shutdown() {
  device_ = ::dawn::Device();
  return {};
}

Result EngineDawn::CreatePipeline(PipelineType) {
  return Result("Dawn:CreatePipeline not implemented");
}

Result EngineDawn::AddRequirement(Feature, const Format*) {
  return Result("Dawn:AddRequirement not implemented");
}

Result EngineDawn::SetShader(ShaderType, const std::vector<uint32_t>&) {
  return Result("Dawn:SetShader not implemented");
}

Result EngineDawn::SetBuffer(BufferType,
                             uint8_t,
                             const Format&,
                             const std::vector<Value>&) {
  return Result("Dawn:SetBuffer not implemented");
}

Result EngineDawn::ExecuteClearColor(const ClearColorCommand*) {
  return Result("Dawn:ExecuteClearColor not implemented");
}

Result EngineDawn::ExecuteClearStencil(const ClearStencilCommand*) {
  return Result("Dawn:ExecuteClearStencil not implemented");
}

Result EngineDawn::ExecuteClearDepth(const ClearDepthCommand*) {
  return Result("Dawn:ExecuteClearDepth not implemented");
}

Result EngineDawn::ExecuteClear(const ClearCommand*) {
  return Result("Dawn:ExecuteClear not implemented");
}

Result EngineDawn::ExecuteDrawRect(const DrawRectCommand*) {
  return Result("Dawn:ExecuteDrawRect not implemented");
}

Result EngineDawn::ExecuteDrawArrays(const DrawArraysCommand*) {
  return Result("Dawn:ExecuteDrawArrays not implemented");
}

Result EngineDawn::ExecuteCompute(const ComputeCommand*) {
  return Result("Dawn:ExecuteCompute not implemented");
}

Result EngineDawn::ExecuteEntryPoint(const EntryPointCommand*) {
  return Result("Dawn:ExecuteEntryPoint not implemented");
}

Result EngineDawn::ExecutePatchParameterVertices(
    const PatchParameterVerticesCommand*) {
  return Result("Dawn:ExecutePatch not implemented");
}

Result EngineDawn::ExecuteProbe(const ProbeCommand*) {
  return Result("Dawn:ExecutePatch not implemented");
}

Result EngineDawn::ExecuteProbeSSBO(const ProbeSSBOCommand*) {
  return Result("Dawn:ExecuteProbeSSBO not implemented");
}

Result EngineDawn::ExecuteBuffer(const BufferCommand*) {
  return Result("Dawn:ExecuteBuffer not implemented");
}

Result EngineDawn::ExecuteTolerance(const ToleranceCommand*) {
  return Result("Dawn:ExecuteTolerance not implemented");
}

}  // namespace dawn
}  // namespace amber
