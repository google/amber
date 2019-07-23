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

TEST_F(AmberScriptParserTest, OpenCLSetMissingKernel) {
  std::string in = R"(
SHADER compute my_shader OPENCL-C
#shader
END
PIPELINE compute my_pipeline
  ATTACH my_shader
  SET ARG_NAME a AS uint32 0
END
)";

  Parser parser;
  auto r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("7: missing KERNEL in SET command", r.Error());
}

TEST_F(AmberScriptParserTest, OpenCLSetMissingArgName) {
  std::string in = R"(
SHADER compute my_shader OPENCL-C
#shader
END
PIPELINE compute my_pipeline
  ATTACH my_shader
  SET KERNEL a AS uint32 0
END
)";

  Parser parser;
  auto r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("7: expected ARG_NAME or ARG_NUMBER", r.Error());
}

TEST_F(AmberScriptParserTest, OpenCLSetMissingArgIdentifier) {
  std::string in = R"(
SHADER compute my_shader OPENCL-C
#shader
END
PIPELINE compute my_pipeline
  ATTACH my_shader
  SET KERNEL ARG_NAME AS uint32 0
END
)";

  Parser parser;
  auto r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("7: missing AS in SET command", r.Error());
}

TEST_F(AmberScriptParserTest, OpenCLSetMissingArgIdentifierNumber) {
  std::string in = R"(
SHADER compute my_shader OPENCL-C
#shader
END
PIPELINE compute my_pipeline
  ATTACH my_shader
  SET KERNEL ARG_NUMBER AS uint32 0
END
)";

  Parser parser;
  auto r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("7: expected argument number", r.Error());
}

TEST_F(AmberScriptParserTest, OpenCLSetMissingAs) {
  std::string in = R"(
SHADER compute my_shader OPENCL-C
#shader
END
PIPELINE compute my_pipeline
  ATTACH my_shader
  SET KERNEL ARG_NAME a uint32 0
END
)";

  Parser parser;
  auto r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("7: missing AS in SET command", r.Error());
}

TEST_F(AmberScriptParserTest, OpenCLSetMissingDataType) {
  std::string in = R"(
SHADER compute my_shader OPENCL-C
#shader
END
PIPELINE compute my_pipeline
  ATTACH my_shader
  SET KERNEL ARG_NAME a AS 0
END
)";

  Parser parser;
  auto r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("7: expected data type", r.Error());
}

TEST_F(AmberScriptParserTest, OpenCLSetMissingDataValue) {
  std::string in = R"(
SHADER compute my_shader OPENCL-C
#shader
END
PIPELINE compute my_pipeline
  ATTACH my_shader
  SET KERNEL ARG_NAME a AS uint32
END
)";

  Parser parser;
  auto r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("8: expected data value", r.Error());
}

TEST_F(AmberScriptParserTest, OpenCLSetExtraTokens) {
  std::string in = R"(
SHADER compute my_shader OPENCL-C
#shader
END
PIPELINE compute my_pipeline
  ATTACH my_shader
  SET KERNEL ARG_NAME a AS uint32 0 BLAH
END
)";

  Parser parser;
  auto r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("7: extra parameters after SET command", r.Error());
}

TEST_F(AmberScriptParserTest, OpenCLSetArgNameNotString) {
  std::string in = R"(
SHADER compute my_shader OPENCL-C
#shader
END
PIPELINE compute my_pipeline
  ATTACH my_shader
  SET KERNEL ARG_NAME 0 AS uint32 0
END
)";

  Parser parser;
  auto r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("7: expected argument identifier", r.Error());
}

TEST_F(AmberScriptParserTest, OpenCLSetArgNumberNotInteger) {
  std::string in = R"(
SHADER compute my_shader OPENCL-C
#shader
END
PIPELINE compute my_pipeline
  ATTACH my_shader
  SET KERNEL ARG_NUMBER 1.0 AS uint32 0
END
)";

  Parser parser;
  auto r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("7: expected argument number", r.Error());
}

TEST_F(AmberScriptParserTest, OpenCLSetDataTypeNotString) {
  std::string in = R"(
SHADER compute my_shader OPENCL-C
#shader
END
PIPELINE compute my_pipeline
  ATTACH my_shader
  SET KERNEL ARG_NUMBER 0 AS 0 0
END
)";

  Parser parser;
  auto r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("7: expected data type", r.Error());
}

TEST_F(AmberScriptParserTest, OpenCLSetDataValueString) {
  std::string in = R"(
SHADER compute my_shader OPENCL-C
#shader
END
PIPELINE compute my_pipeline
  ATTACH my_shader
  SET KERNEL ARG_NUMBER 0 AS uint32 data
END
)";

  Parser parser;
  auto r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("7: expected data value", r.Error());
}

TEST_F(AmberScriptParserTest, OpenCLSetWrongShaderFormat) {
  std::string in = R"(
SHADER compute my_shader SPIRV-ASM
#shader
END
PIPELINE compute my_pipeline
  ATTACH my_shader
  SET KERNEL ARG_NAME arg_a AS uint32 0
END
)";

  Parser parser;
  auto r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("7: SET can only be used with OPENCL-C shaders", r.Error());
}

}  // namespace amberscript
}  // namespace amber
