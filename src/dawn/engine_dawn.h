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

#ifndef SRC_DAWN_ENGINE_DAWN_H_
#define SRC_DAWN_ENGINE_DAWN_H_

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "dawn/dawncpp.h"
#include "src/cast_hash.h"
#include "src/command.h"
#include "src/dawn/pipeline_info.h"
#include "src/engine.h"

namespace amber {
namespace dawn {

class EngineDawn : public Engine {
 public:
  EngineDawn();
  ~EngineDawn() override;

  // Engine
  // Initialize with given configuration data.
  Result Initialize(EngineConfig* config,
                    Delegate*,
                    const std::vector<std::string>& features,
                    const std::vector<std::string>& instance_extensions,
                    const std::vector<std::string>& device_extensions) override;

  Result Shutdown() override;
  // Record info for a pipeline.  The Dawn render pipeline will be created
  // later.  Assumes necessary shader modules have been created.  A compute
  // pipeline requires a compute shader.  A graphics pipeline requires a vertex
  // and a fragment shader.
  Result CreatePipeline(Pipeline*) override;

  Result DoClearColor(const ClearColorCommand* cmd) override;
  Result DoClearStencil(const ClearStencilCommand* cmd) override;
  Result DoClearDepth(const ClearDepthCommand* cmd) override;
  Result DoClear(const ClearCommand* cmd) override;
  Result DoDrawRect(const DrawRectCommand* cmd) override;
  Result DoDrawArrays(const DrawArraysCommand* cmd) override;
  Result DoCompute(const ComputeCommand* cmd) override;
  Result DoEntryPoint(const EntryPointCommand* cmd) override;
  Result DoPatchParameterVertices(
      const PatchParameterVerticesCommand* cmd) override;
  Result DoBuffer(const BufferCommand* cmd) override;
  Result GetFrameBuffer(Buffer*, std::vector<Value>* values) override;

 private:
  // Creates a command buffer builder if it doesn't already exist.
  Result CreateCommandBufferBuilderIfNeeded();
  // Destroys the current command buffer builder.
  void DestroyCommandBufferBuilder();
  // Creates Dawn objects necessary for a render pipeline.  This creates
  // a framebuffer texture, a framebuffer buffer, and a command buffer
  // builder.  Returns a result code.
  Result CreateRenderObjectsIfNeeded();
  // If they don't already exist, creates the framebuffer texture for use
  // on the device, the buffer on the host that will eventually hold the
  // resulting pixels for use in checking expectations, and bookkeeping info
  // for that host-side buffer.
  Result CreateFramebufferIfNeeded();
  Result SetShader(ShaderType type, const std::vector<uint32_t>& data);

  ::dawn::Device* device_ = nullptr;  // Borrowed from the engine config.
  ::dawn::Queue queue_;
  ::dawn::CommandBufferBuilder command_buffer_builder_;

  std::unordered_map<ShaderType, ::dawn::ShaderModule, CastHash<ShaderType>>
      module_for_type_;
  // Accumulated data for the current compute pipeline.
  ComputePipelineInfo compute_pipeline_info_;
  // Accumulated data for the current render pipeline.
  RenderPipelineInfo render_pipeline_info_;
};

}  // namespace dawn
}  // namespace amber

#endif  // SRC_DAWN_ENGINE_DAWN_H_
