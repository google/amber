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

#include "src/pipeline.h"
#include "gtest/gtest.h"

namespace amber {
namespace {

struct ShaderTypeData {
  ShaderType type;
};

}  // namespace

using AmberScriptPipelineTest = testing::Test;

TEST_F(AmberScriptPipelineTest, AddShader) {
  Shader v(kShaderTypeVertex);
  Shader f(kShaderTypeFragment);

  Pipeline p(PipelineType::kGraphics);
  Result r = p.AddShader(&v, kShaderTypeVertex);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  r = p.AddShader(&f, kShaderTypeFragment);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  const auto& shaders = p.GetShaders();
  EXPECT_EQ(2U, shaders.size());

  EXPECT_EQ(&v, shaders[0].GetShader());
  EXPECT_EQ(&f, shaders[1].GetShader());
}

TEST_F(AmberScriptPipelineTest, MissingShader) {
  Pipeline p(PipelineType::kGraphics);
  Result r = p.AddShader(nullptr, kShaderTypeVertex);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("shader can not be null when attached to pipeline", r.Error());
}

TEST_F(AmberScriptPipelineTest, DuplicateShaders) {
  Shader v(kShaderTypeVertex);
  Shader f(kShaderTypeFragment);

  Pipeline p(PipelineType::kGraphics);
  Result r = p.AddShader(&v, kShaderTypeVertex);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  r = p.AddShader(&f, kShaderTypeFragment);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  r = p.AddShader(&v, kShaderTypeVertex);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("can not add duplicate shader to pipeline", r.Error());
}

TEST_F(AmberScriptPipelineTest, DuplicateShaderType) {
  Shader v(kShaderTypeVertex);
  Shader f(kShaderTypeVertex);

  Pipeline p(PipelineType::kGraphics);
  Result r = p.AddShader(&v, kShaderTypeVertex);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  r = p.AddShader(&f, kShaderTypeFragment);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("can not add duplicate shader type to pipeline", r.Error());
}

using AmberScriptPipelineComputePipelineTest =
    testing::TestWithParam<ShaderTypeData>;
TEST_P(AmberScriptPipelineComputePipelineTest,
       SettingGraphicsShaderToComputePipeline) {
  const auto test_data = GetParam();

  Shader s(test_data.type);

  Pipeline p(PipelineType::kCompute);
  Result r = p.AddShader(&s, test_data.type);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("only compute shaders allowed in a compute pipeline", r.Error());
}
INSTANTIATE_TEST_CASE_P(
    AmberScriptPipelineComputePipelineTests,
    AmberScriptPipelineComputePipelineTest,
    testing::Values(
        ShaderTypeData{kShaderTypeVertex},
        ShaderTypeData{kShaderTypeFragment},
        ShaderTypeData{kShaderTypeGeometry},
        ShaderTypeData{kShaderTypeTessellationEvaluation},
        ShaderTypeData{
            kShaderTypeTessellationControl}), );  // NOLINT(whitespace/parens)

TEST_F(AmberScriptPipelineTest, SettingComputeShaderToGraphicsPipeline) {
  Shader c(kShaderTypeCompute);

  Pipeline p(PipelineType::kGraphics);
  Result r = p.AddShader(&c, kShaderTypeCompute);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("can not add a compute shader to a graphics pipeline", r.Error());
}

TEST_F(AmberScriptPipelineTest, SetShaderOptimizations) {
  Shader v(kShaderTypeVertex);
  Shader f(kShaderTypeFragment);

  Pipeline p(PipelineType::kGraphics);
  Result r = p.AddShader(&v, kShaderTypeVertex);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  r = p.AddShader(&f, kShaderTypeFragment);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  std::vector<std::string> first = {"First", "Second"};
  std::vector<std::string> second = {"Third", "Forth"};

  r = p.SetShaderOptimizations(&f, first);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  r = p.SetShaderOptimizations(&v, second);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  const auto& shaders = p.GetShaders();
  EXPECT_EQ(2U, shaders.size());
  EXPECT_EQ(second, shaders[0].GetShaderOptimizations());
  EXPECT_EQ(first, shaders[1].GetShaderOptimizations());
}

TEST_F(AmberScriptPipelineTest, DuplicateShaderOptimizations) {
  Shader v(kShaderTypeVertex);

  Pipeline p(PipelineType::kGraphics);
  Result r = p.AddShader(&v, kShaderTypeVertex);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  std::vector<std::string> data = {"One", "One"};
  r = p.SetShaderOptimizations(&v, data);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("duplicate optimization flag (One) set on shader", r.Error());
}

TEST_F(AmberScriptPipelineTest, SetOptimizationForMissingShader) {
  Pipeline p(PipelineType::kGraphics);
  Result r = p.SetShaderOptimizations(nullptr, {"One", "Two"});
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("invalid shader specified for optimizations", r.Error());
}

TEST_F(AmberScriptPipelineTest, SetOptimizationForInvalidShader) {
  Shader v(kShaderTypeVertex);
  v.SetName("my_shader");

  Pipeline p(PipelineType::kGraphics);
  Result r = p.SetShaderOptimizations(&v, {"One", "Two"});
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("unknown shader specified for optimizations: my_shader", r.Error());
}

TEST_F(AmberScriptPipelineTest,
       GraphicsPipelineRequiresVertexAndFragmentShader) {
  Shader v(kShaderTypeVertex);
  Shader f(kShaderTypeFragment);
  Shader g(kShaderTypeGeometry);

  Pipeline p(PipelineType::kGraphics);
  Result r = p.AddShader(&v, kShaderTypeVertex);
  EXPECT_TRUE(r.IsSuccess()) << r.Error();

  r = p.AddShader(&g, kShaderTypeGeometry);
  EXPECT_TRUE(r.IsSuccess()) << r.Error();

  r = p.AddShader(&f, kShaderTypeFragment);
  EXPECT_TRUE(r.IsSuccess()) << r.Error();

  r = p.Validate();
  EXPECT_TRUE(r.IsSuccess()) << r.Error();
}

TEST_F(AmberScriptPipelineTest, GraphicsPipelineMissingFragmentShader) {
  Shader v(kShaderTypeVertex);
  Shader g(kShaderTypeGeometry);

  Pipeline p(PipelineType::kGraphics);
  Result r = p.AddShader(&v, kShaderTypeVertex);
  EXPECT_TRUE(r.IsSuccess()) << r.Error();

  r = p.AddShader(&g, kShaderTypeGeometry);
  EXPECT_TRUE(r.IsSuccess()) << r.Error();

  r = p.Validate();
  EXPECT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("graphics pipeline requires a fragment shader", r.Error());
}

TEST_F(AmberScriptPipelineTest, GraphicsPipelineMissingVertexShader) {
  Shader f(kShaderTypeFragment);
  Shader g(kShaderTypeGeometry);

  Pipeline p(PipelineType::kGraphics);
  Result r = p.AddShader(&g, kShaderTypeGeometry);
  EXPECT_TRUE(r.IsSuccess()) << r.Error();

  r = p.AddShader(&f, kShaderTypeFragment);
  EXPECT_TRUE(r.IsSuccess()) << r.Error();

  r = p.Validate();
  EXPECT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("graphics pipeline requires a vertex shader", r.Error());
}

TEST_F(AmberScriptPipelineTest,
       GraphicsPipelineMissingVertexAndFragmentShader) {
  Shader g(kShaderTypeGeometry);

  Pipeline p(PipelineType::kGraphics);
  Result r = p.AddShader(&g, kShaderTypeGeometry);
  EXPECT_TRUE(r.IsSuccess()) << r.Error();

  r = p.Validate();
  EXPECT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("graphics pipeline requires vertex and fragment shaders",
            r.Error());
}

TEST_F(AmberScriptPipelineTest, GraphicsPipelineWihoutShaders) {
  Pipeline p(PipelineType::kGraphics);
  Result r = p.Validate();
  EXPECT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("graphics pipeline requires vertex and fragment shaders",
            r.Error());
}

TEST_F(AmberScriptPipelineTest, ComputePipelineRequiresComputeShader) {
  Shader c(kShaderTypeCompute);

  Pipeline p(PipelineType::kCompute);
  Result r = p.AddShader(&c, kShaderTypeCompute);
  EXPECT_TRUE(r.IsSuccess()) << r.Error();

  r = p.Validate();
  EXPECT_TRUE(r.IsSuccess()) << r.Error();
}

TEST_F(AmberScriptPipelineTest, ComputePipelineWithoutShader) {
  Pipeline p(PipelineType::kCompute);
  Result r = p.Validate();
  EXPECT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("compute pipeline requires a compute shader", r.Error());
}

TEST_F(AmberScriptPipelineTest, SetEntryPointForMissingShader) {
  Shader c(kShaderTypeCompute);
  c.SetName("my_shader");

  Pipeline p(PipelineType::kCompute);
  Result r = p.SetShaderEntryPoint(&c, "test");
  EXPECT_FALSE(r.IsSuccess());
  EXPECT_EQ("unknown shader specified for entry point: my_shader", r.Error());
}

TEST_F(AmberScriptPipelineTest, SetEntryPointForNullShader) {
  Pipeline p(PipelineType::kCompute);
  Result r = p.SetShaderEntryPoint(nullptr, "test");
  EXPECT_FALSE(r.IsSuccess());
  EXPECT_EQ("invalid shader specified for entry point", r.Error());
}

TEST_F(AmberScriptPipelineTest, SetBlankEntryPoint) {
  Shader c(kShaderTypeCompute);
  Pipeline p(PipelineType::kCompute);
  Result r = p.AddShader(&c, kShaderTypeCompute);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  r = p.SetShaderEntryPoint(&c, "");
  EXPECT_FALSE(r.IsSuccess());
  EXPECT_EQ("entry point should not be blank", r.Error());
}

TEST_F(AmberScriptPipelineTest, ShaderDefaultEntryPoint) {
  Shader c(kShaderTypeCompute);
  Pipeline p(PipelineType::kCompute);
  Result r = p.AddShader(&c, kShaderTypeCompute);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  const auto& shaders = p.GetShaders();
  ASSERT_EQ(1U, shaders.size());
  EXPECT_EQ("main", shaders[0].GetEntryPoint());
}

TEST_F(AmberScriptPipelineTest, SetShaderEntryPoint) {
  Shader c(kShaderTypeCompute);
  Pipeline p(PipelineType::kCompute);
  Result r = p.AddShader(&c, kShaderTypeCompute);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  r = p.SetShaderEntryPoint(&c, "my_main");
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  const auto& shaders = p.GetShaders();
  ASSERT_EQ(1U, shaders.size());
  EXPECT_EQ("my_main", shaders[0].GetEntryPoint());
}

TEST_F(AmberScriptPipelineTest, SetEntryPointMulitpleTimes) {
  Shader c(kShaderTypeCompute);
  Pipeline p(PipelineType::kCompute);
  Result r = p.AddShader(&c, kShaderTypeCompute);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  r = p.SetShaderEntryPoint(&c, "my_main");
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  r = p.SetShaderEntryPoint(&c, "another_main");
  EXPECT_FALSE(r.IsSuccess());
  EXPECT_EQ("multiple entry points given for the same shader", r.Error());
}

}  // namespace amber
