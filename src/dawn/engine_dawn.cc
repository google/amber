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
#include "src/make_unique.h"
#include "src/sleep.h"

namespace amber {
namespace dawn {

namespace {

// The minimum multiple row pitch observed on Dawn on Metal.  Increase this
// as needed for other Dawn backends.
const uint32_t kMinimumImageRowPitch = 256;

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

uint32_t Align(uint32_t value, size_t alignment) {
  assert(alignment <= UINT32_MAX);
  assert(alignment != 0);
  uint32_t alignment32 = static_cast<uint32_t>(alignment);
  return (value + (alignment32 - 1)) & ~(alignment32 - 1);
}
}  // namespace

// Maps the given buffer.  Assumes the buffer has usage bit
// ::dawn::BufferUsageBit::MapRead set.  Returns a MapResult structure, with
// the status saved in the .result member and the host pointer to the mapped
// data in the |.data| member. Mapping a buffer can fail if the context is
// lost, for example. In the failure case, the .data member of the result will
// be null.
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

MapResult MapTextureToHostBuffer(const RenderPipelineInfo& render_pipeline,
                                 const ::dawn::Device& device) {
  const auto width = render_pipeline.pipeline->GetFramebufferWidth();
  const auto height = render_pipeline.pipeline->GetFramebufferHeight();
  const auto pixelSize = render_pipeline.pipeline->GetColorAttachments()[0]
                             .buffer->GetTexelStride();
  const auto dawn_row_pitch = Align(width * pixelSize, kMinimumImageRowPitch);
  const auto fb_buffer_size = dawn_row_pitch * height + width * pixelSize;

  ::dawn::BufferDescriptor descriptor;
  descriptor.size = fb_buffer_size;
  descriptor.usage =
      ::dawn::BufferUsageBit::MapRead | ::dawn::BufferUsageBit::TransferDst;
  ::dawn::Buffer fb_buffer = device.CreateBuffer(&descriptor);

  ::dawn::TextureCopyView textureCopyView;
  textureCopyView.texture = render_pipeline.fb_texture;
  textureCopyView.level = 0;
  textureCopyView.slice = 0;
  textureCopyView.origin = {0, 0, 0};

  ::dawn::BufferCopyView bufferCopyView;
  bufferCopyView.buffer = fb_buffer;
  bufferCopyView.offset = 0;
  bufferCopyView.rowPitch = dawn_row_pitch;
  bufferCopyView.imageHeight = 0;

  ::dawn::Extent3D copySize = {width, height, 1};

  auto encoder = device.CreateCommandEncoder();
  encoder.CopyTextureToBuffer(&textureCopyView, &bufferCopyView, &copySize);

  auto commands = encoder.Finish();
  auto queue = device.CreateQueue();
  queue.Submit(1, &commands);

  MapResult map = MapBuffer(device, fb_buffer);
  const std::vector<amber::Pipeline::BufferInfo>& out_color_attachment =
      render_pipeline.pipeline->GetColorAttachments();

  for (size_t i = 0; i < out_color_attachment.size(); ++i) {
    auto& info = out_color_attachment[i];
    auto* values = info.buffer->ValuePtr();
    auto row_stride = pixelSize * width;
    assert(row_stride * height == info.buffer->GetSizeInBytes());
    // Each Dawn row has enough data to fill the target row.
    assert(dawn_row_pitch >= row_stride);
    values->resize(info.buffer->GetSizeInBytes());
    // Copy the framebuffer contents back into the host-side
    // framebuffer-buffer. In the Dawn buffer, the row stride is a multiple of
    // kMinimumImageRowPitch bytes, so it might have padding therefore memcpy
    // is done row by row.
    for (uint h = 0; h < height; h++) {
      std::memcpy(values->data() + h * row_stride,
                  static_cast<const uint8_t*>(map.data) + h * dawn_row_pitch,
                  row_stride);
    }
  }
  // Always unmap the buffer at the end of the engine's command.
  fb_buffer.Unmap();

  return map;
}

::dawn::Buffer CreateBufferFromData(const ::dawn::Device& device,
                                    const void* data,
                                    uint64_t size,
                                    ::dawn::BufferUsageBit usage) {
  ::dawn::BufferDescriptor descriptor;
  descriptor.size = size;
  descriptor.usage = usage | ::dawn::BufferUsageBit::TransferDst;

  ::dawn::Buffer buffer = device.CreateBuffer(&descriptor);
  buffer.SetSubData(0, size, reinterpret_cast<const uint8_t*>(data));
  return buffer;
}

::dawn::SamplerDescriptor GetDefaultSamplerDescriptor() {
  ::dawn::SamplerDescriptor desc;

  float kLodMin = 0.0;
  float kLodMax = 1000.0;

  desc.minFilter = ::dawn::FilterMode::Linear;
  desc.magFilter = ::dawn::FilterMode::Linear;
  desc.mipmapFilter = ::dawn::FilterMode::Linear;
  desc.addressModeU = ::dawn::AddressMode::Repeat;
  desc.addressModeV = ::dawn::AddressMode::Repeat;
  desc.addressModeW = ::dawn::AddressMode::Repeat;
  desc.lodMinClamp = kLodMin;
  desc.lodMaxClamp = kLodMax;
  desc.compareFunction = ::dawn::CompareFunction::Never;

  return desc;
}

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

::dawn::TextureCopyView CreateTextureCopyView(::dawn::Texture texture,
                                              uint32_t level,
                                              uint32_t slice,
                                              ::dawn::Origin3D origin) {
  ::dawn::TextureCopyView textureCopyView;
  textureCopyView.texture = texture;
  textureCopyView.level = level;
  textureCopyView.slice = slice;
  textureCopyView.origin = origin;

  return textureCopyView;
}

::dawn::BindGroupLayout MakeBindGroupLayout(
    const ::dawn::Device& device,
    std::initializer_list<::dawn::BindGroupLayoutBinding> bindingsInitializer) {
  constexpr ::dawn::ShaderStageBit kNoStages{};

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

::dawn::PipelineLayout MakeBasicPipelineLayout(
    const ::dawn::Device& device,
    const ::dawn::BindGroupLayout* bindGroupLayout) {
  ::dawn::PipelineLayoutDescriptor descriptor;
  if (bindGroupLayout) {
    descriptor.bindGroupLayoutCount = 1;
    descriptor.bindGroupLayouts = bindGroupLayout;
  } else {
    descriptor.bindGroupLayoutCount = 0;
    descriptor.bindGroupLayouts = nullptr;
  }
  return device.CreatePipelineLayout(&descriptor);
}

::dawn::TextureView CreateDefaultDepthStencilView(
    const ::dawn::Device& device,
    const ::dawn::TextureFormat depth_stencil_format) {
  ::dawn::TextureDescriptor descriptor;
  descriptor.dimension = ::dawn::TextureDimension::e2D;
  descriptor.size.width = 250;
  descriptor.size.height = 250;
  descriptor.size.depth = 1;
  descriptor.arrayLayerCount = 1;
  descriptor.sampleCount = 1;
  descriptor.format = depth_stencil_format;
  descriptor.mipLevelCount = 1;
  descriptor.usage = ::dawn::TextureUsageBit::OutputAttachment;
  auto depthStencilTexture = device.CreateTexture(&descriptor);
  return depthStencilTexture.CreateDefaultView();
}

::dawn::ShaderModule CreateShaderModuleFromResult(
    const ::dawn::Device& device,
    const shaderc::SpvCompilationResult& result) {
  // result.cend and result.cbegin return pointers to uint32_t.
  const uint32_t* resultBegin = result.cbegin();
  const uint32_t* resultEnd = result.cend();
  // So this size is in units of sizeof(uint32_t).
  ptrdiff_t resultSize = resultEnd - resultBegin;
  // SetSource takes data as uint32_t*.

  ::dawn::ShaderModuleDescriptor descriptor;
  descriptor.codeSize = static_cast<uint32_t>(resultSize);
  descriptor.code = result.cbegin();
  return device.CreateShaderModule(&descriptor);
}

shaderc_shader_kind ShadercShaderKind(::dawn::ShaderStage stage) {
  switch (stage) {
    case ::dawn::ShaderStage::Vertex:
      return shaderc_glsl_vertex_shader;
    case ::dawn::ShaderStage::Fragment:
      return shaderc_glsl_fragment_shader;
    case ::dawn::ShaderStage::Compute:
      return shaderc_glsl_compute_shader;
    default:
      return shaderc_glsl_vertex_shader;
  }
}

::dawn::ShaderModule CreateShaderModule(const ::dawn::Device& device,
                                        ::dawn::ShaderStage stage,
                                        const char* source) {
  shaderc_shader_kind kind = ShadercShaderKind(stage);

  shaderc::Compiler compiler;
  auto result =
      compiler.CompileGlslToSpv(source, strlen(source), kind, "myshader?");
  if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
    std::cerr << result.GetErrorMessage();
    return {};
  }
#ifdef DUMP_SPIRV_ASSEMBLY
  {
    shaderc::CompileOptions options;
    auto resultAsm = compiler.CompileGlslToSpvAssembly(
        source, strlen(source), kind, "myshader?", options);
    size_t sizeAsm = (resultAsm.cend() - resultAsm.cbegin());

    char* buffer = reinterpret_cast<char*>(malloc(sizeAsm + 1));
    memcpy(buffer, resultAsm.cbegin(), sizeAsm);
    buffer[sizeAsm] = '\0';
    printf("SPIRV ASSEMBLY DUMP START\n%s\nSPIRV ASSEMBLY DUMP END\n", buffer);
    free(buffer);
  }
#endif

#ifdef DUMP_SPIRV_JS_ARRAY
  printf("SPIRV JS ARRAY DUMP START\n");
  for (size_t i = 0; i < size; i++) {
    printf("%#010x", result.cbegin()[i]);
    if ((i + 1) % 4 == 0) {
      printf(",\n");
    } else {
      printf(", ");
    }
  }
  printf("\n");
  printf("SPIRV JS ARRAY DUMP END\n");
#endif

  return CreateShaderModuleFromResult(device, result);
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
    return Result("Dawn::SetShader: device is not created");
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
      return Result("Dawn::SetShader: failed to create shader");
    }
    if (module_for_type.count(type)) {
      Result("Dawn::SetShader: module for type already exists");
    }
    module_for_type[type] = shader;
  }

