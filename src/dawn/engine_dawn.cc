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
#include <limits>
#include <string>
#include <utility>
#include <vector>

#include "amber/amber_dawn.h"
#include "dawn/dawncpp.h"
#include "src/dawn/pipeline_info.h"
#include "src/format.h"
#include "src/make_unique.h"
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
    ::dawn::VertexInputDescriptor* descriptor =
        reinterpret_cast<::dawn::VertexInputDescriptor*>(this);

    descriptor->indexFormat = ::dawn::IndexFormat::Uint32;
    descriptor->bufferCount = 0;
    descriptor->nextInChain = nullptr;

    // Fill the default values for vertexBuffers and vertexAttributes in
    // buffers.
    ::dawn::VertexAttributeDescriptor vertexAttribute;
    vertexAttribute.shaderLocation = 0;
    vertexAttribute.offset = 0;
    vertexAttribute.format = ::dawn::VertexFormat::Float;
    for (uint32_t i = 0; i < kMaxVertexAttributes; ++i) {
      cAttributes[i] = vertexAttribute;
    }
    for (uint32_t i = 0; i < kMaxVertexBuffers; ++i) {
      cBuffers[i].stride = 0;
      cBuffers[i].stepMode = ::dawn::InputStepMode::Vertex;
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
    descriptor->buffers = &cBuffers[0];
  }

  static const uint32_t kMaxVertexBuffers = 16u;
  static const uint32_t kMaxVertexBufferStride = 2048u;
  const void* nextInChain = nullptr;
  ::dawn::IndexFormat indexFormat;
  uint32_t bufferCount;
  ::dawn::VertexBufferDescriptor const* buffers;

  std::array<::dawn::VertexBufferDescriptor, kMaxVertexBuffers> cBuffers;
  std::array<::dawn::VertexAttributeDescriptor, kMaxVertexAttributes>
      cAttributes;
};

// This structure is a container for a few variables that are created during
// CreateRenderPipelineDescriptor and CreateRenderPassDescriptor and we want to
// make sure they don't go out of scope before we are done with them
struct DawnPipelineHelper {
  Result CreateRenderPipelineDescriptor(
      const RenderPipelineInfo& render_pipeline,
      const ::dawn::Device& device,
      const bool ignore_vertex_and_Index_buffers,
      const PipelineData* pipeline_data);
  Result CreateRenderPassDescriptor(
      const RenderPipelineInfo& render_pipeline,
      const ::dawn::Device& device,
      const std::vector<::dawn::TextureView>& texture_view,
      const ::dawn::LoadOp load_op);
  ::dawn::RenderPipelineDescriptor renderPipelineDescriptor;
  ::dawn::RenderPassDescriptor renderPassDescriptor;

  ComboVertexInputDescriptor vertexInputDescriptor;
  ::dawn::RasterizationStateDescriptor rasterizationState;

 private:
  ::dawn::ProgrammableStageDescriptor fragmentStage;
  ::dawn::ProgrammableStageDescriptor vertexStage;
  ::dawn::RenderPassColorAttachmentDescriptor
      colorAttachmentsInfoPtr[kMaxColorAttachments];
  ::dawn::RenderPassDepthStencilAttachmentDescriptor depthStencilAttachmentInfo;
  std::array<::dawn::ColorStateDescriptor*, kMaxColorAttachments> colorStates;
  ::dawn::DepthStencilStateDescriptor depthStencilState;
  ::dawn::ColorStateDescriptor colorStatesDescriptor[kMaxColorAttachments];
  ::dawn::ColorStateDescriptor colorStateDescriptor;
  ::dawn::StencilStateFaceDescriptor stencil_front;
  ::dawn::StencilStateFaceDescriptor stencil_back;
  ::dawn::BlendDescriptor alpha_blend;
  ::dawn::BlendDescriptor color_blend;
  std::string vertexEntryPoint;
  std::string fragmentEntryPoint;
  std::array<::dawn::RenderPassColorAttachmentDescriptor, kMaxColorAttachments>
      colorAttachmentsInfo;
  ::dawn::TextureDescriptor depthStencilDescriptor;
  ::dawn::Texture depthStencilTexture;
  ::dawn::TextureView depthStencilView;
};

// Creates a device-side texture, and returns it through |result_ptr|.
// Assumes the device exists and is valid.  Assumes result_ptr is not null.
// Returns a result code.
Result MakeTexture(const ::dawn::Device& device,
                   ::dawn::TextureFormat format,
                   uint32_t width,
                   uint32_t height,
                   ::dawn::Texture* result_ptr) {
  assert(device);
  assert(result_ptr);
  assert(width * height > 0);
  ::dawn::TextureDescriptor descriptor;
  descriptor.dimension = ::dawn::TextureDimension::e2D;
  descriptor.size.width = width;
  descriptor.size.height = height;
  descriptor.size.depth = 1;
  descriptor.arrayLayerCount = 1;
  descriptor.format = format;
  descriptor.mipLevelCount = 1;
  descriptor.sampleCount = 1;
  descriptor.usage =
      ::dawn::TextureUsage::CopySrc | ::dawn::TextureUsage::OutputAttachment;
  // TODO(dneto): Get a better message by using the Dawn error callback.
  *result_ptr = device.CreateTexture(&descriptor);
  if (*result_ptr)
    return {};
  return Result("Dawn: Failed to allocate a framebuffer texture");
}

// Creates a device-side texture, and returns it through |result_ptr|.
// Assumes the device exists and is valid.  Assumes result_ptr is not null.
// Returns a result code.
::dawn::Texture MakeDawnTexture(const ::dawn::Device& device,
                                ::dawn::TextureFormat format,
                                uint32_t width,
                                uint32_t height) {
  assert(device);
  assert(width * height > 0);
  ::dawn::TextureDescriptor descriptor;
  descriptor.dimension = ::dawn::TextureDimension::e2D;
  descriptor.size.width = width;
  descriptor.size.height = height;
  descriptor.size.depth = 1;
  descriptor.arrayLayerCount = 1;
  descriptor.format = format;
  descriptor.mipLevelCount = 1;
  descriptor.sampleCount = 1;
  descriptor.usage =
      ::dawn::TextureUsage::CopySrc | ::dawn::TextureUsage::OutputAttachment;

  return device.CreateTexture(&descriptor);
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
                             void* userdata) {
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
    case DAWN_BUFFER_MAP_ASYNC_STATUS_DEVICE_LOST:
      map_result.result = Result("Buffer map for reading failed: device lost");
      break;
  }
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
// ::dawn::BufferUsage::MapRead set.  Returns a MapResult structure, with
// the status saved in the |result| member and the host pointer to the mapped
// data in the |data| member. Mapping a buffer can fail if the context is
// lost, for example. In the failure case, the |data| member will be null.
MapResult MapBuffer(const ::dawn::Device& device, const ::dawn::Buffer& buf) {
  MapResult map_result;

  buf.MapReadAsync(
      HandleBufferMapCallback,
      reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(&map_result)));
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

// Creates and returns a dawn BufferCopyView
// Copied from Dawn utils source code.
::dawn::BufferCopyView CreateBufferCopyView(::dawn::Buffer buffer,
                                            uint64_t offset,
                                            uint32_t rowPitch,
                                            uint32_t imageHeight) {
  ::dawn::BufferCopyView bufferCopyView;
  bufferCopyView.buffer = buffer;
  bufferCopyView.offset = offset;
  bufferCopyView.rowPitch = rowPitch;
  bufferCopyView.imageHeight = imageHeight;

  return bufferCopyView;
}

