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

TEST_F(AmberScriptParserTest, ExpectRGB) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0
END

EXPECT my_fb IDX 5 6 SIZE 250 150 EQ_RGB 2 128 255)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& commands = script->GetCommands();
  ASSERT_EQ(1U, commands.size());

  auto* cmd = commands[0].get();
  ASSERT_TRUE(cmd->IsProbe());

  auto* probe = cmd->AsProbe();
  EXPECT_FALSE(probe->IsRGBA());
  EXPECT_TRUE(probe->IsProbeRect());
  EXPECT_FALSE(probe->IsRelative());
  EXPECT_FALSE(probe->IsWholeWindow());
  EXPECT_EQ(5U, probe->GetX());
  EXPECT_EQ(6U, probe->GetY());
  EXPECT_EQ(250U, probe->GetWidth());
  EXPECT_EQ(150U, probe->GetHeight());
  EXPECT_FLOAT_EQ(2.f / 255.f, probe->GetR());
  EXPECT_FLOAT_EQ(128.f / 255.f, probe->GetG());
  EXPECT_FLOAT_EQ(255.f / 255.f, probe->GetB());
}

TEST_F(AmberScriptParserTest, ExpectRGBA) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0
END

EXPECT my_fb IDX 2 7 SIZE 20 88 EQ_RGBA 2 128 255 99)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& commands = script->GetCommands();
  ASSERT_EQ(1U, commands.size());

  auto* cmd = commands[0].get();
  ASSERT_TRUE(cmd->IsProbe());

  auto* probe = cmd->AsProbe();
  EXPECT_TRUE(probe->IsRGBA());
  EXPECT_TRUE(probe->IsProbeRect());
  EXPECT_FALSE(probe->IsRelative());
  EXPECT_FALSE(probe->IsWholeWindow());
  EXPECT_EQ(2U, probe->GetX());
  EXPECT_EQ(7U, probe->GetY());
  EXPECT_EQ(20U, probe->GetWidth());
  EXPECT_EQ(88U, probe->GetHeight());
  EXPECT_FLOAT_EQ(2.f / 255.f, probe->GetR());
  EXPECT_FLOAT_EQ(128.f / 255.f, probe->GetG());
  EXPECT_FLOAT_EQ(255.f / 255.f, probe->GetB());
  EXPECT_FLOAT_EQ(99.f / 255.f, probe->GetA());
}

TEST_F(AmberScriptParserTest, ExpectMissingBufferName) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0
END

EXPECT IDX 0 0 SIZE 250 250 EQ_RGB 0 128 255)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("15: missing buffer name between EXPECT and IDX", r.Error());
}

TEST_F(AmberScriptParserTest, ExpectInvalidBufferName) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0
END

EXPECT unknown_buffer IDX 0 0 SIZE 250 250 EQ_RGB 0 128 255)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("15: unknown buffer name for EXPECT command: unknown_buffer",
            r.Error());
}

TEST_F(AmberScriptParserTest, ExpectMissingIDX) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0
END

EXPECT my_fb 0 0 SIZE 250 250 EQ_RGB 0 128 255)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("15: invalid comparator in EXPECT command", r.Error());
}

TEST_F(AmberScriptParserTest, ExpectMissingIDXValues) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0
END

EXPECT my_fb IDX SIZE 250 250 EQ_RGB 0 128 255)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("15: invalid X value in EXPECT command", r.Error());
}

TEST_F(AmberScriptParserTest, ExpectMissingIdxY) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0
END

EXPECT my_fb IDX 0 SIZE 250 250 EQ_RGB 0 128 255)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("15: invalid Y value in EXPECT command", r.Error());
}

TEST_F(AmberScriptParserTest, ExpectIdxInvalidX) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0
END

EXPECT my_fb IDX INVAILD 0 SIZE 250 250 EQ_RGB 0 128 255)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("15: invalid X value in EXPECT command", r.Error());
}

TEST_F(AmberScriptParserTest, ExpectIdxInvalidY) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0
END

EXPECT my_fb IDX 0 INVALID SIZE 250 250 EQ_RGB 0 128 255)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("15: unexpected token in EXPECT command: INVALID", r.Error());
}

TEST_F(AmberScriptParserTest, ExpectRGBMissingSize) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0
END

