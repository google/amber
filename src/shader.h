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

#ifndef SRC_SHADER_H_
#define SRC_SHADER_H_

#include <string>

#include "amber/shader_info.h"

namespace amber {

/// Stores information for a shader described in a script.
class Shader {
 public:
  /// Create a shader of |type|.
  explicit Shader(ShaderType type);
  ~Shader();

  ShaderType GetType() const { return shader_type_; }

  void SetName(const std::string& name) { name_ = name; }
  const std::string& GetName() const { return name_; }

  void SetFormat(ShaderFormat fmt) { shader_format_ = fmt; }
  ShaderFormat GetFormat() const { return shader_format_; }

  /// Sets the compiled shader to |data|.
  void SetData(const std::string& data) { data_ = data; }
  /// Returns the compiled shader source.
  const std::string& GetData() const { return data_; }

 private:
  ShaderType shader_type_;
  ShaderFormat shader_format_;
  std::string data_;
  std::string name_;
};

}  // namespace amber

#endif  // SRC_SHADER_H_
