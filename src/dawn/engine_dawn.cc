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

#include <webgpu/webgpu_cpp.h>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits>
#include <string>
#include <utility>
#include <vector>

#include "amber/amber_dawn.h"
#include "src/dawn/pipeline_info.h"
#include "src/format.h"
#include "src/sleep.h"

namespace amber {
namespace dawn {

namespace {

// The minimum multiple row pitch observed on Dawn on Metal.  Increase this
// as needed for other Dawn backends.
static const uint32_t kMinimumImageRowPitch = 256;
static const float kLodMin = 0.0;
static const float kLodMax = 1000.0;
static const uint32_t kMaxColorAttachments = 4u;
static const uint32_t kMaxVertexInputs = 16u;
static const uint32_t kMaxVertexAttributes = 16u;
static const uint32_t kMaxDawnBindGroup = 4u;

// A DS for creating and setting the defaults for VertexInputDescriptor
// Copied from Dawn utils source code.
struct ComboVertexInputDescriptor {
  ComboVertexInputDescriptor() {
    wgpu::VertexState* descriptor = &state;

    descriptor->bufferCount = 0;

    // Fill the default values for vertexBuffers and vertexAttributes in
    // buffers.
    wgpu::VertexAttribute vertexAttribute;
    vertexAttribute.shaderLocation = 0;
    vertexAttribute.offset = 0;
    vertexAttribute.format = wgpu::VertexFormat::Float32;
    for (uint32_t i = 0; i < kMaxVertexAttributes; ++i) {
      cAttributes[i] = vertexAttribute;
    }
    for (uint32_t i = 0; i < kMaxVertexBuffers; ++i) {
      cBuffers[i].arrayStride = 0;
      cBuffers[i].stepMode = wgpu::VertexStepMode::Vertex;
      cBuffers[i].attributeCount = 0;
      cBuffers[i].attributes = nullptr;
    }
    // cBuffers[i].attributes points to somewhere in cAttributes.
    // cBuffers[0].attributes points to &cAttributes[0] by default. Assuming
    // cBuffers[0] has two attributes, then cBuffers[1].attributes should
    // point to &cAttributes[2]. Likewise, if cBuffers[1] has 3 attributes,
    // then cBuffers[2].attributes should point to &cAttributes[5].

    // In amber-dawn, the vertex input descriptor is always created assuming
    // these relationships are one to one i.e. cBuffers[i].attributes is always
    // pointing to &cAttributes[i] and cBuffers[i].attributeCount == 1
    cBuffers[0].attributes = &cAttributes[0];
    descriptor->buffers = cBuffers.data();
  }

  static const uint32_t kMaxVertexBuffers = 16u;
  static const uint32_t kMaxVertexBufferStride = 2048u;
  wgpu::VertexState state;
  std::array<wgpu::VertexBufferLayout, kMaxVertexBuffers> cBuffers;
  std::array<wgpu::VertexAttribute, kMaxVertexAttributes> cAttributes;
};

// This structure is a container for a few variables that are created during
// CreateRenderPipelineDescriptor and CreateRenderPassDescriptor and we want to
// make sure they don't go out of scope before we are done with them
struct DawnPipelineHelper {
  Result CreateRenderPipelineDescriptor(
      const RenderPipelineInfo& render_pipeline,
      const wgpu::Device& device,
      const bool ignore_vertex_and_Index_buffers,
      const PipelineData* pipeline_data);
  Result CreateRenderPassDescriptor(
      const RenderPipelineInfo& render_pipeline,
      const wgpu::Device& device,
      const std::vector<wgpu::TextureView>& texture_view,
      const wgpu::LoadOp load_op);
  wgpu::RenderPipelineDescriptor renderPipelineDescriptor = {};
  wgpu::RenderPassDescriptor renderPassDescriptor = {};

  ComboVertexInputDescriptor vertexInputDescriptor;
  wgpu::PrimitiveState primitiveState = {};
  wgpu::MultisampleState multisampleState = {};

