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

#ifndef SRC_FLOAT16_HELPER_H_
#define SRC_FLOAT16_HELPER_H_

#include <cstdint>

namespace amber {
namespace float16 {

// Convert float |value| whose size is |bits| bits to 32 bits float
// based on IEEE-754.
//
// See https://www.khronos.org/opengl/wiki/Small_Float_Formats
// and https://en.wikipedia.org/wiki/IEEE_754.
//
//    Sign Exponent Mantissa Exponent-Bias
// 16    1        5       10            15
// 11    0        5        6            15
// 10    0        5        5            15
// 32    1        8       23           127
// 64    1       11       52          1023
//
// 11 and 10 bits floats are always positive.
float HexFloatToFloat(const uint8_t* value, uint8_t bits);

// Convert 32 bits float |value| to 16 bits float based on IEEE-754.
uint16_t FloatToHexFloat16(const float value);

}  // namespace float16
}  // namespace amber

#endif  // SRC_FLOAT16_HELPER_H_