EXPECT my_fb IDX 0 0 250 250 EQ_RGB 0 128 255)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("15: unexpected token in EXPECT command: 250", r.Error());
}

TEST_F(AmberScriptParserTest, ExpectSizeMissingValues) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0
END

EXPECT my_fb IDX 0 0 SIZE EQ_RGB 0 128 255)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("15: invalid width in EXPECT command", r.Error());
}

TEST_F(AmberScriptParserTest, ExpectSizeMissingHeight) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0
END

EXPECT my_fb IDX 0 0 SIZE 250 EQ_RGB 0 128 255)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("15: invalid height in EXPECT command", r.Error());
}

TEST_F(AmberScriptParserTest, ExpectSizeInvalidWidth) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0
END

EXPECT my_fb IDX 0 0 SIZE INVALID 250 EQ_RGB 0 128 255)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("15: invalid width in EXPECT command", r.Error());
}

TEST_F(AmberScriptParserTest, ExpectSizeInvalidHeight) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0
END

EXPECT my_fb IDX 0 0 SIZE 250 INVALID EQ_RGB 0 128 255)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("15: invalid height in EXPECT command", r.Error());
}

TEST_F(AmberScriptParserTest, ExpectSizeInvalidComparitor) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0
END

EXPECT my_fb IDX 0 0 SIZE 250 250 INVALID 0 128 255)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("15: unknown comparator type in EXPECT: INVALID", r.Error());
}

TEST_F(AmberScriptParserTest, ExpectRGBMissingValues) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0
END

EXPECT my_fb IDX 0 0 SIZE 250 250 EQ_RGB)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("15: invalid R value in EXPECT command", r.Error());
}

TEST_F(AmberScriptParserTest, ExpectRGBMissingB) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0
END

EXPECT my_fb IDX 0 0 SIZE 250 250 EQ_RGB 0 128)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("15: invalid B value in EXPECT command", r.Error());
}

TEST_F(AmberScriptParserTest, ExpectRGBMissingG) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0
END

EXPECT my_fb IDX 0 0 SIZE 250 250 EQ_RGB 0)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("15: invalid G value in EXPECT command", r.Error());
}

TEST_F(AmberScriptParserTest, ExpectRGBAMissingA) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0
END

EXPECT my_fb IDX 0 0 SIZE 250 250 EQ_RGBA 0 128 255)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("15: invalid A value in EXPECT command", r.Error());
}

TEST_F(AmberScriptParserTest, ExpectRGBInvalidR) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0
END

EXPECT my_fb IDX 0 0 SIZE 250 250 EQ_RGB INVALID 128 255)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("15: invalid R value in EXPECT command", r.Error());
}

TEST_F(AmberScriptParserTest, ExpectRGBInvalidG) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0
END

EXPECT my_fb IDX 0 0 SIZE 250 250 EQ_RGB 0 INVALID 255)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("15: invalid G value in EXPECT command", r.Error());
}

TEST_F(AmberScriptParserTest, ExpectRGBInvalidB) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0
END

EXPECT my_fb IDX 0 0 SIZE 250 250 EQ_RGB 0 128 INVALID)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("15: invalid B value in EXPECT command", r.Error());
}

TEST_F(AmberScriptParserTest, ExpectRGBAInvalidA) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0
END

EXPECT my_fb IDX 0 0 SIZE 250 250 EQ_RGBA 0 128 255 INVALID)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("15: invalid A value in EXPECT command", r.Error());
}

TEST_F(AmberScriptParserTest, ExpectRGBExtraParam) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0
END

EXPECT my_fb IDX 0 0 SIZE 250 250 EQ_RGB 0 128 255 EXTRA)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("15: extra parameters after EXPECT command", r.Error());
}

TEST_F(AmberScriptParserTest, ExpectRGBAExtraParam) {
  std::string in = R"(
SHADER vertex my_shader PASSTHROUGH
SHADER fragment my_fragment GLSL
# GLSL Shader
END
BUFFER my_fb FORMAT R32G32B32A32_SFLOAT

PIPELINE graphics my_pipeline
  ATTACH my_shader
  ATTACH my_fragment

  BIND BUFFER my_fb AS color LOCATION 0
END

EXPECT my_fb IDX 0 0 SIZE 250 250 EQ_RGBA 0 128 255 99 EXTRA)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("15: extra parameters after EXPECT command", r.Error());
}

