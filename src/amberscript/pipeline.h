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

#ifndef SRC_AMBERSCRIPT_PIPELINE_H_
#define SRC_AMBERSCRIPT_PIPELINE_H_

#include <string>
#include <vector>

#include "amber/result.h"
#include "src/shader.h"

namespace amber {
namespace amberscript {

enum class PipelineType { kCompute = 0, kGraphics };

class Pipeline {
 public:
  class ShaderInfo {
   public:
    explicit ShaderInfo(const Shader*);
    ShaderInfo(const ShaderInfo&);
    ~ShaderInfo();

    void SetShaderOptimizations(const std::vector<std::string>& opts) {
      shader_optimizations_ = opts;
    }
    const std::vector<std::string>& GetShaderOptimizations() const {
      return shader_optimizations_;
    }

    const Shader* GetShader() const { return shader_; }

    void SetEntryPoint(const std::string& ep) { entry_point_ = ep; }
    std::string GetEntryPoint() const { return entry_point_; }

   private:
    const Shader* shader_ = nullptr;
    std::vector<std::string> shader_optimizations_;
    std::string entry_point_;
  };

  explicit Pipeline(PipelineType type);
  ~Pipeline();

  PipelineType GetType() const { return pipeline_type_; }

  void SetName(const std::string& name) { name_ = name; }
  const std::string& GetName() const { return name_; }

  Result AddShader(const Shader*);
  const std::vector<ShaderInfo>& GetShaders() const { return shaders_; }

  Result SetShaderEntryPoint(const Shader* shader, const std::string& name);
  Result SetShaderOptimizations(const Shader* shader,
                                const std::vector<std::string>& opts);

  // Validates that the pipeline has been created correctly.
  Result Validate() const;

 private:
  Result ValidateGraphics() const;
  Result ValidateCompute() const;

  PipelineType pipeline_type_ = PipelineType::kCompute;
  std::string name_;
  std::vector<ShaderInfo> shaders_;
};

}  // namespace amberscript
}  // namespace amber

#endif  // SRC_AMBERSCRIPT_PIPELINE_H_
