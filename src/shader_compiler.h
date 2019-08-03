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

#ifndef SRC_SHADER_COMPILER_H_
#define SRC_SHADER_COMPILER_H_

#include <string>
#include <utility>
#include <vector>

#include "amber/amber.h"
#include "amber/result.h"
#include "src/pipeline.h"
#include "src/shader.h"

namespace amber {

/// Class to wrap the compilation of shaders.
class ShaderCompiler {
 public:
  ShaderCompiler();
  ShaderCompiler(const std::string& env, bool disable_spirv_validation);
  ~ShaderCompiler();

  /// Returns a result code and a compilation of the given shader.
  /// If the shader in |shader_info| has a corresponding entry in the
  /// |shader_map|, then the compilation result is copied from that entry.
  /// Otherwise a compiler is invoked to produce the compilation result.
  ///
  /// If |shader_info| specifies shader optimizations to run and there is no
  /// entry in |shader_map| for that shader, then the SPIRV-Tools optimizer will
  /// be invoked to produce the shader binary.
  std::pair<Result, std::vector<uint32_t>> Compile(
      Pipeline::ShaderInfo* shader_info,
      const ShaderMap& shader_map) const;

 private:
  Result ParseHex(const std::string& data, std::vector<uint32_t>* result) const;
  Result CompileGlsl(const Shader* shader, std::vector<uint32_t>* result) const;
  Result CompileHlsl(const Shader* shader, std::vector<uint32_t>* result) const;
  Result CompileOpenCLC(Pipeline::ShaderInfo* shader,
                        std::vector<uint32_t>* result) const;

  std::string spv_env_;
  bool disable_spirv_validation_ = false;
};

// Parses the SPIR-V environment string, and returns the corresponding
// |target_env|, |target_env_version|, and |spirv_versoin|. Returns a failure
// value if the |spv_env| is invalid.
Result ParseSpvEnv(const std::string& spv_env,
                   uint32_t* target_env,
                   uint32_t* target_env_version,
                   uint32_t* spirv_version);

}  // namespace amber

#endif  // SRC_SHADER_COMPILER_H_
