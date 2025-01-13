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

#include <algorithm>
#include <cstring>
#include <utility>
#include <vector>

#include "amber/result.h"
#include "gtest/gtest.h"

namespace amber {
namespace {

const uint8_t kExpectedPPM[] = {
    0x50, 0x36, 0x0a, 0x31, 0x32, 0x20, 0x36, 0x0a, 0x32, 0x35, 0x35, 0x0a,
    0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff,
    0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00,
    0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00,
    0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff,
    0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00,
    0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00,
    0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff,
    0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00,
    0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00,
    0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff,
    0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00,
    0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00,
    0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00,
    0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff,
    0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff,
    0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00,
    0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff,
    0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff};

}  // namespace

using PPMTest = testing::Test;

TEST_F(PPMTest, ConvertToPPM) {
  const uint32_t width = 12;
  const uint32_t height = 6;

  std::vector<amber::Value> data;

  const uint32_t MaskRed = 0x000000FF;
  const uint32_t MaskBlue = 0x0000FF00;
  const uint32_t MaskAplha = 0xFF000000;

  for (uint32_t y = 0; y < height; ++y) {
    for (uint32_t x = 0; x < width; ++x) {
      uint32_t pixel = MaskAplha;
      if (x < width / 2) {
        pixel |= MaskRed;
      } else {
        pixel |= MaskBlue;
      }
      if (y > height / 2) {
        // invert colors
        pixel = ~pixel;
        // reset alpha to 1
        pixel |= MaskAplha;
      }
      Value v;
      v.SetIntValue(static_cast<uint64_t>(pixel));
      data.push_back(v);
    }
  }

  std::vector<uint8_t> out_buf;
  ppm::ConvertToPPM(width, height, data, &out_buf);

  EXPECT_EQ(out_buf.size(), sizeof(kExpectedPPM));
  EXPECT_EQ(std::memcmp(out_buf.data(), kExpectedPPM, sizeof(kExpectedPPM)), 0);
}

}  // namespace amber
