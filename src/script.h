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

#ifndef SRC_SCRIPT_H_
#define SRC_SCRIPT_H_

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "amber/recipe.h"
#include "amber/result.h"
#include "src/buffer.h"
#include "src/command.h"
#include "src/engine.h"
#include "src/feature.h"
#include "src/pipeline.h"
#include "src/shader.h"

namespace amber {

/// Class representing the script to be run against an engine.
class Script : public RecipeImpl {
 public:
  Script();
  ~Script() override;

  /// Retrieves information on the shaders in the given script.
  std::vector<ShaderInfo> GetShaderInfo() const override;

  /// Adds |pipeline| to the list of known pipelines. The |pipeline| must have
  /// a unique name over all pipelines in the script.
  Result AddPipeline(std::unique_ptr<Pipeline> pipeline) {
    if (name_to_pipeline_.count(pipeline->GetName()) > 0)
      return Result("duplicate pipeline name provided");

    pipelines_.push_back(std::move(pipeline));
    name_to_pipeline_[pipelines_.back()->GetName()] = pipelines_.back().get();
    return {};
  }

  /// Retrieves the pipeline with |name|, |nullptr| if not found.
  Pipeline* GetPipeline(const std::string& name) const {
    auto it = name_to_pipeline_.find(name);
    return it == name_to_pipeline_.end() ? nullptr : it->second;
  }

  /// Retrieves a list of all pipelines.
  const std::vector<std::unique_ptr<Pipeline>>& GetPipelines() const {
    return pipelines_;
  }

  /// Adds |shader| to the list of known shaders. The |shader| must have a
  /// unique name over all shaders in the script.
  Result AddShader(std::unique_ptr<Shader> shader) {
    if (name_to_shader_.count(shader->GetName()) > 0)
      return Result("duplicate shader name provided");

    shaders_.push_back(std::move(shader));
    name_to_shader_[shaders_.back()->GetName()] = shaders_.back().get();
    return {};
  }

  /// Retrieves the shader with |name|, |nullptr| if not found.
  Shader* GetShader(const std::string& name) const {
    auto it = name_to_shader_.find(name);
    return it == name_to_shader_.end() ? nullptr : it->second;
  }

  /// Retrieves a list of all shaders.
  const std::vector<std::unique_ptr<Shader>>& GetShaders() const {
    return shaders_;
  }

  /// Adds |buffer| to the list of known buffers. The |buffer| must have a
  /// unique name over all buffers in the script.
  Result AddBuffer(std::unique_ptr<Buffer> buffer) {
    if (name_to_buffer_.count(buffer->GetName()) > 0)
      return Result("duplicate buffer name provided");

    buffers_.push_back(std::move(buffer));
    name_to_buffer_[buffers_.back()->GetName()] = buffers_.back().get();
    return {};
  }

  /// Retrieves the buffer with |name|, |nullptr| if not found.
  Buffer* GetBuffer(const std::string& name) const {
    auto it = name_to_buffer_.find(name);
    return it == name_to_buffer_.end() ? nullptr : it->second;
  }

  /// Retrieves a list of all buffers.
  const std::vector<std::unique_ptr<Buffer>>& GetBuffers() const {
    return buffers_;
  }

  /// Adds |feature| to the list of features that must be supported by the
  /// engine.
  void AddRequiredFeature(Feature feature) {
    engine_info_.required_features.push_back(feature);
  }

  /// Retrieves a list of features required for this script.
  const std::vector<Feature>& RequiredFeatures() const {
    return engine_info_.required_features;
  }

  /// Adds |ext| to the list of extensions that must be supported by the engine.
  void AddRequiredExtension(const std::string& ext) {
    engine_info_.required_extensions.push_back(ext);
  }

  /// Retrieves a list of extensions required for this script.
  const std::vector<std::string>& RequiredExtensions() const {
    return engine_info_.required_extensions;
  }

  /// Retrieves the engine configuration data for this script.
  EngineData& GetEngineData() { return engine_data_; }
  /// Retrieves the engine configuration data for this script.
  const EngineData& GetEngineData() const { return engine_data_; }

  /// Sets |cmds| to the list of commands to execute against the engine.
  void SetCommands(std::vector<std::unique_ptr<Command>> cmds) {
    commands_ = std::move(cmds);
  }

  /// Retrieves the list of commands to execute against the engine.
  const std::vector<std::unique_ptr<Command>>& GetCommands() const {
    return commands_;
  }

 private:
  struct {
    std::vector<Feature> required_features;
    std::vector<std::string> required_extensions;
  } engine_info_;

  EngineData engine_data_;
  std::map<std::string, Shader*> name_to_shader_;
  std::map<std::string, Buffer*> name_to_buffer_;
  std::map<std::string, Pipeline*> name_to_pipeline_;
  std::vector<std::unique_ptr<Shader>> shaders_;
  std::vector<std::unique_ptr<Command>> commands_;
  std::vector<std::unique_ptr<Buffer>> buffers_;
  std::vector<std::unique_ptr<Pipeline>> pipelines_;
};

}  // namespace amber

#endif  // SRC_SCRIPT_H_
