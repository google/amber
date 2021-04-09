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
#include "src/type_parser.h"

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
    depth_stencil_buffer_ = nullptr;
  }

  void SetupColorAttachment(Pipeline* p, uint32_t location) {
    if (!color_buffer_)
      color_buffer_ = p->GenerateDefaultColorAttachmentBuffer();

    p->AddColorAttachment(color_buffer_.get(), location, 0);
  }

  void SetupDepthStencilAttachment(Pipeline* p) {
    if (!depth_stencil_buffer_)
      depth_stencil_buffer_ = p->GenerateDefaultDepthStencilAttachmentBuffer();

    p->SetDepthStencilBuffer(depth_stencil_buffer_.get());
  }

 private:
  std::unique_ptr<Buffer> color_buffer_;
  std::unique_ptr<Buffer> depth_stencil_buffer_;
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
INSTANTIATE_TEST_SUITE_P(
    AmberScriptPipelineComputePipelineTests,
    AmberScriptPipelineComputePipelineTest,
    testing::Values(
        ShaderTypeData{kShaderTypeVertex},
        ShaderTypeData{kShaderTypeFragment},
        ShaderTypeData{kShaderTypeGeometry},
        ShaderTypeData{kShaderTypeTessellationEvaluation},
        ShaderTypeData{
            kShaderTypeTessellationControl}));  // NOLINT(whitespace/parens)

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
  SetupDepthStencilAttachment(&p);

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
  SetupDepthStencilAttachment(&p);

  Result r = p.AddShader(&v, kShaderTypeVertex);
  EXPECT_TRUE(r.IsSuccess()) << r.Error();

  r = p.AddShader(&g, kShaderTypeGeometry);
  EXPECT_TRUE(r.IsSuccess()) << r.Error();

  r = p.AddShader(&f, kShaderTypeFragment);
  EXPECT_TRUE(r.IsSuccess()) << r.Error();

  r = p.Validate();
  EXPECT_TRUE(r.IsSuccess()) << r.Error();
}

TEST_F(PipelineTest, GraphicsPipelineMissingVertexShader) {
  Shader f(kShaderTypeFragment);
  Shader g(kShaderTypeGeometry);

  Pipeline p(PipelineType::kGraphics);
  SetupColorAttachment(&p, 0);
  SetupDepthStencilAttachment(&p);

  Result r = p.AddShader(&g, kShaderTypeGeometry);
  EXPECT_TRUE(r.IsSuccess()) << r.Error();

  r = p.AddShader(&f, kShaderTypeFragment);
  EXPECT_TRUE(r.IsSuccess()) << r.Error();

  r = p.Validate();
  EXPECT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("graphics pipeline requires a vertex shader", r.Error());
}

TEST_F(PipelineTest, ComputePipelineRequiresComputeShader) {
  Shader c(kShaderTypeCompute);

  Pipeline p(PipelineType::kCompute);
  SetupColorAttachment(&p, 0);
  SetupDepthStencilAttachment(&p);

  Result r = p.AddShader(&c, kShaderTypeCompute);
  EXPECT_TRUE(r.IsSuccess()) << r.Error();

  r = p.Validate();
  EXPECT_TRUE(r.IsSuccess()) << r.Error();
}

TEST_F(PipelineTest, ComputePipelineWithoutShader) {
  Pipeline p(PipelineType::kCompute);
  SetupColorAttachment(&p, 0);
  SetupDepthStencilAttachment(&p);

  Result r = p.Validate();
  EXPECT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("compute pipeline requires a compute shader", r.Error());
}

