// Copyright 2020 The Amber Authors.
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

TEST_F(AmberScriptParserTest,
       SubgroupSizeControlMissingRequiredFeaturecomputeFullSubgroups) {
  std::string in = R"(
SHADER compute test_shader GLSL
# GLSL
END
PIPELINE compute pipeline
  ATTACH test_shader
  SUBGROUP test_shader
    FULLY_POPULATED on
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ(
      "8: missing DEVICE_FEATURE SubgroupSizeControl.computeFullSubgroups",
      r.Error());
}

TEST_F(AmberScriptParserTest,
       SubgroupSizeControlMissingRequiredFeaturesubgroupSizeControl) {
  std::string in = R"(
SHADER compute test_shader GLSL
# GLSL
END
PIPELINE compute pipeline
  ATTACH test_shader
  SUBGROUP test_shader
    VARYING_SIZE on
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("8: missing DEVICE_FEATURE SubgroupSizeControl.subgroupSizeControl",
            r.Error());
}

TEST_F(AmberScriptParserTest,
       SubgroupSizeControlMissingRequiredFeaturesubgroupSizeControl2) {
  std::string in = R"(
SHADER compute test_shader GLSL
# GLSL
END
PIPELINE compute pipeline
  ATTACH test_shader
  SUBGROUP test_shader
    REQUIRED_SIZE 32
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("8: missing DEVICE_FEATURE SubgroupSizeControl.subgroupSizeControl",
            r.Error());
}

TEST_F(AmberScriptParserTest, SubgroupSizeControlNoShader) {
  std::string in = R"(
DEVICE_FEATURE SubgroupSizeControl.computeFullSubgroups
SHADER compute test_shader GLSL
# GLSL
END
PIPELINE compute pipeline
  ATTACH test_shader
  SUBGROUP
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("9: missing shader name in SUBGROUP command", r.Error());
}

TEST_F(AmberScriptParserTest, SubgroupSizeControlUnknownShader) {
  std::string in = R"(
DEVICE_FEATURE SubgroupSizeControl.computeFullSubgroups
SHADER compute test_shader GLSL
# GLSL
END
PIPELINE compute pipeline
  ATTACH test_shader
  SUBGROUP unused
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("8: unknown shader in SUBGROUP command", r.Error());
}

TEST_F(AmberScriptParserTest, SubgroupSizeControlUnknownIdentifier) {
  std::string in = R"(
DEVICE_FEATURE SubgroupSizeControl.subgroupSizeControl
SHADER compute test_shader GLSL
# GLSL
END
PIPELINE compute pipeline
  ATTACH test_shader
  SUBGROUP test_shader
    UNKNOWN_SETTING
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("9: SUBGROUP invalid value for SUBGROUP UNKNOWN_SETTING",
            r.Error());
}

TEST_F(AmberScriptParserTest, SubgroupSizeControlUnknownIdentifier2) {
  std::string in = R"(
DEVICE_FEATURE SubgroupSizeControl.subgroupSizeControl
SHADER compute test_shader GLSL
# GLSL
END
PIPELINE compute pipeline
  ATTACH test_shader
  SUBGROUP test_shader
    REQUIRED_SIZE 2 UNKNOWN_SETTING
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("9: SUBGROUP invalid value for SUBGROUP UNKNOWN_SETTING",
            r.Error());
}

TEST_F(AmberScriptParserTest,
       SubgroupSizeControlRequiredSubgroupSizeInvalidSubgroupSize) {
  std::string in = R"(
DEVICE_FEATURE SubgroupSizeControl.subgroupSizeControl
SHADER compute test_shader GLSL
# GLSL
END
PIPELINE compute pipeline
  ATTACH test_shader
  SUBGROUP test_shader
    REQUIRED_SIZE unused
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("9: invalid size for REQUIRED_SIZE command", r.Error());
}

TEST_F(AmberScriptParserTest,
       SubgroupSizeControlRequiredSubgroupSizeInvalidSubgroupSize2) {
  std::string in = R"(
DEVICE_FEATURE SubgroupSizeControl.subgroupSizeControl
SHADER compute test_shader GLSL
# GLSL
END
PIPELINE compute pipeline
  ATTACH test_shader
  SUBGROUP test_shader
    REQUIRED_SIZE 0
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ(
      "9: invalid required subgroup size 0 specified for shader name "
      "test_shader",
      r.Error());
}

