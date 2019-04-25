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
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "gtest/gtest.h"
#include "src/amberscript/parser.h"

namespace amber {
namespace amberscript {

using AmberScriptParserTest = testing::Test;

TEST_F(AmberScriptParserTest, ExtensionInstance) {
  std::string in = "INSTANCE_EXTENSION VK_KHR_storage_buffer_storage_class";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  auto ext = script->GetRequiredInstanceExtensions();
  ASSERT_EQ(1U, ext.size());
  EXPECT_EQ("VK_KHR_storage_buffer_storage_class", ext[0]);
}

TEST_F(AmberScriptParserTest, ExtensionInstanceMulti) {
  std::string in = R"(
INSTANCE_EXTENSION VK_KHR_storage_buffer_storage_class
INSTANCE_EXTENSION VK_KHR_variable_pointers)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  auto ext = script->GetRequiredInstanceExtensions();
  ASSERT_EQ(2U, ext.size());
  EXPECT_EQ("VK_KHR_storage_buffer_storage_class", ext[0]);
  EXPECT_EQ("VK_KHR_variable_pointers", ext[1]);
}

TEST_F(AmberScriptParserTest, ExtensionInstanceMissingName) {
  std::string in = "INSTANCE_EXTENSION";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());

  EXPECT_EQ("1: INSTANCE_EXTENSION missing name", r.Error());
}

TEST_F(AmberScriptParserTest, ExtensionInstanceInvalidName) {
  std::string in = "INSTANCE_EXTENSION 1234";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());

  EXPECT_EQ("1: INSTANCE_EXTENSION invalid name: 1234", r.Error());
}

TEST_F(AmberScriptParserTest, ExtensionInstanceExtraParams) {
  std::string in = "INSTANCE_EXTENSION VK_KHR_variable_pointers EXTRA";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());

  EXPECT_EQ("1: extra parameters after INSTANCE_EXTENSION command", r.Error());
}

TEST_F(AmberScriptParserTest, ExtensionDevice) {
  std::string in = "DEVICE_EXTENSION VK_KHR_get_physical_device_properties2";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  auto ext = script->GetRequiredDeviceExtensions();
  ASSERT_EQ(1U, ext.size());
  EXPECT_EQ("VK_KHR_get_physical_device_properties2", ext[0]);
}

TEST_F(AmberScriptParserTest, ExtensionDeviceMulti) {
  std::string in = R"(
DEVICE_EXTENSION VK_KHR_get_physical_device_properties2
DEVICE_EXTENSION VK_KHR_external_memory)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  auto ext = script->GetRequiredDeviceExtensions();
  ASSERT_EQ(2U, ext.size());
  EXPECT_EQ("VK_KHR_get_physical_device_properties2", ext[0]);
  EXPECT_EQ("VK_KHR_external_memory", ext[1]);
}

TEST_F(AmberScriptParserTest, ExtensionDeviceMissingName) {
  std::string in = "DEVICE_EXTENSION";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: DEVICE_EXTENSION missing name", r.Error());
}

TEST_F(AmberScriptParserTest, ExtensionDeviceInvalidName) {
  std::string in = "DEVICE_EXTENSION 1234";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: DEVICE_EXTENSION invalid name: 1234", r.Error());
}

TEST_F(AmberScriptParserTest, ExtensionDeviceExtraParams) {
  std::string in = "DEVICE_EXTENSION VK_KHR_external_memory EXTRA";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: extra parameters after DEVICE_EXTENSION command", r.Error());
}

}  // namespace amberscript
}  // namespace amber