// Creates and returns a dawn TextureCopyView
// Copied from Dawn utils source code.
::dawn::TextureCopyView CreateTextureCopyView(::dawn::Texture texture,
                                              uint32_t mipLevel,
                                              uint32_t arrayLayer,
                                              ::dawn::Origin3D origin) {
  ::dawn::TextureCopyView textureCopyView;
  textureCopyView.texture = texture;
  textureCopyView.mipLevel = mipLevel;
  textureCopyView.arrayLayer = arrayLayer;
  textureCopyView.origin = origin;

  return textureCopyView;
}

Result EngineDawn::MapDeviceTextureToHostBuffer(
    const RenderPipelineInfo& render_pipeline,
    const ::dawn::Device& device) {
  const auto width = render_pipeline.pipeline->GetFramebufferWidth();
  const auto height = render_pipeline.pipeline->GetFramebufferHeight();

  const auto pixelSize = render_pipeline.pipeline->GetColorAttachments()[0]
                             .buffer->GetElementStride();
  const auto dawn_row_pitch = Align(width * pixelSize, kMinimumImageRowPitch);
  const auto size = height * dawn_row_pitch;
  // Create a temporary buffer to hold the color attachment content and can
  // be mapped
  ::dawn::BufferDescriptor descriptor;
  descriptor.size = size;
  descriptor.usage =
      ::dawn::BufferUsage::CopyDst | ::dawn::BufferUsage::MapRead;
  ::dawn::Buffer copy_buffer = device.CreateBuffer(&descriptor);
  ::dawn::BufferCopyView copy_buffer_view =
      CreateBufferCopyView(copy_buffer, 0, dawn_row_pitch, 0);
  ::dawn::Origin3D origin3D;
  origin3D.x = 0;
  origin3D.y = 0;
  origin3D.z = 0;

  for (uint32_t i = 0;
       i < render_pipeline.pipeline->GetColorAttachments().size(); i++) {
    ::dawn::TextureCopyView device_texture_view =
        CreateTextureCopyView(textures_[i], 0, 0, origin3D);
    ::dawn::Extent3D copySize = {width, height, 1};
    auto encoder = device.CreateCommandEncoder();
    encoder.CopyTextureToBuffer(&device_texture_view, &copy_buffer_view,
                                &copySize);
    auto commands = encoder.Finish();
    auto queue = device.CreateQueue();
    queue.Submit(1, &commands);

    MapResult mapped_device_texture = MapBuffer(device, copy_buffer);
    if (!mapped_device_texture.result.IsSuccess())
      return mapped_device_texture.result;

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
    const ::dawn::Device& device) {
  for (uint32_t i = 0; i < compute_pipeline.pipeline->GetBuffers().size();
       i++) {
    auto& device_buffer = compute_pipeline.buffers[i];
    auto& host_buffer = compute_pipeline.pipeline->GetBuffers()[i];

    // Create a copy of device buffer to use it in a map read operation.
    // It's not possible to simply set this bit on the existing buffers since:
    // Device error: Only CopyDst is allowed with MapRead
    ::dawn::BufferDescriptor descriptor;
    descriptor.size = host_buffer.buffer->GetSizeInBytes();
    descriptor.usage =
        ::dawn::BufferUsage::CopyDst | ::dawn::BufferUsage::MapRead;
    const auto copy_device_buffer = device.CreateBuffer(&descriptor);
    const uint64_t source_offset = 0;
    const uint64_t destination_offset = 0;
    const uint64_t copy_size =
        static_cast<uint64_t>(host_buffer.buffer->GetSizeInBytes());
    auto encoder = device.CreateCommandEncoder();
    encoder.CopyBufferToBuffer(device_buffer, source_offset, copy_device_buffer,
                               destination_offset, copy_size);
    auto commands = encoder.Finish();
    auto queue = device.CreateQueue();
    queue.Submit(1, &commands);

    MapResult mapped_device_buffer = MapBuffer(device, copy_device_buffer);
    auto* values = host_buffer.buffer->ValuePtr();
    values->resize(host_buffer.buffer->GetSizeInBytes());
    std::memcpy(values->data(),
                static_cast<const uint8_t*>(mapped_device_buffer.data),
                copy_size);

    copy_device_buffer.Unmap();
    if (!mapped_device_buffer.result.IsSuccess())
      return mapped_device_buffer.result;
  }
  return {};
}

// Creates a dawn buffer of |size| bytes with TransferDst and the given usage
// copied from Dawn utils source code
::dawn::Buffer CreateBufferFromData(const ::dawn::Device& device,
                                    const void* data,
                                    uint64_t size,
                                    ::dawn::BufferUsage usage) {
  ::dawn::BufferDescriptor descriptor;
  descriptor.size = size;
  descriptor.usage = usage | ::dawn::BufferUsage::CopyDst;

  ::dawn::Buffer buffer = device.CreateBuffer(&descriptor);
  if (data != nullptr)
    buffer.SetSubData(0, size, reinterpret_cast<const uint8_t*>(data));
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
                              const ::dawn::Buffer& buffer,
                              uint64_t offset,
                              uint64_t size)
      : binding(binding), buffer(buffer), offset(offset), size(size) {}

  ::dawn::BindGroupBinding GetAsBinding() const {
    ::dawn::BindGroupBinding result;
    result.binding = binding;
    result.sampler = sampler;
    result.textureView = textureView;
    result.buffer = buffer;
    result.offset = offset;
    result.size = size;
    return result;
  }

  uint32_t binding;
  ::dawn::Sampler sampler;
  ::dawn::TextureView textureView;
  ::dawn::Buffer buffer;
  uint64_t offset = 0;
  uint64_t size = 0;
};

::dawn::BindGroup MakeBindGroup(
    const ::dawn::Device& device,
    const ::dawn::BindGroupLayout& layout,
    const std::vector<BindingInitializationHelper>& bindingsInitializer) {
  std::vector<::dawn::BindGroupBinding> bindings;
  for (const BindingInitializationHelper& helper : bindingsInitializer) {
    bindings.push_back(helper.GetAsBinding());
  }

  ::dawn::BindGroupDescriptor descriptor;
  descriptor.layout = layout;
  descriptor.bindingCount = bindings.size();
  descriptor.bindings = bindings.data();

  return device.CreateBindGroup(&descriptor);
}

// Creates a bind group layout.
// Copied from Dawn utils source code.
::dawn::BindGroupLayout MakeBindGroupLayout(
    const ::dawn::Device& device,
    const std::vector<::dawn::BindGroupLayoutBinding>& bindingsInitializer) {
  constexpr ::dawn::ShaderStage kNoStages{};

  std::vector<::dawn::BindGroupLayoutBinding> bindings;
  for (const ::dawn::BindGroupLayoutBinding& binding : bindingsInitializer) {
    if (binding.visibility != kNoStages) {
      bindings.push_back(binding);
    }
  }

  ::dawn::BindGroupLayoutDescriptor descriptor;
  descriptor.bindingCount = static_cast<uint32_t>(bindings.size());
  descriptor.bindings = bindings.data();
  return device.CreateBindGroupLayout(&descriptor);
}

// Creates a basic pipeline layout.
// Copied from Dawn utils source code.
::dawn::PipelineLayout MakeBasicPipelineLayout(
    const ::dawn::Device& device,
    std::vector<::dawn::BindGroupLayout> bindingInitializer) {
  ::dawn::PipelineLayoutDescriptor descriptor;
  descriptor.bindGroupLayoutCount = bindingInitializer.size();
  descriptor.bindGroupLayouts = bindingInitializer.data();
  return device.CreatePipelineLayout(&descriptor);
}

