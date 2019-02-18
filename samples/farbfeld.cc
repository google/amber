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

#include "samples/farbfeld.h"

#include <cassert>

#include "amber/result.h"
#include "amber/value.h"

namespace farbfeld {

namespace {

char byte0(uint32_t word) {
  return static_cast<char>(word);
}

char byte1(uint32_t word) {
  return static_cast<char>(word >> 8);
}

char byte2(uint32_t word) {
  return static_cast<char>(word >> 16);
}

char byte3(uint32_t word) {
  return static_cast<char>(word >> 24);
}

}  // namespace

std::pair<amber::Result, std::string> ConvertToFarbfeld(
    uint32_t width,
    uint32_t height,
    const std::vector<amber::Value>& values) {
  assert(values.size() == width * height);

  // Farbfeld format details: https://tools.suckless.org/farbfeld/

  // Farbfeld header
  std::string image = "farbfeld";
  // 32 bits big endian unsigned integers for width, and then height
  image.push_back(byte3(width));
  image.push_back(byte2(width));
  image.push_back(byte1(width));
  image.push_back(byte0(width));
  image.push_back(byte3(height));
  image.push_back(byte2(height));
  image.push_back(byte1(height));
  image.push_back(byte0(height));

  // Farbfeld data
  for (amber::Value value : values) {
    const uint32_t pixel = value.AsUint32();
    // We assume R8G8B8A8_UINT here. Note the zero char is added to fill up the
    // 16 bits available for each channel.
    const char zero = static_cast<char>(0);
    image.push_back(byte0(pixel));  // R
    image.push_back(zero);
    image.push_back(byte1(pixel));  // G
    image.push_back(zero);
    image.push_back(byte2(pixel));  // B
    image.push_back(zero);
    image.push_back(byte3(pixel));  // A
    image.push_back(zero);
  }

  return std::make_pair(amber::Result(), image);
}

}  // namespace farbfeld
