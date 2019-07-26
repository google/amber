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
#include <set>
#include <unordered_map>
#include <utility>
#include <vector>

#include "amber/result.h"
#include "dawn/dawncpp.h"
#include "src/command.h"
#include "src/format.h"

namespace amber {
namespace dawn {

struct hash_pair {
  template <class T1, class T2>
  size_t operator()(const std::pair<T1, T2>& p) const {
    auto hash1 = std::hash<T1>{}(p.first);
    auto hash2 = std::hash<T2>{}(p.second);
    return hash1 ^ hash2;
  }
};

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

  // Depth-stencil target.  This resides on the GPU.
  ::dawn::Texture depth_stencil_texture;
  // Vertex buffers
  std::vector<::dawn::Buffer> vertex_buffers;
  // Index buffer
  ::dawn::Buffer index_buffer;
  // Storage and uniform buffers
  std::vector<::dawn::Buffer> buffers;
  // Binding info
  std::vector<::dawn::BindGroup> bind_groups;
  std::vector<::dawn::BindGroupLayout> bind_group_layouts;

  // Mapping from the <descriptor_set, binding> to dawn buffer index in buffers
  std::unordered_map<std::pair<uint32_t, uint32_t>, uint32_t, hash_pair>
      buffer_map;
  std::set<int> used_descriptor_set;
};

/// Stores information relating to a compute pipeline in Dawn.
struct ComputePipelineInfo {
  ComputePipelineInfo() {}
  ComputePipelineInfo(::amber::Pipeline* the_pipeline,
                      ::dawn::ShaderModule comp)
      : pipeline(the_pipeline), compute_shader(comp) {}

  ::amber::Pipeline* pipeline = nullptr;
  ::dawn::ShaderModule compute_shader;

  // storage and uniform buffers
  std::vector<::dawn::Buffer> buffers;

  std::vector<::dawn::BindGroup> bind_groups;
  std::vector<::dawn::BindGroupLayout> bind_group_layouts;

  // Mapping from the <descriptor_set, binding> to dawn buffer index in buffers
  std::unordered_map<std::pair<uint32_t, uint32_t>, uint32_t, hash_pair>
      buffer_map;
  std::set<int> used_descriptor_set;
};

/// Holds either a render or compute pipeline.
struct Pipeline {
  std::unique_ptr<RenderPipelineInfo> render_pipeline;
  std::unique_ptr<ComputePipelineInfo> compute_pipeline;
};

}  // namespace dawn
}  // namespace amber

#endif  // SRC_DAWN_PIPELINE_INFO_H_
