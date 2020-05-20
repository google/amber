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

TEST_F(AmberScriptParserTest, DeviceFeature) {
  std::string in = R"(
DEVICE_FEATURE vertexPipelineStoresAndAtomics
DEVICE_FEATURE VariablePointerFeatures.variablePointersStorageBuffer
DEVICE_FEATURE Float16Int8Features.shaderFloat16
DEVICE_FEATURE Float16Int8Features.shaderInt8
DEVICE_FEATURE Storage8BitFeatures.storageBuffer8BitAccess
DEVICE_FEATURE Storage8BitFeatures.uniformAndStorageBuffer8BitAccess
DEVICE_FEATURE Storage8BitFeatures.storagePushConstant8
DEVICE_FEATURE Storage16BitFeatures.storageBuffer16BitAccess
DEVICE_FEATURE Storage16BitFeatures.uniformAndStorageBuffer16BitAccess
DEVICE_FEATURE Storage16BitFeatures.storagePushConstant16
DEVICE_FEATURE Storage16BitFeatures.storageInputOutput16
DEVICE_FEATURE SubgroupSizeControl.subgroupSizeControl
DEVICE_FEATURE SubgroupSizeControl.computeFullSubgroups)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& features = script->GetRequiredFeatures();
  ASSERT_EQ(13U, features.size());
  EXPECT_EQ("vertexPipelineStoresAndAtomics", features[0]);
  EXPECT_EQ("VariablePointerFeatures.variablePointersStorageBuffer",
            features[1]);
  EXPECT_EQ("Float16Int8Features.shaderFloat16", features[2]);
  EXPECT_EQ("Float16Int8Features.shaderInt8", features[3]);
  EXPECT_EQ("Storage8BitFeatures.storageBuffer8BitAccess", features[4]);
  EXPECT_EQ("Storage8BitFeatures.uniformAndStorageBuffer8BitAccess",
            features[5]);
  EXPECT_EQ("Storage8BitFeatures.storagePushConstant8", features[6]);
  EXPECT_EQ("Storage16BitFeatures.storageBuffer16BitAccess", features[7]);
  EXPECT_EQ("Storage16BitFeatures.uniformAndStorageBuffer16BitAccess",
            features[8]);
  EXPECT_EQ("Storage16BitFeatures.storagePushConstant16", features[9]);
  EXPECT_EQ("Storage16BitFeatures.storageInputOutput16", features[10]);
  EXPECT_EQ("SubgroupSizeControl.subgroupSizeControl", features[11]);
  EXPECT_EQ("SubgroupSizeControl.computeFullSubgroups", features[12]);
}

TEST_F(AmberScriptParserTest, DeviceFeatureMissingFeature) {
  std::string in = "DEVICE_FEATURE";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: missing feature name for DEVICE_FEATURE command", r.Error());
}

TEST_F(AmberScriptParserTest, DeviceFeatureUnknown) {
  std::string in = "DEVICE_FEATURE unknown";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: unknown feature name for DEVICE_FEATURE command", r.Error());
}

TEST_F(AmberScriptParserTest, DeviceFeatureInvalid) {
  std::string in = "DEVICE_FEATURE 12345";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: invalid feature name for DEVICE_FEATURE command", r.Error());
}

TEST_F(AmberScriptParserTest, DeviceFeatureExtraParams) {
  std::string in = "DEVICE_FEATURE vertexPipelineStoresAndAtomics EXTRA";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: extra parameters after DEVICE_FEATURE command: EXTRA",
            r.Error());
}

}  // namespace amberscript
}  // namespace amber
