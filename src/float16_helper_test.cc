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

#include "src/float16_helper.h"

#include "gtest/gtest.h"

namespace amber {
namespace float16 {

using Float16HelperTest = testing::Test;

TEST_F(Float16HelperTest, F32ToF16AndBack) {
  float a = 2.5;

  uint16_t half = float16::FloatToHexFloat16(a);
  float b = float16::HexFloatToFloat(reinterpret_cast<uint8_t*>(&half), 16);
  EXPECT_FLOAT_EQ(a, b);
}

}  // namespace float16
}  // namespace amber
