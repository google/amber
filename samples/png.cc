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

#include "samples/png.h"

#include <cassert>

#include "amber/result.h"
#include "amber/value.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wweak-vtables"
#include "third_party/lodepng/lodepng.h"
#pragma clang diagnostic pop

namespace png {

namespace {

unsigned char byte0(uint32_t word) {
  return static_cast<unsigned char>(word);
}

unsigned char byte1(uint32_t word) {
  return static_cast<unsigned char>(word >> 8);
}

unsigned char byte2(uint32_t word) {
  return static_cast<unsigned char>(word >> 16);
}

unsigned char byte3(uint32_t word) {
  return static_cast<unsigned char>(word >> 24);
}

}  // namespace

amber::Result ConvertToPNG(uint32_t width,
                           uint32_t height,
                           const std::vector<amber::Value>& values,
                           std::vector<uint8_t>* buffer) {
  if (values.size() != (width * height)) {
    return amber::Result("Values size (" + std::to_string(values.size()) +
                         ") != " + "width * height (" +
                         std::to_string(width * height) + ")");
  }

  std::vector<uint8_t> data;

  // Prepare data as lodepng expects it
  for (amber::Value value : values) {
    const uint32_t pixel = value.AsUint32();
    data.push_back(byte2(pixel));  // R
    data.push_back(byte1(pixel));  // G
    data.push_back(byte0(pixel));  // B
    data.push_back(byte3(pixel));  // A
  }

  lodepng::State state;

  // Force RGBA color type, otherwise many PNG decoders will ignore the alpha
  // channel.
  state.encoder.auto_convert = 0;
  state.info_raw.colortype = LodePNGColorType::LCT_RGBA;
  state.info_raw.bitdepth = 8;
  state.info_png.color.colortype = LodePNGColorType::LCT_RGBA;
  state.info_png.color.bitdepth = 8;

  if (lodepng::encode(*buffer, data, width, height, state) != 0)
    return amber::Result("lodepng::encode() returned non-zero");

  return {};
}

}  // namespace png