// Converts an Amber format to a Dawn texture format, and sends the result out
// through |dawn_format_ptr|.  If the conversion fails, return an error
// result.
Result GetDawnTextureFormat(const ::amber::Format& amber_format,
                            ::dawn::TextureFormat* dawn_format_ptr) {
  if (!dawn_format_ptr)
    return Result("Internal error: format pointer argument is null");
  ::dawn::TextureFormat& dawn_format = *dawn_format_ptr;

  switch (amber_format.GetFormatType()) {
    // TODO(dneto): These are all the formats that Dawn currently knows about.
    case FormatType::kR8G8B8A8_UNORM:
      dawn_format = ::dawn::TextureFormat::RGBA8Unorm;
      break;
    case FormatType::kR8G8_UNORM:
      dawn_format = ::dawn::TextureFormat::RG8Unorm;
      break;
    case FormatType::kR8_UNORM:
      dawn_format = ::dawn::TextureFormat::R8Unorm;
      break;
    case FormatType::kR8G8B8A8_UINT:
      dawn_format = ::dawn::TextureFormat::RGBA8Uint;
      break;
    case FormatType::kR8G8_UINT:
      dawn_format = ::dawn::TextureFormat::RG8Uint;
      break;
    case FormatType::kR8_UINT:
      dawn_format = ::dawn::TextureFormat::R8Uint;
      break;
    case FormatType::kB8G8R8A8_UNORM:
      dawn_format = ::dawn::TextureFormat::BGRA8Unorm;
      break;
    case FormatType::kD32_SFLOAT_S8_UINT:
      dawn_format = ::dawn::TextureFormat::Depth24PlusStencil8;
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
// TODO(sarahM0): support other ::dawn::VertexFormat
Result GetDawnVertexFormat(const ::amber::Format& amber_format,
                           ::dawn::VertexFormat* dawn_format_ptr) {
  ::dawn::VertexFormat& dawn_format = *dawn_format_ptr;
  switch (amber_format.GetFormatType()) {
    case FormatType::kR32_SFLOAT:
      dawn_format = ::dawn::VertexFormat::Float;
      break;
    case FormatType::kR32G32_SFLOAT:
      dawn_format = ::dawn::VertexFormat::Float2;
      break;
    case FormatType::kR32G32B32_SFLOAT:
      dawn_format = ::dawn::VertexFormat::Float3;
      break;
    case FormatType::kR32G32B32A32_SFLOAT:
      dawn_format = ::dawn::VertexFormat::Float4;
      break;
    case FormatType::kR8G8_SNORM:
      dawn_format = ::dawn::VertexFormat::Char2Norm;
      break;
    case FormatType::kR8G8B8A8_UNORM:
      dawn_format = ::dawn::VertexFormat::UChar4Norm;
      break;
    case FormatType::kR8G8B8A8_SNORM:
      dawn_format = ::dawn::VertexFormat::Char4Norm;
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
                          ::dawn::IndexFormat* dawn_format_ptr) {
  ::dawn::IndexFormat& dawn_format = *dawn_format_ptr;
  switch (amber_format.GetFormatType()) {
    case FormatType::kR16_UINT:
      dawn_format = ::dawn::IndexFormat::Uint16;
      break;
    case FormatType::kR32_UINT:
      dawn_format = ::dawn::IndexFormat::Uint32;
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
                       ::dawn::PrimitiveTopology* dawn_topology_ptr) {
  ::dawn::PrimitiveTopology& dawn_topology = *dawn_topology_ptr;
  switch (amber_topology) {
    case Topology::kPointList:
      dawn_topology = ::dawn::PrimitiveTopology::PointList;
      break;
    case Topology::kLineList:
      dawn_topology = ::dawn::PrimitiveTopology::LineList;
      break;
    case Topology::kLineStrip:
      dawn_topology = ::dawn::PrimitiveTopology::LineStrip;
      break;
    case Topology::kTriangleList:
      dawn_topology = ::dawn::PrimitiveTopology::TriangleList;
      break;
    case Topology::kTriangleStrip:
      dawn_topology = ::dawn::PrimitiveTopology::TriangleStrip;
      break;
    default:
      return Result("Amber PrimitiveTopology " +
                    std::to_string(static_cast<uint32_t>(amber_topology)) +
                    " is not supported in Dawn");
  }
  return {};
}

::dawn::CompareFunction GetDawnCompareOp(::amber::CompareOp op) {
  switch (op) {
    case CompareOp::kNever:
      return ::dawn::CompareFunction::Never;
    case CompareOp::kLess:
      return ::dawn::CompareFunction::Less;
    case CompareOp::kEqual:
      return ::dawn::CompareFunction::Equal;
    case CompareOp::kLessOrEqual:
      return ::dawn::CompareFunction::LessEqual;
    case CompareOp::kGreater:
      return ::dawn::CompareFunction::Greater;
    case CompareOp::kNotEqual:
      return ::dawn::CompareFunction::NotEqual;
    case CompareOp::kGreaterOrEqual:
      return ::dawn::CompareFunction::GreaterEqual;
    case CompareOp::kAlways:
      return ::dawn::CompareFunction::Always;
    default:
      return ::dawn::CompareFunction::Never;
  }
}

::dawn::StencilOperation GetDawnStencilOp(::amber::StencilOp op) {
  switch (op) {
    case StencilOp::kKeep:
      return ::dawn::StencilOperation::Keep;
    case StencilOp::kZero:
      return ::dawn::StencilOperation::Zero;
    case StencilOp::kReplace:
      return ::dawn::StencilOperation::Replace;
    case StencilOp::kIncrementAndClamp:
      return ::dawn::StencilOperation::IncrementClamp;
    case StencilOp::kDecrementAndClamp:
      return ::dawn::StencilOperation::DecrementClamp;
    case StencilOp::kInvert:
      return ::dawn::StencilOperation::Invert;
    case StencilOp::kIncrementAndWrap:
      return ::dawn::StencilOperation::IncrementWrap;
    case StencilOp::kDecrementAndWrap:
      return ::dawn::StencilOperation::DecrementWrap;
    default:
      return ::dawn::StencilOperation::Keep;
  }
}

::dawn::BlendFactor GetDawnBlendFactor(::amber::BlendFactor factor) {
  switch (factor) {
    case BlendFactor::kZero:
      return ::dawn::BlendFactor::Zero;
    case BlendFactor::kOne:
      return ::dawn::BlendFactor::One;
    case BlendFactor::kSrcColor:
      return ::dawn::BlendFactor::SrcColor;
    case BlendFactor::kOneMinusSrcColor:
      return ::dawn::BlendFactor::OneMinusSrcColor;
    case BlendFactor::kDstColor:
      return ::dawn::BlendFactor::DstColor;
    case BlendFactor::kOneMinusDstColor:
      return ::dawn::BlendFactor::OneMinusDstColor;
    case BlendFactor::kSrcAlpha:
      return ::dawn::BlendFactor::SrcAlpha;
    case BlendFactor::kOneMinusSrcAlpha:
      return ::dawn::BlendFactor::OneMinusSrcAlpha;
    case BlendFactor::kDstAlpha:
      return ::dawn::BlendFactor::DstAlpha;
    case BlendFactor::kOneMinusDstAlpha:
      return ::dawn::BlendFactor::OneMinusDstAlpha;
    case BlendFactor::kSrcAlphaSaturate:
      return ::dawn::BlendFactor::SrcAlphaSaturated;
    default:
      assert(false && "Dawn::Unknown BlendFactor");
      return ::dawn::BlendFactor::One;
  }
}

::dawn::BlendOperation GetDawnBlendOperation(BlendOp op) {
  switch (op) {
    case BlendOp::kAdd:
      return ::dawn::BlendOperation::Add;
    case BlendOp::kSubtract:
      return ::dawn::BlendOperation::Subtract;
    case BlendOp::kReverseSubtract:
      return ::dawn::BlendOperation::ReverseSubtract;
    case BlendOp::kMin:
      return ::dawn::BlendOperation::Min;
    case BlendOp::kMax:
      return ::dawn::BlendOperation::Max;
    default:
      assert(false && "Dawn::Unknown BlendOp");
      return ::dawn::BlendOperation::Add;
  }
}

::dawn::ColorWriteMask GetDawnColorWriteMask(uint8_t amber_color_write_mask) {
  if (amber_color_write_mask == 0x00000000)
    return ::dawn::ColorWriteMask::None;
  else if (amber_color_write_mask == 0x00000001)
    return ::dawn::ColorWriteMask::Red;
  else if (amber_color_write_mask == 0x00000002)
    return ::dawn::ColorWriteMask::Green;
  else if (amber_color_write_mask == 0x00000004)
    return ::dawn::ColorWriteMask::Blue;
  else if (amber_color_write_mask == 0x00000008)
    return ::dawn::ColorWriteMask::Alpha;
  else if (amber_color_write_mask == 0x0000000F)
    return ::dawn::ColorWriteMask::All;
  else
    assert(false && "Dawn::Unknown ColorWriteMask");
  return ::dawn::ColorWriteMask::All;
}

::dawn::FrontFace GetDawnFrontFace(FrontFace amber_front_face) {
  return amber_front_face == FrontFace::kClockwise ? ::dawn::FrontFace::CW
                                                   : ::dawn::FrontFace::CCW;
}

::dawn::CullMode GetDawnCullMode(CullMode amber_cull_mode) {
  switch (amber_cull_mode) {
    case CullMode::kNone:
      return ::dawn::CullMode::None;
    case CullMode::kFront:
      return ::dawn::CullMode::Front;
    case CullMode::kBack:
      return ::dawn::CullMode::Back;
    default:
      assert(false && "Dawn::Unknown CullMode");
      return ::dawn::CullMode::None;
  }
}

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
  if (!device_) {
    return Result("Dawn::CreatePipeline: device is not created");
  }
  std::unordered_map<ShaderType, ::dawn::ShaderModule, CastHash<ShaderType>>
      module_for_type;
  ::dawn::ShaderModuleDescriptor descriptor;
  descriptor.nextInChain = nullptr;

  for (const auto& shader_info : pipeline->GetShaders()) {
    ShaderType type = shader_info.GetShaderType();
    const std::vector<uint32_t>& code = shader_info.GetData();
    descriptor.code = code.data();
    descriptor.codeSize = uint32_t(code.size());

    auto shader = device_->CreateShaderModule(&descriptor);
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
      if (!module)
        return Result("Dawn::CreatePipeline: no compute shader provided");

      pipeline_map_[pipeline].compute_pipeline.reset(
          new ComputePipelineInfo(pipeline, module));
      Result result =
          AttachBuffers(pipeline_map_[pipeline].compute_pipeline.get());
      if (!result.IsSuccess())
        return result;
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
      if (!result.IsSuccess())
        return result;

      break;
    }
  }

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
  Result result;
  RenderPipelineInfo* render_pipeline = GetRenderPipeline(command);
  if (!render_pipeline)
    return Result("Clear invoked on invalid or missing render pipeline");

  DawnPipelineHelper helper;
  result = helper.CreateRenderPipelineDescriptor(*render_pipeline, *device_,
                                                 false, nullptr);
  if (!result.IsSuccess())
    return result;
  result = helper.CreateRenderPassDescriptor(
      *render_pipeline, *device_, texture_views_, ::dawn::LoadOp::Clear);
  if (!result.IsSuccess())
    return result;

  ::dawn::RenderPassDescriptor* renderPassDescriptor =
      &helper.renderPassDescriptor;
  ::dawn::CommandEncoder encoder = device_->CreateCommandEncoder();
  ::dawn::RenderPassEncoder pass =
      encoder.BeginRenderPass(renderPassDescriptor);
  pass.EndPass();

  ::dawn::CommandBuffer commands = encoder.Finish();
  ::dawn::Queue queue = device_->CreateQueue();
  queue.Submit(1, &commands);

  result = MapDeviceTextureToHostBuffer(*render_pipeline, *device_);

  return result;
}

// Creates a Dawn render pipeline descriptor for the given pipeline on the given
// device. When |ignore_vertex_and_Index_buffers| is true, ignores the vertex
// and index buffers attached to |render_pipeline| and instead configures the
// resulting descriptor to have a single vertex buffer with an attribute format
// of Float4 and input stride of 4*sizeof(float)
Result DawnPipelineHelper::CreateRenderPipelineDescriptor(
    const RenderPipelineInfo& render_pipeline,
    const ::dawn::Device& device,
    const bool ignore_vertex_and_Index_buffers,
    const PipelineData* pipeline_data) {
  Result result;

  auto* amber_format =
      render_pipeline.pipeline->GetColorAttachments()[0].buffer->GetFormat();
  if (!amber_format)
    return Result("Color attachment 0 has no format!");
  ::dawn::TextureFormat fb_format{};
  result = GetDawnTextureFormat(*amber_format, &fb_format);
  if (!result.IsSuccess())
    return result;

  ::dawn::TextureFormat depth_stencil_format{};
  auto* depthBuffer = render_pipeline.pipeline->GetDepthBuffer().buffer;
  if (depthBuffer) {
    auto* amber_depth_stencil_format = depthBuffer->GetFormat();
    if (!amber_depth_stencil_format)
      return Result("The depth/stencil attachment has no format!");
    result = GetDawnTextureFormat(*amber_depth_stencil_format,
                                  &depth_stencil_format);
    if (!result.IsSuccess())
      return result;
  } else {
    depth_stencil_format = ::dawn::TextureFormat::Depth24PlusStencil8;
  }

  renderPipelineDescriptor.layout =
      MakeBasicPipelineLayout(device, render_pipeline.bind_group_layouts);

  renderPipelineDescriptor.primitiveTopology =
      ::dawn::PrimitiveTopology::TriangleList;
  renderPipelineDescriptor.sampleCount = 1;

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
  vertexInputDescriptor.bufferCount =
      render_pipeline.pipeline->GetVertexBuffers().size();

  for (uint32_t i = 0; i < kMaxVertexInputs; ++i) {
    if (ignore_vertex_and_Index_buffers) {
      if (i == 0) {
        vertexInputDescriptor.bufferCount = 1;
        vertexInputDescriptor.cBuffers[0].attributeCount = 1;
        vertexInputDescriptor.cBuffers[0].stride = 4 * sizeof(float);
        vertexInputDescriptor.cBuffers[0].attributes =
            &vertexInputDescriptor.cAttributes[0];

        vertexInputDescriptor.cAttributes[0].shaderLocation = 0;
        vertexInputDescriptor.cAttributes[0].format =
            ::dawn::VertexFormat::Float4;
      }
    } else {
      if (i < render_pipeline.pipeline->GetVertexBuffers().size()) {
        vertexInputDescriptor.cBuffers[i].attributeCount = 1;
        vertexInputDescriptor.cBuffers[i].stride =
            render_pipeline.pipeline->GetVertexBuffers()[i]
                .buffer->GetElementStride();
        vertexInputDescriptor.cBuffers[i].stepMode =
            ::dawn::InputStepMode::Vertex;
        vertexInputDescriptor.cBuffers[i].attributes =
            &vertexInputDescriptor.cAttributes[i];

        vertexInputDescriptor.cAttributes[i].shaderLocation = i;
        auto* amber_vertex_format =
            render_pipeline.pipeline->GetVertexBuffers()[i].buffer->GetFormat();
        result = GetDawnVertexFormat(
            *amber_vertex_format, &vertexInputDescriptor.cAttributes[i].format);
        if (!result.IsSuccess())
          return result;
      }
    }
  }

  // set index buffer format
  if (render_pipeline.pipeline->GetIndexBuffer()) {
    auto* amber_index_format =
        render_pipeline.pipeline->GetIndexBuffer()->GetFormat();
    GetDawnIndexFormat(*amber_index_format, &vertexInputDescriptor.indexFormat);
    if (!result.IsSuccess())
      return result;
  }

  renderPipelineDescriptor.vertexInput =
      reinterpret_cast<::dawn::VertexInputDescriptor*>(&vertexInputDescriptor);

  // Set defaults for the vertex stage descriptor.
  vertexStage.module = render_pipeline.vertex_shader;
  vertexStage.entryPoint = vertexEntryPoint.c_str();
  renderPipelineDescriptor.vertexStage = vertexStage;

  // Set defaults for the fragment stage descriptor.
  fragmentStage.module = render_pipeline.fragment_shader;
  fragmentStage.entryPoint = fragmentEntryPoint.c_str();
  renderPipelineDescriptor.fragmentStage = std::move(&fragmentStage);

  // Set defaults for the rasterization state descriptor.
  if (pipeline_data == nullptr) {
    rasterizationState.frontFace = ::dawn::FrontFace::CCW;
    rasterizationState.cullMode = ::dawn::CullMode::None;
    rasterizationState.depthBias = 0;
    rasterizationState.depthBiasSlopeScale = 0.0;
    rasterizationState.depthBiasClamp = 0.0;
    renderPipelineDescriptor.rasterizationState = &rasterizationState;
  } else {
    rasterizationState.frontFace =
        GetDawnFrontFace(pipeline_data->GetFrontFace());
    rasterizationState.cullMode = GetDawnCullMode(pipeline_data->GetCullMode());
    rasterizationState.depthBias = pipeline_data->GetEnableDepthBias();
    rasterizationState.depthBiasSlopeScale =
        pipeline_data->GetDepthBiasSlopeFactor();
    rasterizationState.depthBiasClamp = pipeline_data->GetDepthBiasClamp();
    renderPipelineDescriptor.rasterizationState = &rasterizationState;
  }

  // Set defaults for the color state descriptors.
  if (pipeline_data == nullptr) {
    renderPipelineDescriptor.colorStateCount =
        render_pipeline.pipeline->GetColorAttachments().size();
    alpha_blend.operation = ::dawn::BlendOperation::Add;
    alpha_blend.srcFactor = ::dawn::BlendFactor::One;
    alpha_blend.dstFactor = ::dawn::BlendFactor::Zero;
    colorStateDescriptor.writeMask = ::dawn::ColorWriteMask::All;
    colorStateDescriptor.format = fb_format;
    colorStateDescriptor.alphaBlend = alpha_blend;
    colorStateDescriptor.colorBlend = alpha_blend;
  } else {
    renderPipelineDescriptor.colorStateCount =
        render_pipeline.pipeline->GetColorAttachments().size();

    alpha_blend.operation =
        GetDawnBlendOperation(pipeline_data->GetColorBlendOp());
    alpha_blend.srcFactor =
        GetDawnBlendFactor(pipeline_data->GetSrcAlphaBlendFactor());
    alpha_blend.dstFactor =
        GetDawnBlendFactor(pipeline_data->GetDstAlphaBlendFactor());

    color_blend.operation =
        GetDawnBlendOperation(pipeline_data->GetAlphaBlendOp());
    color_blend.srcFactor =
        GetDawnBlendFactor(pipeline_data->GetSrcColorBlendFactor());
    color_blend.dstFactor =
        GetDawnBlendFactor(pipeline_data->GetDstAlphaBlendFactor());

    colorStateDescriptor.writeMask =
        GetDawnColorWriteMask(pipeline_data->GetColorWriteMask());

    colorStateDescriptor.format = fb_format;
    colorStateDescriptor.alphaBlend = alpha_blend;
    colorStateDescriptor.colorBlend = color_blend;
  }

  for (uint32_t i = 0; i < kMaxColorAttachments; ++i) {
    ::dawn::TextureFormat fb_format{};
    {
      if (i < render_pipeline.pipeline->GetColorAttachments().size()) {
        auto* amber_format = render_pipeline.pipeline->GetColorAttachments()[i]
                                 .buffer->GetFormat();
        if (!amber_format)
          return Result(
              "AttachBuffersAndTextures: One Color attachment has no "
              "format!");
        result = GetDawnTextureFormat(*amber_format, &fb_format);
        if (!result.IsSuccess())
          return result;
      } else {
        fb_format = ::dawn::TextureFormat::RGBA8Unorm;
      }
    }
    colorStatesDescriptor[i] = colorStateDescriptor;
    colorStates[i] = &colorStatesDescriptor[i];
    colorStates[i]->format = fb_format;
  }
  renderPipelineDescriptor.colorStates = colorStates[0];

  // Set defaults for the depth stencil state descriptors.
  if (pipeline_data == nullptr) {
    stencil_front.compare = ::dawn::CompareFunction::Always;
    stencil_front.failOp = ::dawn::StencilOperation::Keep;
    stencil_front.depthFailOp = ::dawn::StencilOperation::Keep;
    stencil_front.passOp = ::dawn::StencilOperation::Keep;
    depthStencilState.depthWriteEnabled = false;
    depthStencilState.depthCompare = ::dawn::CompareFunction::Always;
    depthStencilState.stencilBack = stencil_front;
    depthStencilState.stencilFront = stencil_front;
    depthStencilState.stencilReadMask = 0xff;
    depthStencilState.stencilWriteMask = 0xff;
    depthStencilState.format = depth_stencil_format;
    renderPipelineDescriptor.depthStencilState = &depthStencilState;
  } else {
    stencil_front.compare =
        GetDawnCompareOp(pipeline_data->GetFrontCompareOp());
    stencil_front.failOp = GetDawnStencilOp(pipeline_data->GetFrontFailOp());
    stencil_front.depthFailOp =
        GetDawnStencilOp(pipeline_data->GetFrontDepthFailOp());
    stencil_front.passOp = GetDawnStencilOp(pipeline_data->GetFrontPassOp());

    stencil_back.compare = GetDawnCompareOp(pipeline_data->GetBackCompareOp());
    stencil_back.failOp = GetDawnStencilOp(pipeline_data->GetBackFailOp());
    stencil_back.depthFailOp =
        GetDawnStencilOp(pipeline_data->GetBackDepthFailOp());
    stencil_back.passOp = GetDawnStencilOp(pipeline_data->GetBackPassOp());

    depthStencilState.depthWriteEnabled = pipeline_data->GetEnableDepthWrite();
    depthStencilState.depthCompare =
        GetDawnCompareOp(pipeline_data->GetDepthCompareOp());
    depthStencilState.stencilFront = stencil_front;
    depthStencilState.stencilBack = stencil_back;
    // WebGPU doesn't support separate front and back stencil mask, they has to
    // be the same
    depthStencilState.stencilReadMask =
        (pipeline_data->GetFrontCompareMask() ==
         pipeline_data->GetBackCompareMask())
            ? pipeline_data->GetFrontCompareMask()
            : 0xff;
    depthStencilState.stencilWriteMask = (pipeline_data->GetBackWriteMask() ==
                                          pipeline_data->GetFrontWriteMask())
                                             ? pipeline_data->GetBackWriteMask()
                                             : 0xff;
    depthStencilState.format = depth_stencil_format;
    renderPipelineDescriptor.depthStencilState = &depthStencilState;
  }

  return {};
}

Result DawnPipelineHelper::CreateRenderPassDescriptor(
    const RenderPipelineInfo& render_pipeline,
    const ::dawn::Device& device,
    const std::vector<::dawn::TextureView>& texture_view,
    const ::dawn::LoadOp load_op) {
  for (uint32_t i = 0; i < kMaxColorAttachments; ++i) {
    colorAttachmentsInfo[i].loadOp = load_op;
    colorAttachmentsInfo[i].storeOp = ::dawn::StoreOp::Store;
    colorAttachmentsInfo[i].clearColor = render_pipeline.clear_color_value;
  }

  depthStencilAttachmentInfo.clearDepth = render_pipeline.clear_depth_value;
  depthStencilAttachmentInfo.clearStencil = render_pipeline.clear_stencil_value;
  depthStencilAttachmentInfo.depthLoadOp = load_op;
  depthStencilAttachmentInfo.depthStoreOp = ::dawn::StoreOp::Store;
  depthStencilAttachmentInfo.stencilLoadOp = load_op;
  depthStencilAttachmentInfo.stencilStoreOp = ::dawn::StoreOp::Store;

  renderPassDescriptor.colorAttachmentCount =
      render_pipeline.pipeline->GetColorAttachments().size();
  uint32_t colorAttachmentIndex = 0;
  for (const ::dawn::TextureView& colorAttachment : texture_view) {
    if (colorAttachment.Get() != nullptr) {
      colorAttachmentsInfo[colorAttachmentIndex].attachment = colorAttachment;
      colorAttachmentsInfoPtr[colorAttachmentIndex] =
          colorAttachmentsInfo[colorAttachmentIndex];
    }
    ++colorAttachmentIndex;
  }
  renderPassDescriptor.colorAttachments = colorAttachmentsInfoPtr;

  ::dawn::TextureFormat depth_stencil_format{};
  auto* depthBuffer = render_pipeline.pipeline->GetDepthBuffer().buffer;
  if (depthBuffer) {
    auto* amber_depth_stencil_format = depthBuffer->GetFormat();
    if (!amber_depth_stencil_format)
      return Result("The depth/stencil attachment has no format!");
    Result result = GetDawnTextureFormat(*amber_depth_stencil_format,
                                         &depth_stencil_format);
    if (!result.IsSuccess())
      return result;
  } else {
    depth_stencil_format = ::dawn::TextureFormat::Depth24PlusStencil8;
  }

  depthStencilDescriptor.dimension = ::dawn::TextureDimension::e2D;
  depthStencilDescriptor.size.width =
      render_pipeline.pipeline->GetFramebufferWidth();
  depthStencilDescriptor.size.height =
      render_pipeline.pipeline->GetFramebufferHeight();
  depthStencilDescriptor.size.depth = 1;
  depthStencilDescriptor.arrayLayerCount = 1;
  depthStencilDescriptor.sampleCount = 1;
  depthStencilDescriptor.format = depth_stencil_format;
  depthStencilDescriptor.mipLevelCount = 1;
  depthStencilDescriptor.usage =
      ::dawn::TextureUsage::OutputAttachment | ::dawn::TextureUsage::CopySrc;
  depthStencilTexture = device.CreateTexture(&depthStencilDescriptor);
  depthStencilView = depthStencilTexture.CreateView();

  if (depthStencilView.Get() != nullptr) {
    depthStencilAttachmentInfo.attachment = depthStencilView;
    renderPassDescriptor.depthStencilAttachment = &depthStencilAttachmentInfo;
  } else {
    renderPassDescriptor.depthStencilAttachment = nullptr;
  }

  return {};
}

Result EngineDawn::DoDrawRect(const DrawRectCommand* command) {
  RenderPipelineInfo* render_pipeline = GetRenderPipeline(command);
  if (!render_pipeline)
    return Result("DrawRect invoked on invalid or missing render pipeline");
  if (render_pipeline->vertex_buffers.size() > 1)
    return Result(
        "DrawRect invoked on a render pipeline with more than one "
        "VERTEX_DATA attached");

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
      *device_, indexData, sizeof(indexData), ::dawn::BufferUsage::Index);

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
      *device_, vertexData, sizeof(vertexData), ::dawn::BufferUsage::Vertex);
  DawnPipelineHelper helper;
  helper.CreateRenderPipelineDescriptor(*render_pipeline, *device_, true,
                                        command->GetPipelineData());
  helper.CreateRenderPassDescriptor(*render_pipeline, *device_, texture_views_,
                                    ::dawn::LoadOp::Load);
  ::dawn::RenderPipelineDescriptor* renderPipelineDescriptor =
      &helper.renderPipelineDescriptor;
  ::dawn::RenderPassDescriptor* renderPassDescriptor =
      &helper.renderPassDescriptor;

  const ::dawn::RenderPipeline pipeline =
      device_->CreateRenderPipeline(renderPipelineDescriptor);
  ::dawn::CommandEncoder encoder = device_->CreateCommandEncoder();
  ::dawn::RenderPassEncoder pass =
      encoder.BeginRenderPass(renderPassDescriptor);
  pass.SetPipeline(pipeline);
  for (uint32_t i = 0; i < render_pipeline->bind_groups.size(); i++) {
    if (render_pipeline->bind_groups[i]) {
      pass.SetBindGroup(i, render_pipeline->bind_groups[i], 0, nullptr);
    }
  }
  pass.SetVertexBuffer(0, vertex_buffer, 0);
  pass.SetIndexBuffer(index_buffer, 0);
  pass.DrawIndexed(6, 1, 0, 0, 0);
  pass.EndPass();

  ::dawn::CommandBuffer commands = encoder.Finish();
  ::dawn::Queue queue = device_->CreateQueue();
  queue.Submit(1, &commands);

  Result result = MapDeviceTextureToHostBuffer(*render_pipeline, *device_);

  return result;
}

