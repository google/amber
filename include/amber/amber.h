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

#ifndef AMBER_AMBER_H_
#define AMBER_AMBER_H_

#include <stdint.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "amber/recipe.h"
#include "amber/result.h"
#include "amber/value.h"

namespace amber {

/// The shader map is a map from the name of a shader to the spirv-binary
/// which is the compiled representation of that named shader.
using ShaderMap = std::map<std::string, std::vector<uint32_t>>;

enum class EngineType : uint8_t {
  /// Use the Vulkan backend, if available
  kVulkan = 0,
  /// Use the Dawn backend, if available
  kDawn,
};

/// Override point of engines to add their own configuration.
struct EngineConfig {
  virtual ~EngineConfig();
};

struct BufferInfo {
  BufferInfo();
  BufferInfo(const BufferInfo&);
  ~BufferInfo();

  BufferInfo& operator=(const BufferInfo&);

  /// Holds the buffer name
  std::string buffer_name;
  /// Holds the buffer width
  uint32_t width = 0;
  /// Holds the buffer height
  uint32_t height = 0;
  /// Contains the buffer internal data
  std::vector<Value> values;
};

/// Delegate class for various hook functions
class Delegate {
 public:
  virtual ~Delegate();

  /// Log the given message
  virtual void Log(const std::string& message) = 0;
  /// Tells whether to log the graphics API calls
  virtual bool LogGraphicsCalls() const = 0;
  /// Tells whether to log the duration of graphics API calls
  virtual bool LogGraphicsCallsTime() const = 0;
  /// Returns the current timestamp in nanoseconds
  virtual uint64_t GetTimestampNs() const = 0;
};

struct Options {
  Options();
  ~Options();

  /// Sets the engine to be created. Default Vulkan.
  EngineType engine = EngineType::kVulkan;
  /// Holds engine specific configuration.
  std::unique_ptr<EngineConfig> config;
  /// The SPIR-V environment to target.
  std::string spv_env;
  /// Lists the buffers to extract at the end of the execution
  std::vector<BufferInfo> extractions;
  /// Terminate after creating the pipelines.
  bool pipeline_create_only = false;
  /// Delegate implementation
  Delegate* delegate = nullptr;
};

/// Main interface to the Amber environment.
class Amber {
 public:
  Amber();
  ~Amber();

  /// Parse the given |data| into the |recipe|.
  amber::Result Parse(const std::string& data, amber::Recipe* recipe);

  /// Determines whether the engine supports all features required by the
  /// |recipe|. Modifies the |recipe| by applying some of the |opts| to the
  /// recipe's internal state.
  amber::Result AreAllRequirementsSupported(const amber::Recipe* recipe,
                                            Options* opts);

  /// Executes the given |recipe| with the provided |opts|. Returns a
  /// |Result| which indicates if the execution succeded. Modifies the
  /// |recipe| by applying some of the |opts| to the recipe's internal
  /// state.
  amber::Result Execute(const amber::Recipe* recipe, Options* opts);

  /// Executes the given |recipe| with the provided |opts|. Will use
  /// |shader_map| to lookup shader data before attempting to compile the
  /// shader if possible.
  amber::Result ExecuteWithShaderData(const amber::Recipe* recipe,
                                      Options* opts,
                                      const ShaderMap& shader_data);
};

}  // namespace amber

#endif  // AMBER_AMBER_H_