TEST_F(PipelineTest, PipelineBufferWithoutFormat) {
  Pipeline p(PipelineType::kCompute);

  auto buf = MakeUnique<Buffer>();
  buf->SetName("MyBuffer");
  p.AddBuffer(buf.get(), BufferType::kStorage, 0, 0, 0, 0, 0, 0);

  Result r = p.Validate();
  EXPECT_FALSE(r.IsSuccess()) << r.Error();
  EXPECT_EQ("buffer (0:0) requires a format", r.Error());
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
  SetupDepthStencilAttachment(&p);

  Shader f(kShaderTypeFragment);
  p.AddShader(&f, kShaderTypeFragment);
  Shader v(kShaderTypeVertex);
  p.AddShader(&v, kShaderTypeVertex);
  p.SetShaderEntryPoint(&v, "my_main");

  auto vtex_buf = MakeUnique<Buffer>();
  vtex_buf->SetName("vertex_buffer");
  TypeParser parser;
  auto int_type = parser.Parse("R32_SINT");
  auto int_fmt = MakeUnique<Format>(int_type.get());
  p.AddVertexBuffer(vtex_buf.get(), 1, InputRate::kVertex, int_fmt.get(), 5,
                    10);

  auto idx_buf = MakeUnique<Buffer>();
  idx_buf->SetName("Index Buffer");
  p.SetIndexBuffer(idx_buf.get());

  auto buf1 = MakeUnique<Buffer>();
  buf1->SetName("buf1");
  p.AddBuffer(buf1.get(), BufferType::kStorage, 1, 1, 0, 0, 0, 0);

  auto buf2 = MakeUnique<Buffer>();
  buf2->SetName("buf2");
  p.AddBuffer(buf2.get(), BufferType::kStorage, 1, 2, 0, 16, 256, 512);

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
  EXPECT_EQ(InputRate::kVertex, vtex_buffers[0].input_rate);
  EXPECT_EQ(FormatType::kR32_SINT, vtex_buffers[0].format->GetFormatType());
  EXPECT_EQ(5, vtex_buffers[0].offset);
  EXPECT_EQ(10, vtex_buffers[0].stride);

  auto bufs = clone->GetBuffers();
  ASSERT_EQ(2U, bufs.size());
  EXPECT_EQ("buf1", bufs[0].buffer->GetName());
  EXPECT_EQ(1U, bufs[0].descriptor_set);
  EXPECT_EQ(1U, bufs[0].binding);
  EXPECT_EQ(0U, bufs[0].dynamic_offset);
  EXPECT_EQ(0U, bufs[0].descriptor_offset);
  EXPECT_EQ(0U, bufs[0].descriptor_range);

  EXPECT_EQ("buf2", bufs[1].buffer->GetName());
  EXPECT_EQ(1U, bufs[1].descriptor_set);
  EXPECT_EQ(2U, bufs[1].binding);
  EXPECT_EQ(16U, bufs[1].dynamic_offset);
  EXPECT_EQ(256U, bufs[1].descriptor_offset);
  EXPECT_EQ(512U, bufs[1].descriptor_range);
}

TEST_F(PipelineTest, OpenCLUpdateBindings) {
  Pipeline p(PipelineType::kCompute);
  p.SetName("my_pipeline");

  Shader cs(kShaderTypeCompute);
  cs.SetFormat(kShaderFormatOpenCLC);
  p.AddShader(&cs, kShaderTypeCompute);
  p.SetShaderEntryPoint(&cs, "my_main");

  Pipeline::ShaderInfo::DescriptorMapEntry entry1;
  entry1.kind = Pipeline::ShaderInfo::DescriptorMapEntry::Kind::SSBO;
  entry1.descriptor_set = 4;
  entry1.binding = 5;
  entry1.arg_name = "arg_a";
  entry1.arg_ordinal = 0;
  p.GetShaders()[0].AddDescriptorEntry("my_main", std::move(entry1));

  Pipeline::ShaderInfo::DescriptorMapEntry entry2;
  entry2.kind = Pipeline::ShaderInfo::DescriptorMapEntry::Kind::SSBO;
  entry2.descriptor_set = 3;
  entry2.binding = 1;
  entry2.arg_name = "arg_b";
  entry2.arg_ordinal = 1;
  p.GetShaders()[0].AddDescriptorEntry("my_main", std::move(entry2));

  auto a_buf = MakeUnique<Buffer>();
  a_buf->SetName("buf1");
  p.AddBuffer(a_buf.get(), BufferType::kStorage, "arg_a");

  auto b_buf = MakeUnique<Buffer>();
  b_buf->SetName("buf2");
  p.AddBuffer(b_buf.get(), BufferType::kStorage, 1);

  p.UpdateOpenCLBufferBindings();

  auto& bufs = p.GetBuffers();
  ASSERT_EQ(2U, bufs.size());
  EXPECT_EQ("buf1", bufs[0].buffer->GetName());
  EXPECT_EQ(4U, bufs[0].descriptor_set);
  EXPECT_EQ(5U, bufs[0].binding);
  EXPECT_EQ("buf2", bufs[1].buffer->GetName());
  EXPECT_EQ(3U, bufs[1].descriptor_set);
  EXPECT_EQ(1U, bufs[1].binding);
}

