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

#ifndef SRC_AMBERSCRIPT_SCRIPT_H_
#define SRC_AMBERSCRIPT_SCRIPT_H_

#include <map>
#include <memory>
#include <vector>

#include "src/amberscript/pipeline.h"
#include "src/amberscript/shader.h"
#include "src/script.h"

namespace amber {
namespace amberscript {

class Script : public amber::Script {
 public:
  Script();
  ~Script() override;

  Result AddShader(std::unique_ptr<Shader> shader) {
    if (name_to_shader_.count(shader->GetName()) > 0)
      return Result("duplicate shader name provided");

    shaders_.push_back(std::move(shader));
    name_to_shader_[shaders_.back()->GetName()] = shaders_.back().get();
    return {};
  }
  const std::vector<std::unique_ptr<Shader>>& GetShaders() const {
    return shaders_;
  }

  Result AddPipeline(std::unique_ptr<Pipeline> pipeline) {
    if (name_to_pipeline_.count(pipeline->GetName()) > 0)
      return Result("duplicate pipeline name provided");

    pipelines_.push_back(std::move(pipeline));
    name_to_pipeline_[pipelines_.back()->GetName()] = pipelines_.back().get();
    return {};
  }
  const std::vector<std::unique_ptr<Pipeline>>& GetPipelines() const {
    return pipelines_;
  }

  Shader* GetShader(const std::string& name) const {
    auto it = name_to_shader_.find(name);
    return it == name_to_shader_.end() ? nullptr : it->second;
  }

  Pipeline* GetPipeline(const std::string& name) const {
    auto it = name_to_pipeline_.find(name);
    return it == name_to_pipeline_.end() ? nullptr : it->second;
  }

 private:
  std::map<std::string, Shader*> name_to_shader_;
  std::map<std::string, Pipeline*> name_to_pipeline_;
  std::vector<std::unique_ptr<Shader>> shaders_;
  std::vector<std::unique_ptr<Pipeline>> pipelines_;
};

}  // namespace amberscript

const amberscript::Script* ToAmberScript(const amber::Script* s);

}  // namespace amber

#endif  // SRC_AMBERSCRIPT_SCRIPT_H_
