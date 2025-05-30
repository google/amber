// Copyright 2018 The Amber Authors.
// Copyright (C) 2024 Advanced Micro Devices, Inc. All rights reserved.
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

#include <algorithm>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "amber/recipe.h"
#include "amber/result.h"
#include "src/acceleration_structure.h"
#include "src/buffer.h"
#include "src/command.h"
#include "src/engine.h"
#include "src/format.h"
#include "src/pipeline.h"
#include "src/sampler.h"
#include "src/shader.h"
#include "src/virtual_file_store.h"

namespace amber {

/// Class representing the script to be run against an engine.
class Script : public RecipeImpl {
 public:
  Script();
  ~Script() override;

  bool IsKnownFeature(const std::string& name) const;
  bool IsKnownProperty(const std::string& name) const;

  /// Retrieves information on the shaders in the given script.
  std::vector<ShaderInfo> GetShaderInfo() const override;

  /// Returns required features in the given recipe.
  std::vector<std::string> GetRequiredFeatures() const override {
    return engine_info_.required_features;
  }

  std::vector<std::string> GetRequiredProperties() const override {
    return engine_info_.required_properties;
  }

  /// Returns required device extensions in the given recipe.
  std::vector<std::string> GetRequiredDeviceExtensions() const override {
    return engine_info_.required_device_extensions;
  }

  /// Returns required instance extensions in the given recipe.
  std::vector<std::string> GetRequiredInstanceExtensions() const override {
    return engine_info_.required_instance_extensions;
  }

  /// Sets the fence timeout to |timeout_ms|.
  void SetFenceTimeout(uint32_t timeout_ms) override {
    engine_data_.fence_timeout_ms = timeout_ms;
  }

  /// Sets or clears runtime layer bit to |enabled|.
  void SetPipelineRuntimeLayerEnabled(bool enabled) override {
    engine_data_.pipeline_runtime_layer_enabled = enabled;
  }

