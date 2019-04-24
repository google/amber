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

const uint32_t kMaximumColorValue = 255;

uint8_t byte0(uint32_t word) {
  return static_cast<uint8_t>(word & 0xff);
}

uint8_t byte1(uint32_t word) {
  return static_cast<uint8_t>((word >> 8) & 0xff);
}

uint8_t byte2(uint32_t word) {
  return static_cast<uint8_t>((word >> 16) & 0xff);
}

}  // namespace

amber::Result ConvertToPPM(uint32_t width,
                           uint32_t height,
                           const std::vector<amber::Value>& values,
                           std::vector<uint8_t>* buffer) {
  if (values.size() != (width * height)) {
    return amber::Result("Values size (" + std::to_string(values.size()) +
                         ") != " + "width * height (" +
                         std::to_string(width * height) + ")");
  }

  // Write PPM header
  std::string image = "P6\n";
  image += std::to_string(width) + " " + std::to_string(height) + "\n";
  image += std::to_string(kMaximumColorValue) + "\n";

  for (char ch : image)
    buffer->push_back(static_cast<uint8_t>(ch));

  // Write PPM data
  for (amber::Value value : values) {
    const uint32_t pixel = value.AsUint32();
    // We assume B8G8R8A8_UNORM here:
    buffer->push_back(byte2(pixel));  // R
    buffer->push_back(byte1(pixel));  // G
    buffer->push_back(byte0(pixel));  // B
    // PPM does not support alpha channel
  }

  return {};
}

}  // namespace ppm
