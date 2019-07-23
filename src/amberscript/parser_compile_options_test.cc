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

TEST_F(AmberScriptParserTest, PipelineShaderCompileOptions) {
  std::string in = R"(
SHADER compute my_shader OPENCL-C
#shader
END
PIPELINE compute my_pipeline
  ATTACH my_shader
  COMPILE_OPTIONS my_shader
    --option1
    --option2=blah
    other
    --option3 3
  END
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess());

  auto script = parser.GetScript();
  const auto& pipelines = script->GetPipelines();
  ASSERT_EQ(1U, pipelines.size());

  const auto* pipeline = pipelines[0].get();
  const auto& shaders = pipeline->GetShaders();
  ASSERT_EQ(1U, shaders.size());

  const auto& shader = shaders[0];
  const auto& options = shader.GetCompileOptions();
  ASSERT_EQ(5U, options.size());
  EXPECT_EQ("--option1", options[0]);
  EXPECT_EQ("--option2=blah", options[1]);
  EXPECT_EQ("other", options[2]);
  EXPECT_EQ("--option3", options[3]);
  EXPECT_EQ("3", options[4]);
}

TEST_F(AmberScriptParserTest, PipelineShaderCompileOptionsMissingShader) {
  std::string in = R"(
SHADER compute my_shader OPENCL-C
#shader
END
PIPELINE compute my_pipeline
  ATTACH my_shader
  COMPILE_OPTIONS
END
)";

  Parser parser;
  auto r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("8: missing shader name in COMPILE_OPTIONS command", r.Error());
}

TEST_F(AmberScriptParserTest, PipelineShaderCompileOptionsBadShader) {
  std::string in = R"(
SHADER compute my_shader OPENCL-C
#shader
END
PIPELINE compute my_pipeline
  ATTACH my_shader
  COMPILE_OPTIONS not_my_shader
  END
END
)";

  Parser parser;
  auto r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("7: unknown shader in COMPILE_OPTIONS command", r.Error());
}

TEST_F(AmberScriptParserTest, PipelineShaderCompileOptionsMissingEnd) {
  std::string in = R"(
SHADER compute my_shader OPENCL-C
#shader
END
PIPELINE compute my_pipeline
  ATTACH my_shader
  COMPILE_OPTIONS my_shader
)";

  Parser parser;
  auto r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("8: COMPILE_OPTIONS missing END command", r.Error());
}

TEST_F(AmberScriptParserTest, PipelineShaderCompileOptionsExtraToken) {
  std::string in = R"(
SHADER compute my_shader OPENCL-C
#shader
END
PIPELINE compute my_pipeline
  ATTACH my_shader
  COMPILE_OPTIONS my_shader extra
  END
END
)";

  Parser parser;
  auto r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("7: extra parameters after COMPILE_OPTIONS command", r.Error());
}

TEST_F(AmberScriptParserTest, PipelineShaderCompileOptionsExtraTokenEnd) {
  std::string in = R"(
SHADER compute my_shader OPENCL-C
#shader
END
PIPELINE compute my_pipeline
  ATTACH my_shader
  COMPILE_OPTIONS my_shader
  END token
END
)";

  Parser parser;
  auto r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("8: extra parameters after COMPILE_OPTIONS command", r.Error());
}

TEST_F(AmberScriptParserTest, PipelineShaderCompileOptionsNotOpenCL) {
  std::string in = R"(
SHADER compute my_shader SPIRV-ASM
#shader
END
PIPELINE compute my_pipeline
  ATTACH my_shader
  COMPILE_OPTIONS my_shader
  END token
END
)";

  Parser parser;
  auto r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("7: COMPILE_OPTIONS currently only supports OPENCL-C shaders",
            r.Error());
}

}  // namespace amberscript
}  // namespace amber
