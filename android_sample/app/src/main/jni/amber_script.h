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
  // A struct to load content of asset into memory chunk |content|.
  // If the asset is shader, user must make sure |size_in_bytes| is
  // multiple of sizeof(uint32_t).
  struct AssetContent {
   public:
    AssetContent(size_t allocate_size_in_bytes)
        : size_in_bytes(allocate_size_in_bytes) {
      if (allocate_size_in_bytes) {
        content = new char[allocate_size_in_bytes + 1];
        content[allocate_size_in_bytes] = '\0';
      }
    }
    ~AssetContent() { delete content; }

    std::string ToString() {
      return content ? std::string(content) : std::string();
    }
    std::vector<uint32_t> ToVectorUint32();

    size_t size_in_bytes = 0;
    char* content = nullptr;
  };

  // Find all files with ".amber" extension and set |asset_name| of
  // |script_info_| as their names.
  void FindAllScripts();

  // Return all shader files for a script |script_name|.
  std::vector<std::string> GetShaderNamesForAmberScript(
      const std::string& script_name);

  // Return content of shader named |shader_name| under assets/amber/.
  std::unique_ptr<AssetContent> ReadScript(const std::string& shader_name) {
    return ReadAssetContent(shader_name, true);
  }

  // Return content of asset named |asset_name| under assets/amber/.
  std::unique_ptr<AssetContent> ReadAssetContent(const std::string& asset_name,
                                                 bool is_shader = false);

  android_app* app_context_ = nullptr;
  std::vector<AmberScriptInfo> script_info_;
};

}  // namespace android
}  // namespace amber

#endif  // ANDROID_AMBER_SCRIPT_H_
