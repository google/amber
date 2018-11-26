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
#include <utility>

#include "amber/result.h"
#include "dawn/dawncpp.h"
#include "src/command.h"

namespace amber {
namespace dawn {

struct RenderPipelineInfo {
  RenderPipelineInfo() {}
  RenderPipelineInfo(::dawn::ShaderModule vert, ::dawn::ShaderModule frag)
      : vertex_shader(vert), fragment_shader(frag) {}

  ::dawn::ShaderModule vertex_shader;
  ::dawn::ShaderModule fragment_shader;
  ClearColorCommand clear_color_value;
  float clear_depth_value = 1.0f;
  uint32_t clear_stencil_value = 0;

  // The framebuffer color render target.  This resides on the GPU.
  ::dawn::Texture fb_texture;
  // The buffer to which we will copy the rendered pixel values, for
  // use on the host.
  ::dawn::Buffer fb_buffer;
  // The number of bytes between each row of texels in framebuffer host-side
  // buffer.
  uint32_t fb_row_stride = 0;
  // The number of data bytes in the framebuffer host-side buffer.
  uint32_t fb_size = 0;
  bool fb_is_mapped = false;

  // TODO(dneto): Record index data
  // TODO(dneto): Record buffer data
};

struct ComputePipelineInfo {
  ComputePipelineInfo() {}
  explicit ComputePipelineInfo(::dawn::ShaderModule comp)
      : compute_shader(comp) {}

  ::dawn::ShaderModule compute_shader;
};

}  // namespace dawn
}  // namespace amber

#endif  // SRC_DAWN_PIPELINE_INFO_H_
