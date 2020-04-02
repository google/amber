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

#ifndef SRC_VIRTUAL_FILE_STORE_H_
#define SRC_VIRTUAL_FILE_STORE_H_

#include <cassert>
#include <string>
#include <unordered_map>

#include "amber/result.h"

namespace amber {

/// Stores a number of virtual files by path.
class VirtualFileStore {
 public:
  /// Return the path sanitized into a canonical form.
  static std::string GetCanonical(const std::string& path);

  /// Adds the virtual file with content |content| to the virtual file path
  /// |path|. If there's already a virtual file with the given path, an error is
  /// returned.
  Result Add(const std::string& path, const std::string& content) {
    if (path.length() == 0) {
      return Result("Virtual file path was empty");
    }

    auto canonical = GetCanonical(path);

    auto it = files_by_path_.find(canonical);
    if (it != files_by_path_.end()) {
      return Result("Virtual file '" + path + "' already declared");
    }
    files_by_path_.emplace(canonical, content);
    return {};
  }

  /// Look up the virtual file by path. If the file was found, the content is
  /// assigned to content.
  Result Get(const std::string& path, std::string* content) const {
    assert(content);

    if (path.length() == 0) {
      return Result("Virtual file path was empty");
    }

    auto canonical = GetCanonical(path);

    auto it = files_by_path_.find(canonical);
    if (it == files_by_path_.end()) {
      return Result("Virtual file '" + path + "' not found");
    }
    *content = it->second;
    return {};
  }

 private:
  std::unordered_map<std::string, std::string> files_by_path_;
};

}  // namespace amber

#endif  // SRC_VIRTUAL_FILE_STORE_H_
