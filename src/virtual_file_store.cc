// Copyright 2020 The Amber Authors.
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

#include "src/virtual_file_store.h"

namespace amber {
namespace {

bool HasPrefix(const std::string& str, const std::string& prefix) {
  return str.compare(0, prefix.size(), prefix) == 0;
}

std::string TrimPrefix(const std::string& str, const std::string& prefix) {
  return HasPrefix(str, prefix) ? str.substr(prefix.length()) : str;
}

std::string ReplaceAll(std::string str,
                       const std::string& substr,
                       const std::string& replacement) {
  size_t pos = 0;
  while ((pos = str.find(substr, pos)) != std::string::npos) {
    str.replace(pos, substr.length(), replacement);
    pos += replacement.length();
  }
  return str;
}

}  // namespace

std::string VirtualFileStore::GetCanonical(const std::string& path) {
  auto canonical = path;
  canonical = ReplaceAll(canonical, "\\", "/");
  canonical = TrimPrefix(canonical, "./");
  return canonical;
}

}  // namespace amber
