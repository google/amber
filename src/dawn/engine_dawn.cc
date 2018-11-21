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
#include "src/dawn/device_metal.h"

namespace amber {
namespace dawn {

EngineDawn::EngineDawn() : Engine() {}

EngineDawn::~EngineDawn() = default;

Result EngineDawn::Initialize() {
  if (device_)
    return Result("Dawn:Initialize device_ already exists");

#if AMBER_DAWN_METAL
  return CreateMetalDevice(&device_);
#endif  // AMBER_DAWN_METAL

  return Result("Dawn::Initialize: Can't make a device: Unknown backend");
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

Result EngineDawn::SetShader(ShaderType type,
                             const std::vector<uint32_t>& code) {
  ::dawn::ShaderModuleDescriptor descriptor;
  descriptor.nextInChain = nullptr;
  descriptor.code = code.data();
  descriptor.codeSize = uint32_t(code.size());
  if (!device_) {
    return Result("Dawn::SetShader: device is not created");
  }
  auto shader = device_.CreateShaderModule(&descriptor);
  if (!shader) {
    return Result("Dawn::SetShader: failed to create shader");
  }
  if (module_for_type_.count(type)) {
    Result("Dawn::SetShader: module for type already exists");
  }
  module_for_type_[type] = shader;
  return {};
}

Result EngineDawn::SetBuffer(BufferType,
                             uint8_t,
                             const Format&,
                             const std::vector<Value>&) {
  return Result("Dawn:SetBuffer not implemented");
}

Result EngineDawn::DoClearColor(const ClearColorCommand*) {
  return Result("Dawn:DoClearColor not implemented");
}

Result EngineDawn::DoClearStencil(const ClearStencilCommand*) {
  return Result("Dawn:DoClearStencil not implemented");
}

Result EngineDawn::DoClearDepth(const ClearDepthCommand*) {
  return Result("Dawn:DoClearDepth not implemented");
}

Result EngineDawn::DoClear(const ClearCommand*) {
  return Result("Dawn:DoClear not implemented");
}

Result EngineDawn::DoDrawRect(const DrawRectCommand*) {
  return Result("Dawn:DoDrawRect not implemented");
}

Result EngineDawn::DoDrawArrays(const DrawArraysCommand*) {
  return Result("Dawn:DoDrawArrays not implemented");
}

Result EngineDawn::DoCompute(const ComputeCommand*) {
  return Result("Dawn:DoCompute not implemented");
}

Result EngineDawn::DoEntryPoint(const EntryPointCommand*) {
  return Result("Dawn:DoEntryPoint not implemented");
}

Result EngineDawn::DoPatchParameterVertices(
    const PatchParameterVerticesCommand*) {
  return Result("Dawn:DoPatch not implemented");
}

Result EngineDawn::DoProbe(const ProbeCommand*) {
  return Result("Dawn:DoPatch not implemented");
}

Result EngineDawn::DoProbeSSBO(const ProbeSSBOCommand*) {
  return Result("Dawn:DoProbeSSBO not implemented");
}

Result EngineDawn::DoBuffer(const BufferCommand*) {
  return Result("Dawn:DoBuffer not implemented");
}

Result EngineDawn::DoTolerance(const ToleranceCommand*) {
  return Result("Dawn:DoTolerance not implemented");
}

}  // namespace dawn
}  // namespace amber
