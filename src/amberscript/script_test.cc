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

#include "src/amberscript/script.h"
#include "gtest/gtest.h"
#include "src/amberscript/pipeline.h"
#include "src/amberscript/shader.h"
#include "src/buffer.h"
#include "src/make_unique.h"

namespace amber {
namespace amberscript {

using ScriptTest = testing::Test;

TEST_F(ScriptTest, AddShader) {
  auto shader = MakeUnique<Shader>(ShaderType::kVertex);
  shader->SetName("My Shader");

  Script s;
  Result r = s.AddShader(std::move(shader));
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
}

TEST_F(ScriptTest, AddDuplicateShader) {
  auto shader1 = MakeUnique<Shader>(ShaderType::kVertex);
  shader1->SetName("My Shader");

  Script s;
  Result r = s.AddShader(std::move(shader1));
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto shader2 = MakeUnique<Shader>(ShaderType::kFragment);
  shader2->SetName("My Shader");

  r = s.AddShader(std::move(shader2));
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("duplicate shader name provided", r.Error());
}

TEST_F(ScriptTest, GetShader) {
  auto shader = MakeUnique<Shader>(ShaderType::kVertex);
  shader->SetName("My Shader");

  auto* ptr = shader.get();

  Script s;
  Result r = s.AddShader(std::move(shader));
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  EXPECT_EQ(ptr, s.GetShader("My Shader"));
}

TEST_F(ScriptTest, GetMissingShader) {
  Script s;
  EXPECT_TRUE(s.GetShader("My Shader") == nullptr);
}

TEST_F(ScriptTest, GetShadersEmpty) {
  Script s;
  const auto& shaders = s.GetShaders();
  EXPECT_TRUE(shaders.empty());
}

TEST_F(ScriptTest, GetShaders) {
  auto shader1 = MakeUnique<Shader>(ShaderType::kVertex);
  shader1->SetName("My Shader");

  const auto* ptr1 = shader1.get();

  Script s;
  Result r = s.AddShader(std::move(shader1));
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto shader2 = MakeUnique<Shader>(ShaderType::kFragment);
  shader2->SetName("My Fragment");

  const auto* ptr2 = shader2.get();

  r = s.AddShader(std::move(shader2));
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  const auto& shaders = s.GetShaders();
  ASSERT_EQ(2U, shaders.size());
  EXPECT_EQ(ptr1, shaders[0].get());
  EXPECT_EQ(ptr2, shaders[1].get());
}

TEST_F(ScriptTest, AddPipeline) {
  auto pipeline = MakeUnique<Pipeline>(PipelineType::kCompute);
  pipeline->SetName("my_pipeline");

  Script s;
  Result r = s.AddPipeline(std::move(pipeline));
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
}

TEST_F(ScriptTest, AddDuplicatePipeline) {
  auto pipeline1 = MakeUnique<Pipeline>(PipelineType::kCompute);
  pipeline1->SetName("my_pipeline");

  Script s;
  Result r = s.AddPipeline(std::move(pipeline1));
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto pipeline2 = MakeUnique<Pipeline>(PipelineType::kGraphics);
  pipeline2->SetName("my_pipeline");
  r = s.AddPipeline(std::move(pipeline2));
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("duplicate pipeline name provided", r.Error());
}

TEST_F(ScriptTest, GetPipeline) {
  auto pipeline = MakeUnique<Pipeline>(PipelineType::kCompute);
  pipeline->SetName("my_pipeline");

  const auto* ptr = pipeline.get();

  Script s;
  Result r = s.AddPipeline(std::move(pipeline));
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  EXPECT_EQ(ptr, s.GetPipeline("my_pipeline"));
}

TEST_F(ScriptTest, GetMissingPipeline) {
  Script s;
  EXPECT_TRUE(s.GetPipeline("my_pipeline") == nullptr);
}

TEST_F(ScriptTest, GetPipelinesEmpty) {
  Script s;
  const auto& pipelines = s.GetPipelines();
  EXPECT_TRUE(pipelines.empty());
}

TEST_F(ScriptTest, GetPipelines) {
  auto pipeline1 = MakeUnique<Pipeline>(PipelineType::kCompute);
  pipeline1->SetName("my_pipeline1");

  const auto* ptr1 = pipeline1.get();

  Script s;
  Result r = s.AddPipeline(std::move(pipeline1));
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto pipeline2 = MakeUnique<Pipeline>(PipelineType::kGraphics);
  pipeline2->SetName("my_pipeline2");

  const auto* ptr2 = pipeline2.get();

  r = s.AddPipeline(std::move(pipeline2));
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  const auto& pipelines = s.GetPipelines();
  ASSERT_EQ(2U, pipelines.size());
  EXPECT_EQ(ptr1, pipelines[0].get());
  EXPECT_EQ(ptr2, pipelines[1].get());
}

TEST_F(ScriptTest, AddBuffer) {
  auto buffer = MakeUnique<Buffer>(BufferType::kStorage);
  buffer->SetName("my_buffer");

  Script s;
  Result r = s.AddBuffer(std::move(buffer));
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
}

TEST_F(ScriptTest, AddDuplicateBuffer) {
  auto buffer1 = MakeUnique<Buffer>(BufferType::kStorage);
  buffer1->SetName("my_buffer");

  Script s;
  Result r = s.AddBuffer(std::move(buffer1));
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto buffer2 = MakeUnique<Buffer>(BufferType::kUniform);
  buffer2->SetName("my_buffer");

  r = s.AddBuffer(std::move(buffer2));
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("duplicate buffer name provided", r.Error());
}

TEST_F(ScriptTest, GetBuffer) {
  auto buffer = MakeUnique<Buffer>(BufferType::kStorage);
  buffer->SetName("my_buffer");

  const auto* ptr = buffer.get();

  Script s;
  Result r = s.AddBuffer(std::move(buffer));
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  EXPECT_EQ(ptr, s.GetBuffer("my_buffer"));
}

TEST_F(ScriptTest, GetMissingBuffer) {
  Script s;
  EXPECT_TRUE(s.GetBuffer("my_buffer") == nullptr);
}

TEST_F(ScriptTest, GetBuffersEmpty) {
  Script s;
  const auto& buffers = s.GetBuffers();
  EXPECT_TRUE(buffers.empty());
}

TEST_F(ScriptTest, GetBuffers) {
  auto buffer1 = MakeUnique<Buffer>(BufferType::kStorage);
  buffer1->SetName("my_buffer1");

  const auto* ptr1 = buffer1.get();

  Script s;
  Result r = s.AddBuffer(std::move(buffer1));
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto buffer2 = MakeUnique<Buffer>(BufferType::kUniform);
  buffer2->SetName("my_buffer2");

  const auto* ptr2 = buffer2.get();

  r = s.AddBuffer(std::move(buffer2));
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  const auto& buffers = s.GetBuffers();
  ASSERT_EQ(2U, buffers.size());
  EXPECT_EQ(ptr1, buffers[0].get());
  EXPECT_EQ(ptr2, buffers[1].get());
}

}  // namespace amberscript
}  // namespace amber
