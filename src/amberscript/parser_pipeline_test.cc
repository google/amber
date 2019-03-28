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

TEST_F(AmberScriptParserTest, Pipeline) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  EXPECT_EQ(2U, script->GetShaders().size());

  const auto& pipelines = script->GetPipelines();
  ASSERT_EQ(1U, pipelines.size());

  const auto* pipeline = pipelines[0].get();
  EXPECT_EQ("my_pipeline", pipeline->GetName());
  EXPECT_EQ(PipelineType::kGraphics, pipeline->GetType());

  const auto& shaders = pipeline->GetShaders();
  ASSERT_EQ(2U, shaders.size());

  ASSERT_TRUE(shaders[0].GetShader() != nullptr);
  EXPECT_EQ("my_shader", shaders[0].GetShader()->GetName());
  EXPECT_EQ(kShaderTypeVertex, shaders[0].GetShader()->GetType());
  EXPECT_EQ(static_cast<uint32_t>(0),
            shaders[0].GetShaderOptimizations().size());

  ASSERT_TRUE(shaders[1].GetShader() != nullptr);
  EXPECT_EQ("my_fragment", shaders[1].GetShader()->GetName());
  EXPECT_EQ(kShaderTypeFragment, shaders[1].GetShader()->GetType());
  EXPECT_EQ(static_cast<uint32_t>(0),
            shaders[1].GetShaderOptimizations().size());
}

TEST_F(AmberScriptParserTest, PipelineMissingEnd) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
PIPELINE graphics my_pipeline
  ATTACH my_shader
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("5: PIPELINE missing END command", r.Error());
}

TEST_F(AmberScriptParserTest, PipelineWithExtraParams) {
  std::string in = R"(
PIPELINE graphics my_pipeline INVALID
  ATTACH my_shader
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("2: extra parameters after PIPELINE command", r.Error());
}

TEST_F(AmberScriptParserTest, PipelineInvalidType) {
  std::string in = "PIPELINE my_name\nEND";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: unknown pipeline type: my_name", r.Error());
}

TEST_F(AmberScriptParserTest, PipelineMissingName) {
  std::string in = "PIPELINE compute\nEND";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("2: invalid token when looking for pipeline name", r.Error());
}

TEST_F(AmberScriptParserTest, PipelineWithInvalidTokenType) {
  std::string in = "PIPELINE 123 my_pipeline\nEND";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: invalid token when looking for pipeline type", r.Error());
}

TEST_F(AmberScriptParserTest, PipelineWithInvalidTokenName) {
  std::string in = "PIPELINE compute 123\nEND";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: invalid token when looking for pipeline name", r.Error());
}

TEST_F(AmberScriptParserTest, PipelineEmpty) {
  std::string in = "PIPELINE compute my_pipeline\nEND";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("compute pipeline requires a compute shader", r.Error());
}

TEST_F(AmberScriptParserTest, PipelineWithUnknownCommand) {
  std::string in = R"(
PIPELINE compute my_pipeline
  SHADER vertex my_shader PASSTHROUGH
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: unknown token in pipeline block: SHADER", r.Error());
}

TEST_F(AmberScriptParserTest, DuplicatePipelineName) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# Fragment shader
END

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
END
PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("14: duplicate pipeline name provided", r.Error());
}

TEST_F(AmberScriptParserTest, PipelineDefaultColorBuffer) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
END
PIPELINE graphics my_pipeline2
  ATTACH my_shader
  ATTACH my_fragment
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& pipelines = script->GetPipelines();
  ASSERT_EQ(2U, pipelines.size());

  ASSERT_EQ(1U, pipelines[0]->GetColorAttachments().size());
  const auto& buf1 = pipelines[0]->GetColorAttachments()[0];
  ASSERT_TRUE(buf1.buffer != nullptr);

  Buffer* buffer1 = buf1.buffer;
  ASSERT_TRUE(buffer1->IsFormatBuffer());
  EXPECT_EQ(FormatType::kB8G8R8A8_UNORM,
            buffer1->AsFormatBuffer()->GetFormat().GetFormatType());
  EXPECT_EQ(0, buf1.location);
  EXPECT_EQ(250 * 250, buffer1->GetSize());
  EXPECT_EQ(250 * 250 * sizeof(uint32_t), buffer1->GetSizeInBytes());

  ASSERT_EQ(1U, pipelines[1]->GetColorAttachments().size());
  const auto& buf2 = pipelines[1]->GetColorAttachments()[0];
  ASSERT_TRUE(buf2.buffer != nullptr);
  ASSERT_EQ(buffer1, buf2.buffer);
  EXPECT_EQ(0, buf2.location);
  EXPECT_EQ(FormatType::kB8G8R8A8_UNORM,
            buf2.buffer->AsFormatBuffer()->GetFormat().GetFormatType());
  EXPECT_EQ(250 * 250, buf2.buffer->GetSize());
  EXPECT_EQ(250 * 250 * sizeof(uint32_t), buf2.buffer->GetSizeInBytes());
}

TEST_F(AmberScriptParserTest, PipelineDefaultColorBufferMismatchSize) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
END
PIPELINE graphics my_pipeline2
  ATTACH my_shader
  ATTACH my_fragment
  FRAMEBUFFER_SIZE 256 256
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());

  EXPECT_EQ("shared framebuffer must have same size over all PIPELINES",
            r.Error());
}

TEST_F(AmberScriptParserTest, PipelineDefaultDepthBuffer) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& pipelines = script->GetPipelines();
  ASSERT_EQ(1U, pipelines.size());

  const auto* pipeline = pipelines[0].get();
  const auto& buf = pipeline->GetDepthBuffer();

  ASSERT_TRUE(buf.buffer != nullptr);
  EXPECT_EQ(FormatType::kD32_SFLOAT_S8_UINT,
            buf.buffer->AsFormatBuffer()->GetFormat().GetFormatType());
  EXPECT_EQ(250 * 250, buf.buffer->GetSize());
  EXPECT_EQ(250 * 250 * (sizeof(float) + sizeof(uint8_t)),
            buf.buffer->GetSizeInBytes());
}

}  // namespace amberscript
}  // namespace amber
