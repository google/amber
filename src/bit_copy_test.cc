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

#include "src/bit_copy.h"

#include <cstring>
#include <type_traits>

#include "gtest/gtest.h"
#include "src/value.h"

namespace amber {
namespace {

template <typename T>
void ExpectBitsEQ(const uint8_t* actual, T expected) {
  const T* ptr = reinterpret_cast<const T*>(actual);
  EXPECT_EQ(*ptr, expected);
}

}  // namespace

using BitCopyTest = testing::Test;

TEST_F(BitCopyTest, CopyInt8) {
  Value value;
  uint8_t data = 0;

  // 7      0          0      7
  // 00000000      --> 11011100 (220)
  // 110111   (55)
  value.SetIntValue(55);
  BitCopy::CopyValueToBuffer(&data, value, 2, 6);
  EXPECT_EQ(data, 220);

  // 7      0          0      7
  // 11011100      --> 11011111 (223)
  //       11 (3)
  value.SetIntValue(3);
  BitCopy::CopyValueToBuffer(&data, value, 0, 2);
  EXPECT_EQ(data, 223);

  // 7      0          0      7
  // 11011111      --> 10110111 (183)
  //  011011  (27)
  value.SetIntValue(27);
  BitCopy::CopyValueToBuffer(&data, value, 1, 6);
  EXPECT_EQ(data, 183);

  // 7      0          0      7
  // 10110111      --> 11010111 (215)
  //  1010    (10)
  value.SetIntValue(10);
  BitCopy::CopyValueToBuffer(&data, value, 3, 4);
  EXPECT_EQ(data, 215);
}

TEST_F(BitCopyTest, CopyInt16) {
  Value value;
  uint8_t data[2] = {};

  // 15              0          15              0
  //  0000000000000000      -->  1100000011100100 (49380)
  //  11000000111001   (12345)
  value.SetIntValue(12345);
  BitCopy::CopyValueToBuffer(data, value, 2, 14);
  ExpectBitsEQ<uint16_t>(data, 49380);

  // 15              0          15              0
  //  1100000011100100      -->  1110100000100100 (59428)
  //    101000001      (321)
  value.SetIntValue(321);
  BitCopy::CopyValueToBuffer(data, value, 5, 9);
  ExpectBitsEQ<uint16_t>(data, 59428);

  // 15              0          15              0
  //  1110100000100100      -->  1111000111010111 (61911)
  //     1000111010111 (4567)
  value.SetIntValue(4567);
  BitCopy::CopyValueToBuffer(data, value, 0, 13);
  ExpectBitsEQ<uint16_t>(data, 61911);

  // 15              0          15              0
  //  1111000111010111      -->  1001101111011111 (39903)
  //   001101111011    (891)
  value.SetIntValue(891);
  BitCopy::CopyValueToBuffer(data, value, 3, 12);
  ExpectBitsEQ<uint16_t>(data, 39903);
}

TEST_F(BitCopyTest, CopyInt32) {
  Value value;
  uint8_t data[4] = {};

  // 31                         31
  //  0000000000000000      -->  0001011110001100
  //  0000000000000000           0010100111000000 (395061696)
  //                 0                          0
  //
  //     1011110001100
  //  00101001110      (12345678)
  value.SetIntValue(12345678);
  BitCopy::CopyValueToBuffer(data, value, 5, 24);
  ExpectBitsEQ<uint32_t>(data, 395061696);

  // 31                         31
  //  0001011110001100      -->  0001011110000001
  //  0010100111000000           1100110111000000 (394382784)
  //                 0                          0
  //
  //         110000001
  //  11001101         (98765)
  value.SetIntValue(98765);
  BitCopy::CopyValueToBuffer(data, value, 8, 17);
  ExpectBitsEQ<uint32_t>(data, 394382784);

  // 31                         31
  //  0001011110000001      -->  0001011110000001
  //  1100110111000000           1100111010001110 (394382990)
  //                 0                          0
  //
  //        1010001110 (654)
  value.SetIntValue(654);
  BitCopy::CopyValueToBuffer(data, value, 0, 10);
  ExpectBitsEQ<uint32_t>(data, 394382990);

  // 31                         31
  //  0001011110000001      -->  1101001011111100
  //  1100111010001110           0101011010001110 (3539752590)
  //                 0                          0
  //
  //  1101001011111100
  //  010101           (654)
  value.SetIntValue(3456789);
  BitCopy::CopyValueToBuffer(data, value, 10, 22);
  ExpectBitsEQ<uint32_t>(data, 3539752590);
}

TEST_F(BitCopyTest, CopyInt64) {
  Value value;
  uint8_t data[8] = {};

  // 63                         63
  //  0000000000000000      -->  0010001111101110
  //  0000000000000000           0011111101100110
  //  0000000000000000           0001011110101100
  //  0000000000000000           0000000000000000 (2589076543500976128)
  //                 0                          0
  //
  //    10001111101110
  //  0011111101100110
  //  00010111101011
  //                   (9876543210987)
  value.SetIntValue(9876543210987UL);
  BitCopy::CopyValueToBuffer(data, value, 18, 44);
  ExpectBitsEQ<uint64_t>(data, 2589076543500976128UL);

  // 63                         63
  //  0010001111101110      -->  0011110001001110
  //  0011111101100110           1111110000011110
  //  0001011110101100           1111010011010001
  //  0000000000000000           0101111101011000 (4345687900345687896)
  //                 0                          0
  //
  //    11110001001110
  //  1111110000011110
  //  1111010011010001
  //  0101111101011    (543210987543210987)
  value.SetIntValue(543210987543210987UL);
  BitCopy::CopyValueToBuffer(data, value, 3, 59);
  ExpectBitsEQ<uint64_t>(data, 4345687900345687896UL);

  // 63                         63
  //  0011110001001110      -->  0011110001001110
  //  1111110000011110           1001011111100010
  //  1111010011010001           1011010011010001
  //  0101111101011000           0101111101011000 (4345577690411130712)
  //                 0                          0
  //
  //               110
  //  1001011111100010
  //  101
  //                   (3456789)
  value.SetIntValue(3456789UL);
  BitCopy::CopyValueToBuffer(data, value, 29, 22);
  ExpectBitsEQ<uint64_t>(data, 4345577690411130712UL);
}

TEST_F(BitCopyTest, CopyIntMultiple) {
  uint8_t data[32] = {};
  Value value;

  // Fill [0, 32) bits of data with
  // 11(3) / 0010001111(143) / 0001000011(67) / 1000010001(529)
  value.SetIntValue(529);
  BitCopy::CopyValueToBuffer(data, value, 0, 10);
  value.SetIntValue(67);
  BitCopy::CopyValueToBuffer(data, value, 10, 10);
  value.SetIntValue(143);
  BitCopy::CopyValueToBuffer(data, value, 20, 10);
  value.SetIntValue(3);
  BitCopy::CopyValueToBuffer(data, value, 30, 2);

  // Fill [32, 96) bits of data with
  // 00000111010110111100110100010101(123456789) /
  // 00000000100101101011010000111111(9876543)
  value.SetIntValue(9876543);
  BitCopy::CopyValueToBuffer(data, value, 32, 32);
  value.SetIntValue(123456789);
  BitCopy::CopyValueToBuffer(data, value, 64, 32);

  // Fill [96, 120) bits of data with
  // 00011111(31) / 00001001(9) / 01001101(77)
  value.SetIntValue(77);
  BitCopy::CopyValueToBuffer(data, value, 96, 8);
  value.SetIntValue(9);
  BitCopy::CopyValueToBuffer(data, value, 104, 8);
  value.SetIntValue(31);
  BitCopy::CopyValueToBuffer(data, value, 112, 8);

  // Fill [120, 184) bits of data with
  // 00000001101101101001101101001011
  // 10100110001100001111001101001110(123456789012345678)
  value.SetIntValue(123456789012345678UL);
  BitCopy::CopyValueToBuffer(data, value, 120, 64);

  // Fill [184, 216) bits of data with
  // 10000011110111011011010010000000(34567890)
  value.SetIntValue(34567890);
  BitCopy::CopyValueToBuffer(data, value, 184, 32);

  // Fill [216, 256) bits of data with
  // 01100011(99) / 1000001000110101(33333) / 11011110(222) / 01101111(111)
  value.SetIntValue(111);
  BitCopy::CopyValueToBuffer(data, value, 216, 8);
  value.SetIntValue(222);
  BitCopy::CopyValueToBuffer(data, value, 224, 8);
  value.SetIntValue(33333);
  BitCopy::CopyValueToBuffer(data, value, 232, 16);
  value.SetIntValue(99);
  BitCopy::CopyValueToBuffer(data, value, 248, 8);

  // [0, 32) bits of data
  ExpectBitsEQ<uint32_t>(data, 3371240977);

  // [32, 96) bits of data
  ExpectBitsEQ<uint64_t>(&data[4], 530242871234049087UL);

  // [96, 120) bits of data
  ExpectBitsEQ<uint16_t>(&data[12], 2381);
  EXPECT_EQ(data[14], 31);

  // [120, 184) bits of data
  ExpectBitsEQ<uint64_t>(&data[15], 123456789012345678UL);

  // [184, 216) bits of data
  ExpectBitsEQ<uint32_t>(&data[23], 34567890);

  // [216, 256) bits of data
  ExpectBitsEQ<uint32_t>(&data[27], 2184568431);
  EXPECT_EQ(data[31], 99);
}

TEST_F(BitCopyTest, CopyFloat16) {
  uint8_t data[2] = {};
  Value value;

  // 16 bits
  //         Sig / Exp / Mantissa     Sig / Exp / Mantissa
  // 12.34 =   0 / 130 /  4550820 -->   0 /  18 /      555
  value.SetDoubleValue(12.34);
  data[0] = 0;
  data[1] = 0;
  BitCopy::CopyValueToBuffer(data, value, 0, 16);
  ExpectBitsEQ<uint16_t>(data, 18987);

  // 11 bits
  //         Sig / Exp / Mantissa     Sig / Exp / Mantissa
  // 5.67 =    0 / 129 /  3502244 -->        17 /       26
  value.SetDoubleValue(5.67);
  data[0] = 0;
  data[1] = 0;
  BitCopy::CopyValueToBuffer(data, value, 3, 11);
  ExpectBitsEQ<uint16_t>(data, 8912);

  // 10 bits
  //         Sig / Exp / Mantissa     Sig / Exp / Mantissa
  // 0.89 =    0 / 126 /  6543114 -->        14 /       24
  value.SetDoubleValue(0.89);
  data[0] = 0;
  data[1] = 0;
  BitCopy::CopyValueToBuffer(data, value, 2, 10);
  ExpectBitsEQ<uint16_t>(data, 1888);
}

TEST_F(BitCopyTest, CopyFloat) {
  uint8_t data[4] = {};
  Value value;

  //         Sig / Exp / Mantissa
  // 12.34 =   0 / 130 /  4550820
  value.SetDoubleValue(12.34);
  BitCopy::CopyValueToBuffer(data, value, 0, 32);
  ExpectBitsEQ<uint32_t>(data, 1095069860);

  //         Sig / Exp / Mantissa
  // 5.67 =    0 / 129 /  3502244
  value.SetDoubleValue(5.67);
  data[0] = 0;
  data[1] = 0;
  data[2] = 0;
  data[3] = 0;
  BitCopy::CopyValueToBuffer(data, value, 0, 32);
  ExpectBitsEQ<uint32_t>(data, 1085632676);

  //         Sig / Exp / Mantissa
  // 0.89 =    0 / 126 /  6543114
  value.SetDoubleValue(0.89);
  data[0] = 0;
  data[1] = 0;
  data[2] = 0;
  data[3] = 0;
  BitCopy::CopyValueToBuffer(data, value, 0, 32);
  ExpectBitsEQ<uint32_t>(data, 1063507722);
}

TEST_F(BitCopyTest, CopyDouble) {
  uint8_t data[8] = {};
  Value value;

  //         Sig /  Exp /         Mantissa
  // 12.34 =   0 / 1026 / 2443202797848494
  value.SetDoubleValue(12.34);
  BitCopy::CopyValueToBuffer(data, value, 0, 64);
  ExpectBitsEQ<uint64_t>(data, 4623136420479977390);

  //         Sig /  Exp /         Mantissa
  // 5.67 =    0 / 1025 / 1880252844427182
  value.SetDoubleValue(5.67);
  data[0] = 0;
  data[1] = 0;
  data[2] = 0;
  data[3] = 0;
  data[4] = 0;
  data[5] = 0;
  data[6] = 0;
  data[7] = 0;
  BitCopy::CopyValueToBuffer(data, value, 0, 64);
  ExpectBitsEQ<uint64_t>(data, 4618069870899185582);

  //         Sig /  Exp /         Mantissa
  // 0.89 =    0 / 1022 / 3512807709348987
  value.SetDoubleValue(0.89);
  data[0] = 0;
  data[1] = 0;
  data[2] = 0;
  data[3] = 0;
  data[4] = 0;
  data[5] = 0;
  data[6] = 0;
  data[7] = 0;
  BitCopy::CopyValueToBuffer(data, value, 0, 64);
  ExpectBitsEQ<uint64_t>(data, 4606191626881995899);
}

TEST_F(BitCopyTest, HexFloatToFloat) {
  // 16 bits float to float
  //   Sig / Exp / Mantissa     Sig / Exp / Mantissa
  //     1 /  17 /      512 -->   1 / 129 /  4194304 = -1.1(2) * 2^2 = -6
  uint16_t data = 50688;
  EXPECT_FLOAT_EQ(
      BitCopy::HexFloatToFloat(reinterpret_cast<const uint8_t*>(&data), 16),
      -6.0f);

  // 11 bits float to float
  //   Sig / Exp / Mantissa     Sig / Exp / Mantissa
  //     0 /  18 /       48 -->   0 / 130 / 12582912 = 1.11(2) * 2^3 = 14
  data = 1200;
  EXPECT_FLOAT_EQ(
      BitCopy::HexFloatToFloat(reinterpret_cast<const uint8_t*>(&data), 11),
      14.0f);

  // 10 bits float to float
  //   Sig / Exp / Mantissa     Sig / Exp / Mantissa
  //     0 /  11 /       28 -->   1 / 123 / 14680064 = 1.111(2) * 2^-4 =
  //     0.1171875
  data = 380;
  EXPECT_FLOAT_EQ(
      BitCopy::HexFloatToFloat(reinterpret_cast<const uint8_t*>(&data), 10),
      0.1171875f);
}

}  // namespace amber
