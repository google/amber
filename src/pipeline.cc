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
#include <limits>
#include <set>

#include "src/make_unique.h"
#include "src/type_parser.h"

namespace amber {
namespace {

const char* kDefaultColorBufferFormat = "B8G8R8A8_UNORM";
const char* kDefaultDepthBufferFormat = "D32_SFLOAT_S8_UINT";

}  // namespace

const char* Pipeline::kGeneratedColorBuffer = "framebuffer";
const char* Pipeline::kGeneratedDepthBuffer = "depth_buffer";

Pipeline::ShaderInfo::ShaderInfo(Shader* shader, ShaderType type)
    : shader_(shader), shader_type_(type), entry_point_("main") {}

Pipeline::ShaderInfo::ShaderInfo(const ShaderInfo&) = default;

Pipeline::ShaderInfo::~ShaderInfo() = default;

Pipeline::Pipeline(PipelineType type) : pipeline_type_(type) {}

Pipeline::~Pipeline() = default;

std::unique_ptr<Pipeline> Pipeline::Clone() const {
  auto clone = MakeUnique<Pipeline>(pipeline_type_);
  clone->shaders_ = shaders_;
  clone->color_attachments_ = color_attachments_;
  clone->vertex_buffers_ = vertex_buffers_;
  clone->buffers_ = buffers_;
  clone->depth_buffer_ = depth_buffer_;
  clone->index_buffer_ = index_buffer_;
  clone->fb_width_ = fb_width_;
  clone->fb_height_ = fb_height_;
  clone->set_arg_values_ = set_arg_values_;

  if (!opencl_pod_buffers_.empty()) {
    // Generate specific buffers for the clone.
    clone->GenerateOpenCLPodBuffers();
  }

  return clone;
}

Result Pipeline::AddShader(Shader* shader, ShaderType shader_type) {
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

  for (auto& info : shaders_) {
    const auto* is = info.GetShader();
    if (is == shader)
      return Result("can not add duplicate shader to pipeline");
    if (is->GetType() == shader_type) {
      info.SetShader(shader);
      return {};
    }
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

Result Pipeline::SetShaderCompileOptions(const Shader* shader,
                                         const std::vector<std::string>& opts) {
  if (!shader)
    return Result("invalid shader specified for compile options");

  for (auto& info : shaders_) {
    const auto* is = info.GetShader();
    if (is == shader) {
      info.SetCompileOptions(opts);
      return {};
    }
  }

  return Result("unknown shader specified for compile options: " +
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
    if (attachment.buffer->ElementCount() != fb_size) {
      return Result(
          "shared framebuffer must have same size over all PIPELINES");
    }
  }

  if (depth_buffer_.buffer && depth_buffer_.buffer->ElementCount() != fb_size)
    return Result("shared depth buffer must have same size over all PIPELINES");

  for (auto& buf : GetBuffers()) {
    if (buf.buffer->GetFormat() == nullptr) {
      return Result("buffer (" + std::to_string(buf.descriptor_set) + ":" +
                    std::to_string(buf.binding) + ") requires a format");
    }
  }

  if (pipeline_type_ == PipelineType::kGraphics)
    return ValidateGraphics();
  return ValidateCompute();
}

Result Pipeline::ValidateGraphics() const {
  if (color_attachments_.empty())
    return Result("PIPELINE missing color attachment");

  bool found_vertex = false;
  for (const auto& info : shaders_) {
    const auto* s = info.GetShader();
    if (s->GetType() == kShaderTypeVertex) {
      found_vertex = true;
      break;
    }
  }

  if (!found_vertex)
    return Result("graphics pipeline requires a vertex shader");
  return {};
}

Result Pipeline::ValidateCompute() const {
  if (shaders_.empty())
    return Result("compute pipeline requires a compute shader");

  return {};
}

void Pipeline::UpdateFramebufferSizes() {
  uint32_t size = fb_width_ * fb_height_;
  if (size == 0)
    return;

  for (auto& attachment : color_attachments_) {
    attachment.buffer->SetWidth(fb_width_);
    attachment.buffer->SetHeight(fb_height_);
    attachment.buffer->SetElementCount(size);
  }

  if (depth_buffer_.buffer) {
    depth_buffer_.buffer->SetWidth(fb_width_);
    depth_buffer_.buffer->SetHeight(fb_height_);
    depth_buffer_.buffer->SetElementCount(size);
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
  buf->SetWidth(fb_width_);
  buf->SetHeight(fb_height_);
  buf->SetElementCount(fb_width_ * fb_height_);
  return {};
}

Result Pipeline::GetLocationForColorAttachment(Buffer* buf,
                                               uint32_t* loc) const {
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
  if (buf->GetBufferType() != BufferType::kDepth)
    return Result("expected a depth buffer");

  depth_buffer_.buffer = buf;
  buf->SetWidth(fb_width_);
  buf->SetHeight(fb_height_);
  buf->SetElementCount(fb_width_ * fb_height_);
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
  if (buf->GetBufferType() != BufferType::kVertex)
    return Result("expected a vertex buffer");

  vertex_buffers_.push_back(BufferInfo{buf});
  vertex_buffers_.back().location = location;
  return {};
}

Result Pipeline::SetPushConstantBuffer(Buffer* buf) {
  if (push_constant_buffer_.buffer != nullptr)
    return Result("can only bind one push constant buffer in a PIPELINE");
  if (buf->GetBufferType() != BufferType::kPushConstant)
    return Result("expected a push constant buffer");

  push_constant_buffer_.buffer = buf;
  return {};
}

std::unique_ptr<Buffer> Pipeline::GenerateDefaultColorAttachmentBuffer() {
  TypeParser parser;
  auto type = parser.Parse(kDefaultColorBufferFormat);
  auto fmt = MakeUnique<Format>(type.get());

  std::unique_ptr<Buffer> buf = MakeUnique<Buffer>(BufferType::kColor);
  buf->SetName(kGeneratedColorBuffer);
  buf->SetFormat(fmt.get());

  formats_.push_back(std::move(fmt));
  types_.push_back(std::move(type));
  return buf;
}

std::unique_ptr<Buffer> Pipeline::GenerateDefaultDepthAttachmentBuffer() {
  TypeParser parser;
  auto type = parser.Parse(kDefaultDepthBufferFormat);
  auto fmt = MakeUnique<Format>(type.get());

  std::unique_ptr<Buffer> buf = MakeUnique<Buffer>(BufferType::kDepth);
  buf->SetName(kGeneratedDepthBuffer);
  buf->SetFormat(fmt.get());

  formats_.push_back(std::move(fmt));
  types_.push_back(std::move(type));
  return buf;
}

Buffer* Pipeline::GetBufferForBinding(uint32_t descriptor_set,
                                      uint32_t binding) const {
  for (const auto& info : buffers_) {
    if (info.descriptor_set == descriptor_set && info.binding == binding)
      return info.buffer;
  }
  return nullptr;
}

void Pipeline::AddBuffer(Buffer* buf,
                         uint32_t descriptor_set,
                         uint32_t binding) {
  // If this buffer binding already exists, overwrite with the new buffer.
  for (auto& info : buffers_) {
    if (info.descriptor_set == descriptor_set && info.binding == binding) {
      info.buffer = buf;
      return;
    }
  }

  buffers_.push_back(BufferInfo{buf});

  auto& info = buffers_.back();
  info.descriptor_set = descriptor_set;
  info.binding = binding;
}

void Pipeline::AddBuffer(Buffer* buf, const std::string& arg_name) {
  // If this buffer binding already exists, overwrite with the new buffer.
  for (auto& info : buffers_) {
    if (info.arg_name == arg_name) {
      info.buffer = buf;
      return;
    }
  }

  buffers_.push_back(BufferInfo{buf});

  auto& info = buffers_.back();
  info.arg_name = arg_name;
  info.descriptor_set = std::numeric_limits<uint32_t>::max();
  info.binding = std::numeric_limits<uint32_t>::max();
  info.arg_no = std::numeric_limits<uint32_t>::max();
}

void Pipeline::AddBuffer(Buffer* buf, uint32_t arg_no) {
  // If this buffer binding already exists, overwrite with the new buffer.
  for (auto& info : buffers_) {
    if (info.arg_no == arg_no) {
      info.buffer = buf;
      return;
    }
  }

  buffers_.push_back(BufferInfo{buf});

  auto& info = buffers_.back();
  info.arg_no = arg_no;
  info.descriptor_set = std::numeric_limits<uint32_t>::max();
  info.binding = std::numeric_limits<uint32_t>::max();
}

Result Pipeline::UpdateOpenCLBufferBindings() {
  if (!IsCompute() || GetShaders().empty() ||
      GetShaders()[0].GetShader()->GetFormat() != kShaderFormatOpenCLC)
    return {};

  const auto& shader_info = GetShaders()[0];
  const auto& descriptor_map = shader_info.GetDescriptorMap();
  if (descriptor_map.empty())
    return {};

  const auto iter = descriptor_map.find(shader_info.GetEntryPoint());
  if (iter == descriptor_map.end())
    return {};

  for (auto& info : buffers_) {
    if (info.descriptor_set == std::numeric_limits<uint32_t>::max() &&
        info.binding == std::numeric_limits<uint32_t>::max()) {
      for (const auto& entry : iter->second) {
        if (entry.arg_name == info.arg_name ||
            entry.arg_ordinal == info.arg_no) {
          // Buffer storage class consistency checks.
          if (info.buffer->GetBufferType() == BufferType::kUnknown) {
            // Set the appropriate buffer type.
            switch (entry.kind) {
              case Pipeline::ShaderInfo::DescriptorMapEntry::Kind::UBO:
              case Pipeline::ShaderInfo::DescriptorMapEntry::Kind::POD_UBO:
                info.buffer->SetBufferType(BufferType::kUniform);
                break;
              case Pipeline::ShaderInfo::DescriptorMapEntry::Kind::SSBO:
              case Pipeline::ShaderInfo::DescriptorMapEntry::Kind::POD:
                info.buffer->SetBufferType(BufferType::kStorage);
                break;
              default:
                return Result("Unhandled buffer type for OPENCL-C shader");
            }
          } else if (info.buffer->GetBufferType() == BufferType::kUniform) {
            if (entry.kind !=
                    Pipeline::ShaderInfo::DescriptorMapEntry::Kind::UBO &&
                entry.kind !=
                    Pipeline::ShaderInfo::DescriptorMapEntry::Kind::POD_UBO) {
              return Result("Buffer " + info.buffer->GetName() +
                            " must be an uniform binding");
            }
          } else if (info.buffer->GetBufferType() == BufferType::kStorage) {
            if (entry.kind !=
                    Pipeline::ShaderInfo::DescriptorMapEntry::Kind::SSBO &&
                entry.kind !=
                    Pipeline::ShaderInfo::DescriptorMapEntry::Kind::POD) {
              return Result("Buffer " + info.buffer->GetName() +
                            " must be a storage binding");
            }
          } else {
            return Result("Unhandled buffer type for OPENCL-C shader");
          }
          info.descriptor_set = entry.descriptor_set;
          info.binding = entry.binding;
        }
      }
    }
  }

  return {};
}

Result Pipeline::GenerateOpenCLPodBuffers() {
  if (!IsCompute() || GetShaders().empty() ||
      GetShaders()[0].GetShader()->GetFormat() != kShaderFormatOpenCLC) {
    return {};
  }

  const auto& shader_info = GetShaders()[0];
  const auto& descriptor_map = shader_info.GetDescriptorMap();
  if (descriptor_map.empty())
    return {};

  const auto iter = descriptor_map.find(shader_info.GetEntryPoint());
  if (iter == descriptor_map.end())
    return {};

  // For each SET command, do the following:
  // 1. Find the descriptor map entry for that argument.
  // 2. Find or create the buffer for the descriptor set and binding pair.
  // 3. Write the data for the SET command at the right offset.
  for (const auto& arg_info : SetArgValues()) {
    uint32_t descriptor_set = std::numeric_limits<uint32_t>::max();
    uint32_t binding = std::numeric_limits<uint32_t>::max();
    uint32_t offset = 0;
    uint32_t arg_size = 0;
    bool uses_name = !arg_info.name.empty();
    Pipeline::ShaderInfo::DescriptorMapEntry::Kind kind =
        Pipeline::ShaderInfo::DescriptorMapEntry::Kind::POD;
    for (const auto& entry : iter->second) {
      if (entry.kind != Pipeline::ShaderInfo::DescriptorMapEntry::Kind::POD &&
          entry.kind !=
              Pipeline::ShaderInfo::DescriptorMapEntry::Kind::POD_UBO) {
        continue;
      }

      // Found the right entry.
      if ((uses_name && entry.arg_name == arg_info.name) ||
          entry.arg_ordinal == arg_info.ordinal) {
        descriptor_set = entry.descriptor_set;
        binding = entry.binding;
        offset = entry.pod_offset;
        arg_size = entry.pod_arg_size;
        kind = entry.kind;
        break;
      }
    }

    if (descriptor_set == std::numeric_limits<uint32_t>::max() ||
        binding == std::numeric_limits<uint32_t>::max()) {
      std::string message =
          "could not find descriptor map entry for SET command: kernel " +
          shader_info.GetEntryPoint();
      if (uses_name) {
        message += ", name " + arg_info.name;
      } else {
        message += ", number " + std::to_string(arg_info.ordinal);
      }
      return Result(message);
    }

    auto buf_iter = opencl_pod_buffer_map_.lower_bound(
        std::make_pair(descriptor_set, binding));
    Buffer* buffer = nullptr;
    if (buf_iter == opencl_pod_buffer_map_.end() ||
        buf_iter->first.first != descriptor_set ||
        buf_iter->first.second != binding) {
      // Ensure no buffer was previously bound for this descriptor set and
      // binding pair.
      for (const auto& buf_info : GetBuffers()) {
        if (buf_info.descriptor_set == descriptor_set &&
            buf_info.binding == binding) {
          return Result("previously bound buffer " +
                        buf_info.buffer->GetName() +
                        " to PoD args at descriptor set " +
                        std::to_string(descriptor_set) + " binding " +
                        std::to_string(binding));
        }
      }

      // Add a new buffer for this descriptor set and binding.
      opencl_pod_buffers_.push_back(MakeUnique<Buffer>());
      buffer = opencl_pod_buffers_.back().get();
      buffer->SetBufferType(
          kind == Pipeline::ShaderInfo::DescriptorMapEntry::Kind::POD
              ? BufferType::kStorage
              : BufferType::kUniform);

      // Use an 8-bit type because all the data in the descriptor map is
      // byte-based and it simplifies the logic for sizing below.
      TypeParser parser;
      auto type = parser.Parse("R8_UINT");
      auto fmt = MakeUnique<Format>(type.get());
      buffer->SetFormat(fmt.get());
      formats_.push_back(std::move(fmt));
      types_.push_back(std::move(type));

      buffer->SetName(GetName() + "_pod_buffer_" +
                      std::to_string(descriptor_set) + "_" +
                      std::to_string(binding));
      opencl_pod_buffer_map_.insert(
          buf_iter,
          std::make_pair(std::make_pair(descriptor_set, binding), buffer));
      AddBuffer(buffer, descriptor_set, binding);
    } else {
      buffer = buf_iter->second;
    }

    // Resize if necessary.
    if (buffer->ValueCount() < offset + arg_size) {
      buffer->SetSizeInElements(offset + arg_size);
    }

    // Check the data size.
    if (arg_size != arg_info.fmt->SizeInBytes()) {
      std::string message = "SET command uses incorrect data size: kernel " +
                            shader_info.GetEntryPoint();
      if (uses_name) {
        message += ", name " + arg_info.name;
      } else {
        message += ", number " + std::to_string(arg_info.ordinal);
      }
      return Result(message);
    }

    Result r = buffer->SetDataWithOffset({arg_info.value}, offset);
    if (!r.IsSuccess())
      return r;
  }

  return {};
}

}  // namespace amber