TEST_F(PipelineTest, OpenCLUpdateBindingTypeMismatch) {
  Pipeline p(PipelineType::kCompute);
  p.SetName("my_pipeline");

  Shader cs(kShaderTypeCompute);
  cs.SetFormat(kShaderFormatOpenCLC);
  p.AddShader(&cs, kShaderTypeCompute);
  p.SetShaderEntryPoint(&cs, "my_main");

  Pipeline::ShaderInfo::DescriptorMapEntry entry1;
  entry1.kind = Pipeline::ShaderInfo::DescriptorMapEntry::Kind::SSBO;
  entry1.descriptor_set = 4;
  entry1.binding = 5;
  entry1.arg_name = "arg_a";
  entry1.arg_ordinal = 0;
  p.GetShaders()[0].AddDescriptorEntry("my_main", std::move(entry1));

  Pipeline::ShaderInfo::DescriptorMapEntry entry2;
  entry2.kind = Pipeline::ShaderInfo::DescriptorMapEntry::Kind::SSBO;
  entry2.descriptor_set = 3;
  entry2.binding = 1;
  entry2.arg_name = "arg_b";
  entry2.arg_ordinal = 1;
  p.GetShaders()[0].AddDescriptorEntry("my_main", std::move(entry2));

  auto a_buf = MakeUnique<Buffer>();
  a_buf->SetName("buf1");
  p.AddBuffer(a_buf.get(), BufferType::kStorage, "arg_a");

  auto b_buf = MakeUnique<Buffer>();
  b_buf->SetName("buf2");
  p.AddBuffer(b_buf.get(), BufferType::kUniform, 1);

  auto r = p.UpdateOpenCLBufferBindings();

  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("Buffer buf2 must be a uniform binding", r.Error());
}

