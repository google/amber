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

#ifndef SRC_SAMPLER_H_
#define SRC_SAMPLER_H_

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "amber/result.h"
#include "amber/value.h"
#include "src/command_data.h"
#include "src/format.h"

namespace amber {

enum class FilterType : int8_t { kUnknown = -1, kNearest = 0, kLinear = 1 };

enum class AddressMode : int8_t {
  kUnknown = -1,
  kRepeat = 0,
  kMirroredRepeat = 1,
  kClampToEdge = 2,
  kClampToBorder = 3,
  kMirrorClampToEdge = 4
};

enum class BorderColor : int8_t {
  kUnknown = -1,
  kFloatTransparentBlack = 0,
  kIntTransparentBlack = 1,
  kFloatOpaqueBlack = 2,
  kIntOpaqueBlack = 3,
  kFloatOpaqueWhite = 4,
  kIntOpaqueWhite = 5
};

class Sampler {
 public:
  Sampler();
  ~Sampler();

  void SetName(const std::string& name) { name_ = name; }
  std::string GetName() const { return name_; }

  void SetMagFilter(FilterType filter) { mag_filter_ = filter; }
  FilterType GetMagFilter() const { return mag_filter_; }

  void SetMinFilter(FilterType filter) { min_filter_ = filter; }
  FilterType GetMinFilter() const { return min_filter_; }

  void SetMipmapMode(FilterType filter) { mipmap_mode_ = filter; }
  FilterType GetMipmapMode() const { return mipmap_mode_; }

  void SetAddressModeU(AddressMode mode) { address_mode_u_ = mode; }
  AddressMode GetAddressModeU() const { return address_mode_u_; }

  void SetAddressModeV(AddressMode mode) { address_mode_v_ = mode; }
  AddressMode GetAddressModeV() const { return address_mode_v_; }

  void SetAddressModeW(AddressMode mode) { address_mode_w_ = mode; }
  AddressMode GetAddressModeW() const { return address_mode_w_; }

  void SetBorderColor(BorderColor color) { border_color_ = color; }
  BorderColor GetBorderColor() const { return border_color_; }

  void SetMinLOD(float min_lod) { min_lod_ = min_lod; }
  float GetMinLOD() const { return min_lod_; }

  void SetMaxLOD(float max_lod) { max_lod_ = max_lod; }
  float GetMaxLOD() const { return max_lod_; }

  void SetNormalizedCoords(bool norm) { normalized_coords_ = norm; }
  bool GetNormalizedCoords() const { return normalized_coords_; }

  void SetCompareEnable(bool compare_enable) {
    compare_enable_ = compare_enable;
  }
  bool GetCompareEnable() const { return compare_enable_; }

  void SetCompareOp(CompareOp compare_op) { compare_op_ = compare_op; }
  CompareOp GetCompareOp() const { return compare_op_; }

 private:
  std::string name_;
  FilterType min_filter_ = FilterType::kNearest;
  FilterType mag_filter_ = FilterType::kNearest;
  FilterType mipmap_mode_ = FilterType::kNearest;
  AddressMode address_mode_u_ = AddressMode::kRepeat;
  AddressMode address_mode_v_ = AddressMode::kRepeat;
  AddressMode address_mode_w_ = AddressMode::kRepeat;
  BorderColor border_color_ = BorderColor::kFloatTransparentBlack;
  float min_lod_ = 0.0f;
  float max_lod_ = 1.0f;
  bool normalized_coords_ = true;
  bool compare_enable_ = false;
  CompareOp compare_op_ = CompareOp::kNever;
};

}  // namespace amber

#endif  // SRC_SAMPLER_H_