Result EngineDawn::DoDrawArrays(const DrawArraysCommand* command) {
  Result result;

  RenderPipelineInfo* render_pipeline = GetRenderPipeline(command);
  if (!render_pipeline)
    return Result("DrawArrays invoked on invalid or missing render pipeline");

  if (command->IsIndexed()) {
    if (!render_pipeline->index_buffer)
      return Result("DrawArrays: Draw indexed is used without given indices");
  } else {
    std::vector<uint32_t> indexData;
    for (uint32_t i = 0;
         i < command->GetFirstVertexIndex() + command->GetVertexCount(); i++) {
      indexData.emplace_back(i);
    }
    render_pipeline->index_buffer = CreateBufferFromData(
        *device_, indexData.data(), indexData.size() * sizeof(uint32_t),
        ::dawn::BufferUsage::Index);
  }

  uint32_t instance_count = command->GetInstanceCount();
  if (instance_count == 0 && command->GetVertexCount() != 0)
    instance_count = 1;

  DawnPipelineHelper helper;
  result = helper.CreateRenderPipelineDescriptor(
      *render_pipeline, *device_, false, command->GetPipelineData());
  if (!result.IsSuccess())
    return result;
  result = helper.CreateRenderPassDescriptor(
      *render_pipeline, *device_, texture_views_, ::dawn::LoadOp::Load);
  if (!result.IsSuccess())
    return result;

  ::dawn::RenderPipelineDescriptor* renderPipelineDescriptor =
      &helper.renderPipelineDescriptor;
  ::dawn::RenderPassDescriptor* renderPassDescriptor =
      &helper.renderPassDescriptor;

  result = GetDawnTopology(command->GetTopology(),
                           &renderPipelineDescriptor->primitiveTopology);
  if (!result.IsSuccess())
    return result;

  const ::dawn::RenderPipeline pipeline =
      device_->CreateRenderPipeline(renderPipelineDescriptor);
  ::dawn::CommandEncoder encoder = device_->CreateCommandEncoder();
  ::dawn::RenderPassEncoder pass =
      encoder.BeginRenderPass(renderPassDescriptor);
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
                         0);                                 /* offsets */
  }
  // TODO(sarahM0): figure out what this offset means
  pass.SetIndexBuffer(render_pipeline->index_buffer, /* buffer */
                      0);                            /* offset*/
  pass.DrawIndexed(command->GetVertexCount(),        /* indexCount */
                   instance_count,                   /* instanceCount */
                   0,                                /* firstIndex */
                   command->GetFirstVertexIndex(),   /* baseVertex */
                   0 /* firstInstance */);

  pass.EndPass();
  ::dawn::CommandBuffer commands = encoder.Finish();
  ::dawn::Queue queue = device_->CreateQueue();
  queue.Submit(1, &commands);

  result = MapDeviceTextureToHostBuffer(*render_pipeline, *device_);

  return result;
}