TEST_F(PipelineTest, OpenCLUpdateBindingImagesAndSamplers) {
  Pipeline p(PipelineType::kCompute);
  p.SetName("my_pipeline");

  Shader cs(kShaderTypeCompute);
  cs.SetFormat(kShaderFormatOpenCLC);
  p.AddShader(&cs, kShaderTypeCompute);
  p.SetShaderEntryPoint(&cs, "my_main");

  Pipeline::ShaderInfo::DescriptorMapEntry entry1;
  entry1.kind = Pipeline::ShaderInfo::DescriptorMapEntry::Kind::RO_IMAGE;
  entry1.descriptor_set = 4;
  entry1.binding = 5;
  entry1.arg_name = "arg_a";
  entry1.arg_ordinal = 0;
  p.GetShaders()[0].AddDescriptorEntry("my_main", std::move(entry1));

  Pipeline::ShaderInfo::DescriptorMapEntry entry2;
  entry2.kind = Pipeline::ShaderInfo::DescriptorMapEntry::Kind::WO_IMAGE;
  entry2.descriptor_set = 3;
  entry2.binding = 1;
  entry2.arg_name = "arg_b";
  entry2.arg_ordinal = 1;
  p.GetShaders()[0].AddDescriptorEntry("my_main", std::move(entry2));

  Pipeline::ShaderInfo::DescriptorMapEntry entry3;
  entry2.kind = Pipeline::ShaderInfo::DescriptorMapEntry::Kind::SAMPLER;
  entry2.descriptor_set = 3;
  entry2.binding = 2;
  entry2.arg_name = "arg_c";
  entry2.arg_ordinal = 2;
  p.GetShaders()[0].AddDescriptorEntry("my_main", std::move(entry2));

  auto a_buf = MakeUnique<Buffer>();
  a_buf->SetName("buf1");
  p.AddBuffer(a_buf.get(), BufferType::kSampledImage, "arg_a");

  auto b_buf = MakeUnique<Buffer>();
  b_buf->SetName("buf2");
  p.AddBuffer(b_buf.get(), BufferType::kStorageImage, 1);

  auto s = MakeUnique<Sampler>();
  s->SetName("samp");
  p.AddSampler(s.get(), "arg_c");

  auto r = p.UpdateOpenCLBufferBindings();

  ASSERT_TRUE(r.IsSuccess());
}

TEST_F(PipelineTest, OpenCLGeneratePodBuffers) {
  Pipeline p(PipelineType::kCompute);
  p.SetName("my_pipeline");

  Shader cs(kShaderTypeCompute);
  cs.SetFormat(kShaderFormatOpenCLC);
  p.AddShader(&cs, kShaderTypeCompute);
  p.SetShaderEntryPoint(&cs, "my_main");

  // Descriptor map.
  Pipeline::ShaderInfo::DescriptorMapEntry entry1;
  entry1.kind = Pipeline::ShaderInfo::DescriptorMapEntry::Kind::POD;
  entry1.descriptor_set = 4;
  entry1.binding = 5;
  entry1.arg_name = "arg_a";
  entry1.arg_ordinal = 0;
  entry1.pod_offset = 0;
  entry1.pod_arg_size = 4;
  p.GetShaders()[0].AddDescriptorEntry("my_main", std::move(entry1));

  Pipeline::ShaderInfo::DescriptorMapEntry entry2;
  entry2.kind = Pipeline::ShaderInfo::DescriptorMapEntry::Kind::POD;
  entry2.descriptor_set = 4;
  entry2.binding = 5;
  entry2.arg_name = "arg_b";
  entry2.arg_ordinal = 0;
  entry2.pod_offset = 4;
  entry2.pod_arg_size = 1;
  p.GetShaders()[0].AddDescriptorEntry("my_main", std::move(entry2));

  Pipeline::ShaderInfo::DescriptorMapEntry entry3;
  entry3.kind = Pipeline::ShaderInfo::DescriptorMapEntry::Kind::POD;
  entry3.descriptor_set = 4;
  entry3.binding = 4;
  entry3.arg_name = "arg_c";
  entry3.arg_ordinal = 0;
  entry3.pod_offset = 0;
  entry3.pod_arg_size = 4;
  p.GetShaders()[0].AddDescriptorEntry("my_main", std::move(entry3));

  // Set commands.
  Value int_value;
  int_value.SetIntValue(1);

  TypeParser parser;
  auto int_type = parser.Parse("R32_SINT");
  auto int_fmt = MakeUnique<Format>(int_type.get());
  auto char_type = parser.Parse("R8_SINT");
  auto char_fmt = MakeUnique<Format>(char_type.get());

  Pipeline::ArgSetInfo arg_info1;
  arg_info1.name = "arg_a";
  arg_info1.ordinal = 99;
  arg_info1.fmt = int_fmt.get();
  arg_info1.value = int_value;
  p.SetArg(std::move(arg_info1));

  Pipeline::ArgSetInfo arg_info2;
  arg_info2.name = "arg_b";
  arg_info2.ordinal = 99;
  arg_info2.fmt = char_fmt.get();
  arg_info2.value = int_value;
  p.SetArg(std::move(arg_info2));

  Pipeline::ArgSetInfo arg_info3;
  arg_info3.name = "arg_c";
  arg_info3.ordinal = 99;
  arg_info3.fmt = int_fmt.get();
  arg_info3.value = int_value;
  p.SetArg(std::move(arg_info3));

  auto r = p.GenerateOpenCLPodBuffers();
  ASSERT_TRUE(r.IsSuccess());
  EXPECT_EQ(2U, p.GetBuffers().size());

  const auto& b1 = p.GetBuffers()[0];
  EXPECT_EQ(4U, b1.descriptor_set);
  EXPECT_EQ(5U, b1.binding);
  EXPECT_EQ(5U, b1.buffer->ValueCount());

  const auto& b2 = p.GetBuffers()[1];
  EXPECT_EQ(4U, b2.descriptor_set);
  EXPECT_EQ(4U, b2.binding);
  EXPECT_EQ(4U, b2.buffer->ValueCount());
}

