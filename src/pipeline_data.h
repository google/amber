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

#ifndef SRC_PIPELINE_DATA_H_
#define SRC_PIPELINE_DATA_H_

#include <limits>

#include "src/command_data.h"

namespace amber {

/// Stores information used to configure a pipeline.
class PipelineData {
 public:
  PipelineData();
  ~PipelineData();
  PipelineData(const PipelineData&);

  void SetTopology(Topology topo) { topology_ = topo; }
  Topology GetTopology() const { return topology_; }

  void SetPolygonMode(PolygonMode mode) { polygon_mode_ = mode; }
  PolygonMode GetPolygonMode() const { return polygon_mode_; }

  void SetCullMode(CullMode mode) { cull_mode_ = mode; }
  CullMode GetCullMode() const { return cull_mode_; }

  void SetFrontFace(FrontFace face) { front_face_ = face; }
  FrontFace GetFrontFace() const { return front_face_; }

  void SetDepthCompareOp(CompareOp op) { depth_compare_op_ = op; }
  CompareOp GetDepthCompareOp() const { return depth_compare_op_; }

  void SetColorWriteMask(uint8_t mask) { color_write_mask_ = mask; }
  uint8_t GetColorWriteMask() const { return color_write_mask_; }

  void SetFrontFailOp(StencilOp op) { front_fail_op_ = op; }
  StencilOp GetFrontFailOp() const { return front_fail_op_; }

  void SetFrontPassOp(StencilOp op) { front_pass_op_ = op; }
  StencilOp GetFrontPassOp() const { return front_pass_op_; }

  void SetFrontDepthFailOp(StencilOp op) { front_depth_fail_op_ = op; }
  StencilOp GetFrontDepthFailOp() const { return front_depth_fail_op_; }

  void SetFrontCompareOp(CompareOp op) { front_compare_op_ = op; }
  CompareOp GetFrontCompareOp() const { return front_compare_op_; }

  void SetFrontCompareMask(uint32_t mask) { front_compare_mask_ = mask; }
  uint32_t GetFrontCompareMask() const { return front_compare_mask_; }

  void SetFrontWriteMask(uint32_t mask) { front_write_mask_ = mask; }
  uint32_t GetFrontWriteMask() const { return front_write_mask_; }

  void SetFrontReference(uint32_t ref) { front_reference_ = ref; }
  uint32_t GetFrontReference() const { return front_reference_; }

  void SetBackFailOp(StencilOp op) { back_fail_op_ = op; }
  StencilOp GetBackFailOp() const { return back_fail_op_; }

  void SetBackPassOp(StencilOp op) { back_pass_op_ = op; }
  StencilOp GetBackPassOp() const { return back_pass_op_; }

  void SetBackDepthFailOp(StencilOp op) { back_depth_fail_op_ = op; }
  StencilOp GetBackDepthFailOp() const { return back_depth_fail_op_; }

  void SetBackCompareOp(CompareOp op) { back_compare_op_ = op; }
  CompareOp GetBackCompareOp() const { return back_compare_op_; }

  void SetBackCompareMask(uint32_t mask) { back_compare_mask_ = mask; }
  uint32_t GetBackCompareMask() const { return back_compare_mask_; }

  void SetBackWriteMask(uint32_t mask) { back_write_mask_ = mask; }
  uint32_t GetBackWriteMask() const { return back_write_mask_; }

  void SetBackReference(uint32_t ref) { back_reference_ = ref; }
  uint32_t GetBackReference() const { return back_reference_; }

  void SetLineWidth(float width) { line_width_ = width; }
  float GetLineWidth() const { return line_width_; }

  void SetEnableBlend(bool v) { enable_blend_ = v; }
  bool GetEnableBlend() const { return enable_blend_; }

  void SetEnableDepthTest(bool v) { enable_depth_test_ = v; }
  bool GetEnableDepthTest() const { return enable_depth_test_; }

  void SetEnableDepthWrite(bool v) { enable_depth_write_ = v; }
  bool GetEnableDepthWrite() const { return enable_depth_write_; }

  void SetEnableStencilTest(bool v) { enable_stencil_test_ = v; }
  bool GetEnableStencilTest() const { return enable_stencil_test_; }

  void SetEnablePrimitiveRestart(bool v) { enable_primitive_restart_ = v; }
  bool GetEnablePrimitiveRestart() const { return enable_primitive_restart_; }

  void SetEnableDepthClamp(bool v) { enable_depth_clamp_ = v; }
  bool GetEnableDepthClamp() const { return enable_depth_clamp_; }

  void SetEnableRasterizerDiscard(bool v) { enable_rasterizer_discard_ = v; }
  bool GetEnableRasterizerDiscard() const { return enable_rasterizer_discard_; }

  void SetEnableDepthBias(bool v) { enable_depth_bias_ = v; }
  bool GetEnableDepthBias() const { return enable_depth_bias_; }

  void SetEnableLogicOp(bool v) { enable_logic_op_ = v; }
  bool GetEnableLogicOp() const { return enable_logic_op_; }