Result EngineDawn::DoCompute(const ComputeCommand* command) {
  Result result;

  ComputePipelineInfo* compute_pipeline = GetComputePipeline(command);
  if (!compute_pipeline)
    return Result("DoComput: invoked on invalid or missing compute pipeline");

  ::dawn::ComputePipelineDescriptor computePipelineDescriptor;
  computePipelineDescriptor.layout = MakeBasicPipelineLayout(
      device_->Get(), compute_pipeline->bind_group_layouts);

  ::dawn::ProgrammableStageDescriptor pipelineStageDescriptor;
  pipelineStageDescriptor.module = compute_pipeline->compute_shader;
  pipelineStageDescriptor.entryPoint = "main";
  computePipelineDescriptor.computeStage = pipelineStageDescriptor;
  ::dawn::ComputePipeline pipeline =
      device_->CreateComputePipeline(&computePipelineDescriptor);
  ::dawn::CommandEncoder encoder = device_->CreateCommandEncoder();
  ::dawn::ComputePassEncoder pass = encoder.BeginComputePass();
  pass.SetPipeline(pipeline);
  for (uint32_t i = 0; i < compute_pipeline->bind_groups.size(); i++) {
    if (compute_pipeline->bind_groups[i]) {
      pass.SetBindGroup(i, compute_pipeline->bind_groups[i], 0, nullptr);
    }
  }
  pass.Dispatch(command->GetX(), command->GetY(), command->GetZ());
  pass.EndPass();
  // Finish recording the command buffer.  It only has one command.
  auto command_buffer = encoder.Finish();
  // Submit the command.
  auto queue = device_->CreateQueue();
  queue.Submit(1, &command_buffer);
  // Copy result back
  result = MapDeviceBufferToHostBuffer(*compute_pipeline, *device_);

  return result;
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

  ::dawn::Buffer* dawn_buffer = nullptr;

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

  if (!render_pipeline && !compute_pipeline)
    return Result("DoBuffer: invoked on invalid or missing pipeline");
  if (!command->IsSSBO() && !command->IsUniform())
    return Result("DoBuffer: only supports SSBO and uniform buffer type");
  if (!dawn_buffer) {
    return Result("DoBuffer: no Dawn buffer at descriptor set " +
                  std::to_string(descriptor_set) + " and binding " +
                  std::to_string(binding));
  }

  Buffer* amber_buffer = command->GetBuffer();
  if (amber_buffer) {
    amber_buffer->SetDataWithOffset(command->GetValues(), command->GetOffset());

    dawn_buffer->SetSubData(0, amber_buffer->GetMaxSizeInBytes(),
                            amber_buffer->ValuePtr()->data());
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
        render_pipeline->pipeline->GetColorAttachments().size())
      return Result("color attachment locations must be sequential from 0");
    if (seen_idx[info.location] != -1) {
      return Result("duplicate attachment location: " +
                    std::to_string(info.location));
    }
    seen_idx[info.location] = static_cast<int32_t>(info.location);
  }

  if (textures_.size() == 0) {
    for (uint32_t i = 0; i < kMaxColorAttachments; i++) {
      ::dawn::TextureFormat fb_format{};

      if (i < render_pipeline->pipeline->GetColorAttachments().size()) {
        auto* amber_format = render_pipeline->pipeline->GetColorAttachments()[i]
                                 .buffer->GetFormat();
        if (!amber_format)
          return Result(
              "AttachBuffersAndTextures: One Color attachment has no "
              "format!");
        result = GetDawnTextureFormat(*amber_format, &fb_format);
        if (!result.IsSuccess())
          return result;
      } else {
        fb_format = ::dawn::TextureFormat::RGBA8Unorm;
      }

      textures_.emplace_back(
          MakeDawnTexture(*device_, fb_format, width, height));
      texture_views_.emplace_back(textures_.back().CreateView());
    }
  }

  // Attach depth-stencil texture
  auto* depthBuffer = render_pipeline->pipeline->GetDepthBuffer().buffer;
  if (depthBuffer) {
    if (!depth_stencil_texture_) {
      auto* amber_depth_stencil_format = depthBuffer->GetFormat();
      if (!amber_depth_stencil_format)
        return Result(
            "AttachBuffersAndTextures: The depth/stencil attachment has no "
            "format!");
      ::dawn::TextureFormat depth_stencil_format{};
      result = GetDawnTextureFormat(*amber_depth_stencil_format,
                                    &depth_stencil_format);
      if (!result.IsSuccess())
        return result;

      result = MakeTexture(*device_, depth_stencil_format, width, height,
                           &depth_stencil_texture_);
      if (!result.IsSuccess())
        return result;
      render_pipeline->depth_stencil_texture = depth_stencil_texture_;
    } else {
      render_pipeline->depth_stencil_texture = depth_stencil_texture_;
    }
  }

  // Attach index buffer
  if (render_pipeline->pipeline->GetIndexBuffer()) {
    render_pipeline->index_buffer = CreateBufferFromData(
        *device_,
        render_pipeline->pipeline->GetIndexBuffer()->ValuePtr()->data(),
        render_pipeline->pipeline->GetIndexBuffer()->GetSizeInBytes(),
        ::dawn::BufferUsage::Index);
  }

  // Attach vertex buffers
  for (auto& vertex_info : render_pipeline->pipeline->GetVertexBuffers()) {
    render_pipeline->vertex_buffers.emplace_back(CreateBufferFromData(
        *device_, vertex_info.buffer->ValuePtr()->data(),
        vertex_info.buffer->GetSizeInBytes(), ::dawn::BufferUsage::Vertex));
  }

  // Do not attach pushConstants
  if (render_pipeline->pipeline->GetPushConstantBuffer().buffer != nullptr) {
    return Result(
        "AttachBuffersAndTextures: Dawn does not support push constants!");
  }

  ::dawn::ShaderStage kAllStages =
      ::dawn::ShaderStage::Vertex | ::dawn::ShaderStage::Fragment;
  std::vector<std::vector<BindingInitializationHelper>> bindingInitalizerHelper(
      kMaxDawnBindGroup);
  std::vector<std::vector<::dawn::BindGroupLayoutBinding>> layouts_info(
      kMaxDawnBindGroup);
  uint32_t max_descriptor_set = 0;

  // Attach storage/uniform buffers
  ::dawn::BindGroupLayoutBinding empty_layout_info = {};

  if (!render_pipeline->pipeline->GetBuffers().empty()) {
    std::vector<uint32_t> max_binding_seen(kMaxDawnBindGroup, -1);
    for (auto& buf_info : render_pipeline->pipeline->GetBuffers()) {
      while (layouts_info[buf_info.descriptor_set].size() <= buf_info.binding)
        layouts_info[buf_info.descriptor_set].push_back(empty_layout_info);
    }
  }

  for (const auto& buf_info : render_pipeline->pipeline->GetBuffers()) {
    ::dawn::BufferUsage bufferUsage;
    ::dawn::BindingType bindingType;
    switch (buf_info.buffer->GetBufferType()) {
      case BufferType::kStorage: {
        bufferUsage = ::dawn::BufferUsage::Storage;
        bindingType = ::dawn::BindingType::StorageBuffer;
        break;
      }
      case BufferType::kUniform: {
        bufferUsage = ::dawn::BufferUsage::Uniform;
        bindingType = ::dawn::BindingType::UniformBuffer;
        break;
      }
      default: {
        return Result("AttachBuffersAndTextures: unknown buffer type: " +
                      std::to_string(static_cast<uint32_t>(
                          buf_info.buffer->GetBufferType())));
        break;
      }
    }

    if (buf_info.descriptor_set > kMaxDawnBindGroup - 1) {
      return Result("AttachBuffers: Dawn has a maximum of " +
                    std::to_string(kMaxDawnBindGroup) + " (descriptor sets)");
    }

    render_pipeline->buffers.emplace_back(
        CreateBufferFromData(*device_, buf_info.buffer->ValuePtr()->data(),
                             buf_info.buffer->GetMaxSizeInBytes(),
                             bufferUsage | ::dawn::BufferUsage::CopySrc |
                                 ::dawn::BufferUsage::CopyDst));

    render_pipeline->buffer_map[{buf_info.descriptor_set, buf_info.binding}] =
        render_pipeline->buffers.size() - 1;

    render_pipeline->used_descriptor_set.insert(buf_info.descriptor_set);
    max_descriptor_set = std::max(max_descriptor_set, buf_info.descriptor_set);

    ::dawn::BindGroupLayoutBinding layout_info;
    layout_info.binding = buf_info.binding;
    layout_info.visibility = kAllStages;
    layout_info.type = bindingType;
    layouts_info[buf_info.descriptor_set][buf_info.binding] = layout_info;

    BindingInitializationHelper tempBinding = BindingInitializationHelper(
        buf_info.binding, render_pipeline->buffers.back(), 0,
        buf_info.buffer->GetMaxSizeInBytes());
    bindingInitalizerHelper[buf_info.descriptor_set].push_back(tempBinding);
  }

  for (uint32_t i = 0; i < kMaxDawnBindGroup; i++) {
    if (layouts_info[i].size() > 0 && bindingInitalizerHelper[i].size() > 0) {
      ::dawn::BindGroupLayout bindGroupLayout =
          MakeBindGroupLayout(*device_, layouts_info[i]);
      render_pipeline->bind_group_layouts.push_back(bindGroupLayout);

      ::dawn::BindGroup bindGroup =
          MakeBindGroup(*device_, render_pipeline->bind_group_layouts[i],
                        bindingInitalizerHelper[i]);
      render_pipeline->bind_groups.push_back(bindGroup);
    } else if (i < max_descriptor_set) {
      ::dawn::BindGroupLayout bindGroupLayout =
          MakeBindGroupLayout(*device_, {});
      render_pipeline->bind_group_layouts.push_back(bindGroupLayout);

      ::dawn::BindGroup bindGroup =
          MakeBindGroup(*device_, render_pipeline->bind_group_layouts[i],
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
  std::vector<std::vector<::dawn::BindGroupLayoutBinding>> layouts_info(
      kMaxDawnBindGroup);
  uint32_t max_descriptor_set = 0;

  // Attach storage/uniform buffers
  ::dawn::BindGroupLayoutBinding empty_layout_info = {};

  if (!compute_pipeline->pipeline->GetBuffers().empty()) {
    std::vector<uint32_t> max_binding_seen(kMaxDawnBindGroup, -1);
    for (auto& buf_info : compute_pipeline->pipeline->GetBuffers()) {
      while (layouts_info[buf_info.descriptor_set].size() <= buf_info.binding)
        layouts_info[buf_info.descriptor_set].push_back(empty_layout_info);
    }
  }

  for (const auto& buf_info : compute_pipeline->pipeline->GetBuffers()) {
    ::dawn::BufferUsage bufferUsage;
    ::dawn::BindingType bindingType;
    switch (buf_info.buffer->GetBufferType()) {
      case BufferType::kStorage: {
        bufferUsage = ::dawn::BufferUsage::Storage;
        bindingType = ::dawn::BindingType::StorageBuffer;
        break;
      }
      case BufferType::kUniform: {
        bufferUsage = ::dawn::BufferUsage::Uniform;
        bindingType = ::dawn::BindingType::UniformBuffer;
        break;
      }
      default: {
        return Result("AttachBuffers: unknown buffer type: " +
                      std::to_string(static_cast<uint32_t>(
                          buf_info.buffer->GetBufferType())));
        break;
      }
    }

    if (buf_info.descriptor_set > kMaxDawnBindGroup - 1) {
      return Result("AttachBuffers: Dawn has a maximum of " +
                    std::to_string(kMaxDawnBindGroup) + " (descriptor sets)");
    }

    compute_pipeline->buffers.emplace_back(
        CreateBufferFromData(*device_, buf_info.buffer->ValuePtr()->data(),
                             buf_info.buffer->GetMaxSizeInBytes(),
                             bufferUsage | ::dawn::BufferUsage::CopySrc |
                                 ::dawn::BufferUsage::CopyDst));

    compute_pipeline->buffer_map[{buf_info.descriptor_set, buf_info.binding}] =
        compute_pipeline->buffers.size() - 1;

    compute_pipeline->used_descriptor_set.insert(buf_info.descriptor_set);
    max_descriptor_set = std::max(max_descriptor_set, buf_info.descriptor_set);

    ::dawn::BindGroupLayoutBinding layout_info;
    layout_info.binding = buf_info.binding;
    layout_info.visibility = ::dawn::ShaderStage::Compute;
    layout_info.type = bindingType;
    layouts_info[buf_info.descriptor_set][buf_info.binding] = layout_info;

    BindingInitializationHelper tempBinding = BindingInitializationHelper(
        buf_info.binding, compute_pipeline->buffers.back(), 0,
        buf_info.buffer->GetMaxSizeInBytes());
    bindingInitalizerHelper[buf_info.descriptor_set].push_back(tempBinding);
  }

  for (uint32_t i = 0; i < kMaxDawnBindGroup; i++) {
    if (layouts_info[i].size() > 0 && bindingInitalizerHelper[i].size() > 0) {
      ::dawn::BindGroupLayout bindGroupLayout =
          MakeBindGroupLayout(*device_, layouts_info[i]);
      compute_pipeline->bind_group_layouts.push_back(bindGroupLayout);

      ::dawn::BindGroup bindGroup =
          MakeBindGroup(*device_, compute_pipeline->bind_group_layouts[i],
                        bindingInitalizerHelper[i]);
      compute_pipeline->bind_groups.push_back(bindGroup);
    } else if (i < max_descriptor_set) {
      ::dawn::BindGroupLayout bindGroupLayout =
          MakeBindGroupLayout(*device_, {});
      compute_pipeline->bind_group_layouts.push_back(bindGroupLayout);

      ::dawn::BindGroup bindGroup =
          MakeBindGroup(*device_, compute_pipeline->bind_group_layouts[i],
                        bindingInitalizerHelper[i]);
      compute_pipeline->bind_groups.push_back(bindGroup);
    }
  }

  return {};
}

}  // namespace dawn
}  // namespace amber