TEST_F(PipelineTest, OpenCLGeneratePodBuffersBadName) {
  Pipeline p(PipelineType::kCompute);
  p.SetName("my_pipeline");

  Shader cs(kShaderTypeCompute);
  cs.SetFormat(kShaderFormatOpenCLC);
  p.AddShader(&cs, kShaderTypeCompute);
  p.SetShaderEntryPoint(&cs, "my_main");

  // Descriptor map.
  Pipeline::ShaderInfo::DescriptorMapEntry entry1;
  entry1.kind = Pipeline::ShaderInfo::DescriptorMapEntry::Kind::POD;
  entry1.descriptor_set = 4;
  entry1.binding = 5;
  entry1.arg_name = "arg_a";
  entry1.arg_ordinal = 0;
  entry1.pod_offset = 0;
  entry1.pod_arg_size = 4;
  p.GetShaders()[0].AddDescriptorEntry("my_main", std::move(entry1));

  // Set commands.
  Value int_value;
  int_value.SetIntValue(1);

  TypeParser parser;
  auto int_type = parser.Parse("R32_SINT");
  auto int_fmt = MakeUnique<Format>(int_type.get());

  Pipeline::ArgSetInfo arg_info1;
  arg_info1.name = "arg_z";
  arg_info1.ordinal = 99;
  arg_info1.fmt = int_fmt.get();
  arg_info1.value = int_value;
  p.SetArg(std::move(arg_info1));

  auto r = p.GenerateOpenCLPodBuffers();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ(
      "could not find descriptor map entry for SET command: kernel my_main, "
      "name arg_z",
      r.Error());
}

TEST_F(PipelineTest, OpenCLGeneratePodBuffersBadSize) {
  Pipeline p(PipelineType::kCompute);
  p.SetName("my_pipeline");

  Shader cs(kShaderTypeCompute);
  cs.SetFormat(kShaderFormatOpenCLC);
  p.AddShader(&cs, kShaderTypeCompute);
  p.SetShaderEntryPoint(&cs, "my_main");

  // Descriptor map.
  Pipeline::ShaderInfo::DescriptorMapEntry entry1;
  entry1.kind = Pipeline::ShaderInfo::DescriptorMapEntry::Kind::POD;
  entry1.descriptor_set = 4;
  entry1.binding = 5;
  entry1.arg_name = "arg_a";
  entry1.arg_ordinal = 0;
  entry1.pod_offset = 0;
  entry1.pod_arg_size = 4;
  p.GetShaders()[0].AddDescriptorEntry("my_main", std::move(entry1));

  // Set commands.
  Value int_value;
  int_value.SetIntValue(1);

  TypeParser parser;
  auto short_type = parser.Parse("R16_SINT");
  auto short_fmt = MakeUnique<Format>(short_type.get());

  Pipeline::ArgSetInfo arg_info1;
  arg_info1.name = "";
  arg_info1.ordinal = 0;
  arg_info1.fmt = short_fmt.get();
  arg_info1.value = int_value;
  p.SetArg(std::move(arg_info1));

  auto r = p.GenerateOpenCLPodBuffers();
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("SET command uses incorrect data size: kernel my_main, number 0",
            r.Error());
}

