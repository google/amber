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

  void SetBorderColor(BorderColor color) { border_color_ = color; }
  BorderColor GetBorderColor() const { return border_color_; }

 private:
  std::string name_;
  FilterType min_filter_;
  FilterType mag_filter_;
  FilterType mipmap_mode_;
  AddressMode address_mode_u_;
  AddressMode address_mode_v_;
  BorderColor border_color_;
};

}  // namespace amber

#endif  // SRC_SAMPLER_H_
