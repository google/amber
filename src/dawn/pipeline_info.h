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

#include "amber/result.h"
#include "dawn/dawncpp.h"
#include "src/command.h"

namespace amber {
namespace dawn {

class RenderPipelineInfo {
 public:
  RenderPipelineInfo() {}
  RenderPipelineInfo(::dawn::ShaderModule vert, ::dawn::ShaderModule frag)
      : vertex_shader_(vert), fragment_shader_(frag) {}

  // Returns true if this render pipeline is configured at all.
  bool IsConfigured() const { return vertex_shader_ && fragment_shader_; }

  void SetClearColorValue(const ClearColorCommand& value) {
    clear_color_value_ = value;
  }
  const ClearColorCommand& GetClearColorValue() const {
    return clear_color_value_;
  }

  void SetClearDepthValue(float depth) { clear_depth_value_ = depth; }
  float GetClearDepthValue() const { return clear_depth_value_; }

  void SetClearStencilValue(float depth) { clear_stencil_value_ = depth; }
  uint32_t GetClearStencilValue() const { return clear_stencil_value_; }

 private:
  ::dawn::ShaderModule vertex_shader_;
  ::dawn::ShaderModule fragment_shader_;
  ClearColorCommand clear_color_value_;
  float clear_depth_value_ = 1.0f;
  uint32_t clear_stencil_value_ = 0;

  // TODO(dneto): Record index data
  // TODO(dneto): Record buffer data
};

class ComputePipelineInfo {
 public:
  ComputePipelineInfo() {}
  explicit ComputePipelineInfo(::dawn::ShaderModule comp)
      : compute_shader_(comp) {}

  // Returns true if this render pipeline is configured at all.
  bool IsConfigured() const { return bool(compute_shader_); }

 private:
  ::dawn::ShaderModule compute_shader_;
};

}  // namespace dawn
}  // namespace amber

#endif  // SRC_DAWN_PIPELINE_INFO_H_