TEST_F(PipelineTest, OpenCLClone) {
  Pipeline p(PipelineType::kCompute);
  p.SetName("my_pipeline");

  Shader cs(kShaderTypeCompute);
  cs.SetFormat(kShaderFormatOpenCLC);
  p.AddShader(&cs, kShaderTypeCompute);
  p.SetShaderEntryPoint(&cs, "my_main");

  // Descriptor map.
  Pipeline::ShaderInfo::DescriptorMapEntry entry1;
  entry1.kind = Pipeline::ShaderInfo::DescriptorMapEntry::Kind::POD;
  entry1.descriptor_set = 4;
  entry1.binding = 5;
  entry1.arg_name = "arg_a";
  entry1.arg_ordinal = 0;
  entry1.pod_offset = 0;
  entry1.pod_arg_size = 4;
  p.GetShaders()[0].AddDescriptorEntry("my_main", std::move(entry1));

  Pipeline::ShaderInfo::DescriptorMapEntry entry2;
  entry2.kind = Pipeline::ShaderInfo::DescriptorMapEntry::Kind::POD;
  entry2.descriptor_set = 4;
  entry2.binding = 5;
  entry2.arg_name = "arg_b";
  entry2.arg_ordinal = 0;
  entry2.pod_offset = 4;
  entry2.pod_arg_size = 1;
  p.GetShaders()[0].AddDescriptorEntry("my_main", std::move(entry2));

  Pipeline::ShaderInfo::DescriptorMapEntry entry3;
  entry3.kind = Pipeline::ShaderInfo::DescriptorMapEntry::Kind::POD;
  entry3.descriptor_set = 4;
  entry3.binding = 4;
  entry3.arg_name = "arg_c";
  entry3.arg_ordinal = 0;
  entry3.pod_offset = 0;
  entry3.pod_arg_size = 4;
  p.GetShaders()[0].AddDescriptorEntry("my_main", std::move(entry3));

  // Set commands.
  Value int_value;
  int_value.SetIntValue(1);

  TypeParser parser;
  auto int_type = parser.Parse("R32_SINT");
  auto int_fmt = MakeUnique<Format>(int_type.get());
  auto char_type = parser.Parse("R8_SINT");
  auto char_fmt = MakeUnique<Format>(char_type.get());

  Pipeline::ArgSetInfo arg_info1;
  arg_info1.name = "arg_a";
  arg_info1.ordinal = 99;
  arg_info1.fmt = int_fmt.get();
  arg_info1.value = int_value;
  p.SetArg(std::move(arg_info1));

  Pipeline::ArgSetInfo arg_info2;
  arg_info2.name = "arg_b";
  arg_info2.ordinal = 99;
  arg_info2.fmt = char_fmt.get();
  arg_info2.value = int_value;
  p.SetArg(std::move(arg_info2));

  Pipeline::ArgSetInfo arg_info3;
  arg_info3.name = "arg_c";
  arg_info3.ordinal = 99;
  arg_info3.fmt = int_fmt.get();
  arg_info3.value = int_value;
  p.SetArg(std::move(arg_info3));

  auto clone = p.Clone();
  auto r = clone->GenerateOpenCLPodBuffers();
  ASSERT_TRUE(r.IsSuccess());
  EXPECT_EQ(3U, clone->SetArgValues().size());
  EXPECT_EQ(2U, clone->GetBuffers().size());

  const auto& b1 = clone->GetBuffers()[0];
  EXPECT_EQ(4U, b1.descriptor_set);
  EXPECT_EQ(5U, b1.binding);
  EXPECT_EQ(5U, b1.buffer->ValueCount());

  const auto& b2 = clone->GetBuffers()[1];
  EXPECT_EQ(4U, b2.descriptor_set);
  EXPECT_EQ(4U, b2.binding);
  EXPECT_EQ(4U, b2.buffer->ValueCount());
}