TEST_F(AmberScriptParserTest, ExpectEQ) {
  std::string in = R"(
BUFFER orig_buf DATA_TYPE int32 SIZE 100 FILL 11
EXPECT orig_buf IDX 5 EQ 11)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& commands = script->GetCommands();
  ASSERT_EQ(1U, commands.size());

  auto* cmd = commands[0].get();
  ASSERT_TRUE(cmd->IsProbeSSBO());

  auto* probe = cmd->AsProbeSSBO();
  EXPECT_EQ(ProbeSSBOCommand::Comparator::kEqual, probe->GetComparator());
  EXPECT_EQ(5U, probe->GetOffset());
  EXPECT_TRUE(probe->GetFormat()->IsInt32());
  ASSERT_EQ(1U, probe->GetValues().size());
  EXPECT_EQ(11U, probe->GetValues()[0].AsInt32());
}

TEST_F(AmberScriptParserTest, ExpectEQStruct) {
  std::string in = R"(
STRUCT data
  float a
  int32 b
END

BUFFER orig_buf DATA_TYPE data DATA 2.3 44 4.4 99 END
EXPECT orig_buf IDX 0 EQ 2.3 44
EXPECT orig_buf IDX 8 EQ 2.3 44)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& commands = script->GetCommands();
  ASSERT_EQ(2U, commands.size());

  auto* cmd = commands[0].get();
  ASSERT_TRUE(cmd->IsProbeSSBO());

  auto* probe = cmd->AsProbeSSBO();
  EXPECT_EQ(ProbeSSBOCommand::Comparator::kEqual, probe->GetComparator());
  ASSERT_EQ(2U, probe->GetValues().size());
  EXPECT_EQ(2.3f, probe->GetValues()[0].AsFloat());
  EXPECT_EQ(44, probe->GetValues()[1].AsInt32());

  cmd = commands[1].get();
  ASSERT_TRUE(cmd->IsProbeSSBO());

  probe = cmd->AsProbeSSBO();
  EXPECT_EQ(ProbeSSBOCommand::Comparator::kEqual, probe->GetComparator());
  ASSERT_EQ(2U, probe->GetValues().size());
  EXPECT_EQ(2.3f, probe->GetValues()[0].AsFloat());
  EXPECT_EQ(44, probe->GetValues()[1].AsInt32());
}

TEST_F(AmberScriptParserTest, ExpectEqMissingValue) {
  std::string in = R"(
BUFFER orig_buf DATA_TYPE int32 SIZE 100 FILL 11
EXPECT orig_buf IDX 5 EQ)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: missing comparison values for EXPECT command", r.Error());
}

TEST_F(AmberScriptParserTest, ExpectEQExtraParams) {
  std::string in = R"(
BUFFER orig_buf DATA_TYPE int32 SIZE 100 FILL 11
EXPECT orig_buf IDX 5 EQ 11 EXTRA)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: Invalid value provided to EXPECT command: EXTRA", r.Error());
}

TEST_F(AmberScriptParserTest, MultipleExpect) {
  std::string in = R"(
BUFFER orig_buf DATA_TYPE int32 SIZE 100 FILL 11
BUFFER dest_buf DATA_TYPE int32 SIZE 100 FILL 22

EXPECT orig_buf IDX 0 EQ 11
EXPECT dest_buf IDX 0 EQ 22)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
}

TEST_F(AmberScriptParserTest, ExpectEqBuffer) {
  std::string in = R"(
BUFFER buf_1 DATA_TYPE int32 SIZE 10 FILL 11
BUFFER buf_2 DATA_TYPE int32 SIZE 10 FILL 11
EXPECT buf_1 EQ_BUFFER buf_2)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& commands = script->GetCommands();
  ASSERT_EQ(1U, commands.size());

  auto* cmd = commands[0].get();
  ASSERT_TRUE(cmd->IsCompareBuffer());

  auto* cmp = cmd->AsCompareBuffer();
  EXPECT_EQ(cmp->GetComparator(), CompareBufferCommand::Comparator::kEq);

  ASSERT_TRUE(cmp->GetBuffer1() != nullptr);
  EXPECT_EQ(cmp->GetBuffer1()->GetName(), "buf_1");

  ASSERT_TRUE(cmp->GetBuffer2() != nullptr);
  EXPECT_EQ(cmp->GetBuffer2()->GetName(), "buf_2");
}

