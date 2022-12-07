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
#include <cstring>
#include <limits>
#include <set>

#include "src/make_unique.h"
#include "src/type_parser.h"

namespace amber {
namespace {

const char* kDefaultColorBufferFormat = "B8G8R8A8_UNORM";
const char* kDefaultDepthBufferFormat = "D32_SFLOAT_S8_UINT";

// OpenCL coordinates mode is bit 0
const uint32_t kOpenCLNormalizedCoordsBit = 1;
// OpenCL address mode bits are bits 1,2,3.
const uint32_t kOpenCLAddressModeBits = 0xe;
// OpenCL address mode bit values.
const uint32_t kOpenCLAddressModeNone = 0;
const uint32_t kOpenCLAddressModeClampToEdge = 2;
const uint32_t kOpenCLAddressModeClamp = 4;
const uint32_t kOpenCLAddressModeRepeat = 6;
const uint32_t kOpenCLAddressModeMirroredRepeat = 8;
// OpenCL filter mode bits.
const uint32_t kOpenCLFilterModeNearestBit = 0x10;
const uint32_t kOpenCLFilterModeLinearBit = 0x20;

}  // namespace

const char* Pipeline::kGeneratedColorBuffer = "framebuffer";
const char* Pipeline::kGeneratedDepthBuffer = "depth_buffer";
const char* Pipeline::kGeneratedPushConstantBuffer = "push_constant_buffer";

Pipeline::ShaderInfo::ShaderInfo(Shader* shader, ShaderType type)
    : shader_(shader),
      shader_type_(type),
      entry_point_("main"),
      required_subgroup_size_setting_(RequiredSubgroupSizeSetting::kNotSet),
      required_subgroup_size_(0),
      varying_subgroup_size_(false),
      require_full_subgroups_(false) {}

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
  clone->depth_stencil_buffer_ = depth_stencil_buffer_;
  clone->index_buffer_ = index_buffer_;
  clone->fb_width_ = fb_width_;
  clone->fb_height_ = fb_height_;
  clone->set_arg_values_ = set_arg_values_;
  clone->pipeline_data_ = pipeline_data_;

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

Result Pipeline::SetShaderRequiredSubgroupSize(
    const Shader* shader,
    const ShaderInfo::RequiredSubgroupSizeSetting setting,
    const uint32_t size) {
  if (!shader)
    return Result("invalid shader specified for  required subgroup size");

  for (auto& info : shaders_) {
    const auto* is = info.GetShader();
    if (is == shader) {
      info.SetRequiredSubgroupSizeSetting(setting, size);
      return {};
    }
  }

  return Result("unknown shader specified for required subgroup size: " +
                shader->GetName());
}

Result Pipeline::SetShaderRequiredSubgroupSize(const Shader* shader,
                                               const uint32_t subgroupSize) {
  const bool isPow2 =
      subgroupSize > 0 && (subgroupSize & (subgroupSize - 1)) == 0;
  if (subgroupSize == 0 || subgroupSize > 128 || !isPow2) {
    return Result("invalid required subgroup size " +
                  std::to_string(subgroupSize) + " specified for shader name " +
                  shader->GetName());
  }
  const ShaderInfo::RequiredSubgroupSizeSetting setting =
      ShaderInfo::RequiredSubgroupSizeSetting::kSetToSpecificSize;
  return SetShaderRequiredSubgroupSize(shader, setting, subgroupSize);
}

Result Pipeline::SetShaderRequiredSubgroupSizeToMinimum(const Shader* shader) {
  const ShaderInfo::RequiredSubgroupSizeSetting subgroupSizeSetting =
      ShaderInfo::RequiredSubgroupSizeSetting::kSetToMinimumSize;
  return SetShaderRequiredSubgroupSize(shader, subgroupSizeSetting, 0);
}

Result Pipeline::SetShaderRequiredSubgroupSizeToMaximum(const Shader* shader) {
  const ShaderInfo::RequiredSubgroupSizeSetting subgroupSizeSetting =
      ShaderInfo::RequiredSubgroupSizeSetting::kSetToMaximumSize;
  return SetShaderRequiredSubgroupSize(shader, subgroupSizeSetting, 0);
}

Result Pipeline::SetShaderVaryingSubgroupSize(const Shader* shader,
                                              const bool isSet) {
  if (!shader)
    return Result("invalid shader specified for varying subgroup size");

  for (auto& info : shaders_) {
    const auto* is = info.GetShader();
    if (is == shader) {
      info.SetVaryingSubgroupSize(isSet);
      return {};
    }
  }

  return Result("unknown shader specified for varying subgroup size: " +
                shader->GetName());
}

Result Pipeline::SetShaderRequireFullSubgroups(const Shader* shader,
                                               const bool isSet) {
  if (!shader)
    return Result("invalid shader specified for optimizations");

  for (auto& info : shaders_) {
    const auto* is = info.GetShader();
    if (is == shader) {
      info.SetRequireFullSubgroups(isSet);
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
  for (const auto& attachment : color_attachments_) {
    if (attachment.buffer->ElementCount() !=
        (fb_width_ << attachment.base_mip_level) *
            (fb_height_ << attachment.base_mip_level)) {
      return Result(
          "shared framebuffer must have same size over all PIPELINES");
    }
  }

  if (depth_stencil_buffer_.buffer &&
      depth_stencil_buffer_.buffer->ElementCount() != fb_width_ * fb_height_) {
    return Result("shared depth buffer must have same size over all PIPELINES");
  }

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

  for (const auto& att : color_attachments_) {
    auto width = att.buffer->GetWidth();
    auto height = att.buffer->GetHeight();
    for (uint32_t level = 1; level < att.buffer->GetMipLevels(); level++) {
      width >>= 1;
      if (width == 0)
        return Result("color attachment with " +
                      std::to_string(att.buffer->GetMipLevels()) +
                      " mip levels would have zero width for level " +
                      std::to_string(level));
      height >>= 1;
      if (height == 0)
        return Result("color attachment with " +
                      std::to_string(att.buffer->GetMipLevels()) +
                      " mip levels would have zero height for level " +
                      std::to_string(level));
    }
  }

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
    auto mip0_width = fb_width_ << attachment.base_mip_level;
    auto mip0_height = fb_height_ << attachment.base_mip_level;
    attachment.buffer->SetWidth(mip0_width);
    attachment.buffer->SetHeight(mip0_height);
    attachment.buffer->SetElementCount(mip0_width * mip0_height);
  }

  if (depth_stencil_buffer_.buffer) {
    depth_stencil_buffer_.buffer->SetWidth(fb_width_);
    depth_stencil_buffer_.buffer->SetHeight(fb_height_);
    depth_stencil_buffer_.buffer->SetElementCount(size);
  }
}

Result Pipeline::AddColorAttachment(Buffer* buf,
                                    uint32_t location,
                                    uint32_t base_mip_level) {
  for (const auto& attachment : color_attachments_) {
    if (attachment.location == location)
      return Result("can not bind two color buffers to the same LOCATION");
    if (attachment.buffer == buf)
      return Result("color buffer may only be bound to a PIPELINE once");
  }

  color_attachments_.push_back(BufferInfo{buf});

  auto& info = color_attachments_.back();
  info.location = location;
  info.type = BufferType::kColor;
  info.base_mip_level = base_mip_level;
  auto mip0_width = fb_width_ << base_mip_level;
  auto mip0_height = fb_height_ << base_mip_level;
  buf->SetWidth(mip0_width);
  buf->SetHeight(mip0_height);
  buf->SetElementCount(mip0_width * mip0_height);

  return {};
}

Result Pipeline::AddResolveTarget(Buffer* buf) {
  resolve_targets_.push_back(BufferInfo{buf});

  auto& info = resolve_targets_.back();
  info.type = BufferType::kResolve;
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

Result Pipeline::SetDepthStencilBuffer(Buffer* buf) {
  if (depth_stencil_buffer_.buffer != nullptr)
    return Result("can only bind one depth/stencil buffer in a PIPELINE");

  depth_stencil_buffer_.buffer = buf;
  depth_stencil_buffer_.type = BufferType::kDepthStencil;

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

Result Pipeline::AddVertexBuffer(Buffer* buf,
                                 uint32_t location,
                                 InputRate rate,
                                 Format* format,
                                 uint32_t offset,
                                 uint32_t stride) {
  for (const auto& vtex : vertex_buffers_) {
    if (vtex.location == location)
      return Result("can not bind two vertex buffers to the same LOCATION");
  }

  vertex_buffers_.push_back(BufferInfo{buf});
  vertex_buffers_.back().location = location;
  vertex_buffers_.back().type = BufferType::kVertex;
  vertex_buffers_.back().input_rate = rate;
  vertex_buffers_.back().format = format;
  vertex_buffers_.back().offset = offset;
  vertex_buffers_.back().stride = stride;
  return {};
}

Result Pipeline::SetPushConstantBuffer(Buffer* buf) {
  if (push_constant_buffer_.buffer != nullptr)
    return Result("can only bind one push constant buffer in a PIPELINE");

  push_constant_buffer_.buffer = buf;
  push_constant_buffer_.type = BufferType::kPushConstant;
  return {};
}

Result Pipeline::CreatePushConstantBuffer() {
  if (push_constant_buffer_.buffer != nullptr)
    return Result("can only bind one push constant buffer in a PIPELINE");

  TypeParser parser;
  auto type = parser.Parse("R8_UINT");
  auto fmt = MakeUnique<Format>(type.get());

  std::unique_ptr<Buffer> buf = MakeUnique<Buffer>();
  buf->SetName(kGeneratedPushConstantBuffer);
  buf->SetFormat(fmt.get());

  push_constant_buffer_.buffer = buf.get();
  push_constant_buffer_.type = BufferType::kPushConstant;

  formats_.push_back(std::move(fmt));
  types_.push_back(std::move(type));
  opencl_push_constants_ = std::move(buf);

  return {};
}

std::unique_ptr<Buffer> Pipeline::GenerateDefaultColorAttachmentBuffer() {
  TypeParser parser;
  auto type = parser.Parse(kDefaultColorBufferFormat);
  auto fmt = MakeUnique<Format>(type.get());

  std::unique_ptr<Buffer> buf = MakeUnique<Buffer>();
  buf->SetName(kGeneratedColorBuffer);
  buf->SetFormat(fmt.get());

  formats_.push_back(std::move(fmt));
  types_.push_back(std::move(type));
  return buf;
}

std::unique_ptr<Buffer>
Pipeline::GenerateDefaultDepthStencilAttachmentBuffer() {
  TypeParser parser;
  auto type = parser.Parse(kDefaultDepthBufferFormat);
  auto fmt = MakeUnique<Format>(type.get());

  std::unique_ptr<Buffer> buf = MakeUnique<Buffer>();
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
                         BufferType type,
                         uint32_t descriptor_set,
                         uint32_t binding,
                         uint32_t base_mip_level,
                         uint32_t dynamic_offset,
                         uint64_t descriptor_offset,
                         uint64_t descriptor_range) {
  buffers_.push_back(BufferInfo{buf});

  auto& info = buffers_.back();
  info.descriptor_set = descriptor_set;
  info.binding = binding;
  info.type = type;
  info.base_mip_level = base_mip_level;
  info.dynamic_offset = dynamic_offset;
  info.sampler = buf->GetSampler();
  info.descriptor_offset = descriptor_offset;
  info.descriptor_range = descriptor_range;
}

void Pipeline::AddBuffer(Buffer* buf,
                         BufferType type,
                         const std::string& arg_name) {
  // If this buffer binding already exists, overwrite with the new buffer.
  for (auto& info : buffers_) {
    if (info.arg_name == arg_name) {
      info.buffer = buf;
      return;
    }
  }

  buffers_.push_back(BufferInfo{buf});

  auto& info = buffers_.back();
  info.type = type;
  info.arg_name = arg_name;
  info.descriptor_set = std::numeric_limits<uint32_t>::max();
  info.binding = std::numeric_limits<uint32_t>::max();
  info.arg_no = std::numeric_limits<uint32_t>::max();
  info.base_mip_level = 0;
  info.dynamic_offset = 0;
}

void Pipeline::AddBuffer(Buffer* buf, BufferType type, uint32_t arg_no) {
  // If this buffer binding already exists, overwrite with the new buffer.
  for (auto& info : buffers_) {
    if (info.arg_no == arg_no) {
      info.buffer = buf;
      return;
    }
  }

  buffers_.push_back(BufferInfo{buf});

  auto& info = buffers_.back();
  info.type = type;
  info.arg_no = arg_no;
  info.descriptor_set = std::numeric_limits<uint32_t>::max();
  info.binding = std::numeric_limits<uint32_t>::max();
  info.base_mip_level = 0;
  info.dynamic_offset = 0;
}

void Pipeline::ClearBuffers(uint32_t descriptor_set, uint32_t binding) {
  buffers_.erase(
      std::remove_if(buffers_.begin(), buffers_.end(),
                     [descriptor_set, binding](BufferInfo& info) -> bool {
                       return (info.descriptor_set == descriptor_set &&
                               info.binding == binding);
                     }),
      buffers_.end());
}

void Pipeline::AddSampler(Sampler* sampler,
                          uint32_t descriptor_set,
                          uint32_t binding) {
  samplers_.push_back(SamplerInfo{sampler});

  auto& info = samplers_.back();
  info.descriptor_set = descriptor_set;
  info.binding = binding;
  info.mask = std::numeric_limits<uint32_t>::max();
}

void Pipeline::AddSampler(Sampler* sampler, const std::string& arg_name) {
  for (auto& info : samplers_) {
    if (info.arg_name == arg_name) {
      info.sampler = sampler;
      return;
    }
  }

  samplers_.push_back(SamplerInfo{sampler});

  auto& info = samplers_.back();
  info.arg_name = arg_name;
  info.descriptor_set = std::numeric_limits<uint32_t>::max();
  info.binding = std::numeric_limits<uint32_t>::max();
  info.arg_no = std::numeric_limits<uint32_t>::max();
  info.mask = std::numeric_limits<uint32_t>::max();
}

void Pipeline::AddSampler(Sampler* sampler, uint32_t arg_no) {
  for (auto& info : samplers_) {
    if (info.arg_no == arg_no) {
      info.sampler = sampler;
      return;
    }
  }

  samplers_.push_back(SamplerInfo{sampler});

  auto& info = samplers_.back();
  info.arg_no = arg_no;
  info.descriptor_set = std::numeric_limits<uint32_t>::max();
  info.binding = std::numeric_limits<uint32_t>::max();
  info.mask = std::numeric_limits<uint32_t>::max();
}

void Pipeline::AddSampler(uint32_t mask,
                          uint32_t descriptor_set,
                          uint32_t binding) {
  samplers_.push_back(SamplerInfo{nullptr});

  auto& info = samplers_.back();
  info.arg_name = "";
  info.arg_no = std::numeric_limits<uint32_t>::max();
  info.mask = mask;
  info.descriptor_set = descriptor_set;
  info.binding = binding;
}

void Pipeline::ClearSamplers(uint32_t descriptor_set, uint32_t binding) {
  samplers_.erase(
      std::remove_if(samplers_.begin(), samplers_.end(),
                     [descriptor_set, binding](SamplerInfo& info) -> bool {
                       return (info.descriptor_set == descriptor_set &&
                               info.binding == binding);
                     }),
      samplers_.end());
}

Result Pipeline::UpdateOpenCLBufferBindings() {
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

  for (auto& info : samplers_) {
    if (info.descriptor_set == std::numeric_limits<uint32_t>::max() &&
        info.binding == std::numeric_limits<uint32_t>::max()) {
      for (const auto& entry : iter->second) {
        if (entry.arg_name == info.arg_name ||
            entry.arg_ordinal == info.arg_no) {
          if (entry.kind !=
              Pipeline::ShaderInfo::DescriptorMapEntry::Kind::SAMPLER) {
            return Result("Sampler bound to non-sampler kernel arg");
          }
          info.descriptor_set = entry.descriptor_set;
          info.binding = entry.binding;
        }
      }
    }
  }

  for (auto& info : buffers_) {
    if (info.descriptor_set == std::numeric_limits<uint32_t>::max() &&
        info.binding == std::numeric_limits<uint32_t>::max()) {
      for (const auto& entry : iter->second) {
        if (entry.arg_name == info.arg_name ||
            entry.arg_ordinal == info.arg_no) {
          // Buffer storage class consistency checks.
          if (info.type == BufferType::kUnknown) {
            // Set the appropriate buffer type.
            switch (entry.kind) {
              case Pipeline::ShaderInfo::DescriptorMapEntry::Kind::UBO:
              case Pipeline::ShaderInfo::DescriptorMapEntry::Kind::POD_UBO:
                info.type = BufferType::kUniform;
                break;
              case Pipeline::ShaderInfo::DescriptorMapEntry::Kind::SSBO:
              case Pipeline::ShaderInfo::DescriptorMapEntry::Kind::POD:
                info.type = BufferType::kStorage;
                break;
              case Pipeline::ShaderInfo::DescriptorMapEntry::Kind::RO_IMAGE:
                info.type = BufferType::kSampledImage;
                break;
              case Pipeline::ShaderInfo::DescriptorMapEntry::Kind::WO_IMAGE:
                info.type = BufferType::kStorageImage;
                break;
              default:
                return Result("Unhandled buffer type for OPENCL-C shader");
            }
          } else if (info.type == BufferType::kUniform) {
            if (entry.kind !=
                    Pipeline::ShaderInfo::DescriptorMapEntry::Kind::UBO &&
                entry.kind !=
                    Pipeline::ShaderInfo::DescriptorMapEntry::Kind::POD_UBO) {
              return Result("Buffer " + info.buffer->GetName() +
                            " must be a uniform binding");
            }
          } else if (info.type == BufferType::kStorage) {
            if (entry.kind !=
                    Pipeline::ShaderInfo::DescriptorMapEntry::Kind::SSBO &&
                entry.kind !=
                    Pipeline::ShaderInfo::DescriptorMapEntry::Kind::POD) {
              return Result("Buffer " + info.buffer->GetName() +
                            " must be a storage binding");
            }
          } else if (info.type == BufferType::kSampledImage) {
            if (entry.kind !=
                Pipeline::ShaderInfo::DescriptorMapEntry::Kind::RO_IMAGE) {
              return Result("Buffer " + info.buffer->GetName() +
                            " must be a read-only image binding");
            }
          } else if (info.type == BufferType::kStorageImage) {
            if (entry.kind !=
                Pipeline::ShaderInfo::DescriptorMapEntry::Kind::WO_IMAGE) {
              return Result("Buffer " + info.buffer->GetName() +
                            " must be a write-only image binding");
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
              Pipeline::ShaderInfo::DescriptorMapEntry::Kind::POD_UBO &&
          entry.kind != Pipeline::ShaderInfo::DescriptorMapEntry::Kind::
                            POD_PUSHCONSTANT) {
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

    Buffer* buffer = nullptr;
    if (kind ==
        Pipeline::ShaderInfo::DescriptorMapEntry::Kind::POD_PUSHCONSTANT) {
      if (GetPushConstantBuffer().buffer == nullptr) {
        auto r = CreatePushConstantBuffer();
        if (!r.IsSuccess())
          return r;
      }
      buffer = GetPushConstantBuffer().buffer;
    } else {
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
        auto buffer_type =
            kind == Pipeline::ShaderInfo::DescriptorMapEntry::Kind::POD
                ? BufferType::kStorage
                : BufferType::kUniform;

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
        AddBuffer(buffer, buffer_type, descriptor_set, binding, 0, 0, 0, ~0ULL);
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
    }

    // Convert the argument value into bytes. Currently, only scalar arguments
    // are supported.
    const auto arg_byte_size = arg_info.fmt->SizeInBytes();
    std::vector<Value> data_bytes;
    for (uint32_t i = 0; i < arg_byte_size; ++i) {
      Value v;
      if (arg_info.value.IsFloat()) {
        if (arg_byte_size == sizeof(double)) {
          union {
            uint64_t u;
            double d;
          } u;
          u.d = arg_info.value.AsDouble();
          v.SetIntValue((u.u >> (i * 8)) & 0xff);
        } else {
          union {
            uint32_t u;
            float f;
          } u;
          u.f = arg_info.value.AsFloat();
          v.SetIntValue((u.u >> (i * 8)) & 0xff);
        }
      } else {
        v.SetIntValue((arg_info.value.AsUint64() >> (i * 8)) & 0xff);
      }
      data_bytes.push_back(v);
    }
    Result r = buffer->SetDataWithOffset(data_bytes, offset);
    if (!r.IsSuccess())
      return r;
  }

  return {};
}

Result Pipeline::GenerateOpenCLLiteralSamplers() {
  for (auto& info : samplers_) {
    if (info.sampler || info.mask == std::numeric_limits<uint32_t>::max())
      continue;

    auto literal_sampler = MakeUnique<Sampler>();
    literal_sampler->SetName("literal." + std::to_string(info.descriptor_set) +
                             "." + std::to_string(info.binding));

    // The values for addressing modes, filtering modes and coordinate
    // normalization are all defined in the OpenCL header.

    literal_sampler->SetNormalizedCoords(info.mask &
                                         kOpenCLNormalizedCoordsBit);

    uint32_t addressing_bits = info.mask & kOpenCLAddressModeBits;
    AddressMode addressing_mode = AddressMode::kUnknown;
    if (addressing_bits == kOpenCLAddressModeNone ||
        addressing_bits == kOpenCLAddressModeClampToEdge) {
      // CLK_ADDRESS_NONE
      // CLK_ADDERSS_CLAMP_TO_EDGE
      addressing_mode = AddressMode::kClampToEdge;
    } else if (addressing_bits == kOpenCLAddressModeClamp) {
      // CLK_ADDRESS_CLAMP
      addressing_mode = AddressMode::kClampToBorder;
    } else if (addressing_bits == kOpenCLAddressModeRepeat) {
      // CLK_ADDRESS_REPEAT
      addressing_mode = AddressMode::kRepeat;
    } else if (addressing_bits == kOpenCLAddressModeMirroredRepeat) {
      // CLK_ADDRESS_MIRRORED_REPEAT
      addressing_mode = AddressMode::kMirroredRepeat;
    }
    literal_sampler->SetAddressModeU(addressing_mode);
    literal_sampler->SetAddressModeV(addressing_mode);
    // TODO(alan-baker): If this is used with an arrayed image then W should use
    // kClampToEdge always, but this information is not currently available.
    literal_sampler->SetAddressModeW(addressing_mode);

    // Next bit is filtering mode.
    FilterType filtering_mode = FilterType::kUnknown;
    if (info.mask & kOpenCLFilterModeNearestBit) {
      filtering_mode = FilterType::kNearest;
    } else if (info.mask & kOpenCLFilterModeLinearBit) {
      filtering_mode = FilterType::kLinear;
    }
    literal_sampler->SetMagFilter(filtering_mode);
    literal_sampler->SetMinFilter(filtering_mode);

    // TODO(alan-baker): OpenCL wants the border color to be based on image
    // channel orders which aren't accessible.

    // clspv never generates multiple MIPMAP levels.
    literal_sampler->SetMinLOD(0.0f);
    literal_sampler->SetMaxLOD(0.0f);

    opencl_literal_samplers_.push_back(std::move(literal_sampler));
    info.sampler = opencl_literal_samplers_.back().get();
  }

  return {};
}

Result Pipeline::GenerateOpenCLPushConstants() {
  if (!IsCompute() || GetShaders().empty() ||
      GetShaders()[0].GetShader()->GetFormat() != kShaderFormatOpenCLC) {
    return {};
  }

  const auto& shader_info = GetShaders()[0];
  if (shader_info.GetPushConstants().empty())
    return {};

  Result r = CreatePushConstantBuffer();
  if (!r.IsSuccess())
    return r;

  auto* buf = GetPushConstantBuffer().buffer;
  assert(buf);

  // Determine size and contents of the push constant buffer.
  for (const auto& pc : shader_info.GetPushConstants()) {
    assert(pc.size % sizeof(uint32_t) == 0);
    assert(pc.offset % sizeof(uint32_t) == 0);

    if (buf->GetSizeInBytes() < pc.offset + pc.size)
      buf->SetSizeInBytes(pc.offset + pc.size);

    std::vector<uint32_t> bytes(pc.size / sizeof(uint32_t));
    uint32_t base = 0;
    switch (pc.type) {
      case Pipeline::ShaderInfo::PushConstant::PushConstantType::kDimensions:
        // All compute kernel launches are 3D.
        bytes[base] = 3;
        break;
      case Pipeline::ShaderInfo::PushConstant::PushConstantType::kGlobalOffset:
        // Global offsets are not currently supported.
        bytes[base] = 0;
        bytes[base + 1] = 0;
        bytes[base + 2] = 0;
        break;
      case Pipeline::ShaderInfo::PushConstant::PushConstantType::kRegionOffset:
        // Region offsets are not currently supported.
        bytes[base] = 0;
        bytes[base + 1] = 0;
        bytes[base + 2] = 0;
        break;
    }
    memcpy(buf->ValuePtr()->data() + pc.offset, bytes.data(),
           bytes.size() * sizeof(uint32_t));
  }

  return {};
}

}  // namespace amber
