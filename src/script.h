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

namespace amber {

enum class ScriptType : uint8_t { kVkScript = 0, kAmberScript };

class Script {
 public:
  virtual ~Script();

  bool IsVkScript() const { return script_type_ == ScriptType::kVkScript; }
  bool IsAmberScript() const {
    return script_type_ == ScriptType::kAmberScript;
  }

 protected:
  explicit Script(ScriptType);

 private:
  ScriptType script_type_;
};

}  // namespace amber

#endif  // SRC_SCRIPT_H_