TEST_F(AmberScriptParserTest, ExpectEqBufferMissingFirstBuffer) {
  std::string in = R"(
BUFFER buf_2 DATA_TYPE int32 SIZE 10 FILL 22
EXPECT EQ_BUFFER buf_2)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: missing buffer name between EXPECT and EQ_BUFFER", r.Error());
}

TEST_F(AmberScriptParserTest, ExpectEqBufferMissingSecondBuffer) {
  std::string in = R"(
BUFFER buf_1 DATA_TYPE int32 SIZE 10 FILL 11
EXPECT buf_1 EQ_BUFFER)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: invalid buffer name in EXPECT EQ_BUFFER command", r.Error());
}

TEST_F(AmberScriptParserTest, ExpectEqBufferInvalidFirstBuffer) {
  std::string in = R"(EXPECT 123 EQ_BUFFER)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: invalid buffer name in EXPECT command", r.Error());
}

TEST_F(AmberScriptParserTest, ExpectEqBufferUnknownFirstBuffer) {
  std::string in = R"(EXPECT unknown_buffer EQ_BUFFER)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: unknown buffer name for EXPECT command: unknown_buffer",
            r.Error());
}

TEST_F(AmberScriptParserTest, ExpectEqBufferInvalidSecondBuffer) {
  std::string in = R"(
BUFFER buf DATA_TYPE int32 SIZE 10 FILL 11
EXPECT buf EQ_BUFFER 123)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: invalid buffer name in EXPECT EQ_BUFFER command", r.Error());
}

TEST_F(AmberScriptParserTest, ExpectEqBufferUnknownSecondBuffer) {
  std::string in = R"(
BUFFER buf DATA_TYPE int32 SIZE 10 FILL 11
EXPECT buf EQ_BUFFER unknown_buffer)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ(
      "3: unknown buffer name for EXPECT EQ_BUFFER command: unknown_buffer",
      r.Error());
}

TEST_F(AmberScriptParserTest, ExpectEqBufferDifferentSize) {
  std::string in = R"(
BUFFER buf_1 DATA_TYPE int32 SIZE 10 FILL 11
BUFFER buf_2 DATA_TYPE int32 SIZE 99 FILL 11
EXPECT buf_1 EQ_BUFFER buf_2)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ(
      "4: EXPECT EQ_BUFFER command cannot compare buffers of different size: "
      "10 vs 99",
      r.Error());
}

TEST_F(AmberScriptParserTest, ExpectEqBufferDifferentType) {
  std::string in = R"(
BUFFER buf_1 DATA_TYPE int32 SIZE 10 FILL 11
BUFFER buf_2 FORMAT R32G32B32A32_SFLOAT
EXPECT buf_1 EQ_BUFFER buf_2)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ(
      "4: EXPECT EQ_BUFFER command cannot compare buffers of differing format",
      r.Error());
}

TEST_F(AmberScriptParserTest, ExpectToleranceOneValue) {
  std::string in = R"(
BUFFER orig_buf DATA_TYPE int32 SIZE 100 FILL 11
EXPECT orig_buf IDX 5 TOLERANCE 1 EQ 11)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& commands = script->GetCommands();
  ASSERT_EQ(1U, commands.size());

  auto* cmd = commands[0].get();
  ASSERT_TRUE(cmd->IsProbeSSBO());

  auto* probe = cmd->AsProbeSSBO();
  EXPECT_EQ(ProbeSSBOCommand::Comparator::kFuzzyEqual, probe->GetComparator());
  EXPECT_EQ(5U, probe->GetOffset());
  EXPECT_TRUE(probe->GetFormat()->IsInt32());
  ASSERT_EQ(1U, probe->GetValues().size());
  EXPECT_EQ(11U, probe->GetValues()[0].AsInt32());
  EXPECT_TRUE(probe->HasTolerances());

  auto& tolerances = probe->GetTolerances();
  ASSERT_EQ(1U, tolerances.size());
  EXPECT_FALSE(tolerances[0].is_percent);
  EXPECT_FLOAT_EQ(1.f, static_cast<float>(tolerances[0].value));
}

