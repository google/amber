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

#include "src/pipeline.h"

#include <algorithm>
#include <set>

namespace amber {

Pipeline::ShaderInfo::ShaderInfo(const Shader* shader)
    : shader_(shader), entry_point_("main") {}

Pipeline::ShaderInfo::ShaderInfo(const ShaderInfo&) = default;

Pipeline::ShaderInfo::~ShaderInfo() = default;

Pipeline::Pipeline(PipelineType type) : pipeline_type_(type) {}

Pipeline::~Pipeline() = default;

Result Pipeline::AddShader(const Shader* shader) {
  if (!shader)
    return Result("shader can not be null when attached to pipeline");

  if (pipeline_type_ == PipelineType::kCompute &&
      shader->GetType() != ShaderType::kCompute) {
    return Result("only compute shaders allowed in a compute pipeline");
  }
  if (pipeline_type_ == PipelineType::kGraphics &&
      shader->GetType() == ShaderType::kCompute) {
    return Result("can not add a compute shader to a graphics pipeline");
  }

  for (const auto& info : shaders_) {
    const auto* is = info.GetShader();
    if (is == shader)
      return Result("can not add duplicate shader to pipeline");
    if (is->GetType() == shader->GetType())
      return Result("can not add duplicate shader type to pipeline");
  }

  shaders_.emplace_back(shader);
  return {};
}

Result Pipeline::SetShaderOptimizations(const Shader* shader,
                                        const std::vector<std::string>& opts) {
  if (!shader)
    return Result("invalid shader specified for optimizations");

  std::set<std::string> seen;
  for (const auto& opt : opts) {
    if (seen.count(opt) != 0)
      return Result("duplicate optimization flag (" + opt + ") set on shader");

    seen.insert(opt);
  }

  for (auto& info : shaders_) {
    const auto* is = info.GetShader();
    if (is == shader) {
      info.SetShaderOptimizations(opts);
      return {};
    }
  }

  return Result("unknown shader specified for optimizations: " +
                shader->GetName());
}

Result Pipeline::SetShaderEntryPoint(const Shader* shader,
                                     const std::string& name) {
  if (!shader)
    return Result("invalid shader specified for entry point");
  if (name.empty())
    return Result("entry point should not be blank");

  for (auto& info : shaders_) {
    if (info.GetShader() == shader) {
      if (info.GetEntryPoint() != "main")
        return Result("multiple entry points given for the same shader");

      info.SetEntryPoint(name);
      return {};
    }
  }

  return Result("unknown shader specified for entry point: " +
                shader->GetName());
}

Result Pipeline::Validate() const {
  if (pipeline_type_ == PipelineType::kGraphics)
    return ValidateGraphics();
  return ValidateCompute();
}

Result Pipeline::ValidateGraphics() const {
  if (shaders_.empty())
    return Result("graphics pipeline requires vertex and fragment shaders");

  bool found_vertex = false;
  bool found_fragment = false;
  for (const auto& info : shaders_) {
    const auto* is = info.GetShader();
    if (is->GetType() == ShaderType::kVertex)
      found_vertex = true;
    if (is->GetType() == ShaderType::kFragment)
      found_fragment = true;
    if (found_vertex && found_fragment)
      break;
  }

  if (!found_vertex && !found_fragment)
    return Result("graphics pipeline requires vertex and fragment shaders");
  if (!found_vertex)
    return Result("graphics pipeline requires a vertex shader");
  if (!found_fragment)
    return Result("graphics pipeline requires a fragment shader");
  return {};
}

Result Pipeline::ValidateCompute() const {
  if (shaders_.empty())
    return Result("compute pipeline requires a compute shader");

  return {};
}

}  // namespace amber
