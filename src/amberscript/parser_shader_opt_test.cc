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
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or parseried.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "gtest/gtest.h"
#include "src/amberscript/parser.h"

namespace amber {
namespace amberscript {

using AmberScriptParserTest = testing::Test;

TEST_F(AmberScriptParserTest, PipelineShaderOptimization) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
SHADER geometry my_geom GLSL
# Geom shader
END
PIPELINE graphics my_pipeline
  ATTACH my_shader
  SHADER_OPTIMIZATION my_shader
    opt1
    opt_second
  END

  ATTACH my_fragment
  SHADER_OPTIMIZATION my_fragment
    another_optimization
    third
  END

  ATTACH my_geom
  SHADER_OPTIMIZATION my_geom
  END
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& pipelines = script->GetPipelines();
  ASSERT_EQ(1U, pipelines.size());

  const auto* pipeline = pipelines[0].get();
  const auto& shaders = pipeline->GetShaders();
  ASSERT_EQ(3U, shaders.size());

  ASSERT_TRUE(shaders[0].GetShader() != nullptr);
  EXPECT_EQ(kShaderTypeVertex, shaders[0].GetShader()->GetType());
  std::vector<std::string> my_shader_opts = {"opt1", "opt_second"};
  EXPECT_EQ(my_shader_opts, shaders[0].GetShaderOptimizations());

  ASSERT_TRUE(shaders[1].GetShader() != nullptr);
  EXPECT_EQ(kShaderTypeFragment, shaders[1].GetShader()->GetType());
  std::vector<std::string> my_fragment_opts = {"another_optimization", "third"};
  EXPECT_EQ(my_fragment_opts, shaders[1].GetShaderOptimizations());

  ASSERT_TRUE(shaders[2].GetShader() != nullptr);
  EXPECT_EQ(kShaderTypeGeometry, shaders[2].GetShader()->GetType());
  std::vector<std::string> my_geom_opts = {};
  EXPECT_EQ(my_geom_opts, shaders[2].GetShaderOptimizations());
}

TEST_F(AmberScriptParserTest, PipelineShaderOptmizationInvalidShader) {
  std::string in = R"(
PIPELINE graphics my_pipeline
SHADER_OPTIMIZATION invalid_shader
  opt1
  opt_second
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: unknown shader in SHADER_OPTIMIZATION command", r.Error());
}

TEST_F(AmberScriptParserTest, PipelineShaderOptmizationMissingShader) {
  std::string in = R"(
PIPELINE graphics my_pipeline
SHADER_OPTIMIZATION
  opt1
  opt_second
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("4: missing shader name in SHADER_OPTIMIZATION command", r.Error());
}

TEST_F(AmberScriptParserTest, PipelineShaderOptmizationnUnAttachedShader) {
  std::string in = R"(
SHADER vertex my_vertex PASSTHROUGH
PIPELINE graphics my_pipeline
  SHADER_OPTIMIZATION my_vertex
    opt1
    opt_second
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("7: unknown shader specified for optimizations: my_vertex",
            r.Error());
}

TEST_F(AmberScriptParserTest, PipelineShaderOptimizationMissingEnd) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
PIPELINE graphics my_pipeline
  ATTACH my_shader
  SHADER_OPTIMIZATION my_shader
    opt1
    opt_second)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("7: SHADER_OPTIMIZATION missing END command", r.Error());
}

TEST_F(AmberScriptParserTest, PipelineShaderOptimizationExtraParams) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
PIPELINE graphics my_pipeline
  ATTACH my_shader
  SHADER_OPTIMIZATION my_shader EXTRA
    opt1
    opt_second
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("5: extra parameters after SHADER_OPTIMIZATION command", r.Error());
}

TEST_F(AmberScriptParserTest, PipelineShaderOptimizationNonStringParam) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
PIPELINE graphics my_pipeline
  ATTACH my_shader
  SHADER_OPTIMIZATION my_shader
    123
    opt
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("6: SHADER_OPTIMIZATION options must be strings", r.Error());
}

}  // namespace amberscript
}  // namespace amber