TEST_F(AmberScriptParserTest, ExpectToleranceOneValuePercent) {
  std::string in = R"(
BUFFER orig_buf DATA_TYPE int32 SIZE 100 FILL 11
EXPECT orig_buf IDX 5 TOLERANCE 1% EQ 11)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& commands = script->GetCommands();
  ASSERT_EQ(1U, commands.size());

  auto* cmd = commands[0].get();
  ASSERT_TRUE(cmd->IsProbeSSBO());

  auto* probe = cmd->AsProbeSSBO();
  EXPECT_EQ(ProbeSSBOCommand::Comparator::kFuzzyEqual, probe->GetComparator());
  EXPECT_EQ(5U, probe->GetOffset());
  EXPECT_TRUE(probe->GetFormat()->IsInt32());
  ASSERT_EQ(1U, probe->GetValues().size());
  EXPECT_EQ(11U, probe->GetValues()[0].AsInt32());
  EXPECT_TRUE(probe->HasTolerances());

  auto& tolerances = probe->GetTolerances();
  ASSERT_EQ(1U, tolerances.size());
  EXPECT_TRUE(tolerances[0].is_percent);
  EXPECT_FLOAT_EQ(1.f, static_cast<float>(tolerances[0].value));
}

TEST_F(AmberScriptParserTest, ExpectToleranceMultiValue) {
  std::string in = R"(
BUFFER orig_buf DATA_TYPE int32 SIZE 100 FILL 11
EXPECT orig_buf IDX 5 TOLERANCE 1% .2 3.7% 4 EQ 11)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& commands = script->GetCommands();
  ASSERT_EQ(1U, commands.size());

  auto* cmd = commands[0].get();
  ASSERT_TRUE(cmd->IsProbeSSBO());

  auto* probe = cmd->AsProbeSSBO();
  EXPECT_EQ(ProbeSSBOCommand::Comparator::kFuzzyEqual, probe->GetComparator());
  EXPECT_EQ(5U, probe->GetOffset());
  EXPECT_TRUE(probe->GetFormat()->IsInt32());
  ASSERT_EQ(1U, probe->GetValues().size());
  EXPECT_EQ(11U, probe->GetValues()[0].AsInt32());

  EXPECT_TRUE(probe->HasTolerances());
  auto& tolerances = probe->GetTolerances();
  ASSERT_EQ(4U, tolerances.size());

  EXPECT_TRUE(tolerances[0].is_percent);
  EXPECT_FLOAT_EQ(1.f, static_cast<float>(tolerances[0].value));

  EXPECT_FALSE(tolerances[1].is_percent);
  EXPECT_FLOAT_EQ(.2f, static_cast<float>(tolerances[1].value));

  EXPECT_TRUE(tolerances[2].is_percent);
  EXPECT_FLOAT_EQ(3.7f, static_cast<float>(tolerances[2].value));

  EXPECT_FALSE(tolerances[3].is_percent);
  EXPECT_FLOAT_EQ(4.f, static_cast<float>(tolerances[3].value));
}

TEST_F(AmberScriptParserTest, ExpectToleranceNoValues) {
  std::string in = R"(
BUFFER orig_buf DATA_TYPE int32 SIZE 100 FILL 11
EXPECT orig_buf IDX 5 TOLERANCE EQ 11)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: TOLERANCE specified but no tolerances provided", r.Error());
}

TEST_F(AmberScriptParserTest, ExpectToleranceTooManyValues) {
  std::string in = R"(
BUFFER orig_buf DATA_TYPE int32 SIZE 100 FILL 11
EXPECT orig_buf IDX 5 TOLERANCE 1 2 3 4 5 EQ 11)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: TOLERANCE has a maximum of 4 values", r.Error());
}

TEST_F(AmberScriptParserTest, ExpectToleranceNonEqCompator) {
  std::string in = R"(
BUFFER orig_buf DATA_TYPE int32 SIZE 100 FILL 11
EXPECT orig_buf IDX 5 TOLERANCE 1 2 3 4 NE 11)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: TOLERANCE only available with EQ probes", r.Error());
}

