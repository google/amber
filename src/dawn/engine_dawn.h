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

/// Engine implementation using the Dawn API.
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

  // Record info for a pipeline.  The Dawn render pipeline will be created
  // later.  Assumes necessary shader modules have been created.  A compute
  // pipeline requires a compute shader.  A graphics pipeline requires a vertex
  // and a fragment shader.
  Result CreatePipeline(::amber::Pipeline*) override;

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

 private:
  // Returns the Dawn-specific render pipeline for the given command,
  // if it exists.  Returns nullptr otherwise.
  RenderPipelineInfo* GetRenderPipeline(
      const ::amber::PipelineCommand* command) {
    return pipeline_map_[command->GetPipeline()].render_pipeline.get();
  }
  // Returns the Dawn-specific compute pipeline for the given command,
  // if it exists.  Returns nullptr otherwise.
  ComputePipelineInfo* GetComputePipeline(
      const ::amber::PipelineCommand* command) {
    return pipeline_map_[command->GetPipeline()].compute_pipeline.get();
  }
  // Creates and attaches index, vertex, storage, uniform and depth-stencil
  // buffers. Sets up bindings. Also creates textures and texture views if not
  // created yet. Used in the Graphics pipeline creation.
  Result AttachBuffersAndTextures(RenderPipelineInfo* render_pipeline);
  // Creates and attaches index, vertex, storage, uniform and depth-stencil
  // buffers. Used in the Compute pipeline creation.
  Result AttachBuffers(ComputePipelineInfo* compute_pipeline);
  // Creates and submits a command to copy dawn textures back to amber color
  // attachments.
  Result MapDeviceTextureToHostBuffer(const RenderPipelineInfo& render_pipeline,
                                      const ::dawn::Device& device);
  // Creates and submits a command to copy dawn buffers back to amber buffers
  Result MapDeviceBufferToHostBuffer(
      const ComputePipelineInfo& compute_pipeline,
      const ::dawn::Device& device);

  // Borrowed from the engine config
  ::dawn::Device* device_ = nullptr;
  // Dawn color attachment textures
  std::vector<::dawn::Texture> textures_;
  // Views into Dawn color attachment textures
  std::vector<::dawn::TextureView> texture_views_;
  // Dawn depth/stencil texture
  ::dawn::Texture depth_stencil_texture_;
  // Mapping from the generic engine's Pipeline object to our own Dawn-specific
  // pipelines.
  std::unordered_map<amber::Pipeline*, ::amber::dawn::Pipeline> pipeline_map_;
};

}  // namespace dawn
}  // namespace amber

#endif  // SRC_DAWN_ENGINE_DAWN_H_
