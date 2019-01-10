// Copyright 2019 The Amber Authors.
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

#ifndef ANDROID_AMBER_SCRIPT_H_
#define ANDROID_AMBER_SCRIPT_H_

#include <android_native_app_glue.h>
#include <memory>
#include <string>
#include <vector>

#include "amber/amber.h"
#include "amber/result.h"

namespace amber {
namespace android {

struct AmberScriptInfo {
  std::string asset_name;      // Script asset name. Note it is not a
                               // path and just the name of script file.
  std::string script_content;  // Script itself from the script file.
  amber::ShaderMap shader_map;
};

// A class to load scripts for Amber under assets/amber/ into
// |script_info_|. We assume that file extension of those scripts
// is ".amber" and all files with the extension are scripts for
// Amber.
class AmberScriptLoader {
 public:
  explicit AmberScriptLoader(android_app* app);
  ~AmberScriptLoader();

  Result LoadAllScriptsFromAsset();
  const std::vector<AmberScriptInfo>& GetScripts() const {
    return script_info_;
  }

 private:
  // Find all files with ".amber" extension and set |asset_name| of
  // |script_info_| as their names. In addition, return all shader
  // file names that have ".spv" extensions.
  std::vector<std::string> FindAllScriptsAndReturnShaderNames();

  // Return content of script named |script_name| under
  // assets/amber/ as a std::string.
  std::string ReadScript(const std::string& script_name);

  // Return SPIRV binary of script named |shader_name| under
  // assets/amber/ as a std::vector<uint32_t>.
  std::vector<uint32_t> ReadSpvShader(const std::string& shader_name);

  // Return content of asset named |asset_name| under assets/amber/
  // as a std::vector<uint8_t>.
  std::vector<uint8_t> ReadContent(const std::string& asset_name);

  android_app* app_context_ = nullptr;
  std::vector<AmberScriptInfo> script_info_;
};

}  // namespace android
}  // namespace amber

#endif  // ANDROID_AMBER_SCRIPT_H_