 private:
  wgpu::FragmentState fragmentState = {};
  wgpu::RenderPassColorAttachment
      colorAttachmentsInfoPtr[kMaxColorAttachments] = {};
  wgpu::RenderPassDepthStencilAttachment depthStencilAttachmentInfo = {};
  wgpu::ColorTargetState colorTargetStates[kMaxColorAttachments] = {};
  wgpu::DepthStencilState depthStencilState = {};
  wgpu::BlendState colorBlends[kMaxColorAttachments] = {};
  std::string vertexEntryPoint;
  std::string fragmentEntryPoint;
  wgpu::RenderPassColorAttachment colorAttachmentsInfo[kMaxColorAttachments] =
      {};
  wgpu::TextureDescriptor depthStencilDescriptor = {};
  wgpu::Texture depthStencilTexture;
  wgpu::TextureView depthStencilView;
};

// Creates a device-side texture, and returns it through |result_ptr|.
// Assumes the device exists and is valid.  Assumes result_ptr is not null.
// Returns a result code.
Result MakeTexture(const wgpu::Device& device,
                   wgpu::TextureFormat format,
                   uint32_t width,
                   uint32_t height,
                   wgpu::Texture* result_ptr) {
  assert(device);
  assert(result_ptr);
  assert(width * height > 0);
  wgpu::TextureDescriptor descriptor = {};
  descriptor.size.width = width;
  descriptor.size.height = height;
  descriptor.size.depthOrArrayLayers = 1;
  descriptor.format = format;
  descriptor.usage =
      wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment;
  // TODO(dneto): Get a better message by using the Dawn error callback.
  *result_ptr = device.CreateTexture(&descriptor);
  if (*result_ptr) {
    return {};
  }
  return Result("Dawn: Failed to allocate a framebuffer texture");
}

// Creates a device-side texture, and returns it through |result_ptr|.
// Assumes the device exists and is valid.  Assumes result_ptr is not null.
// Returns a result code.
wgpu::Texture MakeDawnTexture(const wgpu::Device& device,
                              wgpu::TextureFormat format,
                              uint32_t width,
                              uint32_t height) {
  assert(device);
  assert(width * height > 0);
  wgpu::TextureDescriptor descriptor = {};
  descriptor.size.width = width;
  descriptor.size.height = height;
  descriptor.size.depthOrArrayLayers = 1;
  descriptor.format = format;
  descriptor.usage =
      wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment;

  return device.CreateTexture(&descriptor);
}

// Result status object and data pointer resulting from a buffer mapping.
struct MapResult {
  Result result;
  const void* data = nullptr;
  uint64_t dataLength = 0;
  bool completed = false;
};

// Handles the update from an asynchronous buffer map request, updating the
// state of the MapResult object hidden inside the |userdata| parameter.
// On a successful mapping outcome, set the completed flag.
// Otherwise set the map result object to an error.
void HandleBufferMapCallback(wgpu::MapAsyncStatus status,
                             wgpu::StringView message,
                             MapResult* map_result) {
  switch (status) {
    case wgpu::MapAsyncStatus::Success:
      break;
    case wgpu::MapAsyncStatus::Error:
      map_result->result =
          Result("Buffer map failed: " + std::string(message.data));
      break;
    case wgpu::MapAsyncStatus::Aborted:
      map_result->result =
          Result("Buffer map aborted: " + std::string(message.data));
      break;
    case wgpu::MapAsyncStatus::CallbackCancelled:
      map_result->result =
          Result("Buffer map callback cancelled: " + std::string(message.data));
      break;
    default:
      map_result->result = Result("Buffer map failed with unknown status");
      break;
  }
  map_result->completed = true;
}

// Returns |value| but rounded up to a multiple of |alignment|. |alignment| is
// assumed to be a power of 2.
uint32_t Align(uint32_t value, size_t alignment) {
  assert(alignment <= std::numeric_limits<uint32_t>::max());
  assert(alignment != 0);
  uint32_t alignment32 = static_cast<uint32_t>(alignment);
  return (value + (alignment32 - 1)) & ~(alignment32 - 1);
}

}  // namespace

// Maps the given buffer.  Assumes the buffer has usage bit
// wgpu::BufferUsage::MapRead set.  Returns a MapResult structure, with
// the status saved in the |result| member and the host pointer to the mapped
// data in the |data| member. Mapping a buffer can fail if the context is
// lost, for example. In the failure case, the |data| member will be null.
MapResult MapBuffer(wgpu::Instance instance, wgpu::Buffer buf, uint64_t size) {
  MapResult map_result;
  map_result.dataLength = size;

  buf.MapAsync(wgpu::MapMode::Read, 0, size,
               wgpu::CallbackMode::AllowProcessEvents, HandleBufferMapCallback,
               &map_result);

  instance.ProcessEvents();
  // Wait until the callback has been processed.  Use an exponential backoff
  // interval, but cap it at one second intervals.  But never loop forever.
  const int max_iters = 100;
  const int one_second_in_us = 1000000;
  for (int iters = 0, interval = 1;
       !map_result.completed && map_result.result.IsSuccess();
       iters++, interval = std::min(2 * interval, one_second_in_us)) {
    instance.ProcessEvents();
    if (iters > max_iters) {
      map_result.result = Result("MapBuffer timed out after 100 iterations");
      break;
    }
    amber::USleep(interval);
  }

  if (map_result.result.IsSuccess()) {
    map_result.data = buf.GetConstMappedRange(0, size);
  }
  return map_result;
}

// Creates and returns a dawn BufferCopyView
// Copied from Dawn utils source code.
wgpu::TexelCopyBufferInfo CreateBufferCopyView(wgpu::Buffer buffer,
                                               uint64_t offset,
                                               uint32_t bytesPerRow,
                                               uint32_t rowsPerImage) {
  wgpu::TexelCopyBufferInfo bufferCopyView;
  bufferCopyView.buffer = buffer;
  bufferCopyView.layout.offset = offset;
  bufferCopyView.layout.bytesPerRow = bytesPerRow;
  bufferCopyView.layout.rowsPerImage =
      (rowsPerImage == 0) ? wgpu::kCopyStrideUndefined : rowsPerImage;

  return bufferCopyView;
}

// Creates and returns a dawn TextureCopyView
// Copied from Dawn utils source code.
wgpu::TexelCopyTextureInfo CreateTextureCopyView(wgpu::Texture texture,
                                                 uint32_t mipLevel,
                                                 wgpu::Origin3D origin) {
  wgpu::TexelCopyTextureInfo textureCopyView;
  textureCopyView.texture = texture;
  textureCopyView.mipLevel = mipLevel;
  textureCopyView.origin = origin;
  textureCopyView.aspect = wgpu::TextureAspect::All;

  return textureCopyView;
}

Result EngineDawn::MapDeviceTextureToHostBuffer(
    const RenderPipelineInfo& render_pipeline,
    wgpu::Device device) {
  const auto width = render_pipeline.pipeline->GetFramebufferWidth();
  const auto height = render_pipeline.pipeline->GetFramebufferHeight();

  const auto pixelSize = render_pipeline.pipeline->GetColorAttachments()[0]
                             .buffer->GetElementStride();
  const auto dawn_row_pitch = Align(width * pixelSize, kMinimumImageRowPitch);
  const auto size = height * dawn_row_pitch;
  // Create a temporary buffer to hold the color attachment content and can
  // be mapped
  wgpu::BufferDescriptor descriptor = {};
  descriptor.size = size;
  descriptor.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::MapRead;
  wgpu::Buffer copy_buffer = device.CreateBuffer(&descriptor);
  wgpu::TexelCopyBufferInfo copy_buffer_view =
      CreateBufferCopyView(copy_buffer, 0, dawn_row_pitch, 0);
  wgpu::Origin3D origin3D = {0, 0, 0};

  for (uint32_t i = 0;
       i < render_pipeline.pipeline->GetColorAttachments().size(); i++) {
    wgpu::TexelCopyTextureInfo device_texture_view =
        CreateTextureCopyView(textures_[i], 0, origin3D);
    wgpu::Extent3D copySize = {width, height, 1};
    wgpu::CommandEncoderDescriptor encoder_desc = {};

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder(&encoder_desc);
    encoder.CopyTextureToBuffer(&device_texture_view, &copy_buffer_view,
                                &copySize);
    wgpu::CommandBufferDescriptor command_buffer_desc = {};
    wgpu::CommandBuffer commands = encoder.Finish(&command_buffer_desc);
    wgpu::Queue queue = device.GetQueue();
    queue.Submit(1, &commands);

    MapResult mapped_device_texture = MapBuffer(instance_, copy_buffer, size);
    if (!mapped_device_texture.result.IsSuccess()) {
      return mapped_device_texture.result;
    }

    auto& host_texture = render_pipeline.pipeline->GetColorAttachments()[i];
    auto* values = host_texture.buffer->ValuePtr();
    auto row_stride = pixelSize * width;
    assert(row_stride * height == host_texture.buffer->GetSizeInBytes());
    // Each Dawn row has enough data to fill the target row.
    assert(dawn_row_pitch >= row_stride);
    values->resize(host_texture.buffer->GetSizeInBytes());
    // Copy the framebuffer contents back into the host-side
    // framebuffer-buffer. In the Dawn buffer, the row stride is a multiple of
    // kMinimumImageRowPitch bytes, so it might have padding therefore memcpy
    // is done row by row.
    for (uint h = 0; h < height; h++) {
      std::memcpy(values->data() + h * row_stride,
                  static_cast<const uint8_t*>(mapped_device_texture.data) +
                      h * dawn_row_pitch,
                  row_stride);
    }
    // Always unmap the buffer at the end of the engine's command.
    copy_buffer.Unmap();
  }
  return {};
}

Result EngineDawn::MapDeviceBufferToHostBuffer(
    const ComputePipelineInfo& compute_pipeline,
    wgpu::Device device) {
  for (uint32_t i = 0; i < compute_pipeline.pipeline->GetBuffers().size();
       i++) {
    auto& device_buffer = compute_pipeline.buffers[i];
    auto& host_buffer = compute_pipeline.pipeline->GetBuffers()[i];

    // Create a copy of device buffer to use it in a map read operation.
    // It's not possible to simply set this bit on the existing buffers since:
    // Device error: Only CopyDst is allowed with MapRead
    const uint64_t copy_size =
        static_cast<uint64_t>(host_buffer.buffer->GetSizeInBytes());
    wgpu::BufferDescriptor descriptor = {};
    descriptor.size = copy_size;
    descriptor.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::MapRead;
    const wgpu::Buffer copy_device_buffer = device.CreateBuffer(&descriptor);
    const uint64_t source_offset = 0;
    const uint64_t destination_offset = 0;
    wgpu::CommandEncoderDescriptor encoder_desc = {};

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder(&encoder_desc);
    encoder.CopyBufferToBuffer(device_buffer, source_offset, copy_device_buffer,
                               destination_offset, copy_size);
    wgpu::CommandBufferDescriptor command_buffer_desc = {};
    wgpu::CommandBuffer commands = encoder.Finish(&command_buffer_desc);
    wgpu::Queue queue = device.GetQueue();
    queue.Submit(1, &commands);

    MapResult mapped_device_buffer =
        MapBuffer(instance_, copy_device_buffer, copy_size);
    if (!mapped_device_buffer.result.IsSuccess()) {
      return mapped_device_buffer.result;
    }

    auto* values = host_buffer.buffer->ValuePtr();
    values->resize(host_buffer.buffer->GetSizeInBytes());
    std::memcpy(values->data(),
                static_cast<const uint8_t*>(mapped_device_buffer.data),
                copy_size);

    copy_device_buffer.Unmap();
  }
  return {};
}

// Creates a dawn buffer of |size| bytes with TransferDst and the given usage
// copied from Dawn utils source code
wgpu::Buffer CreateBufferFromData(const wgpu::Device& device,
                                  const void* data,
                                  uint64_t size,
                                  wgpu::BufferUsage usage) {
  wgpu::BufferDescriptor descriptor = {};
  descriptor.size = size;
  descriptor.usage = usage | wgpu::BufferUsage::CopyDst;

  wgpu::Buffer buffer = device.CreateBuffer(&descriptor);
  if (data != nullptr) {
    wgpu::Queue queue = device.GetQueue();
    queue.WriteBuffer(buffer, 0, data, size);
  }
  return buffer;
}

// Creates a bind group.
// Helpers to make creating bind groups look nicer:
//
//   utils::MakeBindGroup(device, layout, {
//       {0, mySampler},
//       {1, myBuffer, offset, size},
//       {3, myTexture}
//   });

// Structure with one constructor per-type of bindings, so that the
// initializer_list accepts bindings with the right type and no extra
// information.
struct BindingInitializationHelper {
  BindingInitializationHelper(uint32_t binding,
                              const wgpu::Buffer& buffer,
                              uint64_t offset,
                              uint64_t size)
      : binding(binding), buffer(buffer), offset(offset), size(size) {}

  wgpu::BindGroupEntry GetAsBinding() const {
    wgpu::BindGroupEntry result;
    result.binding = binding;
    result.sampler = sampler;
    result.textureView = textureView;
    result.buffer = buffer;
    result.offset = offset;
    result.size = size;
    return result;
  }

