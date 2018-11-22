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

#ifndef SRC_ENGINE_H_
#define SRC_ENGINE_H_

#include <memory>
#include <vector>

#include "amber/amber.h"
#include "amber/result.h"
#include "src/buffer_data.h"
#include "src/command.h"
#include "src/feature.h"
#include "src/format.h"
#include "src/shader_data.h"

namespace amber {

enum class PipelineType : uint8_t {
  kCompute = 0,
  kGraphics,
};

class Engine {
 public:
  static std::unique_ptr<Engine> Create(EngineType type);

  virtual ~Engine();

  // Initialize the engine.
  virtual Result Initialize() = 0;

  // Initialize the engine with the provided device. The device is _not_ owned
  // by the engine and should not be destroyed.
  virtual Result InitializeWithDevice(void* default_device) = 0;

  // Shutdown the engine and cleanup any resources.
  virtual Result Shutdown() = 0;

  // Enable |feature|. If the feature requires a pixel format it will be
  // provided in |format|, otherwise |format| is a nullptr. If the feature
  // requires a uint32 value it will be set in the |uint32_t|.
  virtual Result AddRequirement(Feature feature,
                                const Format* format,
                                uint32_t) = 0;

  // Create graphics pipeline.
  virtual Result CreatePipeline(PipelineType type) = 0;

  // Set the shader of |type| to the binary |data|.
  virtual Result SetShader(ShaderType type,
                           const std::vector<uint32_t>& data) = 0;

  // Provides the data for a given buffer to be bound at the given location.
  virtual Result SetBuffer(BufferType type,
                           uint8_t location,
                           const Format& format,
                           const std::vector<Value>& data) = 0;

  // Execute the clear color command
  virtual Result DoClearColor(const ClearColorCommand* cmd) = 0;

  // Execute the clear stencil command
  virtual Result DoClearStencil(const ClearStencilCommand* cmd) = 0;

  // Execute the clear depth command
  virtual Result DoClearDepth(const ClearDepthCommand* cmd) = 0;

  // Execute the clear command
  virtual Result DoClear(const ClearCommand* cmd) = 0;

  // Execute the draw rect command
  virtual Result DoDrawRect(const DrawRectCommand* cmd) = 0;

  // Execute the draw arrays command
  virtual Result DoDrawArrays(const DrawArraysCommand* cmd) = 0;

  // Execute the compute command
  virtual Result DoCompute(const ComputeCommand* cmd) = 0;

  // Execute the entry point command
  virtual Result DoEntryPoint(const EntryPointCommand* cmd) = 0;

  // Execute the patch command
  virtual Result DoPatchParameterVertices(
      const PatchParameterVerticesCommand* cmd) = 0;

  // Execute the probe command
  virtual Result DoProbe(const ProbeCommand* cmd) = 0;

  // Execute the probe ssbo command
  virtual Result DoProbeSSBO(const ProbeSSBOCommand* cmd) = 0;

  // Execute the buffer command
  virtual Result DoBuffer(const BufferCommand* cmd) = 0;

  // Execute the tolerance command
  virtual Result DoTolerance(const ToleranceCommand* cmd) = 0;

 protected:
  Engine();
};

}  // namespace amber

#endif  // SRC_ENGINE_H_
