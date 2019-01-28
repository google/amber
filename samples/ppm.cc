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

#include "samples/ppm.h"

#include <cassert>

#include "amber/result.h"
#include "amber/value.h"

namespace ppm {

namespace {

uint8_t byte0(uint32_t word) {
  return (uint8_t)(word);
}

uint8_t byte1(uint32_t word) {
  return (uint8_t)(word >> 8);
}

uint8_t byte2(uint32_t word) {
  return (uint8_t)(word >> 16);
}

}  // namespace

std::pair<amber::Result, std::string> ConvertToPPM(
    uint32_t width,
    uint32_t height,
    const std::vector<amber::Value>& values) {
  assert(values.size() == width * height);

  // Write PPM header
  const uint32_t maximum_color_value = 255;
  std::string image = "P6\n";
  image += std::to_string(width) + " " + std::to_string(height) + "\n";
  image += std::to_string(maximum_color_value) + "\n";

  // Write PPM data
  for (amber::Value value : values) {
    const uint32_t pixel = value.AsUint32();
    // We assume R8G8B8A8_UINT here:
    image.push_back(byte0(pixel));  // R
    image.push_back(byte1(pixel));  // G
    image.push_back(byte2(pixel));  // B
    // PPM does not support alpha channel
  }

  return std::make_pair(amber::Result(), image);
}

}  // namespace ppm
