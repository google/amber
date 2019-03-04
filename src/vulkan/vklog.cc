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

#include "src/vulkan/vklog.h"

#include <iostream>

// Use C string library for preprocessor strings like __FILE__
#include <string.h>

namespace amber {
namespace vulkan {

namespace {
#ifdef _WIN32
const char kSeparator = '\\';
#else  // _WIN32
const char kSeparator = '/';
#endif
}  // namespace

void vklog(const char* filepath, int line, const char* expr) {
  // Strip file path to only the file basename
  const char* basename = strrchr(filepath, kSeparator);
  if (basename == nullptr) {
    basename = filepath;
  } else {
    if (strlen(basename + 1) <= 0) {
      // Nothing beyond the separator, weird. Use the plain filepath.
      basename = filepath;
    } else {
      // Move beyond the separator character
      basename++;
    }
  }

  // Clean up the api call: remove anything prefixing 'vk'
  const char* call = strstr(expr, "vk");
  if (call == nullptr) {
    // All calls should contain "vk", this is suspicious. Use the plain expr.
    call = expr;
  }

  // Log the call in compiler output style: file:line call
  std::cout << basename << ":" << line << " " << call << std::endl;
}

}  // namespace vulkan
}  // namespace amber
