// Copyright 2018 The Amber Authors.
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

#include "src/command_data.h"
#include <map>

namespace amber {

Topology NameToTopology(const std::string& name) {
  static const struct {
    const char* name;
    Topology val;
  } topologies[] = {
      {"PATCH_LIST", Topology::kPatchList},
      {"POINT_LIST", Topology::kPointList},
      {"GL_LINE_STRIP_ADJACENCY", Topology::kLineStripWithAdjacency},
      {"GL_LINE_STRIP", Topology::kLineStrip},
      {"GL_LINES", Topology::kLineList},
      {"GL_LINES_ADJACENCY", Topology::kLineListWithAdjacency},
      {"GL_PATCHES", Topology::kPatchList},
      {"GL_POINTS", Topology::kPointList},
      {"GL_TRIANGLE_STRIP", Topology::kTriangleStrip},
      {"GL_TRIANGLE_FAN", Topology::kTriangleFan},
      {"GL_TRIANGLES", Topology::kTriangleList},
      {"GL_TRIANGLES_ADJACENCY", Topology::kTriangleListWithAdjacency},
      {"GL_TRIANGLE_STRIP_ADJACENCY", Topology::kTriangleStripWithAdjacency},
      {"LINE_LIST", Topology::kLineList},
      {"LINE_LIST_WITH_ADJACENCY", Topology::kLineListWithAdjacency},
      {"LINE_STRIP", Topology::kLineStrip},
      {"LINE_STRIP_WITH_ADJACENCY", Topology::kLineStripWithAdjacency},
      {"TRIANGLE_FAN", Topology::kTriangleFan},
      {"TRIANGLE_LIST", Topology::kTriangleList},
      {"TRIANGLE_LIST_WITH_ADJACENCY", Topology::kTriangleListWithAdjacency},
      {"TRIANGLE_STRIP", Topology::kTriangleStrip},
      {"TRIANGLE_STRIP_WITH_ADJACENCY", Topology::kTriangleStripWithAdjacency},
  };

  // TODO(dsinclair): Make smarter if needed
  for (auto& topo : topologies) {
    if (topo.name == name)
      return topo.val;
  }

  return Topology::kUnknown;
}

BlendFactor NameToBlendFactor(const std::string& name) {
  static const std::map<const std::string, BlendFactor> map = {
      {"zero", BlendFactor::kZero},
      {"one", BlendFactor::kOne},
      {"src_color", BlendFactor::kSrcColor},
      {"one_minus_src_color", BlendFactor::kOneMinusSrcColor},
      {"dst_color", BlendFactor::kDstColor},
      {"one_minus_dst_color", BlendFactor::kOneMinusDstColor},
      {"src_alpha", BlendFactor::kSrcAlpha},
      {"one_minus_src_alpha", BlendFactor::kOneMinusSrcAlpha},
      {"dst_alpha", BlendFactor::kDstAlpha},
      {"one_minus_dst_alpha", BlendFactor::kOneMinusDstAlpha},
      {"constant_color", BlendFactor::kConstantColor},
      {"one_minus_constant_color", BlendFactor::kOneMinusConstantColor},
      {"costant_alpha", BlendFactor::kConstantAlpha},
      {"one_minus_constant_alpha", BlendFactor::kOneMinusConstantAlpha},
      {"src_alpha_saturate", BlendFactor::kSrcAlphaSaturate},
      {"src1_color", BlendFactor::kSrc1Color},
      {"one_minus_src1_color", BlendFactor::kOneMinusSrc1Color},
      {"src1_alpha", BlendFactor::kSrc1Alpha},
      {"one_minus_src1_alpha", BlendFactor::kOneMinusSrc1Alpha}};

  auto item = map.find(name);
  if (item != map.end())
    return item->second;

  return BlendFactor::kUnknown;
}

BlendOp NameToBlendOp(const std::string& name) {
  static const std::map<const std::string, BlendOp> map = {
      {"add", BlendOp::kAdd},
      {"substract", BlendOp::kSubtract},
      {"reverse_substract", BlendOp::kReverseSubtract},
      {"min", BlendOp::kMin},
      {"max", BlendOp::kMax},
      {"zero", BlendOp::kZero},
      {"src", BlendOp::kSrc},
      {"dst", BlendOp::kDst},
      {"src_over", BlendOp::kSrcOver},
      {"dst_over", BlendOp::kDstOver},
      {"src_in", BlendOp::kSrcIn},
      {"dst_in", BlendOp::kDstIn},
      {"src_out", BlendOp::kSrcOut},
      {"dst_out", BlendOp::kDstOut},
      {"src_atop", BlendOp::kSrcAtop},
      {"dst_atop", BlendOp::kDstAtop},
      {"xor", BlendOp::kXor},
      {"multiply", BlendOp::kMultiply},
      {"screen", BlendOp::kScreen},
      {"overlay", BlendOp::kOverlay},
      {"darken", BlendOp::kDarken},
      {"lighten", BlendOp::kLighten},
      {"color_dodge", BlendOp::kColorDodge},
      {"color_burn", BlendOp::kColorBurn},
      {"hard_light", BlendOp::kHardLight},
      {"soft_light", BlendOp::kSoftLight},
      {"difference", BlendOp::kDifference},
      {"exclusion", BlendOp::kExclusion},
      {"invert", BlendOp::kInvert},
      {"invert_rgb", BlendOp::kInvertRGB},
      {"linear_dodge", BlendOp::kLinearDodge},
      {"linear_burn", BlendOp::kLinearBurn},
      {"vivid_light", BlendOp::kVividLight},
      {"linear_light", BlendOp::kLinearLight},
      {"pin_light", BlendOp::kPinLight},
      {"hard_mix", BlendOp::kHardMix},
      {"hsl_hue", BlendOp::kHslHue},
      {"hsl_saturation", BlendOp::kHslSaturation},
      {"hsl_color", BlendOp::kHslColor},
      {"hsl_luminosity", BlendOp::kHslLuminosity},
      {"plus", BlendOp::kPlus},
      {"plus_clamped", BlendOp::kPlusClamped},
      {"plus_clamped_alpha", BlendOp::kPlusClampedAlpha},
      {"plus_darker", BlendOp::kPlusDarker},
      {"minus", BlendOp::kMinus},
      {"minus_clamped", BlendOp::kMinusClamped},
      {"contrast", BlendOp::kContrast},
      {"invert_ovg", BlendOp::kInvertOvg},
      {"red", BlendOp::kRed},
      {"green", BlendOp::kGreen},
      {"blue", BlendOp::kBlue}};

  auto item = map.find(name);
  if (item != map.end())
    return item->second;

  return BlendOp::kUnknown;
}

}  // namespace amber
