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

#ifndef SRC_DAWN_PIPELINE_INFO_H_
#define SRC_DAWN_PIPELINE_INFO_H_

#include <cstdint>
#include <memory>
#include <utility>

#include "amber/result.h"
#include "dawn/dawncpp.h"
#include "src/command.h"
#include "src/format.h"

namespace amber {
namespace dawn {

/// Stores information relating to a graphics pipeline in Dawn.
struct RenderPipelineInfo {
  RenderPipelineInfo() {}
  RenderPipelineInfo(::amber::Pipeline* the_pipeline,
                     ::dawn::ShaderModule vert,
                     ::dawn::ShaderModule frag)
      : pipeline(the_pipeline), vertex_shader(vert), fragment_shader(frag) {}

  ::amber::Pipeline* pipeline = nullptr;

  ::dawn::ShaderModule vertex_shader;
  ::dawn::ShaderModule fragment_shader;
  ::dawn::Color clear_color_value = {0.f, 0.f, 0.f, 0.f};
  float clear_depth_value = 1.0f;
  uint32_t clear_stencil_value = 0;

  /// The framebuffer color render target.  This resides on the GPU.
  ::dawn::Texture fb_texture;
  /// The depth and stencil target.  This resides on the GPU.
  ::dawn::Texture depth_stencil_texture;
  /// The buffer to which we will copy the rendered pixel values, for
  /// use on the host.
  ::dawn::Buffer fb_buffer;
  ::dawn::Buffer vertex_buffer;
  ::dawn::Buffer index_buffer;

  ::dawn::BindGroup bind_group;
  ::dawn::BindGroupLayout bind_group_layout;
};

/// Stores information relating to a compute pipeline in Dawn.
struct ComputePipelineInfo {
  ComputePipelineInfo() {}
  ComputePipelineInfo(::amber::Pipeline* the_pipeline,
                      ::dawn::ShaderModule comp)
      : pipeline(the_pipeline), compute_shader(comp) {}

  ::amber::Pipeline* pipeline = nullptr;
  ::dawn::ShaderModule compute_shader;
};

/// Holds either a render or compute pipeline.
struct Pipeline {
  std::unique_ptr<RenderPipelineInfo> render_pipeline;
  std::unique_ptr<ComputePipelineInfo> compute_pipeline;
};

}  // namespace dawn
}  // namespace amber

#endif  // SRC_DAWN_PIPELINE_INFO_H_
