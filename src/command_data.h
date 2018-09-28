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

#ifndef SRC_COMMAND_DATA_H_
#define SRC_COMMAND_DATA_H_

#include <cstdint>
#include <string>

namespace amber {

enum class Topology : uint8_t {
  kUnknown = 0,
  kPointList,
  kLineList,
  kLineStrip,
  kTriangleList,
  kTriangleStrip,
  kTriangleFan,
  kLineListWithAdjacency,
  kLineStripWithAdjacency,
  kTriangleListWithAdjacency,
  kTriangleStripWithAdjacency,
  kPatchList,
};

enum class PolygonMode : uint8_t {
  kFill = 0,
  kLine,
  kPoint,
};

enum class CullMode : uint8_t {
  kNone = 0,
  kFront,
  kBack,
  kFrontAndBack,
};

enum class FrontFace : uint8_t {
  kCounterClockwise = 0,
  kClockwise,
};

enum ColorMask {
  kColorMaskR = 1 << 0,
  kColorMaskG = 1 << 1,
  kColorMaskB = 1 << 2,
  kColorMaskA = 1 << 3,
};

enum class CompareOp : uint8_t {
  kNever = 0,
  kLess,
  kEqual,
  kLessOrEqual,
  kGreater,
  kNotEqual,
  kGreaterOrEqual,
  kAlways,
};

enum class StencilOp : uint8_t {
  kKeep = 0,
  kZero,
  kReplace,
  kIncrementAndClamp,
  kDecrementAndClamp,
  kInvert,
  kIncrementAndWrap,
  kDecrementAndWrap,
};

enum class LogicOp : uint8_t {
  kClear = 0,
  kAnd,
  kAndReverse,
  kCopy,
  kAndInverted,
  kNoOp,
  kXor,
  kOr,
  kNor,
  kEquivalent,
  kInvert,
  kOrReverse,
  kCopyInverted,
  kOrInverted,
  kNand,
  kSet,
};

enum class BlendOp : uint8_t {
  kAdd = 0,
  kSubtract,
  kReverseSubtract,
  kMin,
  kMax,
  kZero,
  kSrc,
  kDst,
  kSrcOver,
  kDstOver,
  kSrcIn,
  kDstIn,
  kSrcOut,
  kDstOut,
  kSrcAtop,
  kDstAtop,
  kXor,
  kMultiply,
  kScreen,
  kOverlay,
  kDarken,
  kLighten,
  kColorDodge,
  kColorBurn,
  kHardLight,
  kSoftLight,
  kDifference,
  kExclusion,
  kInvert,
  kInvertRGB,
  kLinearDodge,
  kLinearBurn,
  kVividLight,
  kLinearLight,
  kPinLight,
  kHardMix,
  kHslHue,
  kHslSaturation,
  kHslColor,
  kHslLuminosity,
  kPlus,
  kPlusClamped,
  kPlusClampedAlpha,
  kPlusDarker,
  kMinus,
  kMinusClamped,
  kContrast,
  kInvertOvg,
  kRed,
  kGreen,
  kBlue,
};

enum class BlendFactor : uint8_t {
  kZero = 0,
  kOne,
  kSrcColor,
  kOneMinusSrcColor,
  kDstColor,
  kOneMinusDstColor,
  kSrcAlpha,
  kOneMinusSrcAlpha,
  kDstAlpha,
  kOneMinusDstAlpha,
  kConstantColor,
  kOneMinusConstantColor,
  kConstantAlpha,
  kOneMinusConstantAlpha,
  kSrcAlphaSaturate,
  kSrc1Color,
  kOneMinusSrc1Color,
  kSrc1Alpha,
  kOneMinusSrc1Alpha,
};

Topology NameToTopology(const std::string& name);

}  // namespace amber

#endif  // SRC_COMMAND_DATA_H_