TEST_F(AmberScriptParserTest,
       SubgroupSizeControlRequiredSubgroupSizeInvalidSubgroupSize3) {
  std::string in = R"(
DEVICE_FEATURE SubgroupSizeControl.subgroupSizeControl
SHADER compute test_shader GLSL
# GLSL
END
PIPELINE compute pipeline
  ATTACH test_shader
  SUBGROUP test_shader
    REQUIRED_SIZE 256
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ(
      "9: invalid required subgroup size 256 specified for shader name "
      "test_shader",
      r.Error());
}

TEST_F(AmberScriptParserTest,
       SubgroupSizeControlRequiredSubgroupSizeInvalidSubgroupSize4) {
  std::string in = R"(
DEVICE_FEATURE SubgroupSizeControl.subgroupSizeControl
SHADER compute test_shader GLSL
# GLSL
END
PIPELINE compute pipeline
  ATTACH test_shader
  SUBGROUP test_shader
    REQUIRED_SIZE 7
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ(
      "9: invalid required subgroup size 7 specified for shader name "
      "test_shader",
      r.Error());
}

TEST_F(AmberScriptParserTest,
       SubgroupSizeControlRequireFullSubgroupsInvalidValue) {
  std::string in = R"(
DEVICE_FEATURE SubgroupSizeControl.computeFullSubgroups
SHADER compute test_shader GLSL
# GLSL
END
PIPELINE compute pipeline
  ATTACH test_shader
  SUBGROUP test_shader
    FULLY_POPULATED unused
  END
END)";
  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("9: invalid value for FULLY_POPULATED command", r.Error());
}

TEST_F(AmberScriptParserTest,
       SubgroupSizeControlRequireVaryingSubgroupsInvalidValue) {
  std::string in = R"(
DEVICE_FEATURE SubgroupSizeControl.subgroupSizeControl
SHADER compute test_shader GLSL
# GLSL
END
PIPELINE compute pipeline
  ATTACH test_shader
  SUBGROUP test_shader
    VARYING_SIZE unused
  END
END)";
  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("9: invalid value for VARYING_SIZE command", r.Error());
}

TEST_F(AmberScriptParserTest, SubgroupSizeControlRequireFullSubgroupsSet) {
  std::string in = R"(
DEVICE_FEATURE SubgroupSizeControl.computeFullSubgroups
SHADER compute test_shader GLSL
# GLSL
END
PIPELINE compute pipeline
  ATTACH test_shader
  SUBGROUP test_shader
    FULLY_POPULATED on
  END
END)";
  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  auto script = parser.GetScript();
  const auto& pipelines = script->GetPipelines();
  ASSERT_EQ(1U, pipelines.size());

  const auto* pipeline = pipelines[0].get();
  const auto& shaders = pipeline->GetShaders();
  ASSERT_EQ(1U, shaders.size());
  ASSERT_TRUE(shaders[0].GetShader() != nullptr);
  ASSERT_TRUE(shaders[0].GetRequireFullSubgroups() == true);
}

TEST_F(AmberScriptParserTest, SubgroupSizeControlVaryingSubgroupsSet) {
  std::string in = R"(
DEVICE_FEATURE SubgroupSizeControl.subgroupSizeControl
SHADER compute test_shader GLSL
# GLSL
END
PIPELINE compute pipeline
  ATTACH test_shader
  SUBGROUP test_shader
    VARYING_SIZE on
  END
END)";
  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  auto script = parser.GetScript();
  const auto& pipelines = script->GetPipelines();
  ASSERT_EQ(1U, pipelines.size());

  const auto* pipeline = pipelines[0].get();
  const auto& shaders = pipeline->GetShaders();
  ASSERT_EQ(1U, shaders.size());
  ASSERT_TRUE(shaders[0].GetShader() != nullptr);
  ASSERT_TRUE(shaders[0].GetVaryingSubgroupSize() == true);
}

TEST_F(AmberScriptParserTest, SubgroupSizeControlRequiredSubgroupSizeSetTo8) {
  std::string in = R"(
DEVICE_FEATURE SubgroupSizeControl.subgroupSizeControl
SHADER compute test_shader GLSL
# GLSL
END
PIPELINE compute pipeline
  ATTACH test_shader
  SUBGROUP test_shader
    REQUIRED_SIZE 8
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  auto script = parser.GetScript();
  const auto& pipelines = script->GetPipelines();
  ASSERT_EQ(1U, pipelines.size());

  const auto* pipeline = pipelines[0].get();
  const auto& shaders = pipeline->GetShaders();
  ASSERT_EQ(1U, shaders.size());
  ASSERT_TRUE(shaders[0].GetShader() != nullptr);
  ASSERT_TRUE(shaders[0].GetRequiredSubgroupSizeSetting() ==
              amber::Pipeline::ShaderInfo::RequiredSubgroupSizeSetting::
                  kSetToSpecificSize);
  ASSERT_EQ(8U, shaders[0].GetRequiredSubgroupSize());
}