TEST_F(AmberScriptParserTest, ExpectRMSEBuffer) {
  std::string in = R"(
BUFFER buf_1 DATA_TYPE int32 SIZE 10 FILL 11
BUFFER buf_2 DATA_TYPE int32 SIZE 10 FILL 12
EXPECT buf_1 RMSE_BUFFER buf_2 TOLERANCE 0.1)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& commands = script->GetCommands();
  ASSERT_EQ(1U, commands.size());

  auto* cmd = commands[0].get();
  ASSERT_TRUE(cmd->IsCompareBuffer());

  auto* cmp = cmd->AsCompareBuffer();
  EXPECT_EQ(cmp->GetComparator(), CompareBufferCommand::Comparator::kRmse);
  EXPECT_FLOAT_EQ(cmp->GetTolerance(), 0.1f);

  ASSERT_TRUE(cmp->GetBuffer1() != nullptr);
  EXPECT_EQ(cmp->GetBuffer1()->GetName(), "buf_1");

  ASSERT_TRUE(cmp->GetBuffer2() != nullptr);
  EXPECT_EQ(cmp->GetBuffer2()->GetName(), "buf_2");
}

TEST_F(AmberScriptParserTest, ExpectRMSEBufferMissingFirstBuffer) {
  std::string in = R"(
BUFFER buf_2 DATA_TYPE int32 SIZE 10 FILL 22
EXPECT RMSE_BUFFER buf_2)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: missing buffer name between EXPECT and RMSE_BUFFER", r.Error());
}

TEST_F(AmberScriptParserTest, ExpectRMSEBufferMissingSecondBuffer) {
  std::string in = R"(
BUFFER buf_1 DATA_TYPE int32 SIZE 10 FILL 11
EXPECT buf_1 RMSE_BUFFER)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: invalid buffer name in EXPECT RMSE_BUFFER command", r.Error());
}

TEST_F(AmberScriptParserTest, ExpectRMSEBufferInvalidFirstBuffer) {
  std::string in = R"(EXPECT 123 RMSE_BUFFER)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: invalid buffer name in EXPECT command", r.Error());
}

TEST_F(AmberScriptParserTest, ExpectRMSEBufferUnknownFirstBuffer) {
  std::string in = R"(EXPECT unknown_buffer RMSE_BUFFER)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: unknown buffer name for EXPECT command: unknown_buffer",
            r.Error());
}

TEST_F(AmberScriptParserTest, ExpectRMSEBufferInvalidSecondBuffer) {
  std::string in = R"(
BUFFER buf DATA_TYPE int32 SIZE 10 FILL 11
EXPECT buf RMSE_BUFFER 123)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: invalid buffer name in EXPECT RMSE_BUFFER command", r.Error());
}

TEST_F(AmberScriptParserTest, ExpectRMSEBufferUnknownSecondBuffer) {
  std::string in = R"(
BUFFER buf DATA_TYPE int32 SIZE 10 FILL 11
EXPECT buf RMSE_BUFFER unknown_buffer)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ(
      "3: unknown buffer name for EXPECT RMSE_BUFFER command: unknown_buffer",
      r.Error());
}

TEST_F(AmberScriptParserTest, ExpectRMSEBufferDifferentSize) {
  std::string in = R"(
BUFFER buf_1 DATA_TYPE int32 SIZE 10 FILL 11
BUFFER buf_2 DATA_TYPE int32 SIZE 99 FILL 11
EXPECT buf_1 RMSE_BUFFER buf_2)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ(
      "4: EXPECT RMSE_BUFFER command cannot compare buffers of different size: "
      "10 vs 99",
      r.Error());
}

TEST_F(AmberScriptParserTest, ExpectRMSEBufferDifferentType) {
  std::string in = R"(
BUFFER buf_1 DATA_TYPE int32 SIZE 10 FILL 11
BUFFER buf_2 FORMAT R32G32B32A32_SFLOAT
EXPECT buf_1 RMSE_BUFFER buf_2)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ(
      "4: EXPECT RMSE_BUFFER command cannot compare buffers of differing "
      "format",
      r.Error());
}

}  // namespace amberscript
}  // namespace amber
