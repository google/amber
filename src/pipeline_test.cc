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
#include "src/make_unique.h"

namespace amber {
namespace {

struct ShaderTypeData {
  ShaderType type;
};

}  // namespace

class PipelineTest : public testing::Test {
 public:
  void TearDown() override {
    color_buffer_ = nullptr;
    depth_buffer_ = nullptr;
  }

  void SetupColorAttachment(Pipeline* p, uint32_t location) {
    if (!color_buffer_)
      color_buffer_ = p->GenerateDefaultColorAttachmentBuffer();

    p->AddColorAttachment(color_buffer_.get(), location);
  }

  void SetupDepthAttachment(Pipeline* p) {
    if (!depth_buffer_)
      depth_buffer_ = p->GenerateDefaultDepthAttachmentBuffer();

    p->SetDepthBuffer(depth_buffer_.get());
  }

 private:
  std::unique_ptr<Buffer> color_buffer_;
  std::unique_ptr<Buffer> depth_buffer_;
};

TEST_F(PipelineTest, AddShader) {
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

TEST_F(PipelineTest, MissingShader) {
  Pipeline p(PipelineType::kGraphics);
  Result r = p.AddShader(nullptr, kShaderTypeVertex);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("shader can not be null when attached to pipeline", r.Error());
}

TEST_F(PipelineTest, DuplicateShaders) {
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

TEST_F(PipelineTest, SettingComputeShaderToGraphicsPipeline) {
  Shader c(kShaderTypeCompute);

  Pipeline p(PipelineType::kGraphics);
  Result r = p.AddShader(&c, kShaderTypeCompute);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("can not add a compute shader to a graphics pipeline", r.Error());
}

TEST_F(PipelineTest, SetShaderOptimizations) {
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

TEST_F(PipelineTest, DuplicateShaderOptimizations) {
  Shader v(kShaderTypeVertex);

  Pipeline p(PipelineType::kGraphics);
  Result r = p.AddShader(&v, kShaderTypeVertex);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  std::vector<std::string> data = {"One", "One"};
  r = p.SetShaderOptimizations(&v, data);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("duplicate optimization flag (One) set on shader", r.Error());
}

TEST_F(PipelineTest, SetOptimizationForMissingShader) {
  Pipeline p(PipelineType::kGraphics);
  Result r = p.SetShaderOptimizations(nullptr, {"One", "Two"});
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("invalid shader specified for optimizations", r.Error());
}

TEST_F(PipelineTest, SetOptimizationForInvalidShader) {
  Shader v(kShaderTypeVertex);
  v.SetName("my_shader");

  Pipeline p(PipelineType::kGraphics);
  Result r = p.SetShaderOptimizations(&v, {"One", "Two"});
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("unknown shader specified for optimizations: my_shader", r.Error());
}

TEST_F(PipelineTest, GraphicsPipelineRequiresColorAttachment) {
  Pipeline p(PipelineType::kGraphics);
  SetupDepthAttachment(&p);

  Result r = p.Validate();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("PIPELINE missing color attachment", r.Error());
}

TEST_F(PipelineTest, GraphicsPipelineRequiresVertexAndFragmentShader) {
  Shader v(kShaderTypeVertex);
  Shader f(kShaderTypeFragment);
  Shader g(kShaderTypeGeometry);

  Pipeline p(PipelineType::kGraphics);
  SetupColorAttachment(&p, 0);
  SetupDepthAttachment(&p);

  Result r = p.AddShader(&v, kShaderTypeVertex);
  EXPECT_TRUE(r.IsSuccess()) << r.Error();

  r = p.AddShader(&g, kShaderTypeGeometry);
  EXPECT_TRUE(r.IsSuccess()) << r.Error();

  r = p.AddShader(&f, kShaderTypeFragment);
  EXPECT_TRUE(r.IsSuccess()) << r.Error();

  r = p.Validate();
  EXPECT_TRUE(r.IsSuccess()) << r.Error();
}

TEST_F(PipelineTest, GraphicsPipelineMissingFragmentShader) {
  Shader v(kShaderTypeVertex);
  Shader g(kShaderTypeGeometry);

  Pipeline p(PipelineType::kGraphics);
  SetupColorAttachment(&p, 0);
  SetupDepthAttachment(&p);

  Result r = p.AddShader(&v, kShaderTypeVertex);
  EXPECT_TRUE(r.IsSuccess()) << r.Error();

  r = p.AddShader(&g, kShaderTypeGeometry);
  EXPECT_TRUE(r.IsSuccess()) << r.Error();

  r = p.Validate();
  EXPECT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("graphics pipeline requires a fragment shader", r.Error());
}

TEST_F(PipelineTest, GraphicsPipelineMissingVertexShader) {
  Shader f(kShaderTypeFragment);
  Shader g(kShaderTypeGeometry);

  Pipeline p(PipelineType::kGraphics);
  SetupColorAttachment(&p, 0);
  SetupDepthAttachment(&p);

  Result r = p.AddShader(&g, kShaderTypeGeometry);
  EXPECT_TRUE(r.IsSuccess()) << r.Error();

  r = p.AddShader(&f, kShaderTypeFragment);
  EXPECT_TRUE(r.IsSuccess()) << r.Error();

  r = p.Validate();
  EXPECT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("graphics pipeline requires a vertex shader", r.Error());
}

TEST_F(PipelineTest, GraphicsPipelineMissingVertexAndFragmentShader) {
  Shader g(kShaderTypeGeometry);

  Pipeline p(PipelineType::kGraphics);
  SetupColorAttachment(&p, 0);
  SetupDepthAttachment(&p);

  Result r = p.AddShader(&g, kShaderTypeGeometry);
  EXPECT_TRUE(r.IsSuccess()) << r.Error();

  r = p.Validate();
  EXPECT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("graphics pipeline requires vertex and fragment shaders",
            r.Error());
}

TEST_F(PipelineTest, GraphicsPipelineWihoutShaders) {
  Pipeline p(PipelineType::kGraphics);
  SetupColorAttachment(&p, 0);
  SetupDepthAttachment(&p);

  Result r = p.Validate();
  EXPECT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("graphics pipeline requires vertex and fragment shaders",
            r.Error());
}

TEST_F(PipelineTest, ComputePipelineRequiresComputeShader) {
  Shader c(kShaderTypeCompute);

  Pipeline p(PipelineType::kCompute);
  SetupColorAttachment(&p, 0);
  SetupDepthAttachment(&p);

  Result r = p.AddShader(&c, kShaderTypeCompute);
  EXPECT_TRUE(r.IsSuccess()) << r.Error();

  r = p.Validate();
  EXPECT_TRUE(r.IsSuccess()) << r.Error();
}

TEST_F(PipelineTest, ComputePipelineWithoutShader) {
  Pipeline p(PipelineType::kCompute);
  SetupColorAttachment(&p, 0);
  SetupDepthAttachment(&p);

  Result r = p.Validate();
  EXPECT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("compute pipeline requires a compute shader", r.Error());
}

TEST_F(PipelineTest, SetEntryPointForMissingShader) {
  Shader c(kShaderTypeCompute);
  c.SetName("my_shader");

  Pipeline p(PipelineType::kCompute);
  Result r = p.SetShaderEntryPoint(&c, "test");
  EXPECT_FALSE(r.IsSuccess());
  EXPECT_EQ("unknown shader specified for entry point: my_shader", r.Error());
}

TEST_F(PipelineTest, SetEntryPointForNullShader) {
  Pipeline p(PipelineType::kCompute);
  Result r = p.SetShaderEntryPoint(nullptr, "test");
  EXPECT_FALSE(r.IsSuccess());
  EXPECT_EQ("invalid shader specified for entry point", r.Error());
}

TEST_F(PipelineTest, SetBlankEntryPoint) {
  Shader c(kShaderTypeCompute);
  Pipeline p(PipelineType::kCompute);
  Result r = p.AddShader(&c, kShaderTypeCompute);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  r = p.SetShaderEntryPoint(&c, "");
  EXPECT_FALSE(r.IsSuccess());
  EXPECT_EQ("entry point should not be blank", r.Error());
}

TEST_F(PipelineTest, ShaderDefaultEntryPoint) {
  Shader c(kShaderTypeCompute);
  Pipeline p(PipelineType::kCompute);
  Result r = p.AddShader(&c, kShaderTypeCompute);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  const auto& shaders = p.GetShaders();
  ASSERT_EQ(1U, shaders.size());
  EXPECT_EQ("main", shaders[0].GetEntryPoint());
}

TEST_F(PipelineTest, SetShaderEntryPoint) {
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

TEST_F(PipelineTest, SetEntryPointMulitpleTimes) {
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

TEST_F(PipelineTest, Clone) {
  Pipeline p(PipelineType::kGraphics);
  p.SetName("my_pipeline");
  p.SetFramebufferWidth(800);
  p.SetFramebufferHeight(600);

  SetupColorAttachment(&p, 0);
  SetupDepthAttachment(&p);

  Shader f(kShaderTypeFragment);
  p.AddShader(&f, kShaderTypeFragment);
  Shader v(kShaderTypeVertex);
  p.AddShader(&v, kShaderTypeVertex);
  p.SetShaderEntryPoint(&v, "my_main");

  auto vtex_buf = MakeUnique<DataBuffer>(BufferType::kVertex);
  vtex_buf->SetName("vertex_buffer");
  p.AddVertexBuffer(vtex_buf.get(), 1);

  auto idx_buf = MakeUnique<DataBuffer>(BufferType::kIndex);
  idx_buf->SetName("Index Buffer");
  p.SetIndexBuffer(idx_buf.get());

  auto buf1 = MakeUnique<DataBuffer>(BufferType::kStorage);
  buf1->SetName("buf1");
  p.AddBuffer(buf1.get(), 1, 1);

  auto buf2 = MakeUnique<DataBuffer>(BufferType::kStorage);
  buf2->SetName("buf2");
  p.AddBuffer(buf2.get(), 1, 2);

  auto clone = p.Clone();
  EXPECT_EQ("", clone->GetName());
  EXPECT_EQ(800U, clone->GetFramebufferWidth());
  EXPECT_EQ(600U, clone->GetFramebufferHeight());

  auto shaders = clone->GetShaders();
  ASSERT_EQ(2U, shaders.size());
  EXPECT_EQ(kShaderTypeFragment, shaders[0].GetShaderType());
  EXPECT_EQ(kShaderTypeVertex, shaders[1].GetShaderType());
  EXPECT_EQ("my_main", shaders[1].GetEntryPoint());

  ASSERT_TRUE(clone->GetIndexBuffer() != nullptr);
  EXPECT_EQ("Index Buffer", clone->GetIndexBuffer()->GetName());

  auto vtex_buffers = clone->GetVertexBuffers();
  ASSERT_EQ(1U, vtex_buffers.size());
  EXPECT_EQ(1, vtex_buffers[0].location);
  EXPECT_EQ("vertex_buffer", vtex_buffers[0].buffer->GetName());

  auto bufs = clone->GetBuffers();
  ASSERT_EQ(2U, bufs.size());
  EXPECT_EQ("buf1", bufs[0].buffer->GetName());
  EXPECT_EQ(1U, bufs[0].descriptor_set);
  EXPECT_EQ(1U, bufs[0].binding);

  EXPECT_EQ("buf2", bufs[1].buffer->GetName());
  EXPECT_EQ(1U, bufs[1].descriptor_set);
  EXPECT_EQ(2U, bufs[1].binding);
}

}  // namespace amber