TEST_F(AmberScriptParserTest, SubgroupSizeControlRequiredSubgroupSizeSetToMax) {
  std::string in = R"(
DEVICE_FEATURE SubgroupSizeControl.subgroupSizeControl
SHADER compute test_shader GLSL
# GLSL
END
PIPELINE compute pipeline
  ATTACH test_shader
  SUBGROUP test_shader
    REQUIRED_SIZE MAX
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  auto script = parser.GetScript();
  const auto& pipelines = script->GetPipelines();
  ASSERT_EQ(1U, pipelines.size());

  const auto* pipeline = pipelines[0].get();
  const auto& shaders = pipeline->GetShaders();
  ASSERT_EQ(1U, shaders.size());
  ASSERT_TRUE(shaders[0].GetShader() != nullptr);
  ASSERT_TRUE(shaders[0].GetRequiredSubgroupSizeSetting() ==
              amber::Pipeline::ShaderInfo::RequiredSubgroupSizeSetting::
                  kSetToMaximumSize);
}

TEST_F(AmberScriptParserTest, SubgroupSizeControlRequiredSubgroupSizeSetToMin) {
  std::string in = R"(
DEVICE_FEATURE SubgroupSizeControl.subgroupSizeControl
SHADER compute test_shader GLSL
# GLSL
END
PIPELINE compute pipeline
  ATTACH test_shader
  SUBGROUP test_shader
    REQUIRED_SIZE MIN
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  auto script = parser.GetScript();
  const auto& pipelines = script->GetPipelines();
  ASSERT_EQ(1U, pipelines.size());

  const auto* pipeline = pipelines[0].get();
  const auto& shaders = pipeline->GetShaders();
  ASSERT_EQ(1U, shaders.size());
  ASSERT_TRUE(shaders[0].GetShader() != nullptr);
  ASSERT_TRUE(shaders[0].GetRequiredSubgroupSizeSetting() ==
              amber::Pipeline::ShaderInfo::RequiredSubgroupSizeSetting::
                  kSetToMinimumSize);
}

TEST_F(AmberScriptParserTest,
       SubgroupSizeControlRequireFullAndVaryingSubgroups) {
  std::string in = R"(
DEVICE_FEATURE SubgroupSizeControl.computeFullSubgroups
DEVICE_FEATURE SubgroupSizeControl.subgroupSizeControl
SHADER compute test_shader GLSL
# GLSL
END
PIPELINE compute pipeline
  ATTACH test_shader
  SUBGROUP test_shader
    FULLY_POPULATED on
    VARYING_SIZE on
  END
END)";
  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  auto script = parser.GetScript();
  const auto& pipelines = script->GetPipelines();
  ASSERT_EQ(1U, pipelines.size());

  const auto* pipeline = pipelines[0].get();
  const auto& shaders = pipeline->GetShaders();
  ASSERT_EQ(1U, shaders.size());
  ASSERT_TRUE(shaders[0].GetShader() != nullptr);
  ASSERT_TRUE(shaders[0].GetRequireFullSubgroups() == true);
  ASSERT_TRUE(shaders[0].GetVaryingSubgroupSize() == true);
}

TEST_F(AmberScriptParserTest, SubgroupSizeControlRequireFullAndMinSubgroups) {
  std::string in = R"(
DEVICE_FEATURE SubgroupSizeControl.computeFullSubgroups
DEVICE_FEATURE SubgroupSizeControl.subgroupSizeControl
SHADER compute test_shader GLSL
# GLSL
END
PIPELINE compute pipeline
  ATTACH test_shader
  SUBGROUP test_shader
    FULLY_POPULATED on
    REQUIRED_SIZE MIN
  END
END)";
  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();
  auto script = parser.GetScript();
  const auto& pipelines = script->GetPipelines();
  ASSERT_EQ(1U, pipelines.size());

  const auto* pipeline = pipelines[0].get();
  const auto& shaders = pipeline->GetShaders();
  ASSERT_EQ(1U, shaders.size());
  ASSERT_TRUE(shaders[0].GetShader() != nullptr);
  ASSERT_TRUE(shaders[0].GetRequireFullSubgroups() == true);
  ASSERT_TRUE(shaders[0].GetRequiredSubgroupSizeSetting() ==
              amber::Pipeline::ShaderInfo::RequiredSubgroupSizeSetting::
                  kSetToMinimumSize);
}

}  // namespace amberscript
}  // namespace amber
