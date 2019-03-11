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
#include <iostream>
#include <utility>
#include <vector>

#include "amber/amber_dawn.h"
#include "dawn/dawncpp.h"
#include "src/format.h"
#include "src/sleep.h"

namespace amber {
namespace dawn {

namespace {

// The dimensions of the framebuffer.  The code also assumes the framebuffer
// is 2D and does not have any array layers.
const uint32_t kFramebufferWidth = 250, kFramebufferHeight = 250;
// The minimum multiple row pitch observed on Dawn on Metal.  Increase this
// as needed for other Dawn backends.
const uint32_t kMinimumImageRowPitch = 256;
const auto kFramebufferFormat = ::dawn::TextureFormat::R8G8B8A8Unorm;
const auto kAmberFramebufferFormatType = FormatType::kR8G8B8A8_UNORM;

// Creates a device-side texture for the framebuffer, and returns it through
// |result_ptr|.  Assumes the device exists and is valid.  Assumes result_ptr
// is not null.  Returns a result code.
Result MakeFramebufferTexture(const ::dawn::Device& device,
                              ::dawn::TextureFormat format,
                              ::dawn::Texture* result_ptr) {
  assert(device);
  assert(result_ptr);
  ::dawn::TextureDescriptor descriptor;
  descriptor.dimension = ::dawn::TextureDimension::e2D;
  descriptor.size.width = kFramebufferWidth;
  descriptor.size.height = kFramebufferHeight;
  descriptor.size.depth = 1;
  descriptor.arraySize = 1;
  descriptor.format = format;
  descriptor.levelCount = 1;
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
                             ::dawn::TextureFormat format,
                             ::dawn::Buffer* result_ptr,
                             uint32_t* texel_stride_ptr,
                             uint32_t* row_stride_ptr,
                             uint32_t* num_rows_ptr,
                             uint32_t* size_ptr) {
  assert(device);
  assert(result_ptr);
  assert(texel_stride_ptr);
  assert(row_stride_ptr);
  assert(num_rows_ptr);
  assert(size_ptr);

  // TODO(dneto): Handle other formats.
  if (format != ::dawn::TextureFormat::R8G8B8A8Unorm) {
    return Result("Dawn::MakeFramebufferBuffer: Unhandled framebuffer format");
  }
  // Number of bytes for each texel in the default format.
  const uint32_t default_texel_bytes = 4;

  const uint32_t num_rows = kFramebufferHeight;

  uint32_t row_stride = default_texel_bytes * kFramebufferWidth;
  {
    // Round up the stride to the minimum image row pitch.
    const uint32_t spillover = row_stride % kMinimumImageRowPitch;
    if (spillover > 0)
      row_stride += (kMinimumImageRowPitch - spillover);
    assert(0 == (row_stride % kMinimumImageRowPitch));
  }

  ::dawn::BufferDescriptor descriptor;
  descriptor.size = row_stride * num_rows;
  descriptor.usage =
      ::dawn::BufferUsageBit::TransferDst | ::dawn::BufferUsageBit::MapRead;
  *result_ptr = device.CreateBuffer(&descriptor);
  *texel_stride_ptr = default_texel_bytes;
  *row_stride_ptr = row_stride;
  *num_rows_ptr = num_rows;
  *size_ptr = descriptor.size;
  return {};
}

// Result status object and data pointer resulting from a buffer mapping.
struct MapResult {
  Result result;
  const void* data = nullptr;
};

// Handles the update from an asynchronous buffer map request, updating the
// state of the MapResult object hidden inside the |userdata| parameter.
// On a successful mapping outcome, set the data pointer in the map result.
// Otherwise set the map result object to an error, and the data member is
// not changed.
void HandleBufferMapCallback(dawnBufferMapAsyncStatus status,
                             const void* data,
                             dawnCallbackUserdata userdata) {
  MapResult& map_result = *reinterpret_cast<MapResult*>(userdata);
  switch (status) {
    case DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS:
      map_result.data = data;
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
MapResult MapBuffer(const ::dawn::Device& device,
                    const ::dawn::Buffer& buf,
                    uint32_t size) {
  MapResult map_result;
  buf.MapReadAsync(0, size, HandleBufferMapCallback,
                   static_cast<dawnCallbackUserdata>(
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

Result EngineDawn::Shutdown() {
  device_ = nullptr;
  return {};
}

Result EngineDawn::CreatePipeline(Pipeline* pipeline) {
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
      compute_pipeline_info_ = ComputePipelineInfo(module);
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
      render_pipeline_info_ = RenderPipelineInfo(vs, fs);
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
  render_pipeline_info_.clear_color_value = ::dawn::Color{
      command->GetR(), command->GetG(), command->GetB(), command->GetA()};
  return {};
}

Result EngineDawn::DoClearStencil(const ClearStencilCommand* command) {
  render_pipeline_info_.clear_stencil_value = command->GetValue();
  return {};
}

Result EngineDawn::DoClearDepth(const ClearDepthCommand* command) {
  render_pipeline_info_.clear_depth_value = command->GetValue();
  return {};
}

Result EngineDawn::DoClear(const ClearCommand*) {
  Result result = CreateRenderObjectsIfNeeded();
  if (!result.IsSuccess())
    return result;

  ::dawn::RenderPassColorAttachmentDescriptor color_attachment =
      ::dawn::RenderPassColorAttachmentDescriptor();
  color_attachment.attachment =
      render_pipeline_info_.fb_texture.CreateDefaultTextureView();
  color_attachment.resolveTarget = nullptr;
  color_attachment.clearColor = render_pipeline_info_.clear_color_value;
  color_attachment.loadOp = ::dawn::LoadOp::Clear;
  color_attachment.storeOp = ::dawn::StoreOp::Store;

  ::dawn::RenderPassDescriptor rpd =
      device_->CreateRenderPassDescriptorBuilder()
          .SetColorAttachments(1, &color_attachment)
          .GetResult();
  command_buffer_builder_.BeginRenderPass(rpd).EndPass();
  return {};
}

Result EngineDawn::DoDrawRect(const DrawRectCommand*) {
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

Result EngineDawn::DoProcessCommands(Pipeline* pipeline) {
  Result result;

  // TODO(dneto): How to distinguish the compute case: It won't have a
  // framebuffer?

  // Add one more command to the command buffer builder, which is to
  // copy the framebuffer texture to the framebuffer host-side buffer.
  ::dawn::Buffer& fb_buffer = render_pipeline_info_.fb_buffer;
  if (!fb_buffer) {
    return Result(
        "Dawn::DoProcessCommands: Framebuffer was not created.  Did you run "
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
  auto command_buffer = command_buffer_builder_.GetResult();

  if (render_pipeline_info_.fb_data != nullptr) {
    fb_buffer.Unmap();
    render_pipeline_info_.fb_data = nullptr;
  }

  queue_.Submit(1, &command_buffer);

  // And any further commands start afresh.
  DestroyCommandBufferBuilder();

  MapResult map = MapBuffer(*device_, fb_buffer, render_pipeline_info_.fb_size);
  render_pipeline_info_.fb_data = map.data;
  return map.result;
}

Result EngineDawn::CreateCommandBufferBuilderIfNeeded() {
  if (command_buffer_builder_)
    return {};

  // Create the command buffer builder because it doesn't exist yet.
  if (!device_) {
    return Result(
        "EngineDawn: Can't create command buffer builder: device is not "
        "initialized");
  }
  command_buffer_builder_ = device_->CreateCommandBufferBuilder();
  if (command_buffer_builder_)
    return {};
  return Result("EngineDawn: Can't create command buffer builder");
}

void EngineDawn::DestroyCommandBufferBuilder() {
  command_buffer_builder_ = ::dawn::CommandBufferBuilder();
}

Result EngineDawn::CreateRenderObjectsIfNeeded() {
  Result result = CreateCommandBufferBuilderIfNeeded();
  if (!result.IsSuccess())
    return result;
  return CreateFramebufferIfNeeded();
}

Result EngineDawn::CreateFramebufferIfNeeded() {
  Result result;
  {
    ::dawn::Texture fb_texture;
    result = MakeFramebufferTexture(*device_, kFramebufferFormat, &fb_texture);
    if (!result.IsSuccess())
      return result;
    render_pipeline_info_.fb_texture = std::move(fb_texture);
  }

  {
    ::dawn::Buffer fb_buffer;
    uint32_t texel_stride = 0;
    uint32_t row_stride = 0;
    uint32_t num_rows = 0;
    uint32_t size = 0;
    result =
        MakeFramebufferBuffer(*device_, kFramebufferFormat, &fb_buffer,
                              &texel_stride, &row_stride, &num_rows, &size);
    if (!result.IsSuccess())
      return result;
    render_pipeline_info_.fb_buffer = std::move(fb_buffer);
    render_pipeline_info_.fb_texel_stride = texel_stride;
    render_pipeline_info_.fb_row_stride = row_stride;
    render_pipeline_info_.fb_num_rows = num_rows;
    render_pipeline_info_.fb_size = size;
    render_pipeline_info_.fb_data = nullptr;

    // TODO(dneto): support other formats
    render_pipeline_info_.fb_format = ::amber::Format();
    render_pipeline_info_.fb_format.SetFormatType(kAmberFramebufferFormatType);
  }
  return {};
}

Result EngineDawn::GetFrameBuffer(Buffer*, std::vector<Value>*) {
  return Result("Dawn::GetFrameBuffer not implemented");
}

}  // namespace dawn
}  // namespace amber