  void SetEnableDepthBoundsTest(bool v) { enable_depth_bounds_test_ = v; }
  bool GetEnableDepthBoundsTest() const { return enable_depth_bounds_test_; }

  void SetDepthBiasConstantFactor(float f) { depth_bias_constant_factor_ = f; }
  float GetDepthBiasConstantFactor() const {
    return depth_bias_constant_factor_;
  }

  void SetDepthBiasClamp(float f) { depth_bias_clamp_ = f; }
  float GetDepthBiasClamp() const { return depth_bias_clamp_; }

  void SetDepthBiasSlopeFactor(float f) { depth_bias_slope_factor_ = f; }
  float GetDepthBiasSlopeFactor() const { return depth_bias_slope_factor_; }

  void SetMinDepthBounds(float f) { min_depth_bounds_ = f; }
  float GetMinDepthBounds() const { return min_depth_bounds_; }

  void SetMaxDepthBounds(float f) { max_depth_bounds_ = f; }
  float GetMaxDepthBounds() const { return max_depth_bounds_; }

  void SetLogicOp(LogicOp op) { logic_op_ = op; }
  LogicOp GetLogicOp() const { return logic_op_; }

  void SetSrcColorBlendFactor(BlendFactor f) { src_color_blend_factor_ = f; }
  BlendFactor GetSrcColorBlendFactor() const { return src_color_blend_factor_; }

  void SetDstColorBlendFactor(BlendFactor f) { dst_color_blend_factor_ = f; }
  BlendFactor GetDstColorBlendFactor() const { return dst_color_blend_factor_; }

  void SetSrcAlphaBlendFactor(BlendFactor f) { src_alpha_blend_factor_ = f; }
  BlendFactor GetSrcAlphaBlendFactor() const { return src_alpha_blend_factor_; }

  void SetDstAlphaBlendFactor(BlendFactor f) { dst_alpha_blend_factor_ = f; }
  BlendFactor GetDstAlphaBlendFactor() const { return dst_alpha_blend_factor_; }

  void SetColorBlendOp(BlendOp op) { color_blend_op_ = op; }
  BlendOp GetColorBlendOp() const { return color_blend_op_; }

  void SetAlphaBlendOp(BlendOp op) { alpha_blend_op_ = op; }
  BlendOp GetAlphaBlendOp() const { return alpha_blend_op_; }

 private:
  StencilOp front_fail_op_ = StencilOp::kKeep;
  StencilOp front_pass_op_ = StencilOp::kKeep;
  StencilOp front_depth_fail_op_ = StencilOp::kKeep;
  CompareOp front_compare_op_ = CompareOp::kAlways;

  StencilOp back_fail_op_ = StencilOp::kKeep;
  StencilOp back_pass_op_ = StencilOp::kKeep;
  StencilOp back_depth_fail_op_ = StencilOp::kKeep;
  CompareOp back_compare_op_ = CompareOp::kAlways;

  Topology topology_ = Topology::kTriangleStrip;
  PolygonMode polygon_mode_ = PolygonMode::kFill;
  CullMode cull_mode_ = CullMode::kNone;
  FrontFace front_face_ = FrontFace::kCounterClockwise;
  CompareOp depth_compare_op_ = CompareOp::kAlways;
  LogicOp logic_op_ = LogicOp::kClear;
  BlendFactor src_color_blend_factor_ = BlendFactor::kOne;
  BlendFactor dst_color_blend_factor_ = BlendFactor::kZero;
  BlendFactor src_alpha_blend_factor_ = BlendFactor::kOne;
  BlendFactor dst_alpha_blend_factor_ = BlendFactor::kZero;
  BlendOp color_blend_op_ = BlendOp::kAdd;
  BlendOp alpha_blend_op_ = BlendOp::kAdd;

  uint32_t front_compare_mask_ = std::numeric_limits<uint32_t>::max();
  uint32_t front_write_mask_ = std::numeric_limits<uint32_t>::max();
  uint32_t front_reference_ = 0;

  uint32_t back_compare_mask_ = std::numeric_limits<uint32_t>::max();
  uint32_t back_write_mask_ = std::numeric_limits<uint32_t>::max();
  uint32_t back_reference_ = 0;

  uint8_t color_write_mask_ =
      kColorMaskR | kColorMaskG | kColorMaskB | kColorMaskA;

  bool enable_blend_ = false;
  bool enable_depth_test_ = false;
  bool enable_depth_write_ = false;
  bool enable_depth_clamp_ = false;
  bool enable_depth_bias_ = false;
  bool enable_depth_bounds_test_ = false;
  bool enable_stencil_test_ = false;
  bool enable_primitive_restart_ = false;
  bool enable_rasterizer_discard_ = false;
  bool enable_logic_op_ = false;

  float line_width_ = 1.0f;
  float depth_bias_constant_factor_ = 0.0f;
  float depth_bias_clamp_ = 0.0f;
  float depth_bias_slope_factor_ = 0.0f;
  float min_depth_bounds_ = 0.0f;
  float max_depth_bounds_ = 0.0f;
};

}  // namespace amber

#endif  // SRC_PIPELINE_DATA_H_