  switch (pipeline->GetType()) {
    case PipelineType::kCompute: {
      auto module = module_for_type[kShaderTypeCompute];
      if (!module)
        return Result("CreatePipeline: no compute shader provided");
      pipeline_map_[pipeline].compute_pipeline.reset(
          new ComputePipelineInfo(pipeline, module));
      break;
    }

    case PipelineType::kGraphics: {
      // TODO(dneto): Handle other shader types as well.  They are optional.
      auto vs = module_for_type[kShaderTypeVertex];
      auto fs = module_for_type[kShaderTypeFragment];
      if (!vs) {
        return Result(
            "CreatePipeline: no vertex shader provided for graphics "
            "pipeline");
      }
      if (!fs) {
        return Result(
            "CreatePipeline: no vertex shader provided for graphics "
            "pipeline");
      }
      pipeline_map_[pipeline].render_pipeline.reset(
          new RenderPipelineInfo(pipeline, vs, fs));
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
  RenderPipelineInfo* render_pipeline = GetRenderPipeline(command);
  if (!render_pipeline)
    return Result("Clear invoked on invalid or missing render pipeline");

  // TODO(dneto): Likely, we can create the render objects during
  // CreatePipeline.
  Result result = CreateFramebufferIfNeeded(render_pipeline);
  if (!result.IsSuccess())
    return result;

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
  ::dawn::RenderPassColorAttachmentDescriptor* color_attachment_descriptor[] = {
      &color_attachment};

  // Then describe the depthStencil attachment, and how it is initialized
  // via the load ops. Both load op are "clear" to the clear values.
  ::dawn::RenderPassDepthStencilAttachmentDescriptor depth_stencil_attachment =
      ::dawn::RenderPassDepthStencilAttachmentDescriptor();
  ::dawn::RenderPassDepthStencilAttachmentDescriptor* depth_stencil_descriptor =
      nullptr;
  if (render_pipeline->depth_stencil_texture) {
    depth_stencil_attachment.attachment =
        render_pipeline->depth_stencil_texture.CreateDefaultView();
    depth_stencil_attachment.clearDepth = render_pipeline->clear_depth_value;
    depth_stencil_attachment.clearStencil =
        render_pipeline->clear_stencil_value;
    depth_stencil_attachment.depthLoadOp = ::dawn::LoadOp::Clear;
    depth_stencil_attachment.depthStoreOp = ::dawn::StoreOp::Store;
    depth_stencil_attachment.stencilLoadOp = ::dawn::LoadOp::Clear;
    depth_stencil_attachment.stencilStoreOp = ::dawn::StoreOp::Store;
    depth_stencil_descriptor = &depth_stencil_attachment;
  }

  // Attach the depth/stencil and colour attachments to the render pass.
  ::dawn::RenderPassDescriptor rpd;
  rpd.colorAttachmentCount = 1;
  rpd.colorAttachments = color_attachment_descriptor;
  rpd.depthStencilAttachment = depth_stencil_descriptor;

  // Record the render pass as a command.
  auto encoder = device_->CreateCommandEncoder();
  ::dawn::RenderPassEncoder pass = encoder.BeginRenderPass(&rpd);
  pass.EndPass();
  // Finish recording the command buffer.  It only has one command.
  auto command_buffer = encoder.Finish();
  // Submit the command.
  auto queue = device_->CreateQueue();
  queue.Submit(1, &command_buffer);
  // Copy result back
  MapResult map = MapTextureToHostBuffer(*render_pipeline, *device_);

  return map.result;
}

Result EngineDawn::Helper::CreateRenderPipelineDescriptor(
    const RenderPipelineInfo& render_pipeline,
    const ::dawn::Device& device) {
  Result result;

  auto* amber_format =
      render_pipeline.pipeline->GetColorAttachments()[0].buffer->GetFormat();
  if (!amber_format)
    return Result("Color attachment 0 has no format!");
  ::dawn::TextureFormat fb_format{};
  GetDawnTextureFormat(*amber_format, &fb_format);
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
    depth_stencil_format = ::dawn::TextureFormat::D32FloatS8Uint;
  }
  renderPipelineDescriptor.layout = MakeBasicPipelineLayout(device, nullptr);
  renderPipelineDescriptor.primitiveTopology =
      ::dawn::PrimitiveTopology::TriangleList;
  renderPipelineDescriptor.sampleCount = 1;

  // Set defaults for the vertex stage descriptor.
  cVertexStage.module = render_pipeline.vertex_shader;
  cVertexStage.entryPoint = "main";
  renderPipelineDescriptor.vertexStage = std::move(&cVertexStage);

  // Set defaults for the fragment stage desriptor.
  cFragmentStage.module = render_pipeline.fragment_shader;
  cFragmentStage.entryPoint = "main";
  renderPipelineDescriptor.fragmentStage = std::move(&cFragmentStage);

  /// Set defaults for the input state descriptors.
  tempInputState.indexFormat = ::dawn::IndexFormat::Uint32;
  // Fill the default values for vertexInput.
  vertexInput.inputSlot = 0;
  vertexInput.stride = 0;
  vertexInput.stepMode = ::dawn::InputStepMode::Vertex;
  for (uint32_t i = 0; i < kMaxVertexInputs; ++i) {
    tempInputs[i] = vertexInput;
  }
  tempInputs[0].stride = 4 * sizeof(float);
  tempInputState.inputs = std::move(&tempInputs[0]);
  // Fill the default values for vertexAttribute.
  vertexAttribute.shaderLocation = 0;
  vertexAttribute.inputSlot = 0;
  vertexAttribute.offset = 0;
  vertexAttribute.format = ::dawn::VertexFormat::Float;
  for (uint32_t i = 0; i < kMaxVertexAttributes; ++i) {
    tempAttributes[i] = vertexAttribute;
  }
  tempAttributes[0].format = ::dawn::VertexFormat::Float4;
  tempInputState.attributes = &tempAttributes[0];
  tempInputState.numAttributes = 1;
  tempInputState.numInputs = 1;
  renderPipelineDescriptor.inputState = &tempInputState;

  /// Set defaults for the color state descriptors.
  renderPipelineDescriptor.colorStateCount = 1;
  blend.operation = ::dawn::BlendOperation::Add;
  blend.srcFactor = ::dawn::BlendFactor::One;
  blend.dstFactor = ::dawn::BlendFactor::Zero;
  colorStateDescriptor.format = fb_format;
  colorStateDescriptor.alphaBlend = blend;
  colorStateDescriptor.colorBlend = blend;
  colorStateDescriptor.colorWriteMask = ::dawn::ColorWriteMask::All;
  for (uint32_t i = 0; i < kMaxColorAttachments; ++i) {
    mColorStates[i] = colorStateDescriptor;
    cColorStates[i] = &mColorStates[i];
  }
  cColorStates[0]->format = fb_format;
  renderPipelineDescriptor.colorStates = &cColorStates[0];

  /// Set defaults for the depth stencil state descriptors.
  stencilFace.compare = ::dawn::CompareFunction::Always;
  stencilFace.failOp = ::dawn::StencilOperation::Keep;
  stencilFace.depthFailOp = ::dawn::StencilOperation::Keep;
  stencilFace.passOp = ::dawn::StencilOperation::Keep;
  cDepthStencilState.format = fb_format;
  cDepthStencilState.depthWriteEnabled = false;
  cDepthStencilState.depthCompare = ::dawn::CompareFunction::Always;
  cDepthStencilState.stencilBack = stencilFace;
  cDepthStencilState.stencilFront = stencilFace;
  cDepthStencilState.stencilReadMask = 0xff;
  cDepthStencilState.stencilWriteMask = 0xff;
  cDepthStencilState.format = depth_stencil_format;
  renderPipelineDescriptor.depthStencilState = &cDepthStencilState;

  return {};
}

Result EngineDawn::Helper::CreateRenderPassDescriptor(
    const RenderPipelineInfo& render_pipeline,
    const ::dawn::Device& device) {
  std::initializer_list<::dawn::TextureView> colorAttachmentInfo = {
      render_pipeline.fb_texture.CreateDefaultView()};

  for (uint32_t i = 0; i < kMaxColorAttachments; ++i) {
    mColorAttachmentsInfo[i].loadOp = ::dawn::LoadOp::Clear;
    mColorAttachmentsInfo[i].storeOp = ::dawn::StoreOp::Store;
    mColorAttachmentsInfo[i].clearColor = {0.0f, 0.0f, 0.0f, 0.0f};
    cColorAttachmentsInfoPtr[i] = nullptr;
  }

  cDepthStencilAttachmentInfo.clearDepth = 1.0f;
  cDepthStencilAttachmentInfo.clearStencil = 0;
  cDepthStencilAttachmentInfo.depthLoadOp = ::dawn::LoadOp::Clear;
  cDepthStencilAttachmentInfo.depthStoreOp = ::dawn::StoreOp::Store;
  cDepthStencilAttachmentInfo.stencilLoadOp = ::dawn::LoadOp::Clear;
  cDepthStencilAttachmentInfo.stencilStoreOp = ::dawn::StoreOp::Store;

  renderPassDescriptor.colorAttachmentCount =
      static_cast<uint32_t>(colorAttachmentInfo.size());
  uint32_t colorAttachmentIndex = 0;
  for (const ::dawn::TextureView& colorAttachment : colorAttachmentInfo) {
    if (colorAttachment.Get() != nullptr) {
      mColorAttachmentsInfo[colorAttachmentIndex].attachment = colorAttachment;
      cColorAttachmentsInfoPtr[colorAttachmentIndex] =
          &mColorAttachmentsInfo[colorAttachmentIndex];
    }
    ++colorAttachmentIndex;
  }
  renderPassDescriptor.colorAttachments = cColorAttachmentsInfoPtr;

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
    depth_stencil_format = ::dawn::TextureFormat::D32FloatS8Uint;
  }