TEST_F(PipelineTest, OpenCLGenerateLiteralSamplers) {
  Pipeline p(PipelineType::kCompute);
  p.SetName("my_pipeline");

  p.AddSampler(16, 0, 0);
  p.AddSampler(41, 0, 1);

  auto r = p.GenerateOpenCLLiteralSamplers();
  ASSERT_TRUE(r.IsSuccess());
  for (auto& info : p.GetSamplers()) {
    if (info.mask == 16) {
      EXPECT_NE(nullptr, info.sampler);
      EXPECT_EQ(FilterType::kNearest, info.sampler->GetMagFilter());
      EXPECT_EQ(FilterType::kNearest, info.sampler->GetMinFilter());
      EXPECT_EQ(AddressMode::kClampToEdge, info.sampler->GetAddressModeU());
      EXPECT_EQ(AddressMode::kClampToEdge, info.sampler->GetAddressModeV());
      EXPECT_EQ(AddressMode::kClampToEdge, info.sampler->GetAddressModeW());
      EXPECT_EQ(0.0f, info.sampler->GetMinLOD());
      EXPECT_EQ(0.0f, info.sampler->GetMaxLOD());
    } else {
      EXPECT_NE(nullptr, info.sampler);
      EXPECT_EQ(FilterType::kLinear, info.sampler->GetMagFilter());
      EXPECT_EQ(FilterType::kLinear, info.sampler->GetMinFilter());
      EXPECT_EQ(AddressMode::kMirroredRepeat, info.sampler->GetAddressModeU());
      EXPECT_EQ(AddressMode::kMirroredRepeat, info.sampler->GetAddressModeV());
      EXPECT_EQ(AddressMode::kMirroredRepeat, info.sampler->GetAddressModeW());
      EXPECT_EQ(0.0f, info.sampler->GetMinLOD());
      EXPECT_EQ(0.0f, info.sampler->GetMaxLOD());
    }
  }
}

TEST_F(PipelineTest, OpenCLGeneratePushConstants) {
  Pipeline p(PipelineType::kCompute);
  p.SetName("my_pipeline");

  Shader cs(kShaderTypeCompute);
  cs.SetFormat(kShaderFormatOpenCLC);
  p.AddShader(&cs, kShaderTypeCompute);
  p.SetShaderEntryPoint(&cs, "my_main");

  Pipeline::ShaderInfo::PushConstant pc1;
  pc1.type = Pipeline::ShaderInfo::PushConstant::PushConstantType::kDimensions;
  pc1.offset = 0;
  pc1.size = 4;
  p.GetShaders()[0].AddPushConstant(std::move(pc1));

  Pipeline::ShaderInfo::PushConstant pc2;
  pc2.type =
      Pipeline::ShaderInfo::PushConstant::PushConstantType::kGlobalOffset;
  pc2.offset = 16;
  pc2.size = 12;
  p.GetShaders()[0].AddPushConstant(std::move(pc2));

  auto r = p.GenerateOpenCLPushConstants();
  ASSERT_TRUE(r.IsSuccess());

  const auto& buf = p.GetPushConstantBuffer();
  EXPECT_EQ(28U, buf.buffer->GetSizeInBytes());

  const uint32_t* bytes = buf.buffer->GetValues<uint32_t>();
  EXPECT_EQ(3U, bytes[0]);
  EXPECT_EQ(0U, bytes[4]);
  EXPECT_EQ(0U, bytes[5]);
  EXPECT_EQ(0U, bytes[6]);
}

