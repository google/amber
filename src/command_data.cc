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
  if (name == "zero")
    return BlendFactor::kZero;
  else if (name == "one")
    return BlendFactor::kOne;
  else if (name == "src_color")
    return BlendFactor::kSrcColor;
  else if (name == "one_minus_src_color")
    return BlendFactor::kOneMinusSrcColor;
  else if (name == "dst_color")
    return BlendFactor::kDstColor;
  else if (name == "one_minus_dst_color")
    return BlendFactor::kOneMinusDstColor;
  else if (name == "src_alpha")
    return BlendFactor::kSrcAlpha;
  else if (name == "one_minus_src_alpha")
    return BlendFactor::kOneMinusSrcAlpha;
  else if (name == "dst_alpha")
    return BlendFactor::kDstAlpha;
  else if (name == "one_minus_dst_alpha")
    return BlendFactor::kOneMinusDstAlpha;
  else if (name == "constant_color")
    return BlendFactor::kConstantColor;
  else if (name == "one_minus_constant_color")
    return BlendFactor::kOneMinusConstantColor;
  else if (name == "costant_alpha")
    return BlendFactor::kConstantAlpha;
  else if (name == "one_minus_constant_alpha")
    return BlendFactor::kOneMinusConstantAlpha;
  else if (name == "src_alpha_saturate")
    return BlendFactor::kSrcAlphaSaturate;
  else if (name == "src1_color")
    return BlendFactor::kSrc1Color;
  else if (name == "one_minus_src1_color")
    return BlendFactor::kOneMinusSrc1Color;
  else if (name == "src1_alpha")
    return BlendFactor::kSrc1Alpha;
  else if (name == "one_minus_src1_alpha")
    return BlendFactor::kOneMinusSrc1Alpha;
  else
    return BlendFactor::kUnknown;
}

BlendOp NameToBlendOp(const std::string& name) {
  if (name == "add")
    return BlendOp::kAdd;
  else if (name == "substract")
    return BlendOp::kSubtract;
  else if (name == "reverse_substract")
    return BlendOp::kReverseSubtract;
  else if (name == "min")
    return BlendOp::kMin;
  else if (name == "max")
    return BlendOp::kMax;
  else if (name == "zero")
    return BlendOp::kZero;
  else if (name == "src")
    return BlendOp::kSrc;
  else if (name == "dst")
    return BlendOp::kDst;
  else if (name == "src_over")
    return BlendOp::kSrcOver;
  else if (name == "dst_over")
    return BlendOp::kDstOver;
  else if (name == "src_in")
    return BlendOp::kSrcIn;
  else if (name == "dst_in")
    return BlendOp::kDstIn;
  else if (name == "src_out")
    return BlendOp::kSrcOut;
  else if (name == "dst_out")
    return BlendOp::kDstOut;
  else if (name == "src_atop")
    return BlendOp::kSrcAtop;
  else if (name == "dst_atop")
    return BlendOp::kDstAtop;
  else if (name == "xor")
    return BlendOp::kXor;
  else if (name == "multiply")
    return BlendOp::kMultiply;
  else if (name == "screen")
    return BlendOp::kScreen;
  else if (name == "overlay")
    return BlendOp::kOverlay;
  else if (name == "darken")
    return BlendOp::kDarken;
  else if (name == "lighten")
    return BlendOp::kLighten;
  else if (name == "color_dodge")
    return BlendOp::kColorDodge;
  else if (name == "color_burn")
    return BlendOp::kColorBurn;
  else if (name == "hard_light")
    return BlendOp::kHardLight;
  else if (name == "soft_light")
    return BlendOp::kSoftLight;
  else if (name == "difference")
    return BlendOp::kDifference;
  else if (name == "exclusion")
    return BlendOp::kExclusion;
  else if (name == "invert")
    return BlendOp::kInvert;
  else if (name == "invert_rgb")
    return BlendOp::kInvertRGB;
  else if (name == "linear_dodge")
    return BlendOp::kLinearDodge;
  else if (name == "linear_burn")
    return BlendOp::kLinearBurn;
  else if (name == "vivid_light")
    return BlendOp::kVividLight;
  else if (name == "linear_light")
    return BlendOp::kLinearLight;
  else if (name == "pin_light")
    return BlendOp::kPinLight;
  else if (name == "hard_mix")
    return BlendOp::kHardMix;
  else if (name == "hsl_hue")
    return BlendOp::kHslHue;
  else if (name == "hsl_saturation")
    return BlendOp::kHslSaturation;
  else if (name == "hsl_color")
    return BlendOp::kHslColor;
  else if (name == "hsl_luminosity")
    return BlendOp::kHslLuminosity;
  else if (name == "plus")
    return BlendOp::kPlus;
  else if (name == "plus_clamped")
    return BlendOp::kPlusClamped;
  else if (name == "plus_clamped_alpha")
    return BlendOp::kPlusClampedAlpha;
  else if (name == "plus_darker")
    return BlendOp::kPlusDarker;
  else if (name == "minus")
    return BlendOp::kMinus;
  else if (name == "minus_clamped")
    return BlendOp::kMinusClamped;
  else if (name == "contrast")
    return BlendOp::kContrast;
  else if (name == "invert_ovg")
    return BlendOp::kInvertOvg;
  else if (name == "red")
    return BlendOp::kRed;
  else if (name == "green")
    return BlendOp::kGreen;
  else if (name == "blue")
    return BlendOp::kBlue;
  else
    return BlendOp::kUnknown;
}

}  // namespace amber