  ::dawn::TextureView depthStencilView =
      CreateDefaultDepthStencilView(device, depth_stencil_format);
  if (depthStencilView.Get() != nullptr) {
    cDepthStencilAttachmentInfo.attachment = depthStencilView;
    renderPassDescriptor.depthStencilAttachment = &cDepthStencilAttachmentInfo;
  } else {
    renderPassDescriptor.depthStencilAttachment = nullptr;
  }

  return {};
}

Result EngineDawn::DoDrawRect(const DrawRectCommand* command) {
  RenderPipelineInfo* render_pipeline = GetRenderPipeline(command);
  if (!render_pipeline)
    return Result("Clear invoked on invalid or missing render pipeline");
  Result result = CreateFramebufferIfNeeded(render_pipeline);
  if (!result.IsSuccess())
    return result;

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
  ::dawn::Buffer indexBuffer = CreateBufferFromData(
      *device_, indexData, sizeof(indexData), ::dawn::BufferUsageBit::Index);

  std::vector<Value> values(8);
  // Bottom left
  values[0].SetDoubleValue(static_cast<double>(x));
  values[1].SetDoubleValue(static_cast<double>(y + rectangleHeight));
  // Top left
  values[2].SetDoubleValue(static_cast<double>(x));
  values[3].SetDoubleValue(static_cast<double>(y));
  // Top right
  values[4].SetDoubleValue(static_cast<double>(x + rectangleWidth));
  values[5].SetDoubleValue(static_cast<double>(y));
  // Bottom right
  values[6].SetDoubleValue(static_cast<double>(x + rectangleWidth));
  values[7].SetDoubleValue(static_cast<double>(y + rectangleHeight));

  static const float vertexData[4 * 4] = {
      values[0].AsFloat(), values[1].AsFloat(), 0.0f, 1.0f,
      values[2].AsFloat(), values[3].AsFloat(), 0.0f, 1.0f,
      values[4].AsFloat(), values[5].AsFloat(), 0.0f, 1.0f,
      values[6].AsFloat(), values[7].AsFloat(), 0.0f, 1.0f,
  };

  ::dawn::Buffer vertexBuffer = CreateBufferFromData(
      *device_, vertexData, sizeof(vertexData), ::dawn::BufferUsageBit::Vertex);

  Helper helper;
  helper.CreateRenderPipelineDescriptor(*render_pipeline, *device_);
  helper.CreateRenderPassDescriptor(*render_pipeline, *device_);
  ::dawn::RenderPipelineDescriptor* renderPipelineDescriptor =
      &helper.renderPipelineDescriptor;
  ::dawn::RenderPassDescriptor* renderPassDescriptor =
      &helper.renderPassDescriptor;

  const ::dawn::RenderPipeline pipeline =
      device_->CreateRenderPipeline(renderPipelineDescriptor);
  static const uint64_t vertexBufferOffsets[1] = {0};
  ::dawn::CommandEncoder encoder = device_->CreateCommandEncoder();
  ::dawn::RenderPassEncoder pass =
      encoder.BeginRenderPass(renderPassDescriptor);
  pass.SetPipeline(pipeline);
  pass.SetVertexBuffers(0, 1, &vertexBuffer, vertexBufferOffsets);
  pass.SetIndexBuffer(indexBuffer, 0);
  pass.DrawIndexed(6, 1, 0, 0, 0);
  pass.EndPass();

  ::dawn::CommandBuffer commands = encoder.Finish();
  ::dawn::Queue queue = device_->CreateQueue();
  queue.Submit(1, &commands);

  MapResult map = MapTextureToHostBuffer(*render_pipeline, *device_);

  return map.result;
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

  // First make the Dawn color attachment textures that the render pipeline
  // will write into.
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

  {
    ::dawn::Texture fb_texture;

    result = MakeTexture(*device_, fb_format, width, height, &fb_texture);
    if (!result.IsSuccess())
      return result;
    render_pipeline->fb_texture = std::move(fb_texture);
  }

  // After that, only create the Dawn depth-stencil texture if the Amber
  // depth-stencil texture exists.
  auto* depthBuffer = render_pipeline->pipeline->GetDepthBuffer().buffer;
  if (depthBuffer) {
    auto* amber_depth_stencil_format = depthBuffer->GetFormat();

    if (!amber_depth_stencil_format)
      return Result("The depth/stencil attachment has no format!");

    ::dawn::TextureFormat depth_stencil_format{};
    result = GetDawnTextureFormat(*amber_depth_stencil_format,
                                  &depth_stencil_format);
    if (!result.IsSuccess())
      return result;

    ::dawn::Texture depth_stencil_texture;

    result = MakeTexture(*device_, depth_stencil_format, width, height,
                         &depth_stencil_texture);
    if (!result.IsSuccess())
      return result;
    render_pipeline->depth_stencil_texture = std::move(depth_stencil_texture);
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