TEST_F(PipelineTest, OpenCLPodPushConstants) {
  Pipeline p(PipelineType::kCompute);
  p.SetName("my_pipeline");

  Shader cs(kShaderTypeCompute);
  cs.SetFormat(kShaderFormatOpenCLC);
  p.AddShader(&cs, kShaderTypeCompute);
  p.SetShaderEntryPoint(&cs, "my_main");

  // Descriptor map.
  Pipeline::ShaderInfo::DescriptorMapEntry entry1;
  entry1.kind =
      Pipeline::ShaderInfo::DescriptorMapEntry::Kind::POD_PUSHCONSTANT;
  entry1.descriptor_set = static_cast<uint32_t>(-1);
  entry1.binding = static_cast<uint32_t>(-1);
  entry1.arg_name = "arg_a";
  entry1.arg_ordinal = 0;
  entry1.pod_offset = 0;
  entry1.pod_arg_size = 4;
  p.GetShaders()[0].AddDescriptorEntry("my_main", std::move(entry1));

  Pipeline::ShaderInfo::DescriptorMapEntry entry2;
  entry2.kind =
      Pipeline::ShaderInfo::DescriptorMapEntry::Kind::POD_PUSHCONSTANT;
  entry2.descriptor_set = static_cast<uint32_t>(-1);
  entry2.binding = static_cast<uint32_t>(-1);
  entry2.arg_name = "arg_b";
  entry2.arg_ordinal = 1;
  entry2.pod_offset = 4;
  entry2.pod_arg_size = 1;
  p.GetShaders()[0].AddDescriptorEntry("my_main", std::move(entry2));

  Pipeline::ShaderInfo::DescriptorMapEntry entry3;
  entry3.kind =
      Pipeline::ShaderInfo::DescriptorMapEntry::Kind::POD_PUSHCONSTANT;
  entry3.descriptor_set = static_cast<uint32_t>(-1);
  entry3.binding = static_cast<uint32_t>(-1);
  entry3.arg_name = "arg_c";
  entry3.arg_ordinal = 2;
  entry3.pod_offset = 8;
  entry3.pod_arg_size = 4;
  p.GetShaders()[0].AddDescriptorEntry("my_main", std::move(entry3));

  // Set commands.
  Value int_value;
  int_value.SetIntValue(1);

  TypeParser parser;
  auto int_type = parser.Parse("R32_SINT");
  auto int_fmt = MakeUnique<Format>(int_type.get());
  auto char_type = parser.Parse("R8_SINT");
  auto char_fmt = MakeUnique<Format>(char_type.get());

  Pipeline::ArgSetInfo arg_info1;
  arg_info1.name = "arg_a";
  arg_info1.ordinal = 99;
  arg_info1.fmt = int_fmt.get();
  arg_info1.value = int_value;
  p.SetArg(std::move(arg_info1));

  Pipeline::ArgSetInfo arg_info2;
  arg_info2.name = "arg_b";
  arg_info2.ordinal = 99;
  arg_info2.fmt = char_fmt.get();
  arg_info2.value = int_value;
  p.SetArg(std::move(arg_info2));

  Pipeline::ArgSetInfo arg_info3;
  arg_info3.name = "arg_c";
  arg_info3.ordinal = 99;
  arg_info3.fmt = int_fmt.get();
  arg_info3.value = int_value;
  p.SetArg(std::move(arg_info3));

  auto r = p.GenerateOpenCLPodBuffers();
  auto* buf = p.GetPushConstantBuffer().buffer;
  EXPECT_NE(nullptr, buf);
  EXPECT_EQ(12U, buf->GetSizeInBytes());

  const uint32_t* ints = buf->GetValues<uint32_t>();
  const uint8_t* bytes = buf->GetValues<uint8_t>();
  EXPECT_EQ(1U, ints[0]);
  EXPECT_EQ(1U, bytes[4]);
  EXPECT_EQ(1U, ints[2]);
}

}  // namespace amber
