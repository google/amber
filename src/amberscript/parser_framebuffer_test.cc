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

TEST_F(AmberScriptParserTest, FramebufferDefaultSize) {
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
  const auto& pipelines = script->GetPipelines();
  ASSERT_EQ(1U, pipelines.size());

  const auto* pipeline = pipelines[0].get();
  EXPECT_EQ(250, pipeline->GetFramebufferWidth());
  EXPECT_EQ(250, pipeline->GetFramebufferHeight());
}

TEST_F(AmberScriptParserTest, FramebufferSize) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
  FRAMEBUFFER_SIZE 256 246
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& pipelines = script->GetPipelines();
  ASSERT_EQ(1U, pipelines.size());

  const auto* pipeline = pipelines[0].get();
  EXPECT_EQ(256, pipeline->GetFramebufferWidth());
  EXPECT_EQ(246, pipeline->GetFramebufferHeight());
}

TEST_F(AmberScriptParserTest, FramebufferSizeMissingSize) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
  FRAMEBUFFER_SIZE
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());

  EXPECT_EQ("10: missing size for FRAMEBUFFER_SIZE command", r.Error());
}

TEST_F(AmberScriptParserTest, FramebufferSizeMissingHeight) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
  FRAMEBUFFER_SIZE 222
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());

  EXPECT_EQ("10: missing height for FRAMEBUFFER_SIZE command", r.Error());
}

TEST_F(AmberScriptParserTest, FramebufferSizeExtraParams) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
  FRAMEBUFFER_SIZE 222 233 INVALID
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());

  EXPECT_EQ("9: extra parameters after FRAMEBUFFER_SIZE command", r.Error());
}

TEST_F(AmberScriptParserTest, FramebufferInvalidWidth) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
  FRAMEBUFFER_SIZE INVALID 245
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());

  EXPECT_EQ("9: invalid width for FRAMEBUFFER_SIZE command", r.Error());
}

TEST_F(AmberScriptParserTest, FramebufferInvalidHeight) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment
  FRAMEBUFFER_SIZE 245 INVALID
END
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());

  EXPECT_EQ("9: invalid height for FRAMEBUFFER_SIZE command", r.Error());
}

}  // namespace amberscript
}  // namespace amber
