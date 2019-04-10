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

#include "src/dawn/engine_dawn.h"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "amber/amber_dawn.h"
#include "dawn/dawncpp.h"
#include "src/dawn/pipeline_info.h"
#include "src/format.h"
#include "src/sleep.h"

namespace amber {
namespace dawn {

namespace {

// The minimum multiple row pitch observed on Dawn on Metal.  Increase this
// as needed for other Dawn backends.
const uint32_t kMinimumImageRowPitch = 256;

// Creates a device-side texture for the framebuffer, and returns it through
// |result_ptr|.  Assumes the device exists and is valid.  Assumes result_ptr
// is not null.  Returns a result code.
Result MakeFramebufferTexture(const ::dawn::Device& device,
                              ::dawn::TextureFormat format,
                              uint32_t width,
                              uint32_t height,
                              ::dawn::Texture* result_ptr) {
  assert(device);
  assert(result_ptr);
  ::dawn::TextureDescriptor descriptor;
  descriptor.dimension = ::dawn::TextureDimension::e2D;
  descriptor.size.width = width;
  descriptor.size.height = height;
  descriptor.size.depth = 1;
  descriptor.arrayLayerCount = 1;
  descriptor.format = format;
  descriptor.mipLevelCount = 1;
  descriptor.sampleCount = 1;
  descriptor.usage = ::dawn::TextureUsageBit::TransferSrc |
                     ::dawn::TextureUsageBit::OutputAttachment;
  // TODO(dneto): Get a better message by using the Dawn error callback.
  *result_ptr = device.CreateTexture(&descriptor);
  if (*result_ptr)
    return {};
  return Result("Dawn: Failed to allocate a framebuffer texture");
}

// Creates a host-side buffer for the framebuffer, and returns it through
// |result_ptr|. The buffer will be used as a transfer destination and
// for mapping-for-read.  Returns a result code.
Result MakeFramebufferBuffer(const ::dawn::Device& device,
                             uint32_t width,
                             uint32_t height,
                             ::dawn::TextureFormat format,
                             ::dawn::Buffer* result_ptr,
                             uint32_t* texel_stride_ptr,
                             uint32_t* row_stride_ptr,
                             uint32_t* size_ptr) {
  assert(device);
  assert(width > 0);
  assert(height > 0);
  assert(result_ptr);
  assert(texel_stride_ptr);
  assert(row_stride_ptr);
  assert(size_ptr);

  // TODO(dneto): Handle other formats.
  // Actually, even better would be to pass the ::amber::Format down here,
  // and use its GetSizeInBytes() method to determine the texel size in bytes.
  if (format != ::dawn::TextureFormat::B8G8R8A8Unorm) {
    return Result(
        "Dawn::MakeFramebufferBuffer: Unhandled framebuffer format: " +
        std::to_string(static_cast<uint32_t>(format)));
  }
  // Number of bytes for each texel in the default format.
  const uint32_t default_texel_bytes = 4;

  uint32_t row_stride = default_texel_bytes * width;
  {
    // Round up the stride to the minimum image row pitch.
    const uint32_t spillover = row_stride % kMinimumImageRowPitch;
    if (spillover > 0)
      row_stride += (kMinimumImageRowPitch - spillover);
    assert(0 == (row_stride % kMinimumImageRowPitch));
  }

  ::dawn::BufferDescriptor descriptor;
  descriptor.size = row_stride * height;
  descriptor.usage =
      ::dawn::BufferUsageBit::TransferDst | ::dawn::BufferUsageBit::MapRead;
  *result_ptr = device.CreateBuffer(&descriptor);
  *texel_stride_ptr = default_texel_bytes;
  *row_stride_ptr = row_stride;
  *size_ptr = descriptor.size;
  return {};
}

// Result status object and data pointer resulting from a buffer mapping.
struct MapResult {
  Result result;
  const void* data = nullptr;
  uint64_t dataLength = 0;
};

// Handles the update from an asynchronous buffer map request, updating the
// state of the MapResult object hidden inside the |userdata| parameter.
// On a successful mapping outcome, set the data pointer in the map result.
// Otherwise set the map result object to an error, and the data member is
// not changed.
void HandleBufferMapCallback(DawnBufferMapAsyncStatus status,
                             const void* data,
                             uint64_t dataLength,
                             DawnCallbackUserdata userdata) {
  MapResult& map_result = *reinterpret_cast<MapResult*>(userdata);
  switch (status) {
    case DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS:
      map_result.data = data;
      map_result.dataLength = dataLength;
      break;
    case DAWN_BUFFER_MAP_ASYNC_STATUS_ERROR:
      map_result.result = Result("Buffer map for reading failed: error");
      break;
    case DAWN_BUFFER_MAP_ASYNC_STATUS_UNKNOWN:
    case DAWN_BUFFER_MAP_ASYNC_STATUS_FORCE32:
      map_result.result = Result("Buffer map for reading failed: unknown");
      break;
    case DAWN_BUFFER_MAP_ASYNC_STATUS_CONTEXT_LOST:
      map_result.result = Result("Buffer map for reading failed: context lost");
      break;
  }
}

// Maps the given buffer.  Assumes the buffer has usage bit
// dawn::BufferUsageBit::MapRead set.  Returns a MapResult structure, with the
// status saved in the .result member and the host pointer to the mapped data
// in the |.data| member. Mapping a buffer can fail if the context is lost, for
// example. In the failure case, the .data member of the result will be null.
MapResult MapBuffer(const ::dawn::Device& device, const ::dawn::Buffer& buf) {
  MapResult map_result;
  buf.MapReadAsync(HandleBufferMapCallback,
                   static_cast<DawnCallbackUserdata>(
                       reinterpret_cast<uintptr_t>(&map_result)));
  device.Tick();
  // Wait until the callback has been processed.  Use an exponential backoff
  // interval, but cap it at one second intervals.  But never loop forever.
  const int max_iters = 100;
  const int one_second_in_us = 1000000;
  for (int iters = 0, interval = 1;
       !map_result.data && map_result.result.IsSuccess();
       iters++, interval = std::min(2 * interval, one_second_in_us)) {
    device.Tick();
    if (iters > max_iters) {
      map_result.result = Result("MapBuffer timed out after 100 iterations");
      break;
    }
    USleep(uint32_t(interval));
  }
  return map_result;
}

// Converts an Amber format to a Dawn texture format, and sends the result out
// through |dawn_format_ptr|.  If the conversion fails, return an error result.
Result GetDawnTextureFormat(const ::amber::Format& amber_format,
                            ::dawn::TextureFormat* dawn_format_ptr) {
  if (!dawn_format_ptr)
    return Result("Internal error: format pointer argument is null");
  ::dawn::TextureFormat& dawn_format = *dawn_format_ptr;

  switch (amber_format.GetFormatType()) {
    // TODO(dneto): These are all the formats that Dawn currently knows about.
    case FormatType::kR8G8B8A8_UNORM:
      dawn_format = ::dawn::TextureFormat::R8G8B8A8Unorm;
      break;
    case FormatType::kR8G8_UNORM:
      dawn_format = ::dawn::TextureFormat::R8G8Unorm;
      break;
    case FormatType::kR8_UNORM:
      dawn_format = ::dawn::TextureFormat::R8Unorm;
      break;
    case FormatType::kR8G8B8A8_UINT:
      dawn_format = ::dawn::TextureFormat::R8G8B8A8Uint;
      break;
    case FormatType::kR8G8_UINT:
      dawn_format = ::dawn::TextureFormat::R8G8Uint;
      break;
    case FormatType::kR8_UINT:
      dawn_format = ::dawn::TextureFormat::R8Uint;
      break;
    case FormatType::kB8G8R8A8_UNORM:
      dawn_format = ::dawn::TextureFormat::B8G8R8A8Unorm;
      break;
    case FormatType::kD32_SFLOAT_S8_UINT:
      dawn_format = ::dawn::TextureFormat::D32FloatS8Uint;
      break;
    default:
      return Result(
          "Amber format " +
          std::to_string(static_cast<uint32_t>(amber_format.GetFormatType())) +
          " is invalid for Dawn");
  }

  return {};
}

}  // namespace

EngineDawn::EngineDawn() : Engine() {}

EngineDawn::~EngineDawn() = default;

Result EngineDawn::Initialize(EngineConfig* config,
                              Delegate*,
                              const std::vector<std::string>&,
                              const std::vector<std::string>&,
                              const std::vector<std::string>&) {
  if (device_)
    return Result("Dawn:Initialize device_ already exists");

  if (!config)
    return Result("Dawn::Initialize config is null");
  DawnEngineConfig* dawn_config = static_cast<DawnEngineConfig*>(config);
  if (dawn_config->device == nullptr)
    return Result("Dawn:Initialize device is a null pointer");

  device_ = dawn_config->device;

  return {};
}

Result EngineDawn::CreatePipeline(::amber::Pipeline* pipeline) {
  for (const auto& shader_info : pipeline->GetShaders()) {
    Result r = SetShader(shader_info.GetShaderType(), shader_info.GetData());
    if (!r.IsSuccess())
      return r;
  }

  switch (pipeline->GetType()) {
    case PipelineType::kCompute: {
      auto module = module_for_type_[kShaderTypeCompute];
      if (!module)
        return Result("CreatePipeline: no compute shader provided");
      pipeline_map_[pipeline].compute_pipeline.reset(
          new ComputePipelineInfo(pipeline, module));
      break;
    }

    case PipelineType::kGraphics: {
      // TODO(dneto): Handle other shader types as well.  They are optional.
      auto vs = module_for_type_[kShaderTypeVertex];
      auto fs = module_for_type_[kShaderTypeFragment];
      if (!vs) {
        return Result(
            "CreatePipeline: no vertex shader provided for graphics pipeline");
      }
      if (!fs) {
        return Result(
            "CreatePipeline: no vertex shader provided for graphics pipeline");
      }
      pipeline_map_[pipeline].render_pipeline.reset(
          new RenderPipelineInfo(pipeline, vs, fs));
      break;
    }
  }

  return {};
}

Result EngineDawn::SetShader(ShaderType type,
                             const std::vector<uint32_t>& code) {
  ::dawn::ShaderModuleDescriptor descriptor;
  descriptor.nextInChain = nullptr;
  descriptor.code = code.data();
  descriptor.codeSize = uint32_t(code.size());
  if (!device_) {
    return Result("Dawn::SetShader: device is not created");
  }
  auto shader = device_->CreateShaderModule(&descriptor);
  if (!shader) {
    return Result("Dawn::SetShader: failed to create shader");
  }
  if (module_for_type_.count(type)) {
    Result("Dawn::SetShader: module for type already exists");
  }
  module_for_type_[type] = shader;
  return {};
}

Result EngineDawn::DoClearColor(const ClearColorCommand* command) {
  RenderPipelineInfo* render_pipeline = GetRenderPipeline(command);
  if (!render_pipeline)
    return Result("ClearColor invoked on invalid or missing render pipeline");

  render_pipeline->clear_color_value = ::dawn::Color{
      command->GetR(), command->GetG(), command->GetB(), command->GetA()};
  return {};
}

Result EngineDawn::DoClearStencil(const ClearStencilCommand* command) {
  RenderPipelineInfo* render_pipeline = GetRenderPipeline(command);
  if (!render_pipeline)
    return Result("ClearStencil invoked on invalid or missing render pipeline");

  render_pipeline->clear_stencil_value = command->GetValue();
  return {};
}

Result EngineDawn::DoClearDepth(const ClearDepthCommand* command) {
  RenderPipelineInfo* render_pipeline = GetRenderPipeline(command);
  if (!render_pipeline)
    return Result("ClearDepth invoked on invalid or missing render pipeline");

  render_pipeline->clear_depth_value = command->GetValue();
  return {};
}

Result EngineDawn::DoClear(const ClearCommand* command) {
  RenderPipelineInfo* render_pipeline = GetRenderPipeline(command);
  if (!render_pipeline)
    return Result("Clear invoked on invalid or missing render pipeline");

  // TODO(dneto): Likely, we can create the render objects during
  // CreatePipeline.
  Result result = CreateFramebufferIfNeeded(render_pipeline);
  if (!result.IsSuccess())
    return result;

  // TODO(sarahM0): Adapt this to the case where there might be many colour
  // attachments. Also when there is a depth/stencil attachment.

  // Record a render pass in a command on the command buffer.
  //
  // First describe the color attachments, and how they are initialized
  // via the load op. The load op is "clear" to the clear colour.
  ::dawn::RenderPassColorAttachmentDescriptor color_attachment =
      ::dawn::RenderPassColorAttachmentDescriptor();
  color_attachment.attachment = render_pipeline->fb_texture.CreateDefaultView();
  color_attachment.resolveTarget = nullptr;
  color_attachment.clearColor = render_pipeline->clear_color_value;
  color_attachment.loadOp = ::dawn::LoadOp::Clear;
  color_attachment.storeOp = ::dawn::StoreOp::Store;

  // Attach that colour attachment to the render pass.
  ::dawn::RenderPassColorAttachmentDescriptor* rpca[] = {&color_attachment};
  ::dawn::RenderPassDescriptor rpd;
  rpd.colorAttachmentCount = 1;
  rpd.colorAttachments = rpca;
  rpd.depthStencilAttachment = nullptr;

  // Record the render pass as a command.
  auto encoder = device_->CreateCommandEncoder();
  ::dawn::RenderPassEncoder pass = encoder.BeginRenderPass(&rpd);
  pass.EndPass();

  // Finish recording the command buffer.  It only has one command.
  auto command_buffer = encoder.Finish();

  // Submit the command.
  auto queue = device_->CreateQueue();
  queue.Submit(1, &command_buffer);

  // Copy the framebuffer contents back into the host-side framebuffer-buffer.
  ::dawn::Buffer& fb_buffer = render_pipeline->fb_buffer;
  MapResult map = MapBuffer(*device_, fb_buffer);
  // For now, we assume there is only one colour attachment, and that
  // corresponds to the framebuffer texture.
  const std::vector<amber::Pipeline::BufferInfo>& out_color_attachment =
      render_pipeline->pipeline->GetColorAttachments();
  for (size_t i = 0; i < 1; ++i) {
    auto& info = out_color_attachment[i];
    auto* values = info.buffer->ValuePtr();
    values->resize(map.dataLength);
    // TODO(sarahM0): Resolve the difference between the Dawn buffer's size
    // and the Amber buffer's size.
    std::memcpy(values->data(), map.data, map.dataLength);
  }
  // Always unmap the buffer at the end of the engine's command.
  fb_buffer.Unmap();

  return map.result;
}

Result EngineDawn::DoDrawRect(const DrawRectCommand*) {
  Result result;

#if 0
  // Add one more command to the command buffer builder, which is to
  // copy the framebuffer texture to the framebuffer host-side buffer.
  ::dawn::Buffer& fb_buffer = render_pipeline_info_.fb_buffer;
  if (!fb_buffer) {
    return Result(
        "Dawn::DoDrawRect: Framebuffer was not created.  Did you run "
        "any graphics pipeline commands?");
  }

  result = CreateCommandBufferBuilderIfNeeded();
  if (!result.IsSuccess())
    return result;

  {
    const int x = 0, y = 0, z = 0, depth = 1, level = 0, slice = 0,
              buffer_offset = 0;

    ::dawn::TextureCopyView texture_copy_view = ::dawn::TextureCopyView();
    texture_copy_view.texture = render_pipeline_info_.fb_texture;
    texture_copy_view.level = level;
    texture_copy_view.slice = slice;
    texture_copy_view.origin = {x, y, z};

    ::dawn::BufferCopyView buffer_copy_view = ::dawn::BufferCopyView();
    buffer_copy_view.buffer = fb_buffer;
    buffer_copy_view.offset = buffer_offset;
    buffer_copy_view.rowPitch = render_pipeline_info_.fb_row_stride;
    buffer_copy_view.imageHeight = render_pipeline_info_.fb_num_rows;

    ::dawn::Extent3D extent = {kFramebufferWidth, kFramebufferHeight, depth};

    command_buffer_builder_.CopyTextureToBuffer(&texture_copy_view,
                                                &buffer_copy_view, &extent);
  }

  // Make sure we have a queue.
  if (!queue_)
    queue_ = device_->CreateQueue();

  // Now run the commands.
  auto command_buffer = command_buffer_builder_.Finish();

  if (render_pipeline_info_.fb_data != nullptr) {
    fb_buffer.Unmap();
    render_pipeline_info_.fb_data = nullptr;
  }

  queue_.Submit(1, &command_buffer);

  // And any further commands start afresh.
  DestroyCommandBufferBuilder();

  MapResult map = MapBuffer(*device_, fb_buffer);
  render_pipeline_info_.fb_data = map.data;
  return map.result;
#endif
  return Result("Dawn:DoDrawRect not implemented");
}

Result EngineDawn::DoDrawArrays(const DrawArraysCommand*) {
  return Result("Dawn:DoDrawArrays not implemented");
}

Result EngineDawn::DoCompute(const ComputeCommand*) {
  return Result("Dawn:DoCompute not implemented");
}

Result EngineDawn::DoEntryPoint(const EntryPointCommand*) {
  return Result("Dawn:DoEntryPoint not implemented");
}

Result EngineDawn::DoPatchParameterVertices(
    const PatchParameterVerticesCommand*) {
  return Result("Dawn:DoPatch not implemented");
}

Result EngineDawn::DoBuffer(const BufferCommand*) {
  return Result("Dawn:DoBuffer not implemented");
}

Result EngineDawn::CreateFramebufferIfNeeded(
    RenderPipelineInfo* render_pipeline) {
  Result result;

  const uint32_t width = render_pipeline->pipeline->GetFramebufferWidth();
  const uint32_t height = render_pipeline->pipeline->GetFramebufferHeight();

  // TODO(dneto): For now, assume color attachment 0 is the framebuffer.
  auto* amber_format =
      render_pipeline->pipeline->GetColorAttachments()[0].buffer->GetFormat();
  if (!amber_format)
    return Result("Color attachment 0 has no format!");

  ::dawn::TextureFormat fb_format{};
  result = GetDawnTextureFormat(*amber_format, &fb_format);
  if (!result.IsSuccess())
    return result;

  // First make the Dawn color attachment textures that the render pipeline will
  // write into.
  {
    ::dawn::Texture fb_texture;

    result =
        MakeFramebufferTexture(*device_, fb_format, width, height, &fb_texture);
    if (!result.IsSuccess())
      return result;
    render_pipeline->fb_texture = std::move(fb_texture);
  }

  // Now create the Dawn buffer to hold the framebuffer contents, but on the
  // host side.  This has to match dimensions of the framebuffer, but also
  // be linearly addressible by the CPU.
  {
    ::dawn::Buffer fb_buffer;
    uint32_t texel_stride = 0;
    uint32_t row_stride = 0;
    uint32_t size = 0;
    result =
        MakeFramebufferBuffer(*device_, width, height, fb_format, &fb_buffer,
                              &texel_stride, &row_stride, &size);
    if (!result.IsSuccess())
      return result;
    render_pipeline->fb_buffer = std::move(fb_buffer);
    render_pipeline->fb_texel_stride = texel_stride;
    render_pipeline->fb_row_stride = row_stride;
    render_pipeline->fb_num_rows = height;
    render_pipeline->fb_size = size;
  }
  return {};
}

}  // namespace dawn
}  // namespace amber
