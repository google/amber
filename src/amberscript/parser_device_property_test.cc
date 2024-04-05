// Copyright 2024 The Amber Authors.
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

TEST_F(AmberScriptParserTest, DeviceProperty) {
  std::string in = R"(
DEVICE_PROPERTY FloatControls.shaderSignedZeroInfNanPreserveFloat16
DEVICE_PROPERTY FloatControls.shaderSignedZeroInfNanPreserveFloat32
DEVICE_PROPERTY FloatControls.shaderSignedZeroInfNanPreserveFloat64
DEVICE_PROPERTY FloatControls.shaderDenormPreserveFloat16
DEVICE_PROPERTY FloatControls.shaderDenormPreserveFloat32
DEVICE_PROPERTY FloatControls.shaderDenormPreserveFloat64
DEVICE_PROPERTY FloatControls.shaderDenormFlushToZeroFloat16
DEVICE_PROPERTY FloatControls.shaderDenormFlushToZeroFloat32
DEVICE_PROPERTY FloatControls.shaderDenormFlushToZeroFloat64
DEVICE_PROPERTY FloatControls.shaderRoundingModeRTEFloat16
DEVICE_PROPERTY FloatControls.shaderRoundingModeRTEFloat32
DEVICE_PROPERTY FloatControls.shaderRoundingModeRTEFloat64
DEVICE_PROPERTY FloatControls.shaderRoundingModeRTZFloat16
DEVICE_PROPERTY FloatControls.shaderRoundingModeRTZFloat32
DEVICE_PROPERTY FloatControls.shaderRoundingModeRTZFloat64)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& properties = script->GetRequiredProperties();
  ASSERT_EQ(15U, properties.size());
  EXPECT_EQ("FloatControls.shaderSignedZeroInfNanPreserveFloat16",
            properties[0]);
  EXPECT_EQ("FloatControls.shaderSignedZeroInfNanPreserveFloat32",
            properties[1]);
  EXPECT_EQ("FloatControls.shaderSignedZeroInfNanPreserveFloat64",
            properties[2]);
  EXPECT_EQ("FloatControls.shaderDenormPreserveFloat16", properties[3]);
  EXPECT_EQ("FloatControls.shaderDenormPreserveFloat32", properties[4]);
  EXPECT_EQ("FloatControls.shaderDenormPreserveFloat64", properties[5]);
  EXPECT_EQ("FloatControls.shaderDenormFlushToZeroFloat16", properties[6]);
  EXPECT_EQ("FloatControls.shaderDenormFlushToZeroFloat32", properties[7]);
  EXPECT_EQ("FloatControls.shaderDenormFlushToZeroFloat64", properties[8]);
  EXPECT_EQ("FloatControls.shaderRoundingModeRTEFloat16", properties[9]);
  EXPECT_EQ("FloatControls.shaderRoundingModeRTEFloat32", properties[10]);
  EXPECT_EQ("FloatControls.shaderRoundingModeRTEFloat64", properties[11]);
  EXPECT_EQ("FloatControls.shaderRoundingModeRTZFloat16", properties[12]);
  EXPECT_EQ("FloatControls.shaderRoundingModeRTZFloat32", properties[13]);
  EXPECT_EQ("FloatControls.shaderRoundingModeRTZFloat64", properties[14]);
}

TEST_F(AmberScriptParserTest, DevicePropertyMissingProperty) {
  std::string in = "DEVICE_PROPERTY";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: missing property name for DEVICE_PROPERTY command", r.Error());
}

TEST_F(AmberScriptParserTest, DevicePropertyUnknown) {
  std::string in = "DEVICE_PROPERTY unknown";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: unknown property name for DEVICE_PROPERTY command", r.Error());
}

TEST_F(AmberScriptParserTest, DevicePropertyInvalid) {
  std::string in = "DEVICE_PROPERTY 12345";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: invalid property name for DEVICE_PROPERTY command", r.Error());
}

TEST_F(AmberScriptParserTest, DevicePropertyExtraParams) {
  std::string in =
      "DEVICE_PROPERTY FloatControls.shaderDenormPreserveFloat16 EXTRA";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: extra parameters after DEVICE_PROPERTY command: EXTRA",
            r.Error());
}

}  // namespace amberscript
}  // namespace amber
