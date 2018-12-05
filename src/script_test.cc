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

#include <utility>

#include "gtest/gtest.h"
#include "src/make_unique.h"
#include "src/shader.h"

namespace amber {
namespace {

class ScriptProxy : public Script {
 public:
  ScriptProxy() : Script(ScriptType::kVkScript) {}
  ~ScriptProxy() override = default;
};

}  // namespace

using ScriptTest = testing::Test;

TEST_F(ScriptTest, GetShaderInfo) {
  ScriptProxy sp;

  auto shader = MakeUnique<Shader>(ShaderType::kVertex);
  shader->SetName("Shader1");
  shader->SetFormat(ShaderFormat::kGlsl);
  shader->SetData("This is my shader data");
  sp.AddShader(std::move(shader));

  shader = MakeUnique<Shader>(ShaderType::kFragment);
  shader->SetName("Shader2");
  shader->SetFormat(ShaderFormat::kSpirvAsm);
  shader->SetData("More shader data");
  sp.AddShader(std::move(shader));

  auto info = sp.GetShaderInfo();
  ASSERT_EQ(2U, info.size());

  EXPECT_EQ("Shader1", info[0].shader_name);
  EXPECT_EQ(ShaderFormat::kGlsl, info[0].format);
  EXPECT_EQ(ShaderType::kVertex, info[0].type);
  EXPECT_EQ("This is my shader data", info[0].shader_source);
  EXPECT_TRUE(info[0].optimizations.empty());

  EXPECT_EQ("Shader2", info[1].shader_name);
  EXPECT_EQ(ShaderFormat::kSpirvAsm, info[1].format);
  EXPECT_EQ(ShaderType::kFragment, info[1].type);
  EXPECT_EQ("More shader data", info[1].shader_source);
  EXPECT_TRUE(info[1].optimizations.empty());
}

TEST_F(ScriptTest, GetShaderInfoNoShaders) {
  ScriptProxy sp;
  auto info = sp.GetShaderInfo();
  EXPECT_TRUE(info.empty());
}

}  // namespace amber
