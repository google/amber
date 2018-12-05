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
#include "src/command.h"
#include "src/engine.h"
#include "src/feature.h"
#include "src/shader.h"

namespace amber {

enum class ScriptType : uint8_t { kVkScript = 0, kAmberScript };

class Script : public RecipeImpl {
 public:
  ~Script() override;

  bool IsVkScript() const { return script_type_ == ScriptType::kVkScript; }
  bool IsAmberScript() const {
    return script_type_ == ScriptType::kAmberScript;
  }

  std::vector<ShaderInfo> GetShaderInfo() const override;

  Result AddShader(std::unique_ptr<Shader> shader) {
    if (name_to_shader_.count(shader->GetName()) > 0)
      return Result("duplicate shader name provided");

    shaders_.push_back(std::move(shader));
    name_to_shader_[shaders_.back()->GetName()] = shaders_.back().get();
    return {};
  }

  Shader* GetShader(const std::string& name) const {
    auto it = name_to_shader_.find(name);
    return it == name_to_shader_.end() ? nullptr : it->second;
  }

  const std::vector<std::unique_ptr<Shader>>& GetShaders() const {
    return shaders_;
  }

  void AddRequiredFeature(Feature feature) {
    engine_info_.required_features.push_back(feature);
  }
  const std::vector<Feature>& RequiredFeatures() const {
    return engine_info_.required_features;
  }

  void AddRequiredExtension(const std::string& ext) {
    engine_info_.required_extensions.push_back(ext);
  }
  const std::vector<std::string>& RequiredExtensions() const {
    return engine_info_.required_extensions;
  }

  EngineData& GetEngineData() { return engine_data_; }
  const EngineData& GetEngineData() const { return engine_data_; }

  void SetCommands(std::vector<std::unique_ptr<Command>> cmds) {
    commands_ = std::move(cmds);
  }
  const std::vector<std::unique_ptr<Command>>& GetCommands() const {
    return commands_;
  }

 protected:
  explicit Script(ScriptType);

 private:
  struct {
    std::vector<Feature> required_features;
    std::vector<std::string> required_extensions;
  } engine_info_;

  ScriptType script_type_;
  EngineData engine_data_;
  std::map<std::string, Shader*> name_to_shader_;
  std::vector<std::unique_ptr<Shader>> shaders_;
  std::vector<std::unique_ptr<Command>> commands_;
};

}  // namespace amber

#endif  // SRC_SCRIPT_H_
