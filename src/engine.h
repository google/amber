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
#include "src/format.h"
#include "src/pipeline.h"

namespace amber {

/// EngineData stores information used during engine execution.
struct EngineData {
  /// The timeout to use for fences, in milliseconds.
  uint32_t fence_timeout_ms = 100;
};

/// Contains information relating to a backing resource from the engine.
struct ResourceInfo {
  /// The size in bytes of Vulkan memory pointed by |cpu_memory|.
  /// For the case when it is an image resource, |size_in_bytes| must
  /// be |image_info.row_stride * image_info.height * image_info.depth|.
  size_t size_in_bytes = 0;

  /// If the primitive type of resource is the same with the type
  /// of actual data, the alignment must be properly determined by
  /// Vulkan's internal memory allocation. In these cases, script
  /// writers can assume that there is no alignment issues.
  const void* cpu_memory = nullptr;
};

/// Abstract class which describes a backing engine for Amber.
class Engine {
 public:
  /// Creates a new engine of the requested |type|.
  static std::unique_ptr<Engine> Create(EngineType type);

  virtual ~Engine();

  /// Initialize the engine with the provided config. The config is _not_ owned
  /// by the engine and will not be destroyed. The |features| and |extensions|
  /// are for validation purposes only. If possible the engine should verify
  /// that the constraints in |features| and |extensions| are valid and fail
  /// otherwise.
  virtual Result Initialize(
      EngineConfig* config,
      Delegate* delegate,
      const std::vector<std::string>& features,
      const std::vector<std::string>& instance_extensions,
      const std::vector<std::string>& device_extensions) = 0;

  /// Shutdown the engine and cleanup any resources.
  virtual Result Shutdown() = 0;

  /// Create graphics pipeline.
  virtual Result CreatePipeline(Pipeline* pipeline) = 0;

  /// Execute the clear color command
  virtual Result DoClearColor(const ClearColorCommand* cmd) = 0;

  /// Execute the clear stencil command
  virtual Result DoClearStencil(const ClearStencilCommand* cmd) = 0;

  /// Execute the clear depth command
  virtual Result DoClearDepth(const ClearDepthCommand* cmd) = 0;

  /// Execute the clear command
  virtual Result DoClear(const ClearCommand* cmd) = 0;

  /// Execute the draw rect command
  virtual Result DoDrawRect(const DrawRectCommand* cmd) = 0;

  /// Execute the draw arrays command
  virtual Result DoDrawArrays(const DrawArraysCommand* cmd) = 0;

  /// Execute the compute command
  virtual Result DoCompute(const ComputeCommand* cmd) = 0;

  /// Execute the entry point command
  virtual Result DoEntryPoint(const EntryPointCommand* cmd) = 0;

  /// Execute the patch command
  virtual Result DoPatchParameterVertices(
      const PatchParameterVerticesCommand* cmd) = 0;

  /// Execute the buffer command.
  /// This declares an Amber buffer to be bound to a descriptor.
  /// This covers both Vulkan buffers and images.
  virtual Result DoBuffer(const BufferCommand* cmd) = 0;

  /// Run all queued commands and copy frame buffer data to the host
  /// if graphics pipeline.
  virtual Result DoProcessCommands(amber::Pipeline* pipeline) = 0;

  /// Copy the content of the framebuffer into |values|, each value is a pixel
  /// in R8G8B8A8 format.
  virtual Result GetFrameBuffer(Buffer* buffer, std::vector<Value>* values) = 0;

  /// Copy the contents of the resource bound to the given descriptor
  /// and get the resource information e.g., size for buffer, width,
  /// height, depth for image of descriptor given as |descriptor_set|
  /// and |binding|.
  virtual Result GetDescriptorInfo(Pipeline* pipeline,
                                   const uint32_t descriptor_set,
                                   const uint32_t binding,
                                   ResourceInfo* info) = 0;

  /// Sets the engine data to use.
  void SetEngineData(const EngineData& data) { engine_data_ = data; }

 protected:
  Engine();

  /// Retrieves the engine data.
  const EngineData& GetEngineData() const { return engine_data_; }

 private:
  EngineData engine_data_;
};

}  // namespace amber

#endif  // SRC_ENGINE_H_
