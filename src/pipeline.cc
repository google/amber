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

#include "src/pipeline.h"

#include <algorithm>
#include <set>

#include "src/format_parser.h"
#include "src/make_unique.h"

namespace amber {

Pipeline::ShaderInfo::ShaderInfo(const Shader* shader, ShaderType type)
    : shader_(shader), shader_type_(type), entry_point_("main") {}

Pipeline::ShaderInfo::ShaderInfo(const ShaderInfo&) = default;

Pipeline::ShaderInfo::~ShaderInfo() = default;

const char* Pipeline::kDefaultColorBufferFormat = "B8G8R8A8_UNORM";
const char* Pipeline::kDefaultDepthBufferFormat = "D32_SFLOAT_S8_UINT";
const char* Pipeline::kGeneratedColorBuffer = "framebuffer";
const char* Pipeline::kGeneratedDepthBuffer = "depth_buffer";

Pipeline::Pipeline(PipelineType type) : pipeline_type_(type) {}

Pipeline::~Pipeline() = default;

Result Pipeline::AddShader(const Shader* shader, ShaderType shader_type) {
  if (!shader)
    return Result("shader can not be null when attached to pipeline");

  if (pipeline_type_ == PipelineType::kCompute &&
      shader_type != kShaderTypeCompute) {
    return Result("only compute shaders allowed in a compute pipeline");
  }
  if (pipeline_type_ == PipelineType::kGraphics &&
      shader_type == kShaderTypeCompute) {
    return Result("can not add a compute shader to a graphics pipeline");
  }

  for (const auto& info : shaders_) {
    const auto* is = info.GetShader();
    if (is == shader)
      return Result("can not add duplicate shader to pipeline");
    if (is->GetType() == shader_type)
      return Result("can not add duplicate shader type to pipeline");
  }

  shaders_.emplace_back(shader, shader_type);
  return {};
}

Result Pipeline::SetShaderOptimizations(const Shader* shader,
                                        const std::vector<std::string>& opts) {
  if (!shader)
    return Result("invalid shader specified for optimizations");

  std::set<std::string> seen;
  for (const auto& opt : opts) {
    if (seen.count(opt) != 0)
      return Result("duplicate optimization flag (" + opt + ") set on shader");

    seen.insert(opt);
  }

  for (auto& info : shaders_) {
    const auto* is = info.GetShader();
    if (is == shader) {
      info.SetShaderOptimizations(opts);
      return {};
    }
  }

  return Result("unknown shader specified for optimizations: " +
                shader->GetName());
}

Result Pipeline::SetShaderEntryPoint(const Shader* shader,
                                     const std::string& name) {
  if (!shader)
    return Result("invalid shader specified for entry point");
  if (name.empty())
    return Result("entry point should not be blank");

  for (auto& info : shaders_) {
    if (info.GetShader() == shader) {
      if (info.GetEntryPoint() != "main")
        return Result("multiple entry points given for the same shader");

      info.SetEntryPoint(name);
      return {};
    }
  }

  return Result("unknown shader specified for entry point: " +
                shader->GetName());
}

Result Pipeline::SetShaderType(const Shader* shader, ShaderType type) {
  if (!shader)
    return Result("invalid shader specified for shader type");

  for (auto& info : shaders_) {
    if (info.GetShader() == shader) {
      info.SetShaderType(type);
      return {};
    }
  }

  return Result("unknown shader specified for shader type: " +
                shader->GetName());
}

Result Pipeline::Validate() const {
  size_t fb_size = fb_width_ * fb_height_;
  for (const auto& attachment : color_attachments_) {
    if (attachment.buffer->GetSize() != fb_size) {
      return Result(
          "shared framebuffer must have same size over all PIPELINES");
    }
  }

  if (depth_buffer_.buffer == nullptr)
    return Result("PIPELINE missing depth buffer");
  if (depth_buffer_.buffer->GetSize() != fb_size)
    return Result("shared depth buffer must have same size over all PIPELINES");

  if (pipeline_type_ == PipelineType::kGraphics)
    return ValidateGraphics();
  return ValidateCompute();
}

Result Pipeline::ValidateGraphics() const {
  if (color_attachments_.empty())
    return Result("PIPELINE missing color attachment");
  if (shaders_.empty())
    return Result("graphics pipeline requires vertex and fragment shaders");

  bool found_vertex = false;
  bool found_fragment = false;
  for (const auto& info : shaders_) {
    const auto* is = info.GetShader();
    if (is->GetType() == kShaderTypeVertex)
      found_vertex = true;
    if (is->GetType() == kShaderTypeFragment)
      found_fragment = true;
    if (found_vertex && found_fragment)
      break;
  }

  if (!found_vertex && !found_fragment)
    return Result("graphics pipeline requires vertex and fragment shaders");
  if (!found_vertex)
    return Result("graphics pipeline requires a vertex shader");
  if (!found_fragment)
    return Result("graphics pipeline requires a fragment shader");

  return {};
}

Result Pipeline::ValidateCompute() const {
  if (shaders_.empty())
    return Result("compute pipeline requires a compute shader");

  return {};
}

void Pipeline::UpdateFramebufferSizes() {
  size_t size = fb_width_ * fb_height_;
  if (size == 0)
    return;

  for (auto& attachment : color_attachments_) {
    attachment.buffer->SetSize(size);
    attachment.width = fb_width_;
    attachment.height = fb_height_;
  }

  if (depth_buffer_.buffer) {
    depth_buffer_.buffer->SetSize(size);
    depth_buffer_.width = fb_width_;
    depth_buffer_.height = fb_height_;
  }
}

Result Pipeline::AddColorAttachment(Buffer* buf, uint32_t location) {
  for (const auto& attachment : color_attachments_) {
    if (attachment.location == location)
      return Result("can not bind two color buffers to the same LOCATION");
    if (attachment.buffer == buf)
      return Result("color buffer may only be bound to a PIPELINE once");
  }

  color_attachments_.push_back(BufferInfo{buf});

  auto& info = color_attachments_.back();
  info.location = location;
  info.width = fb_width_;
  info.height = fb_height_;

  buf->SetSize(fb_width_ * fb_height_);
  return {};
}

Result Pipeline::GetLocationForColorAttachment(Buffer* buf, uint32_t* loc) {
  for (const auto& info : color_attachments_) {
    if (info.buffer == buf) {
      *loc = info.location;
      return {};
    }
  }
  return Result("Unable to find requested buffer");
}

Result Pipeline::SetDepthBuffer(Buffer* buf) {
  if (depth_buffer_.buffer != nullptr)
    return Result("can only bind one depth buffer in a PIPELINE");

  depth_buffer_.type = BufferType::kDepth;
  depth_buffer_.buffer = buf;
  depth_buffer_.width = fb_width_;
  depth_buffer_.height = fb_height_;

  buf->SetSize(fb_width_ * fb_height_);

  return {};
}

Result Pipeline::SetIndexBuffer(Buffer* buf) {
  if (index_buffer_ != nullptr)
    return Result("can only bind one INDEX_DATA buffer in a pipeline");

  index_buffer_ = buf;
  return {};
}

Result Pipeline::AddVertexBuffer(Buffer* buf, uint32_t location) {
  for (const auto& vtex : vertex_buffers_) {
    if (vtex.location == location)
      return Result("can not bind two vertex buffers to the same LOCATION");
    if (vtex.buffer == buf)
      return Result("vertex buffer may only be bound to a PIPELINE once");
  }

  vertex_buffers_.push_back(BufferInfo{buf});
  vertex_buffers_.back().location = location;
  vertex_buffers_.back().type = BufferType::kVertex;
  return {};
}

std::unique_ptr<Buffer> Pipeline::GenerateDefaultColorAttachmentBuffer() const {
  FormatParser fp;

  std::unique_ptr<Buffer> buf = MakeUnique<FormatBuffer>(BufferType::kColor);
  buf->SetName(kGeneratedColorBuffer);
  buf->AsFormatBuffer()->SetFormat(fp.Parse(kDefaultColorBufferFormat));
  return buf;
}

std::unique_ptr<Buffer> Pipeline::GenerateDefaultDepthAttachmentBuffer() const {
  FormatParser fp;

  std::unique_ptr<Buffer> buf = MakeUnique<FormatBuffer>(BufferType::kDepth);
  buf->SetName(kGeneratedDepthBuffer);
  buf->AsFormatBuffer()->SetFormat(fp.Parse(kDefaultDepthBufferFormat));
  return buf;
}

}  // namespace amber
