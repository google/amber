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
#include "src/buffer.h"
#include "src/command.h"
#include "src/format.h"
#include "src/pipeline.h"

namespace amber {

/// EngineData stores information used during engine execution.
struct EngineData {
  /// The timeout to use for fences, in milliseconds.
  uint32_t fence_timeout_ms = 1000;
};

/// Abstract class which describes a backing engine for Amber.
///
/// The engine class has a defined lifecycle.
///  1. The engine is created through Engine::Create.
///  2. Engine::Initialize is called to provide the engine with the configured
///     graphics device.
///  3. Engine::CreatePipeline is called for each pipeline. The pipelines are
///     fully specified at this point and include:
///     * All compiled shader binaries
///     * Vertex, Index, Storage, Uniform, Push Constant buffers
///     * Colour attachment, and depth/stencil attachment buffers.
///     * Extra engine data.
///     The buffers all may have default values to be loaded into the device.
///  4. Engine::Do* is called for each command.
///     Note, it is assumed that the amber::Buffers are updated at the end of
///     each Do* command and can be used immediately for comparisons.
///  5. Engine destructor is called.
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