  uint32_t binding;
  wgpu::Sampler sampler;
  wgpu::TextureView textureView;
  wgpu::Buffer buffer;
  uint64_t offset = 0;
  uint64_t size = 0;
};

wgpu::BindGroup MakeBindGroup(
    const wgpu::Device& device,
    const wgpu::BindGroupLayout& layout,
    const std::vector<BindingInitializationHelper>& bindingsInitializer) {
  std::vector<wgpu::BindGroupEntry> bindings;
  for (const BindingInitializationHelper& helper : bindingsInitializer) {
    bindings.push_back(helper.GetAsBinding());
  }

  wgpu::BindGroupDescriptor descriptor = {};
  descriptor.layout = layout;
  descriptor.entryCount = static_cast<uint32_t>(bindings.size());
  descriptor.entries = bindings.data();

  return device.CreateBindGroup(&descriptor);
}

// Creates a bind group layout.
// Copied from Dawn utils source code.
wgpu::BindGroupLayout MakeBindGroupLayout(
    const wgpu::Device& device,
    const std::vector<wgpu::BindGroupLayoutEntry>& bindingsInitializer) {
  constexpr wgpu::ShaderStage kNoStages = wgpu::ShaderStage::None;

  std::vector<wgpu::BindGroupLayoutEntry> bindings;
  for (const wgpu::BindGroupLayoutEntry& binding : bindingsInitializer) {
    if (binding.visibility != kNoStages) {
      bindings.push_back(binding);
    }
  }

  wgpu::BindGroupLayoutDescriptor descriptor = {};
  descriptor.entryCount = static_cast<uint32_t>(bindings.size());
  descriptor.entries = bindings.data();
  return device.CreateBindGroupLayout(&descriptor);
}

// Creates a basic pipeline layout.
// Copied from Dawn utils source code.
wgpu::PipelineLayout MakeBasicPipelineLayout(
    const wgpu::Device& device,
    const std::vector<wgpu::BindGroupLayout>& bindingInitializer) {
  wgpu::PipelineLayoutDescriptor descriptor = {};
  descriptor.bindGroupLayoutCount =
      static_cast<uint32_t>(bindingInitializer.size());
  descriptor.bindGroupLayouts = bindingInitializer.data();
  return device.CreatePipelineLayout(&descriptor);
}

// Converts an Amber format to a Dawn texture format, and sends the result out
// through |dawn_format_ptr|.  If the conversion fails, return an error
// result.
Result GetDawnTextureFormat(const ::amber::Format& amber_format,
                            wgpu::TextureFormat* dawn_format_ptr) {
  if (!dawn_format_ptr) {
    return Result("Internal error: format pointer argument is null");
  }
  wgpu::TextureFormat& dawn_format = *dawn_format_ptr;

  switch (amber_format.GetFormatType()) {
    // TODO(dneto): These are all the formats that Dawn currently knows about.
    case FormatType::kR8G8B8A8_UNORM:
      dawn_format = wgpu::TextureFormat::RGBA8Unorm;
      break;
    case FormatType::kR8G8_UNORM:
      dawn_format = wgpu::TextureFormat::RG8Unorm;
      break;
    case FormatType::kR8_UNORM:
      dawn_format = wgpu::TextureFormat::R8Unorm;
      break;
    case FormatType::kR8G8B8A8_UINT:
      dawn_format = wgpu::TextureFormat::RGBA8Uint;
      break;
    case FormatType::kR8G8_UINT:
      dawn_format = wgpu::TextureFormat::RG8Uint;
      break;
    case FormatType::kR8_UINT:
      dawn_format = wgpu::TextureFormat::R8Uint;
      break;
    case FormatType::kB8G8R8A8_UNORM:
      dawn_format = wgpu::TextureFormat::BGRA8Unorm;
      break;
    case FormatType::kD32_SFLOAT_S8_UINT:
      dawn_format = wgpu::TextureFormat::Depth24PlusStencil8;
      break;
    default:
      return Result(
          "Amber format " +
          std::to_string(static_cast<uint32_t>(amber_format.GetFormatType())) +
          " is invalid for Dawn");
  }

  return {};
}
// Converts an Amber format to a Dawn Vertex format, and sends the result out
// through |dawn_format_ptr|.  If the conversion fails, return an error
// result.
// TODO(sarahM0): support other wgpu::VertexFormat
Result GetDawnVertexFormat(const ::amber::Format& amber_format,
                           wgpu::VertexFormat* dawn_format_ptr) {
  wgpu::VertexFormat& dawn_format = *dawn_format_ptr;
  switch (amber_format.GetFormatType()) {
    case FormatType::kR32_SFLOAT:
      dawn_format = wgpu::VertexFormat::Float32;
      break;
    case FormatType::kR32G32_SFLOAT:
      dawn_format = wgpu::VertexFormat::Float32x2;
      break;
    case FormatType::kR32G32B32_SFLOAT:
      dawn_format = wgpu::VertexFormat::Float32x3;
      break;
    case FormatType::kR32G32B32A32_SFLOAT:
      dawn_format = wgpu::VertexFormat::Float32x4;
      break;
    case FormatType::kR8G8_SNORM:
      dawn_format = wgpu::VertexFormat::Snorm8x2;
      break;
    case FormatType::kR8G8B8A8_UNORM:
      dawn_format = wgpu::VertexFormat::Unorm8x4;
      break;
    case FormatType::kR8G8B8A8_SNORM:
      dawn_format = wgpu::VertexFormat::Snorm8x4;
      break;
    default:
      return Result(
          "Amber vertex format " +
          std::to_string(static_cast<uint32_t>(amber_format.GetFormatType())) +
          " is invalid for Dawn or is not supported in amber-dawn");
  }
  return {};
}

// Converts an Amber format to a Dawn Index format, and sends the result out
// through |dawn_format_ptr|.  If the conversion fails, return an error
// result.
Result GetDawnIndexFormat(const ::amber::Format& amber_format,
                          wgpu::IndexFormat* dawn_format_ptr) {
  wgpu::IndexFormat& dawn_format = *dawn_format_ptr;
  switch (amber_format.GetFormatType()) {
    case FormatType::kR16_UINT:
      dawn_format = wgpu::IndexFormat::Uint16;
      break;
    case FormatType::kR32_UINT:
      dawn_format = wgpu::IndexFormat::Uint32;
      break;
    default:
      return Result(
          "Amber index format " +
          std::to_string(static_cast<uint32_t>(amber_format.GetFormatType())) +
          " is invalid for Dawn");
  }
  return {};
}

// Converts an Amber topology to a Dawn topology, and sends the result out
// through |dawn_topology_ptr|. It the conversion fails, return an error result.
Result GetDawnTopology(const ::amber::Topology& amber_topology,
                       wgpu::PrimitiveTopology* dawn_topology_ptr) {
  wgpu::PrimitiveTopology& dawn_topology = *dawn_topology_ptr;
  switch (amber_topology) {
    case Topology::kPointList:
      dawn_topology = wgpu::PrimitiveTopology::PointList;
      break;
    case Topology::kLineList:
      dawn_topology = wgpu::PrimitiveTopology::LineList;
      break;
    case Topology::kLineStrip:
      dawn_topology = wgpu::PrimitiveTopology::LineStrip;
      break;
    case Topology::kTriangleList:
      dawn_topology = wgpu::PrimitiveTopology::TriangleList;
      break;
    case Topology::kTriangleStrip:
      dawn_topology = wgpu::PrimitiveTopology::TriangleStrip;
      break;
    default:
      return Result("Amber PrimitiveTopology " +
                    std::to_string(static_cast<uint32_t>(amber_topology)) +
                    " is not supported in Dawn");
  }
  return {};
}

wgpu::CompareFunction GetDawnCompareOp(::amber::CompareOp op) {
  switch (op) {
    case CompareOp::kNever:
      return wgpu::CompareFunction::Never;
    case CompareOp::kLess:
      return wgpu::CompareFunction::Less;
    case CompareOp::kEqual:
      return wgpu::CompareFunction::Equal;
    case CompareOp::kLessOrEqual:
      return wgpu::CompareFunction::LessEqual;
    case CompareOp::kGreater:
      return wgpu::CompareFunction::Greater;
    case CompareOp::kNotEqual:
      return wgpu::CompareFunction::NotEqual;
    case CompareOp::kGreaterOrEqual:
      return wgpu::CompareFunction::GreaterEqual;
    case CompareOp::kAlways:
      return wgpu::CompareFunction::Always;
    default:
      return wgpu::CompareFunction::Never;
  }
}

wgpu::StencilOperation GetDawnStencilOp(::amber::StencilOp op) {
  switch (op) {
    case StencilOp::kKeep:
      return wgpu::StencilOperation::Keep;
    case StencilOp::kZero:
      return wgpu::StencilOperation::Zero;
    case StencilOp::kReplace:
      return wgpu::StencilOperation::Replace;
    case StencilOp::kIncrementAndClamp:
      return wgpu::StencilOperation::IncrementClamp;
    case StencilOp::kDecrementAndClamp:
      return wgpu::StencilOperation::DecrementClamp;
    case StencilOp::kInvert:
      return wgpu::StencilOperation::Invert;
    case StencilOp::kIncrementAndWrap:
      return wgpu::StencilOperation::IncrementWrap;
    case StencilOp::kDecrementAndWrap:
      return wgpu::StencilOperation::DecrementWrap;
    default:
      return wgpu::StencilOperation::Keep;
  }
}

wgpu::BlendFactor GetDawnBlendFactor(::amber::BlendFactor factor) {
  switch (factor) {
    case BlendFactor::kZero:
      return wgpu::BlendFactor::Zero;
    case BlendFactor::kOne:
      return wgpu::BlendFactor::One;
    case BlendFactor::kSrcColor:
      return wgpu::BlendFactor::Src;
    case BlendFactor::kOneMinusSrcColor:
      return wgpu::BlendFactor::OneMinusSrc;
    case BlendFactor::kDstColor:
      return wgpu::BlendFactor::Dst;
    case BlendFactor::kOneMinusDstColor:
      return wgpu::BlendFactor::OneMinusDst;
    case BlendFactor::kSrcAlpha:
      return wgpu::BlendFactor::SrcAlpha;
    case BlendFactor::kOneMinusSrcAlpha:
      return wgpu::BlendFactor::OneMinusSrcAlpha;
    case BlendFactor::kDstAlpha:
      return wgpu::BlendFactor::DstAlpha;
    case BlendFactor::kOneMinusDstAlpha:
      return wgpu::BlendFactor::OneMinusDstAlpha;
    case BlendFactor::kSrcAlphaSaturate:
      return wgpu::BlendFactor::SrcAlphaSaturated;
    default:
      assert(false && "Dawn::Unknown BlendFactor");
      return wgpu::BlendFactor::One;
  }
}

wgpu::BlendOperation GetDawnBlendOperation(BlendOp op) {
  switch (op) {
    case BlendOp::kAdd:
      return wgpu::BlendOperation::Add;
    case BlendOp::kSubtract:
      return wgpu::BlendOperation::Subtract;
    case BlendOp::kReverseSubtract:
      return wgpu::BlendOperation::ReverseSubtract;
    case BlendOp::kMin:
      return wgpu::BlendOperation::Min;
    case BlendOp::kMax:
      return wgpu::BlendOperation::Max;
    default:
      assert(false && "Dawn::Unknown BlendOp");
      return wgpu::BlendOperation::Add;
  }
}

wgpu::ColorWriteMask GetDawnColorWriteMask(uint8_t amber_color_write_mask) {
  wgpu::ColorWriteMask result = wgpu::ColorWriteMask::None;
  if (amber_color_write_mask & 0x00000001) {
    result |= wgpu::ColorWriteMask::Red;
  }
  if (amber_color_write_mask & 0x00000002) {
    result |= wgpu::ColorWriteMask::Green;
  }
  if (amber_color_write_mask & 0x00000004) {
    result |= wgpu::ColorWriteMask::Blue;
  }
  if (amber_color_write_mask & 0x00000008) {
    result |= wgpu::ColorWriteMask::Alpha;
  }
  return result;
}

wgpu::FrontFace GetDawnFrontFace(FrontFace amber_front_face) {
  return amber_front_face == FrontFace::kClockwise ? wgpu::FrontFace::CW
                                                   : wgpu::FrontFace::CCW;
}

wgpu::CullMode GetDawnCullMode(CullMode amber_cull_mode) {
  switch (amber_cull_mode) {
    case CullMode::kNone:
      return wgpu::CullMode::None;
    case CullMode::kFront:
      return wgpu::CullMode::Front;
    case CullMode::kBack:
      return wgpu::CullMode::Back;
    default:
      assert(false && "Dawn::Unknown CullMode");
      return wgpu::CullMode::None;
  }
}

EngineDawn::EngineDawn() : Engine() {}

EngineDawn::~EngineDawn() = default;

Result EngineDawn::Initialize(EngineConfig* config,
                              Delegate*,
                              const std::vector<std::string>&,
                              const std::vector<std::string>&,
                              const std::vector<std::string>&,
                              const std::vector<std::string>&) {
  if (device_) {
    return Result("Dawn:Initialize device_ already exists");
  }

  if (!config) {
    return Result("Dawn::Initialize config is null");
  }
  DawnEngineConfig* dawn_config = static_cast<DawnEngineConfig*>(config);
  if (dawn_config->device == nullptr) {
    return Result("Dawn:Initialize device is a null pointer");
  }

  instance_ = dawn_config->instance;
  device_ = dawn_config->device;

  return {};
}

Result EngineDawn::CreatePipeline(::amber::Pipeline* pipeline) {
  if (!device_) {
    return Result("Dawn::CreatePipeline: device is not created");
  }
  std::unordered_map<ShaderType, wgpu::ShaderModule, CastHash<ShaderType>>
      module_for_type;

  for (const auto& shader_info : pipeline->GetShaders()) {
    ShaderType type = shader_info.GetShaderType();
    const std::vector<uint32_t>& code = shader_info.GetData();

    wgpu::ShaderSourceSPIRV spirv_desc;
    spirv_desc.codeSize = uint32_t(code.size());
    spirv_desc.code = code.data();

    wgpu::ShaderModuleDescriptor descriptor = {};
    descriptor.nextInChain = &spirv_desc;

    auto shader = device_.CreateShaderModule(&descriptor);
    if (!shader) {
      return Result("Dawn::CreatePipeline: failed to create shader");
    }
    if (module_for_type.count(type)) {
      return Result("Dawn::CreatePipeline: module for type already exists");
    }
    module_for_type[type] = shader;
  }

  switch (pipeline->GetType()) {
    case PipelineType::kCompute: {
      auto& module = module_for_type[kShaderTypeCompute];
      if (!module) {
        return Result("Dawn::CreatePipeline: no compute shader provided");
      }

      pipeline_map_[pipeline].compute_pipeline.reset(
          new ComputePipelineInfo(pipeline, module));
      Result result =
          AttachBuffers(pipeline_map_[pipeline].compute_pipeline.get());
      if (!result.IsSuccess()) {
        return result;
      }
      break;
    }

    case PipelineType::kGraphics: {
      // TODO(dneto): Handle other shader types as well.  They are optional.
      auto& vs = module_for_type[kShaderTypeVertex];
      auto& fs = module_for_type[kShaderTypeFragment];
      if (!vs) {
        return Result(
            "Dawn::CreatePipeline: no vertex shader provided for graphics "
            "pipeline");
      }
      if (!fs) {
        return Result(
            "Dawn::CreatePipeline: no fragment shader provided for graphics "
            "pipeline");
      }

      pipeline_map_[pipeline].render_pipeline.reset(
          new RenderPipelineInfo(pipeline, vs, fs));
      Result result = AttachBuffersAndTextures(
          pipeline_map_[pipeline].render_pipeline.get());
      if (!result.IsSuccess()) {
        return result;
      }

      break;
    }
    case PipelineType::kRayTracing:
      return Result("Dawn::CreatePipeline: ray tracing not supported");
  }

  return {};
}

Result EngineDawn::DoClearColor(const ClearColorCommand* command) {
  RenderPipelineInfo* render_pipeline = GetRenderPipeline(command);
  if (!render_pipeline) {
    return Result("ClearColor invoked on invalid or missing render pipeline");
  }

  render_pipeline->clear_color_value = wgpu::Color{
      command->GetR(), command->GetG(), command->GetB(), command->GetA()};

  return {};
}

Result EngineDawn::DoClearStencil(const ClearStencilCommand* command) {
  RenderPipelineInfo* render_pipeline = GetRenderPipeline(command);
  if (!render_pipeline) {
    return Result("ClearStencil invoked on invalid or missing render pipeline");
  }

  render_pipeline->clear_stencil_value = command->GetValue();
  return {};
}

Result EngineDawn::DoClearDepth(const ClearDepthCommand* command) {
  RenderPipelineInfo* render_pipeline = GetRenderPipeline(command);
  if (!render_pipeline) {
    return Result("ClearDepth invoked on invalid or missing render pipeline");
  }

  render_pipeline->clear_depth_value = command->GetValue();
  return {};
}

Result EngineDawn::DoClear(const ClearCommand* command) {
  Result result;
  RenderPipelineInfo* render_pipeline = GetRenderPipeline(command);
  if (!render_pipeline) {
    return Result("Clear invoked on invalid or missing render pipeline");
  }

  DawnPipelineHelper helper;
  result = helper.CreateRenderPipelineDescriptor(*render_pipeline, device_,
                                                 false, nullptr);
  if (!result.IsSuccess()) {
    return result;
  }
  result = helper.CreateRenderPassDescriptor(
      *render_pipeline, device_, texture_views_, wgpu::LoadOp::Clear);
  if (!result.IsSuccess()) {
    return result;
  }

  wgpu::RenderPassDescriptor* renderPassDescriptor =
      &helper.renderPassDescriptor;
  wgpu::CommandEncoderDescriptor encoder_desc = {};
  wgpu::CommandEncoder encoder = device_.CreateCommandEncoder(&encoder_desc);
  wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(renderPassDescriptor);
  pass.End();

  wgpu::CommandBufferDescriptor command_buffer_desc = {};
  wgpu::CommandBuffer commands = encoder.Finish(&command_buffer_desc);
  wgpu::Queue queue = device_.GetQueue();
  queue.Submit(1, &commands);

  result = MapDeviceTextureToHostBuffer(*render_pipeline, device_);

  return result;
}

// Creates a Dawn render pipeline descriptor for the given pipeline on the given
// device. When |ignore_vertex_and_Index_buffers| is true, ignores the vertex
// and index buffers attached to |render_pipeline| and instead configures the
// resulting descriptor to have a single vertex buffer with an attribute format
// of Float4 and input stride of 4*sizeof(float)
Result DawnPipelineHelper::CreateRenderPipelineDescriptor(
    const RenderPipelineInfo& render_pipeline,
    const wgpu::Device& device,
    const bool ignore_vertex_and_Index_buffers,
    const PipelineData* pipeline_data) {
  Result result;

  auto* amber_format =
      render_pipeline.pipeline->GetColorAttachments()[0].buffer->GetFormat();
  if (!amber_format) {
    return Result("Color attachment 0 has no format!");
  }
  wgpu::TextureFormat fb_format{};
  result = GetDawnTextureFormat(*amber_format, &fb_format);
  if (!result.IsSuccess()) {
    return result;
  }

  wgpu::TextureFormat depth_stencil_format{};
  auto* depthBuffer = render_pipeline.pipeline->GetDepthStencilBuffer().buffer;
  if (depthBuffer) {
    auto* amber_depth_stencil_format = depthBuffer->GetFormat();
    if (!amber_depth_stencil_format) {
      return Result("The depth/stencil attachment has no format!");
    }
    result = GetDawnTextureFormat(*amber_depth_stencil_format,
                                  &depth_stencil_format);
    if (!result.IsSuccess()) {
      return result;
    }
  } else {
    depth_stencil_format = wgpu::TextureFormat::Depth24PlusStencil8;
  }

  renderPipelineDescriptor.layout =
      MakeBasicPipelineLayout(device, render_pipeline.bind_group_layouts);

  primitiveState.topology = wgpu::PrimitiveTopology::TriangleList;
  renderPipelineDescriptor.primitive = primitiveState;
  multisampleState.count = 1;
  renderPipelineDescriptor.multisample = multisampleState;

  // Lookup shaders' entrypoints
  for (const auto& shader_info : render_pipeline.pipeline->GetShaders()) {
    if (shader_info.GetShaderType() == kShaderTypeVertex) {
      vertexEntryPoint = shader_info.GetEntryPoint();
    } else if (shader_info.GetShaderType() == kShaderTypeFragment) {
      fragmentEntryPoint = shader_info.GetEntryPoint();
    } else {
      return Result(
          "CreateRenderPipelineDescriptor: An unknown shader is attached to "
          "the render pipeline");
    }
  }
  // Fill the default values for vertexInput (buffers and attributes).
  // assuming #buffers == #attributes
  vertexInputDescriptor.state.bufferCount =
      render_pipeline.pipeline->GetVertexBuffers().size();

  for (uint32_t i = 0; i < kMaxVertexInputs; ++i) {
    if (ignore_vertex_and_Index_buffers) {
      if (i == 0) {
        vertexInputDescriptor.state.bufferCount = 1;
        vertexInputDescriptor.cBuffers[0].attributeCount = 1;
        vertexInputDescriptor.cBuffers[0].arrayStride = 4 * sizeof(float);
        vertexInputDescriptor.cBuffers[0].attributes =
            &vertexInputDescriptor.cAttributes[0];

        vertexInputDescriptor.cAttributes[0].shaderLocation = 0;
        vertexInputDescriptor.cAttributes[0].format =
            wgpu::VertexFormat::Float32x4;
      }
    } else {
      if (i < render_pipeline.pipeline->GetVertexBuffers().size()) {
        vertexInputDescriptor.cBuffers[i].attributeCount = 1;
        vertexInputDescriptor.cBuffers[i].arrayStride =
            render_pipeline.pipeline->GetVertexBuffers()[i]
                .buffer->GetElementStride();
        vertexInputDescriptor.cBuffers[i].stepMode =
            wgpu::VertexStepMode::Vertex;
        vertexInputDescriptor.cBuffers[i].attributes =
            &vertexInputDescriptor.cAttributes[i];

        vertexInputDescriptor.cAttributes[i].shaderLocation = i;
        auto* amber_vertex_format =
            render_pipeline.pipeline->GetVertexBuffers()[i].buffer->GetFormat();
        result = GetDawnVertexFormat(
            *amber_vertex_format, &vertexInputDescriptor.cAttributes[i].format);
        if (!result.IsSuccess()) {
          return result;
        }
      }
    }
  }

  renderPipelineDescriptor.vertex = vertexInputDescriptor.state;

  // Set defaults for the vertex stage descriptor.
  renderPipelineDescriptor.vertex.module = render_pipeline.vertex_shader;
  renderPipelineDescriptor.vertex.entryPoint = vertexEntryPoint.c_str();

  // Set defaults for the fragment stage descriptor.
  fragmentState.module = render_pipeline.fragment_shader;
  fragmentState.entryPoint = fragmentEntryPoint.c_str();
  renderPipelineDescriptor.fragment = &fragmentState;

  // Set defaults for the primitive state descriptor.
  if (pipeline_data != nullptr) {
    primitiveState.frontFace = GetDawnFrontFace(pipeline_data->GetFrontFace());
    primitiveState.cullMode = GetDawnCullMode(pipeline_data->GetCullMode());
    renderPipelineDescriptor.primitive = primitiveState;
  }

  // Set defaults for the color state descriptors.
  if (pipeline_data == nullptr) {
    fragmentState.targetCount =
        render_pipeline.pipeline->GetColorAttachments().size();
    colorBlends[0].color.operation = wgpu::BlendOperation::Add;
    colorBlends[0].color.srcFactor = wgpu::BlendFactor::One;
    colorBlends[0].color.dstFactor = wgpu::BlendFactor::Zero;
    colorBlends[0].alpha.operation = wgpu::BlendOperation::Add;
    colorBlends[0].alpha.srcFactor = wgpu::BlendFactor::One;
    colorBlends[0].alpha.dstFactor = wgpu::BlendFactor::Zero;
    colorTargetStates[0].writeMask = wgpu::ColorWriteMask::All;
    colorTargetStates[0].format = fb_format;
    colorTargetStates[0].blend = &colorBlends[0];
  } else {
    fragmentState.targetCount =
        render_pipeline.pipeline->GetColorAttachments().size();

    colorBlends[0].alpha.operation =
        GetDawnBlendOperation(pipeline_data->GetAlphaBlendOp());
    colorBlends[0].alpha.srcFactor =
        GetDawnBlendFactor(pipeline_data->GetSrcAlphaBlendFactor());
    colorBlends[0].alpha.dstFactor =
        GetDawnBlendFactor(pipeline_data->GetDstAlphaBlendFactor());

    colorBlends[0].color.operation =
        GetDawnBlendOperation(pipeline_data->GetColorBlendOp());
    colorBlends[0].color.srcFactor =
        GetDawnBlendFactor(pipeline_data->GetSrcColorBlendFactor());
    colorBlends[0].color.dstFactor =
        GetDawnBlendFactor(pipeline_data->GetDstColorBlendFactor());

    colorTargetStates[0].writeMask =
        GetDawnColorWriteMask(pipeline_data->GetColorWriteMask());

    colorTargetStates[0].format = fb_format;
    colorTargetStates[0].blend = &colorBlends[0];
  }

  for (uint32_t i = 0; i < kMaxColorAttachments; ++i) {
    wgpu::TextureFormat fb_format{};
    {
      if (i < render_pipeline.pipeline->GetColorAttachments().size()) {
        auto* amber_format = render_pipeline.pipeline->GetColorAttachments()[i]
                                 .buffer->GetFormat();
        if (!amber_format) {
          return Result(
              "AttachBuffersAndTextures: One Color attachment has no "
              "format!");
        }
        result = GetDawnTextureFormat(*amber_format, &fb_format);
        if (!result.IsSuccess()) {
          return result;
        }
      } else {
        fb_format = wgpu::TextureFormat::RGBA8Unorm;
      }
    }
    colorTargetStates[i] = colorTargetStates[0];
    colorTargetStates[i].format = fb_format;
  }
  fragmentState.targets = colorTargetStates;

  // Set defaults for the depth stencil state descriptors.
  depthStencilState.format = depth_stencil_format;
  depthStencilState.depthWriteEnabled = wgpu::OptionalBool::False;
  depthStencilState.depthCompare = wgpu::CompareFunction::Always;
  depthStencilState.stencilBack.compare = wgpu::CompareFunction::Always;
  depthStencilState.stencilBack.failOp = wgpu::StencilOperation::Keep;
  depthStencilState.stencilBack.depthFailOp = wgpu::StencilOperation::Keep;
  depthStencilState.stencilBack.passOp = wgpu::StencilOperation::Keep;
  depthStencilState.stencilFront = depthStencilState.stencilBack;
  depthStencilState.stencilReadMask = 0xff;
  depthStencilState.stencilWriteMask = 0xff;

  if (pipeline_data != nullptr) {
    depthStencilState.stencilFront.compare =
        GetDawnCompareOp(pipeline_data->GetFrontCompareOp());
    depthStencilState.stencilFront.failOp =
        GetDawnStencilOp(pipeline_data->GetFrontFailOp());
    depthStencilState.stencilFront.depthFailOp =
        GetDawnStencilOp(pipeline_data->GetFrontDepthFailOp());
    depthStencilState.stencilFront.passOp =
        GetDawnStencilOp(pipeline_data->GetFrontPassOp());

    depthStencilState.stencilBack.compare =
        GetDawnCompareOp(pipeline_data->GetBackCompareOp());
    depthStencilState.stencilBack.failOp =
        GetDawnStencilOp(pipeline_data->GetBackFailOp());
    depthStencilState.stencilBack.depthFailOp =
        GetDawnStencilOp(pipeline_data->GetBackDepthFailOp());
    depthStencilState.stencilBack.passOp =
        GetDawnStencilOp(pipeline_data->GetBackPassOp());

    depthStencilState.depthWriteEnabled = pipeline_data->GetEnableDepthWrite()
                                              ? wgpu::OptionalBool::True
                                              : wgpu::OptionalBool::False;
    depthStencilState.depthCompare =
        GetDawnCompareOp(pipeline_data->GetDepthCompareOp());

    depthStencilState.stencilReadMask =
        (pipeline_data->GetFrontCompareMask() ==
         pipeline_data->GetBackCompareMask())
            ? pipeline_data->GetFrontCompareMask()
            : 0xff;
    depthStencilState.stencilWriteMask = (pipeline_data->GetBackWriteMask() ==
                                          pipeline_data->GetFrontWriteMask())
                                             ? pipeline_data->GetBackWriteMask()
                                             : 0xff;

    depthStencilState.depthBias = pipeline_data->GetEnableDepthBias();
    depthStencilState.depthBiasSlopeScale =
        pipeline_data->GetDepthBiasSlopeFactor();
    depthStencilState.depthBiasClamp = pipeline_data->GetDepthBiasClamp();
  }
  renderPipelineDescriptor.depthStencil = &depthStencilState;

  return {};
}

Result DawnPipelineHelper::CreateRenderPassDescriptor(
    const RenderPipelineInfo& render_pipeline,
    const wgpu::Device& device,
    const std::vector<wgpu::TextureView>& texture_view,
    const wgpu::LoadOp load_op) {
  for (uint32_t i = 0; i < kMaxColorAttachments; ++i) {
    colorAttachmentsInfo[i].loadOp = load_op;
    colorAttachmentsInfo[i].storeOp = wgpu::StoreOp::Store;
    colorAttachmentsInfo[i].clearValue = render_pipeline.clear_color_value;
    colorAttachmentsInfo[i].depthSlice = wgpu::kDepthSliceUndefined;
  }

  depthStencilAttachmentInfo.depthClearValue =
      render_pipeline.clear_depth_value;
  depthStencilAttachmentInfo.stencilClearValue =
      render_pipeline.clear_stencil_value;
  depthStencilAttachmentInfo.depthLoadOp = load_op;
  depthStencilAttachmentInfo.depthStoreOp = wgpu::StoreOp::Store;
  depthStencilAttachmentInfo.stencilLoadOp = load_op;
  depthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Store;

  renderPassDescriptor.colorAttachmentCount =
      render_pipeline.pipeline->GetColorAttachments().size();
  uint32_t colorAttachmentIndex = 0;
  for (const wgpu::TextureView& colorAttachment : texture_view) {
    if (colorAttachment != nullptr) {
      colorAttachmentsInfo[colorAttachmentIndex].view = colorAttachment;
    }
    ++colorAttachmentIndex;
  }
  renderPassDescriptor.colorAttachments = colorAttachmentsInfo;

  wgpu::TextureFormat depth_stencil_format{};
  auto* depthBuffer = render_pipeline.pipeline->GetDepthStencilBuffer().buffer;
  if (depthBuffer) {
    auto* amber_depth_stencil_format = depthBuffer->GetFormat();
    if (!amber_depth_stencil_format) {
      return Result("The depth/stencil attachment has no format!");
    }
    Result result = GetDawnTextureFormat(*amber_depth_stencil_format,
                                         &depth_stencil_format);
    if (!result.IsSuccess()) {
      return result;
    }
  } else {
    depth_stencil_format = wgpu::TextureFormat::Depth24PlusStencil8;
  }

  depthStencilDescriptor.size.width =
      render_pipeline.pipeline->GetFramebufferWidth();
  depthStencilDescriptor.size.height =
      render_pipeline.pipeline->GetFramebufferHeight();
  depthStencilDescriptor.size.depthOrArrayLayers = 1;
  depthStencilDescriptor.sampleCount = 1;
  depthStencilDescriptor.format = depth_stencil_format;
  depthStencilDescriptor.usage =
      wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
  depthStencilTexture = device.CreateTexture(&depthStencilDescriptor);

  wgpu::TextureViewDescriptor depth_view_desc = {};
  depthStencilView = depthStencilTexture.CreateView(&depth_view_desc);

  if (depthStencilView != nullptr) {
    depthStencilAttachmentInfo.view = depthStencilView;
    renderPassDescriptor.depthStencilAttachment = &depthStencilAttachmentInfo;
  } else {
    renderPassDescriptor.depthStencilAttachment = nullptr;
  }

  return {};
}

Result EngineDawn::DoDrawRect(const DrawRectCommand* command) {
  RenderPipelineInfo* render_pipeline = GetRenderPipeline(command);
  if (!render_pipeline) {
    return Result("DrawRect invoked on invalid or missing render pipeline");
  }
  if (render_pipeline->vertex_buffers.size() > 1) {
    return Result(
        "DrawRect invoked on a render pipeline with more than one "
        "VERTEX_DATA attached");
  }

  float x = command->GetX();
  float y = command->GetY();
  float rectangleWidth = command->GetWidth();
  float rectangleHeight = command->GetHeight();

  const uint32_t frameWidth = render_pipeline->pipeline->GetFramebufferWidth();
  const uint32_t frameHeight =
      render_pipeline->pipeline->GetFramebufferHeight();

  if (command->IsOrtho()) {
    x = ((x / frameWidth) * 2.0f) - 1.0f;
    y = ((y / frameHeight) * 2.0f) - 1.0f;
    rectangleWidth = (rectangleWidth / frameWidth) * 2.0f;
    rectangleHeight = (rectangleHeight / frameHeight) * 2.0f;
  }

  static const uint32_t indexData[3 * 2] = {
      0, 1, 2, 0, 2, 3,
  };
  auto index_buffer = CreateBufferFromData(
      device_, indexData, sizeof(indexData), wgpu::BufferUsage::Index);

  const float vertexData[4 * 4] = {
      // Bottom left
      x,
      y + rectangleHeight,
      0.0f,
      1.0f,
      // Top left
      x,
      y,
      0.0f,
      1.0f,
      // Top right
      x + rectangleWidth,
      y,
      0.0f,
      1.0f,
      // Bottom right
      x + rectangleWidth,
      y + rectangleHeight,
      0.0f,
      1.0f,
  };

  auto vertex_buffer = CreateBufferFromData(
      device_, vertexData, sizeof(vertexData), wgpu::BufferUsage::Vertex);
  DawnPipelineHelper helper;
  helper.CreateRenderPipelineDescriptor(*render_pipeline, device_, true,
                                        command->GetPipelineData());
  helper.CreateRenderPassDescriptor(*render_pipeline, device_, texture_views_,
                                    wgpu::LoadOp::Load);
  wgpu::RenderPipelineDescriptor* renderPipelineDescriptor =
      &helper.renderPipelineDescriptor;
  wgpu::RenderPassDescriptor* renderPassDescriptor =
      &helper.renderPassDescriptor;

  const wgpu::RenderPipeline pipeline =
      device_.CreateRenderPipeline(renderPipelineDescriptor);
  wgpu::CommandEncoderDescriptor encoder_desc = {};
  wgpu::CommandEncoder encoder = device_.CreateCommandEncoder(&encoder_desc);
  wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(renderPassDescriptor);
  pass.SetPipeline(pipeline);
  for (uint32_t i = 0; i < render_pipeline->bind_groups.size(); i++) {
    if (render_pipeline->bind_groups[i]) {
      pass.SetBindGroup(i, render_pipeline->bind_groups[i], 0, nullptr);
    }
  }
  pass.SetVertexBuffer(0, vertex_buffer, 0, sizeof(vertexData));
  pass.SetIndexBuffer(index_buffer, wgpu::IndexFormat::Uint32, 0,
                      sizeof(indexData));
  pass.DrawIndexed(6, 1, 0, 0, 0);
  pass.End();

  wgpu::CommandBufferDescriptor command_buffer_desc = {};
  wgpu::CommandBuffer commands = encoder.Finish(&command_buffer_desc);
  wgpu::Queue queue = device_.GetQueue();
  queue.Submit(1, &commands);

  Result result = MapDeviceTextureToHostBuffer(*render_pipeline, device_);

  return result;
}

Result EngineDawn::DoDrawGrid(const DrawGridCommand* /* command */) {
  return Result("DRAW_GRID not implemented on Dawn");
}

Result EngineDawn::DoDrawArrays(const DrawArraysCommand* command) {
  Result result;

  RenderPipelineInfo* render_pipeline = GetRenderPipeline(command);
  if (!render_pipeline) {
    return Result("DrawArrays invoked on invalid or missing render pipeline");
  }

  if (command->IsIndexed()) {
    if (!render_pipeline->index_buffer) {
      return Result("DrawArrays: Draw indexed is used without given indices");
    }
  } else {
    std::vector<uint32_t> indexData;
    for (uint32_t i = 0;
         i < command->GetFirstVertexIndex() + command->GetVertexCount(); i++) {
      indexData.emplace_back(i);
    }
    render_pipeline->index_buffer = CreateBufferFromData(
        device_, indexData.data(), indexData.size() * sizeof(uint32_t),
        wgpu::BufferUsage::Index);
  }

  uint32_t instance_count = command->GetInstanceCount();
  if (instance_count == 0 && command->GetVertexCount() != 0) {
    instance_count = 1;
  }

  DawnPipelineHelper helper;
  result = helper.CreateRenderPipelineDescriptor(
      *render_pipeline, device_, false, command->GetPipelineData());
  if (!result.IsSuccess()) {
    return result;
  }
  result = helper.CreateRenderPassDescriptor(
      *render_pipeline, device_, texture_views_, wgpu::LoadOp::Load);
  if (!result.IsSuccess()) {
    return result;
  }

  wgpu::RenderPipelineDescriptor* renderPipelineDescriptor =
      &helper.renderPipelineDescriptor;
  wgpu::RenderPassDescriptor* renderPassDescriptor =
      &helper.renderPassDescriptor;

  result = GetDawnTopology(command->GetTopology(),
                           &renderPipelineDescriptor->primitive.topology);
  if (!result.IsSuccess()) {
    return result;
  }

  const wgpu::RenderPipeline pipeline =
      device_.CreateRenderPipeline(renderPipelineDescriptor);
  wgpu::CommandEncoderDescriptor encoder_desc = {};
  wgpu::CommandEncoder encoder = device_.CreateCommandEncoder(&encoder_desc);
  wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(renderPassDescriptor);
  pass.SetPipeline(pipeline);
  for (uint32_t i = 0; i < render_pipeline->bind_groups.size(); i++) {
    if (render_pipeline->bind_groups[i]) {
      pass.SetBindGroup(i, render_pipeline->bind_groups[i], 0, nullptr);
    }
  }
  // TODO(sarahM0): figure out what are startSlot, count and offsets
  for (uint32_t i = 0; i < render_pipeline->vertex_buffers.size(); i++) {
    pass.SetVertexBuffer(i,                                  /* slot */
                         render_pipeline->vertex_buffers[i], /* buffer */
                         0,                                  /* offset */
                         wgpu::kWholeSize);
  }
  // TODO(sarahM0): figure out what this offset means
  wgpu::IndexFormat indexFormat =
      renderPipelineDescriptor->primitive.stripIndexFormat;
  if (indexFormat == wgpu::IndexFormat::Undefined) {
    indexFormat = wgpu::IndexFormat::Uint32;
  }
  pass.SetIndexBuffer(render_pipeline->index_buffer, /* buffer */
                      indexFormat, 0,                /* offset*/
                      wgpu::kWholeSize);
  pass.DrawIndexed(command->GetVertexCount(),      /* indexCount */
                   instance_count,                 /* instanceCount */
                   0,                              /* firstIndex */
                   command->GetFirstVertexIndex(), /* baseVertex */
                   0 /* firstInstance */);

  pass.End();
  wgpu::CommandBufferDescriptor command_buffer_desc = {};
  wgpu::CommandBuffer commands = encoder.Finish(&command_buffer_desc);
  wgpu::Queue queue = device_.GetQueue();
  queue.Submit(1, &commands);

  result = MapDeviceTextureToHostBuffer(*render_pipeline, device_);

  return result;
}

Result EngineDawn::DoCompute(const ComputeCommand* command) {
  Result result;

  ComputePipelineInfo* compute_pipeline = GetComputePipeline(command);
  if (!compute_pipeline) {
    return Result("DoComput: invoked on invalid or missing compute pipeline");
  }

  wgpu::ComputePipelineDescriptor computePipelineDescriptor = {};
  computePipelineDescriptor.layout =
      MakeBasicPipelineLayout(device_, compute_pipeline->bind_group_layouts);

  computePipelineDescriptor.compute.module = compute_pipeline->compute_shader;
  computePipelineDescriptor.compute.entryPoint = "main";
  wgpu::ComputePipeline pipeline =
      device_.CreateComputePipeline(&computePipelineDescriptor);
  wgpu::CommandEncoderDescriptor encoder_desc = {};
  wgpu::CommandEncoder encoder = device_.CreateCommandEncoder(&encoder_desc);
  wgpu::ComputePassDescriptor pass_desc = {};
  wgpu::ComputePassEncoder pass = encoder.BeginComputePass(&pass_desc);
  pass.SetPipeline(pipeline);
  for (uint32_t i = 0; i < compute_pipeline->bind_groups.size(); i++) {
    if (compute_pipeline->bind_groups[i]) {
      pass.SetBindGroup(i, compute_pipeline->bind_groups[i], 0, nullptr);
    }
  }
  pass.DispatchWorkgroups(command->GetX(), command->GetY(), command->GetZ());
  pass.End();
  // Finish recording the command buffer.  It only has one command.
  wgpu::CommandBufferDescriptor command_buffer_desc = {};
  auto command_buffer = encoder.Finish(&command_buffer_desc);
  // Submit the command.
  auto queue = device_.GetQueue();
  queue.Submit(1, &command_buffer);
  // Copy result back
  result = MapDeviceBufferToHostBuffer(*compute_pipeline, device_);

  return result;
}

Result EngineDawn::DoTraceRays(const RayTracingCommand* /* command */) {
  return Result("Dawn: Trace Rays not yet implemented");
}

Result EngineDawn::DoEntryPoint(const EntryPointCommand*) {
  return Result("Dawn: Entry point must be \"main\" in Dawn");
}

Result EngineDawn::DoPatchParameterVertices(
    const PatchParameterVerticesCommand*) {
  return Result("Dawn: PatchParameterVertices is not supported in Dawn");
}

Result EngineDawn::DoBuffer(const BufferCommand* command) {
  Result result;

  wgpu::Buffer* dawn_buffer = nullptr;

  const auto descriptor_set = command->GetDescriptorSet();
  const auto binding = command->GetBinding();

  RenderPipelineInfo* render_pipeline = GetRenderPipeline(command);
  if (render_pipeline) {
    auto where = render_pipeline->buffer_map.find({descriptor_set, binding});
    if (where != render_pipeline->buffer_map.end()) {
      const auto dawn_buffer_index = where->second;
      dawn_buffer = &render_pipeline->buffers[dawn_buffer_index];
    }
  }

  ComputePipelineInfo* compute_pipeline = GetComputePipeline(command);
  if (compute_pipeline) {
    auto where = compute_pipeline->buffer_map.find({descriptor_set, binding});
    if (where != compute_pipeline->buffer_map.end()) {
      const auto dawn_buffer_index = where->second;
      dawn_buffer = &compute_pipeline->buffers[dawn_buffer_index];
    }
  }

  if (!render_pipeline && !compute_pipeline) {
    return Result("DoBuffer: invoked on invalid or missing pipeline");
  }
  if (!command->IsSSBO() && !command->IsUniform()) {
    return Result("DoBuffer: only supports SSBO and uniform buffer type");
  }
  if (!dawn_buffer) {
    return Result("DoBuffer: no Dawn buffer at descriptor set " +
                  std::to_string(descriptor_set) + " and binding " +
                  std::to_string(binding));
  }

  Buffer* amber_buffer = command->GetBuffer();
  if (amber_buffer) {
    amber_buffer->SetDataWithOffset(command->GetValues(), command->GetOffset());

    wgpu::Queue queue = device_.GetQueue();
    queue.WriteBuffer(*dawn_buffer, 0, amber_buffer->ValuePtr()->data(),
                      amber_buffer->GetMaxSizeInBytes());
  }

  return {};
}

Result EngineDawn::AttachBuffersAndTextures(
    RenderPipelineInfo* render_pipeline) {
  Result result;
  const uint32_t width = render_pipeline->pipeline->GetFramebufferWidth();
  const uint32_t height = render_pipeline->pipeline->GetFramebufferHeight();

  // Create textures and texture views if we haven't already
  std::vector<int32_t> seen_idx(
      render_pipeline->pipeline->GetColorAttachments().size(), -1);
  for (auto info : render_pipeline->pipeline->GetColorAttachments()) {
    if (info.location >=
        render_pipeline->pipeline->GetColorAttachments().size()) {
      return Result("color attachment locations must be sequential from 0");
    }
    if (seen_idx[info.location] != -1) {
      return Result("duplicate attachment location: " +
                    std::to_string(info.location));
    }
    seen_idx[info.location] = static_cast<int32_t>(info.location);
  }

  if (textures_.size() == 0) {
    for (uint32_t i = 0; i < kMaxColorAttachments; i++) {
      wgpu::TextureFormat fb_format{};

      if (i < render_pipeline->pipeline->GetColorAttachments().size()) {
        auto* amber_format = render_pipeline->pipeline->GetColorAttachments()[i]
                                 .buffer->GetFormat();
        if (!amber_format) {
          return Result(
              "AttachBuffersAndTextures: One Color attachment has no "
              "format!");
        }
        result = GetDawnTextureFormat(*amber_format, &fb_format);
        if (!result.IsSuccess()) {
          return result;
        }
      } else {
        fb_format = wgpu::TextureFormat::RGBA8Unorm;
      }

      textures_.emplace_back(
          MakeDawnTexture(device_, fb_format, width, height));

      wgpu::TextureViewDescriptor view_desc = {};
      texture_views_.emplace_back(textures_.back().CreateView(&view_desc));
    }
  }

  // Attach depth-stencil texture
  auto* depthBuffer = render_pipeline->pipeline->GetDepthStencilBuffer().buffer;
  if (depthBuffer) {
    if (!depth_stencil_texture_) {
      auto* amber_depth_stencil_format = depthBuffer->GetFormat();
      if (!amber_depth_stencil_format) {
        return Result(
            "AttachBuffersAndTextures: The depth/stencil attachment has no "
            "format!");
      }
      wgpu::TextureFormat depth_stencil_format{};
      result = GetDawnTextureFormat(*amber_depth_stencil_format,
                                    &depth_stencil_format);
      if (!result.IsSuccess()) {
        return result;
      }

      result = MakeTexture(device_, depth_stencil_format, width, height,
                           &depth_stencil_texture_);
      if (!result.IsSuccess()) {
        return result;
      }
      render_pipeline->depth_stencil_texture = depth_stencil_texture_;
    } else {
      render_pipeline->depth_stencil_texture = depth_stencil_texture_;
    }
  }

  // Attach index buffer
  if (render_pipeline->pipeline->GetIndexBuffer()) {
    render_pipeline->index_buffer = CreateBufferFromData(
        device_,
        render_pipeline->pipeline->GetIndexBuffer()->ValuePtr()->data(),
        render_pipeline->pipeline->GetIndexBuffer()->GetSizeInBytes(),
        wgpu::BufferUsage::Index);
  }

  // Attach vertex buffers
  for (auto& vertex_info : render_pipeline->pipeline->GetVertexBuffers()) {
    render_pipeline->vertex_buffers.emplace_back(CreateBufferFromData(
        device_, vertex_info.buffer->ValuePtr()->data(),
        vertex_info.buffer->GetSizeInBytes(), wgpu::BufferUsage::Vertex));
  }

  // Do not attach pushConstants
  if (render_pipeline->pipeline->GetPushConstantBuffer().buffer != nullptr) {
    return Result(
        "AttachBuffersAndTextures: Dawn does not support push constants!");
  }

  wgpu::ShaderStage kAllStages =
      wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
  std::vector<std::vector<BindingInitializationHelper>> bindingInitalizerHelper(
      kMaxDawnBindGroup);
  std::vector<std::vector<wgpu::BindGroupLayoutEntry>> layouts_info(
      kMaxDawnBindGroup);
  uint32_t max_descriptor_set = 0;

  // Attach storage/uniform buffers
  wgpu::BindGroupLayoutEntry empty_layout_info = {};

  if (!render_pipeline->pipeline->GetBuffers().empty()) {
    for (auto& buf_info : render_pipeline->pipeline->GetBuffers()) {
      while (layouts_info[buf_info.descriptor_set].size() <= buf_info.binding) {
        layouts_info[buf_info.descriptor_set].push_back(empty_layout_info);
      }
    }
  }

  for (const auto& buf_info : render_pipeline->pipeline->GetBuffers()) {
    wgpu::BufferUsage bufferUsage;
    wgpu::BufferBindingType bindingType;
    switch (buf_info.type) {
      case BufferType::kStorage: {
        bufferUsage = wgpu::BufferUsage::Storage;
        bindingType = wgpu::BufferBindingType::Storage;
        break;
      }
      case BufferType::kUniform: {
        bufferUsage = wgpu::BufferUsage::Uniform;
        bindingType = wgpu::BufferBindingType::Uniform;
        break;
      }
      default: {
        return Result("AttachBuffersAndTextures: unknown buffer type: " +
                      std::to_string(static_cast<uint32_t>(buf_info.type)));
        break;
      }
    }

    if (buf_info.descriptor_set > kMaxDawnBindGroup - 1) {
      return Result("AttachBuffers: Dawn has a maximum of " +
                    std::to_string(kMaxDawnBindGroup) + " (descriptor sets)");
    }

    render_pipeline->buffers.emplace_back(CreateBufferFromData(
        device_, buf_info.buffer->ValuePtr()->data(),
        buf_info.buffer->GetMaxSizeInBytes(),
        bufferUsage | wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst));

    render_pipeline->buffer_map[{buf_info.descriptor_set, buf_info.binding}] =
        static_cast<uint32_t>(render_pipeline->buffers.size() - 1);

    render_pipeline->used_descriptor_set.insert(buf_info.descriptor_set);
    max_descriptor_set = std::max(max_descriptor_set, buf_info.descriptor_set);

    wgpu::BindGroupLayoutEntry layout_info = empty_layout_info;
    layout_info.binding = buf_info.binding;
    layout_info.visibility = kAllStages;
    layout_info.buffer.type = bindingType;
    layouts_info[buf_info.descriptor_set][buf_info.binding] = layout_info;

    BindingInitializationHelper tempBinding = BindingInitializationHelper(
        buf_info.binding, render_pipeline->buffers.back(), 0,
        buf_info.buffer->GetMaxSizeInBytes());
    bindingInitalizerHelper[buf_info.descriptor_set].push_back(tempBinding);
  }

  for (uint32_t i = 0; i < kMaxDawnBindGroup; i++) {
    if (layouts_info[i].size() > 0 && bindingInitalizerHelper[i].size() > 0) {
      wgpu::BindGroupLayout bindGroupLayout =
          MakeBindGroupLayout(device_, layouts_info[i]);
      render_pipeline->bind_group_layouts.push_back(bindGroupLayout);

      wgpu::BindGroup bindGroup =
          MakeBindGroup(device_, render_pipeline->bind_group_layouts.back(),
                        bindingInitalizerHelper[i]);
      render_pipeline->bind_groups.push_back(bindGroup);
    } else if (i < max_descriptor_set) {
      wgpu::BindGroupLayout bindGroupLayout = MakeBindGroupLayout(device_, {});
      render_pipeline->bind_group_layouts.push_back(bindGroupLayout);

      wgpu::BindGroup bindGroup =
          MakeBindGroup(device_, render_pipeline->bind_group_layouts.back(),
                        bindingInitalizerHelper[i]);
      render_pipeline->bind_groups.push_back(bindGroup);
    }
  }
  return {};
}

Result EngineDawn::AttachBuffers(ComputePipelineInfo* compute_pipeline) {
  Result result;

  // Do not attach pushConstants
  if (compute_pipeline->pipeline->GetPushConstantBuffer().buffer != nullptr) {
    return Result("AttachBuffers: Dawn does not support push constants!");
  }

  std::vector<std::vector<BindingInitializationHelper>> bindingInitalizerHelper(
      kMaxDawnBindGroup);
  std::vector<std::vector<wgpu::BindGroupLayoutEntry>> layouts_info(
      kMaxDawnBindGroup);
  uint32_t max_descriptor_set = 0;

  // Attach storage/uniform buffers
  wgpu::BindGroupLayoutEntry empty_layout_info = {};

  if (!compute_pipeline->pipeline->GetBuffers().empty()) {
    for (auto& buf_info : compute_pipeline->pipeline->GetBuffers()) {
      while (layouts_info[buf_info.descriptor_set].size() <= buf_info.binding) {
        layouts_info[buf_info.descriptor_set].push_back(empty_layout_info);
      }
    }
  }

  for (const auto& buf_info : compute_pipeline->pipeline->GetBuffers()) {
    wgpu::BufferUsage bufferUsage;
    wgpu::BufferBindingType bindingType;
    switch (buf_info.type) {
      case BufferType::kStorage: {
        bufferUsage = wgpu::BufferUsage::Storage;
        bindingType = wgpu::BufferBindingType::Storage;
        break;
      }
      case BufferType::kUniform: {
        bufferUsage = wgpu::BufferUsage::Uniform;
        bindingType = wgpu::BufferBindingType::Uniform;
        break;
      }
      default: {
        return Result("AttachBuffers: unknown buffer type: " +
                      std::to_string(static_cast<uint32_t>(buf_info.type)));
        break;
      }
    }

    if (buf_info.descriptor_set > kMaxDawnBindGroup - 1) {
      return Result("AttachBuffers: Dawn has a maximum of " +
                    std::to_string(kMaxDawnBindGroup) + " (descriptor sets)");
    }

    compute_pipeline->buffers.emplace_back(CreateBufferFromData(
        device_, buf_info.buffer->ValuePtr()->data(),
        buf_info.buffer->GetMaxSizeInBytes(),
        bufferUsage | wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst));

    compute_pipeline->buffer_map[{buf_info.descriptor_set, buf_info.binding}] =
        static_cast<uint32_t>(compute_pipeline->buffers.size() - 1);

    compute_pipeline->used_descriptor_set.insert(buf_info.descriptor_set);
    max_descriptor_set = std::max(max_descriptor_set, buf_info.descriptor_set);

    wgpu::BindGroupLayoutEntry layout_info = empty_layout_info;
    layout_info.binding = buf_info.binding;
    layout_info.visibility = wgpu::ShaderStage::Compute;
    layout_info.buffer.type = bindingType;
    layouts_info[buf_info.descriptor_set][buf_info.binding] = layout_info;

    BindingInitializationHelper tempBinding = BindingInitializationHelper(
        buf_info.binding, compute_pipeline->buffers.back(), 0,
        buf_info.buffer->GetMaxSizeInBytes());
    bindingInitalizerHelper[buf_info.descriptor_set].push_back(tempBinding);
  }

  for (uint32_t i = 0; i < kMaxDawnBindGroup; i++) {
    if (layouts_info[i].size() > 0 && bindingInitalizerHelper[i].size() > 0) {
      wgpu::BindGroupLayout bindGroupLayout =
          MakeBindGroupLayout(device_, layouts_info[i]);
      compute_pipeline->bind_group_layouts.push_back(bindGroupLayout);

      wgpu::BindGroup bindGroup =
          MakeBindGroup(device_, compute_pipeline->bind_group_layouts.back(),
                        bindingInitalizerHelper[i]);
      compute_pipeline->bind_groups.push_back(bindGroup);
    } else if (i < max_descriptor_set) {
      wgpu::BindGroupLayout bindGroupLayout = MakeBindGroupLayout(device_, {});
      compute_pipeline->bind_group_layouts.push_back(bindGroupLayout);

      wgpu::BindGroup bindGroup =
          MakeBindGroup(device_, compute_pipeline->bind_group_layouts.back(),
                        bindingInitalizerHelper[i]);
      compute_pipeline->bind_groups.push_back(bindGroup);
    }
  }

  return {};
}

}  // namespace dawn
}  // namespace amber
