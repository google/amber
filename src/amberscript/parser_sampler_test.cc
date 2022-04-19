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

TEST_F(AmberScriptParserTest, SamplerDefaultValues) {
  std::string in = "SAMPLER sampler";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& samplers = script->GetSamplers();
  ASSERT_EQ(1U, samplers.size());

  ASSERT_TRUE(samplers[0] != nullptr);
  EXPECT_EQ("sampler", samplers[0]->GetName());

  auto* sampler = samplers[0].get();
  EXPECT_EQ(FilterType::kNearest, sampler->GetMagFilter());
  EXPECT_EQ(FilterType::kNearest, sampler->GetMinFilter());
  EXPECT_EQ(FilterType::kNearest, sampler->GetMipmapMode());
  EXPECT_EQ(AddressMode::kRepeat, sampler->GetAddressModeU());
  EXPECT_EQ(AddressMode::kRepeat, sampler->GetAddressModeV());
  EXPECT_EQ(AddressMode::kRepeat, sampler->GetAddressModeW());
  EXPECT_EQ(BorderColor::kFloatTransparentBlack, sampler->GetBorderColor());
  EXPECT_EQ(0.0, sampler->GetMinLOD());
  EXPECT_EQ(1.0, sampler->GetMaxLOD());
  EXPECT_EQ(true, sampler->GetNormalizedCoords());
  EXPECT_EQ(false, sampler->GetCompareEnable());
  EXPECT_EQ(CompareOp::kNever, sampler->GetCompareOp());
}

TEST_F(AmberScriptParserTest, SamplerCustomValues) {
  std::string in = R"(
SAMPLER sampler MAG_FILTER linear \
  MIN_FILTER linear \
  ADDRESS_MODE_U clamp_to_edge \
  ADDRESS_MODE_V clamp_to_border \
  ADDRESS_MODE_W mirrored_repeat \
  BORDER_COLOR float_opaque_white \
  MIN_LOD 2.5 \
  MAX_LOD 5.0 \
  NORMALIZED_COORDS \
  COMPARE on \
  COMPARE_OP greater)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& samplers = script->GetSamplers();
  ASSERT_EQ(1U, samplers.size());

  ASSERT_TRUE(samplers[0] != nullptr);
  EXPECT_EQ("sampler", samplers[0]->GetName());

  auto* sampler = samplers[0].get();
  EXPECT_EQ(FilterType::kLinear, sampler->GetMagFilter());
  EXPECT_EQ(FilterType::kLinear, sampler->GetMinFilter());
  EXPECT_EQ(FilterType::kNearest, sampler->GetMipmapMode());
  EXPECT_EQ(AddressMode::kClampToEdge, sampler->GetAddressModeU());
  EXPECT_EQ(AddressMode::kClampToBorder, sampler->GetAddressModeV());
  EXPECT_EQ(AddressMode::kMirroredRepeat, sampler->GetAddressModeW());
  EXPECT_EQ(BorderColor::kFloatOpaqueWhite, sampler->GetBorderColor());
  EXPECT_EQ(2.5, sampler->GetMinLOD());
  EXPECT_EQ(5.0, sampler->GetMaxLOD());
  EXPECT_EQ(true, sampler->GetNormalizedCoords());
  EXPECT_EQ(true, sampler->GetCompareEnable());
  EXPECT_EQ(CompareOp::kGreater, sampler->GetCompareOp());
}

TEST_F(AmberScriptParserTest, SamplerUnexpectedParameter) {
  std::string in = R"(
SAMPLER sampler MAG_FILTER linear \
  FOO \
  ADDRESS_MODE_U clamp_to_edge)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("3: unexpected sampler parameter FOO", r.Error());
}

TEST_F(AmberScriptParserTest, SamplerInvalidMagFilter) {
  std::string in = "SAMPLER sampler MAG_FILTER foo";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: invalid MAG_FILTER value foo", r.Error());
}

TEST_F(AmberScriptParserTest, SamplerInvalidMinFilter) {
  std::string in = "SAMPLER sampler MIN_FILTER foo";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: invalid MIN_FILTER value foo", r.Error());
}

TEST_F(AmberScriptParserTest, SamplerInvalidAddressModeU) {
  std::string in = "SAMPLER sampler ADDRESS_MODE_U foo";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: invalid ADDRESS_MODE_U value foo", r.Error());
}

TEST_F(AmberScriptParserTest, SamplerInvalidAddressModeV) {
  std::string in = "SAMPLER sampler ADDRESS_MODE_V foo";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: invalid ADDRESS_MODE_V value foo", r.Error());
}

TEST_F(AmberScriptParserTest, SamplerInvalidBorderColor) {
  std::string in = "SAMPLER sampler BORDER_COLOR foo";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: invalid BORDER_COLOR value foo", r.Error());
}

TEST_F(AmberScriptParserTest, SamplerInvalidMinLod) {
  std::string in = "SAMPLER sampler MIN_LOD foo";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: invalid token when looking for MIN_LOD value", r.Error());
}

TEST_F(AmberScriptParserTest, SamplerInvalidMaxLod) {
  std::string in = "SAMPLER sampler MAX_LOD foo";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: invalid token when looking for MAX_LOD value", r.Error());
}

TEST_F(AmberScriptParserTest, SamplerMaxLodSmallerThanMinLod) {
  std::string in = "SAMPLER sampler MIN_LOD 2.0 MAX_LOD 1.0";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("1: max LOD needs to be greater than or equal to min LOD",
            r.Error());
}

TEST_F(AmberScriptParserTest, SamplerUnnormalizedCoordsSetsLod) {
  std::string in = R"(
SAMPLER sampler \
  MIN_LOD 2.0 \
  MAX_LOD 3.0 \
  UNNORMALIZED_COORDS
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess());
  auto script = parser.GetScript();
  const auto& samplers = script->GetSamplers();
  ASSERT_EQ(1U, samplers.size());

  ASSERT_TRUE(samplers[0] != nullptr);
  EXPECT_EQ("sampler", samplers[0]->GetName());

  auto* sampler = samplers[0].get();
  EXPECT_EQ(0.0f, sampler->GetMinLOD());
  EXPECT_EQ(0.0f, sampler->GetMaxLOD());
}

}  // namespace amberscript
}  // namespace amber