  /// Adds |pipeline| to the list of known pipelines. The |pipeline| must have
  /// a unique name over all pipelines in the script.
  Result AddPipeline(std::unique_ptr<Pipeline> pipeline) {
    if (name_to_pipeline_.count(pipeline->GetName()) > 0) {
      return Result("duplicate pipeline name provided");
    }

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
    if (name_to_shader_.count(shader->GetName()) > 0) {
      return Result("duplicate shader name provided");
    }

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

  /// Search |pipeline| and all included into pipeline libraries whether shader
  /// with |name| is present in pipeline groups. Returns shader if found,
  /// |nullptr| if not found.
  Shader* FindShader(const Pipeline* pipeline, Shader* shader) const {
    if (shader) {
      for (auto group : pipeline->GetShaderGroups()) {
        Shader* test_shader = group->GetShaderByType(shader->GetType());
        if (test_shader == shader) {
          return shader;
        }
      }

      for (auto lib : pipeline->GetPipelineLibraries()) {
        shader = FindShader(lib, shader);
        if (shader) {
          return shader;
        }
      }
    }

    return nullptr;
  }

  /// Search |pipeline| and all included into pipeline libraries whether shader
  /// group with |name| is present. Returns shader group if found, |nullptr|
  /// if not found. |index| is an shader group index in pipeline or library.
  ShaderGroup* FindShaderGroup(const Pipeline* pipeline,
                               const std::string& name,
                               uint32_t* index) const {
    ShaderGroup* result = nullptr;
    uint32_t shader_group_index = pipeline->GetShaderGroupIndex(name);
    if (shader_group_index != static_cast<uint32_t>(-1)) {
      (*index) += shader_group_index;
      result = pipeline->GetShaderGroupByIndex(shader_group_index);
      return result;
    } else {
      (*index) += static_cast<uint32_t>(pipeline->GetShaderGroups().size());
    }

    for (auto lib : pipeline->GetPipelineLibraries()) {
      result = FindShaderGroup(lib, name, index);
      if (result) {
        return result;
      }
    }

    *index = static_cast<uint32_t>(-1);

    return nullptr;
  }

  /// Adds |buffer| to the list of known buffers. The |buffer| must have a
  /// unique name over all buffers in the script.
  Result AddBuffer(std::unique_ptr<Buffer> buffer) {
    if (name_to_buffer_.count(buffer->GetName()) > 0) {
      return Result("duplicate buffer name provided");
    }

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

  /// Adds |sampler| to the list of known sampler. The |sampler| must have a
  /// unique name over all samplers in the script.
  Result AddSampler(std::unique_ptr<Sampler> sampler) {
    if (name_to_sampler_.count(sampler->GetName()) > 0) {
      return Result("duplicate sampler name provided");
    }

    samplers_.push_back(std::move(sampler));
    name_to_sampler_[samplers_.back()->GetName()] = samplers_.back().get();
    return {};
  }

  /// Retrieves the sampler with |name|, |nullptr| if not found.
  Sampler* GetSampler(const std::string& name) const {
    auto it = name_to_sampler_.find(name);
    return it == name_to_sampler_.end() ? nullptr : it->second;
  }

  /// Retrieves a list of all samplers.
  const std::vector<std::unique_ptr<Sampler>>& GetSamplers() const {
    return samplers_;
  }

  /// Adds |blas| to the list of known bottom level acceleration structures.
  /// The |blas| must have a unique name over all BLASes in the script.
  Result AddBLAS(std::unique_ptr<BLAS> blas) {
    if (name_to_blas_.count(blas->GetName()) > 0) {
      return Result("duplicate BLAS name provided");
    }

    blases_.push_back(std::move(blas));
    name_to_blas_[blases_.back()->GetName()] = blases_.back().get();

    return {};
  }

  /// Retrieves the BLAS with |name|, |nullptr| if not found.
  BLAS* GetBLAS(const std::string& name) const {
    auto it = name_to_blas_.find(name);
    return it == name_to_blas_.end() ? nullptr : it->second;
  }

  /// Retrieves a list of all BLASes.
  const std::vector<std::unique_ptr<BLAS>>& GetBLASes() const {
    return blases_;
  }

  /// Adds |tlas| to the list of known top level acceleration structures.
  /// The |tlas| must have a unique name over all TLASes in the script.
  Result AddTLAS(std::unique_ptr<TLAS> tlas) {
    if (name_to_tlas_.count(tlas->GetName()) > 0) {
      return Result("duplicate TLAS name provided");
    }

    tlases_.push_back(std::move(tlas));
    name_to_tlas_[tlases_.back()->GetName()] = tlases_.back().get();

    return {};
  }

  /// Retrieves the TLAS with |name|, |nullptr| if not found.
  TLAS* GetTLAS(const std::string& name) const {
    auto it = name_to_tlas_.find(name);
    return it == name_to_tlas_.end() ? nullptr : it->second;
  }

  /// Retrieves a list of all TLASes.
  const std::vector<std::unique_ptr<TLAS>>& GetTLASes() const {
    return tlases_;
  }

  /// Adds |feature| to the list of features that must be supported by the
  /// engine.
  void AddRequiredFeature(const std::string& feature) {
    engine_info_.required_features.push_back(feature);
  }

  /// Adds |prop| to the list of properties that must be supported by the
  /// engine.
  void AddRequiredProperty(const std::string& prop) {
    engine_info_.required_properties.push_back(prop);
  }

  /// Checks if |feature| is in required features
  bool IsRequiredFeature(const std::string& feature) const {
    return std::find(engine_info_.required_features.begin(),
                     engine_info_.required_features.end(),
                     feature) != engine_info_.required_features.end();
  }

  /// Checks if |prop| is in required features
  bool IsRequiredProperty(const std::string& prop) const {
    return std::find(engine_info_.required_properties.begin(),
                     engine_info_.required_properties.end(),
                     prop) != engine_info_.required_properties.end();
  }

  /// Adds |ext| to the list of device extensions that must be supported.
  void AddRequiredDeviceExtension(const std::string& ext) {
    engine_info_.required_device_extensions.push_back(ext);
  }

  /// Adds |ext| to the list of instance extensions that must be supported.
  void AddRequiredInstanceExtension(const std::string& ext) {
    engine_info_.required_instance_extensions.push_back(ext);
  }

  /// Adds |ext| to the list of extensions that must be supported by the engine.
  /// Note, this should only be used by the VkScript engine where there is no
  /// differentiation between the types of extensions.
  void AddRequiredExtension(const std::string& ext);

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

  /// Sets the SPIR-V target environment.
  void SetSpvTargetEnv(const std::string& env) { spv_env_ = env; }
  /// Retrieves the SPIR-V target environment.
  const std::string& GetSpvTargetEnv() const { return spv_env_; }

  /// Assign ownership of the format to the script.
  Format* RegisterFormat(std::unique_ptr<Format> fmt) {
    formats_.push_back(std::move(fmt));
    return formats_.back().get();
  }

  /// Assigns ownership of the type to the script.
  type::Type* RegisterType(std::unique_ptr<type::Type> type) {
    types_.push_back(std::move(type));
    return types_.back().get();
  }

  /// Adds |type| to the list of known types. The |type| must have
  /// a unique name over all types in the script.
  Result AddType(const std::string& name, std::unique_ptr<type::Type> type) {
    if (name_to_type_.count(name) > 0) {
      return Result("duplicate type name provided");
    }

    name_to_type_[name] = std::move(type);
    return {};
  }

  /// Retrieves the type with |name|, |nullptr| if not found.
  type::Type* GetType(const std::string& name) const {
    auto it = name_to_type_.find(name);
    return it == name_to_type_.end() ? nullptr : it->second.get();
  }

  // Returns the virtual file store.
  VirtualFileStore* GetVirtualFiles() const { return virtual_files_.get(); }

  /// Adds the virtual file with content |content| to the virtual file path
  /// |path|. If there's already a virtual file with the given path, an error is
  /// returned.
  Result AddVirtualFile(const std::string& path, const std::string& content) {
    return virtual_files_->Add(path, content);
  }

  /// Look up the virtual file by path. If the file was found, the content is
  /// assigned to content.
  Result GetVirtualFile(const std::string& path, std::string* content) const {
    return virtual_files_->Get(path, content);
  }

  type::Type* ParseType(const std::string& str);

 private:
  struct {
    std::vector<std::string> required_features;
    std::vector<std::string> required_properties;
    std::vector<std::string> required_device_extensions;
    std::vector<std::string> required_instance_extensions;
  } engine_info_;

  EngineData engine_data_;
  std::string spv_env_;
  std::map<std::string, Shader*> name_to_shader_;
  std::map<std::string, Buffer*> name_to_buffer_;
  std::map<std::string, Sampler*> name_to_sampler_;
  std::map<std::string, Pipeline*> name_to_pipeline_;
  std::map<std::string, BLAS*> name_to_blas_;
  std::map<std::string, TLAS*> name_to_tlas_;
  std::map<std::string, std::unique_ptr<type::Type>> name_to_type_;
  std::vector<std::unique_ptr<Shader>> shaders_;
  std::vector<std::unique_ptr<Command>> commands_;
  std::vector<std::unique_ptr<Buffer>> buffers_;
  std::vector<std::unique_ptr<Sampler>> samplers_;
  std::vector<std::unique_ptr<Pipeline>> pipelines_;
  std::vector<std::unique_ptr<BLAS>> blases_;
  std::vector<std::unique_ptr<TLAS>> tlases_;
  std::vector<std::unique_ptr<type::Type>> types_;
  std::vector<std::unique_ptr<Format>> formats_;
  std::unique_ptr<VirtualFileStore> virtual_files_;
};

}  // namespace amber

#endif  // SRC_SCRIPT_H_
