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

#ifndef SRC_ENGINE_H_
#define SRC_ENGINE_H_

#include <memory>
#include <string>
#include <vector>

#include "amber/amber.h"
#include "amber/result.h"
#include "src/buffer_data.h"
#include "src/command.h"
#include "src/feature.h"
#include "src/format.h"
#include "src/shader_data.h"

namespace amber {

enum class PipelineType : uint8_t {
  kCompute = 0,
  kGraphics,
};

enum class ResourceInfoType : uint8_t {
  kBuffer = 0,
  kImage,
};

struct ResourceInfo {
  ResourceInfoType type = ResourceInfoType::kBuffer;

  // Key metrics of a 2D image.
  // For higher dimensions or arrayed images, we would need more strides.
  // For example, see VkSubresourceLayout.
  struct {
    uint32_t texel_stride = 0;  // Number of bytes for a single texel.
    uint32_t row_stride = 0;  // Number of bytes between successive pixel rows.
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t depth = 0;
  } image_info;

  // The size in bytes of Vulkan memory pointed by |cpu_memory|.
  // For the case when it is an image resource, |size_in_bytes| must
  // be |image_info.row_stride * image_info.height * image_info.depth|.
  size_t size_in_bytes = 0;

  // If the primitive type of resource is the same with the type
  // of actual data, the alignment must be properly determined by
  // Vulkan's internal memory allocation. In these cases, script
  // writers can assume that there is no alignment issues.
  const void* cpu_memory = nullptr;
};

class Engine {
 public:
  static std::unique_ptr<Engine> Create(EngineType type);

  virtual ~Engine();

  // Initialize the engine. The |features| list is a set of required features
  // for the device. The |extensions| list is a list of required extensions.
  virtual Result Initialize(const std::vector<Feature>& features,
                            const std::vector<std::string>& extensions) = 0;

  // Initialize the engine with the provided device. The device is _not_ owned
  // by the engine and should not be destroyed. The |features| and |extensions|
  // are passed into InitializeWithConfig for validation purposes only. If
  // possible the engine should verify the provided config specifies the
  // constraints in |features| and |extensions| and fail if those constrains
  // are not met.
  virtual Result InitializeWithConfig(
      EngineConfig* config,
      const std::vector<Feature>& features,
      const std::vector<std::string>& extensions) = 0;

  // Shutdown the engine and cleanup any resources.
  virtual Result Shutdown() = 0;

  // Enable |feature|. If the feature requires a pixel format it will be
  // provided in |format|, otherwise |format| is a nullptr. If the feature
  // requires a uint32 value it will be set in the |uint32_t|.
  virtual Result AddRequirement(Feature feature,
                                const Format* format,
                                uint32_t) = 0;

  // Create graphics pipeline.
  virtual Result CreatePipeline(PipelineType type) = 0;

  // Set the shader of |type| to the binary |data|.
  virtual Result SetShader(ShaderType type,
                           const std::vector<uint32_t>& data) = 0;

  // Provides the data for a given buffer to be bound at the given location
  // This is used to declare and populate vertex and index inputs to a graphics
  // pipeline.
  virtual Result SetBuffer(BufferType type,
                           uint8_t location,
                           const Format& format,
                           const std::vector<Value>& data) = 0;

  // Execute the clear color command
  virtual Result DoClearColor(const ClearColorCommand* cmd) = 0;

  // Execute the clear stencil command
  virtual Result DoClearStencil(const ClearStencilCommand* cmd) = 0;

  // Execute the clear depth command
  virtual Result DoClearDepth(const ClearDepthCommand* cmd) = 0;

  // Execute the clear command
  virtual Result DoClear(const ClearCommand* cmd) = 0;

  // Execute the draw rect command
  virtual Result DoDrawRect(const DrawRectCommand* cmd) = 0;

  // Execute the draw arrays command
  virtual Result DoDrawArrays(const DrawArraysCommand* cmd) = 0;

  // Execute the compute command
  virtual Result DoCompute(const ComputeCommand* cmd) = 0;

  // Execute the entry point command
  virtual Result DoEntryPoint(const EntryPointCommand* cmd) = 0;

  // Execute the patch command
  virtual Result DoPatchParameterVertices(
      const PatchParameterVerticesCommand* cmd) = 0;

  // Execute the buffer command.
  // This declares an Amber buffer to be bound to a descriptor.
  // This covers both Vulkan buffers and images.
  virtual Result DoBuffer(const BufferCommand* cmd) = 0;

  // Run all queued commands and copy frame buffer data to the host
  // if graphics pipeline.
  virtual Result DoProcessCommands() = 0;

  // Get stride, width, height, and memory pointer of frame buffer.
  // This is only valid if the buffer of framebuffer is mapped into
  // the host address space. In particular, if we have run
  // DoProcessCommands() and since then no graphics pipeline drawing
  // commands have occurred e.g., DoClear, DoDrawArrays, DoDrawRect.
  virtual Result GetFrameBufferInfo(ResourceInfo* info) = 0;

  // Copy the contents of the resource bound to the given descriptor
  // and get the resource information e.g., size for buffer, width,
  // height, depth for image of descriptor given as |descriptor_set|
  // and |binding|.
  virtual Result GetDescriptorInfo(const uint32_t descriptor_set,
                                   const uint32_t binding,
                                   ResourceInfo* info) = 0;

 protected:
  Engine();
};

}  // namespace amber

#endif  // SRC_ENGINE_H_
