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

#include "src/script.h"

namespace amber {

Script::Script() = default;

Script::~Script() = default;

std::vector<ShaderInfo> Script::GetShaderInfo() const {
  std::vector<ShaderInfo> ret;
  for (const auto& shader : shaders_) {
    // TODO(dsinclair): The name returned should be the
    // `pipeline_name + shader_name` instead of just shader name when we have
    // pipelines everywhere

    // TODO(dsinclair): The optimization passes should be retrieved from the
    // pipeline and returned here instead of an empty array.
    ret.emplace_back(ShaderInfo{shader->GetFormat(),
                                shader->GetType(),
                                shader->GetName(),
                                shader->GetData(),
                                {}});
  }
  return ret;
}

void Script::AddRequiredExtension(const std::string& ext) {
  // Make this smarter when we have more instance extensions to match.
  if (ext == "VK_KHR_get_physical_device_properties2")
    engine_info_.required_instance_extensions.push_back(ext);
  else
    engine_info_.required_device_extensions.push_back(ext);
}

}  // namespace amber
