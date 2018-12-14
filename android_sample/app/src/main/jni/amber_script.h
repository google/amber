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
  std::string file_name;       // Script file name. Note it is not a path
                               // and just the name of script file.
  std::string script_content;  // Script itself from the script file.
  amber::ShaderMap shader_map;
};

// A class to load scripts for Amber under assets/amber/ into
// |script_info_|.
class AmberScriptLoader {
 public:
  explicit AmberScriptLoader(android_app* app);
  ~AmberScriptLoader();

  Result LoadAllScriptsFromAsset();
  const std::vector<AmberScriptInfo>& GetScriptInfo() const {
    return script_info_;
  }

 private:
  // A struct to load file content into memory chunk |content|.
  struct FileContent {
   public:
    FileContent(size_t allocate_size_in_bytes)
        : size_in_bytes(allocate_size_in_bytes) {
      if (allocate_size_in_bytes)
        content = new char[allocate_size_in_bytes];
    }
    ~FileContent() { delete content; }

    std::string ToString() { return std::string(content); }
    std::vector<uint32_t> ToVectorUint32() {
      const uint32_t* ptr = reinterpret_cast<const uint32_t*>(content);
      return std::vector<uint32_t>(ptr, ptr + size_in_bytes / sizeof(uint32_t));
    }

    size_t size_in_bytes = 0;
    char* content = nullptr;
  };

  // Find all files with .amber extension and set |file_name| of
  // |script_info_| as their names.
  void FindAllScripts();

  // Return all shader files for a script |script_name|.
  std::vector<std::string> GetShaderNamesForAmberScript(
      const std::string& script_name);

  // Return file content of |file_name|.
  std::unique_ptr<FileContent> ReadFileContent(const std::string& file_name);

  android_app* app_context_ = nullptr;
  std::vector<AmberScriptInfo> script_info_;
};

}  // namespace android
}  // namespace amber

#endif  // ANDROID_AMBER_SCRIPT_H_
