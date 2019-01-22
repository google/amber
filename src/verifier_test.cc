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

#include <memory>
#include <utility>
#include <vector>

#include "amber/result.h"
#include "gtest/gtest.h"
#include "src/command.h"
#include "src/datum_type.h"
#include "src/make_unique.h"
#include "src/value.h"
#include "src/verifier.h"

namespace amber {
namespace {

class VerifierTest : public testing::Test {
 public:
  VerifierTest() = default;
  ~VerifierTest() = default;

  const Format* GetColorFormat() {
    if (color_frame_format_)
      return color_frame_format_.get();

    // Set VK_FORMAT_R8G8B8A8_UNORM for color frame buffer.
    color_frame_format_ = MakeUnique<Format>();
    color_frame_format_->SetFormatType(FormatType::kB8G8R8A8_UNORM);
    color_frame_format_->AddComponent(FormatComponentType::kR,
                                      FormatMode::kUNorm, 8);
    color_frame_format_->AddComponent(FormatComponentType::kG,
                                      FormatMode::kUNorm, 8);
    color_frame_format_->AddComponent(FormatComponentType::kB,
                                      FormatMode::kUNorm, 8);
    color_frame_format_->AddComponent(FormatComponentType::kA,
                                      FormatMode::kUNorm, 8);
    return color_frame_format_.get();
  }

 private:
  std::unique_ptr<Format> color_frame_format_;
};

}  // namespace

TEST_F(VerifierTest, ProbeFrameBufferWholeWindow) {
  ProbeCommand probe;
  probe.SetWholeWindow();
  probe.SetIsRGBA();
  probe.SetR(0.5f);
  probe.SetG(0.25f);
  probe.SetB(0.2f);
  probe.SetA(0.8f);

  const uint8_t frame_buffer[3][3][4] = {
      {
          {128, 64, 51, 204},
          {128, 64, 51, 204},
          {128, 64, 51, 204},
      },
      {
          {128, 64, 51, 204},
          {128, 64, 51, 204},
          {128, 64, 51, 204},
      },
      {
          {128, 64, 51, 204},
          {128, 64, 51, 204},
          {128, 64, 51, 204},
      },
  };

  Verifier verifier;
  Result r = verifier.Probe(&probe, GetColorFormat(), 4, 12, 3, 3,
                            static_cast<const void*>(frame_buffer));
  EXPECT_TRUE(r.IsSuccess());
}

TEST_F(VerifierTest, ProbeFrameBufferRelative) {
  ProbeCommand probe;
  probe.SetRelative();
  probe.SetIsRGBA();
  probe.SetX(0.1f);
  probe.SetY(0.2f);
  probe.SetWidth(0.4f);
  probe.SetHeight(0.6f);
  probe.SetR(0.5f);
  probe.SetG(0.25f);
  probe.SetB(0.2f);
  probe.SetA(0.8f);

  uint8_t frame_buffer[10][10][4] = {};
  for (uint8_t x = 1; x < 5; ++x) {
    for (uint8_t y = 2; y < 8; ++y) {
      frame_buffer[y][x][0] = 128;
      frame_buffer[y][x][1] = 64;
      frame_buffer[y][x][2] = 51;
      frame_buffer[y][x][3] = 204;
    }
  }

  Verifier verifier;
  Result r = verifier.Probe(&probe, GetColorFormat(), 4, 40, 10, 10,
                            static_cast<const void*>(frame_buffer));
  EXPECT_TRUE(r.IsSuccess());
}

TEST_F(VerifierTest, ProbeFrameBufferRelativeSmallExpectFail) {
  ProbeCommand probe;
  probe.SetRelative();
  probe.SetIsRGBA();
  probe.SetX(0.9f);
  probe.SetY(0.9f);
  probe.SetWidth(0.1f);
  probe.SetHeight(0.1f);
  probe.SetR(0.1f);
  probe.SetG(0.0);
  probe.SetB(0.0f);
  probe.SetA(0.0f);

  uint8_t frame_buffer[250][250][4] = {};

  Verifier verifier;
  Result r = verifier.Probe(&probe, GetColorFormat(), 4, 1000, 250, 250,
                            static_cast<const void*>(frame_buffer));
  EXPECT_STREQ(
      "Line 1: Probe failed at: 225, 225\n  Expected RGBA: 25.500000, "
      "0.000000, 0.000000, 0.000000\n  Actual RGBA: 0.000000, 0.000000, "
      "0.000000, 0.000000\nProbe failed in 625 pixels",
      r.Error().c_str());
}

TEST_F(VerifierTest, ProbeFrameBuffer) {
  ProbeCommand probe;
  probe.SetIsRGBA();
  probe.SetX(1.0f);
  probe.SetY(2.0f);
  probe.SetWidth(4.0f);
  probe.SetHeight(6.0f);
  probe.SetR(0.5f);
  probe.SetG(0.25f);
  probe.SetB(0.2f);
  probe.SetA(0.8f);

  uint8_t frame_buffer[10][10][4] = {};
  for (uint8_t x = 1; x < 5; ++x) {
    for (uint8_t y = 2; y < 8; ++y) {
      frame_buffer[y][x][0] = 128;
      frame_buffer[y][x][1] = 64;
      frame_buffer[y][x][2] = 51;
      frame_buffer[y][x][3] = 204;
    }
  }

  Verifier verifier;
  Result r = verifier.Probe(&probe, GetColorFormat(), 4, 40, 10, 10,
                            static_cast<const void*>(frame_buffer));
  EXPECT_TRUE(r.IsSuccess());
}

TEST_F(VerifierTest, ProbeFrameBufferUInt8) {
  ProbeCommand probe;
  probe.SetIsRGBA();
  probe.SetX(0.0f);
  probe.SetY(0.0f);
  probe.SetR(255);
  probe.SetG(14);
  probe.SetB(75);
  probe.SetA(8);

  uint8_t frame_buffer[4] = {255, 14, 75, 8};

  Format format;
  format.SetFormatType(FormatType::kR8G8B8A8_UINT);
  format.AddComponent(FormatComponentType::kR, FormatMode::kUInt, 8);
  format.AddComponent(FormatComponentType::kG, FormatMode::kUInt, 8);
  format.AddComponent(FormatComponentType::kB, FormatMode::kUInt, 8);
  format.AddComponent(FormatComponentType::kA, FormatMode::kUInt, 8);

  Verifier verifier;
  Result r = verifier.Probe(&probe, &format,
                            4 * static_cast<uint32_t>(sizeof(uint8_t)),
                            4 * static_cast<uint32_t>(sizeof(uint8_t)), 1, 1,
                            static_cast<const void*>(&frame_buffer));
  EXPECT_TRUE(r.IsSuccess());
}

TEST_F(VerifierTest, ProbeFrameBufferUInt16) {
  ProbeCommand probe;
  probe.SetIsRGBA();
  probe.SetX(0.0f);
  probe.SetY(0.0f);
  probe.SetR(65535);
  probe.SetG(14);
  probe.SetB(1875);
  probe.SetA(8);

  uint16_t frame_buffer[4] = {65535, 14, 1875, 8};

  Format format;
  format.SetFormatType(FormatType::kR16G16B16A16_UINT);
  format.AddComponent(FormatComponentType::kR, FormatMode::kUInt, 16);
  format.AddComponent(FormatComponentType::kG, FormatMode::kUInt, 16);
  format.AddComponent(FormatComponentType::kB, FormatMode::kUInt, 16);
  format.AddComponent(FormatComponentType::kA, FormatMode::kUInt, 16);

  Verifier verifier;
  Result r = verifier.Probe(&probe, &format,
                            4 * static_cast<uint32_t>(sizeof(uint16_t)),
                            4 * static_cast<uint32_t>(sizeof(uint16_t)), 1, 1,
                            static_cast<const void*>(&frame_buffer));
  EXPECT_TRUE(r.IsSuccess());
}

TEST_F(VerifierTest, ProbeFrameBufferUInt32) {
  ProbeCommand probe;
  probe.SetIsRGBA();
  probe.SetX(0.0f);
  probe.SetY(0.0f);
  probe.SetR(6);
  probe.SetG(14);
  probe.SetB(1171875);
  probe.SetA(8);

  uint32_t frame_buffer[4] = {6, 14, 1171875, 8};

  Format format;
  format.SetFormatType(FormatType::kR32G32B32A32_UINT);
  format.AddComponent(FormatComponentType::kR, FormatMode::kUInt, 32);
  format.AddComponent(FormatComponentType::kG, FormatMode::kUInt, 32);
  format.AddComponent(FormatComponentType::kB, FormatMode::kUInt, 32);
  format.AddComponent(FormatComponentType::kA, FormatMode::kUInt, 32);

  Verifier verifier;
  Result r = verifier.Probe(&probe, &format,
                            4 * static_cast<uint32_t>(sizeof(uint32_t)),
                            4 * static_cast<uint32_t>(sizeof(uint32_t)), 1, 1,
                            static_cast<const void*>(&frame_buffer));
  EXPECT_TRUE(r.IsSuccess());
}

TEST_F(VerifierTest, ProbeFrameBufferUInt64) {
  ProbeCommand probe;
  probe.SetIsRGBA();
  probe.SetX(0.0f);
  probe.SetY(0.0f);
  probe.SetR(6);
  probe.SetG(14);
  probe.SetB(1171875);
  probe.SetA(8);

  uint64_t frame_buffer[4] = {6, 14, 1171875, 8};

  Format format;
  format.SetFormatType(FormatType::kR64G64B64A64_UINT);
  format.AddComponent(FormatComponentType::kR, FormatMode::kUInt, 64);
  format.AddComponent(FormatComponentType::kG, FormatMode::kUInt, 64);
  format.AddComponent(FormatComponentType::kB, FormatMode::kUInt, 64);
  format.AddComponent(FormatComponentType::kA, FormatMode::kUInt, 64);

  Verifier verifier;
  Result r = verifier.Probe(&probe, &format,
                            4 * static_cast<uint32_t>(sizeof(uint64_t)),
                            4 * static_cast<uint32_t>(sizeof(uint64_t)), 1, 1,
                            static_cast<const void*>(&frame_buffer));
  EXPECT_TRUE(r.IsSuccess());
}

TEST_F(VerifierTest, ProbeFrameBufferSInt8) {
  ProbeCommand probe;
  probe.SetIsRGBA();
  probe.SetX(0.0f);
  probe.SetY(0.0f);
  probe.SetR(-6);
  probe.SetG(14);
  probe.SetB(75);
  probe.SetA(8);

  int8_t frame_buffer[4] = {-6, 14, 75, 8};

  Format format;
  format.SetFormatType(FormatType::kR8G8B8A8_SINT);
  format.AddComponent(FormatComponentType::kR, FormatMode::kSInt, 8);
  format.AddComponent(FormatComponentType::kG, FormatMode::kSInt, 8);
  format.AddComponent(FormatComponentType::kB, FormatMode::kSInt, 8);
  format.AddComponent(FormatComponentType::kA, FormatMode::kSInt, 8);

  Verifier verifier;
  Result r =
      verifier.Probe(&probe, &format, 4 * static_cast<uint32_t>(sizeof(int8_t)),
                     4 * static_cast<uint32_t>(sizeof(int8_t)), 1, 1,
                     static_cast<const void*>(&frame_buffer));
  EXPECT_TRUE(r.IsSuccess());
}

TEST_F(VerifierTest, ProbeFrameBufferSInt16) {
  ProbeCommand probe;
  probe.SetIsRGBA();
  probe.SetX(0.0f);
  probe.SetY(0.0f);
  probe.SetR(-6);
  probe.SetG(14);
  probe.SetB(1875);
  probe.SetA(8);

  int16_t frame_buffer[4] = {-6, 14, 1875, 8};

  Format format;
  format.SetFormatType(FormatType::kR16G16B16A16_SINT);
  format.AddComponent(FormatComponentType::kR, FormatMode::kSInt, 16);
  format.AddComponent(FormatComponentType::kG, FormatMode::kSInt, 16);
  format.AddComponent(FormatComponentType::kB, FormatMode::kSInt, 16);
  format.AddComponent(FormatComponentType::kA, FormatMode::kSInt, 16);

  Verifier verifier;
  Result r = verifier.Probe(&probe, &format,
                            4 * static_cast<uint32_t>(sizeof(int16_t)),
                            4 * static_cast<uint32_t>(sizeof(int16_t)), 1, 1,
                            static_cast<const void*>(&frame_buffer));
  EXPECT_TRUE(r.IsSuccess());
}

TEST_F(VerifierTest, ProbeFrameBufferSInt32) {
  ProbeCommand probe;
  probe.SetIsRGBA();
  probe.SetX(0.0f);
  probe.SetY(0.0f);
  probe.SetR(-6);
  probe.SetG(14);
  probe.SetB(1171875);
  probe.SetA(8);

  int32_t frame_buffer[4] = {-6, 14, 1171875, 8};

  Format format;
  format.SetFormatType(FormatType::kR32G32B32A32_SINT);
  format.AddComponent(FormatComponentType::kR, FormatMode::kSInt, 32);
  format.AddComponent(FormatComponentType::kG, FormatMode::kSInt, 32);
  format.AddComponent(FormatComponentType::kB, FormatMode::kSInt, 32);
  format.AddComponent(FormatComponentType::kA, FormatMode::kSInt, 32);

  Verifier verifier;
  Result r = verifier.Probe(&probe, &format,
                            4 * static_cast<uint32_t>(sizeof(int32_t)),
                            4 * static_cast<uint32_t>(sizeof(int32_t)), 1, 1,
                            static_cast<const void*>(&frame_buffer));
  EXPECT_TRUE(r.IsSuccess());
}

TEST_F(VerifierTest, ProbeFrameBufferSInt64) {
  ProbeCommand probe;
  probe.SetIsRGBA();
  probe.SetX(0.0f);
  probe.SetY(0.0f);
  probe.SetR(-6);
  probe.SetG(14);
  probe.SetB(1171875);
  probe.SetA(8);

  int64_t frame_buffer[4] = {-6, 14, 1171875, 8};

  Format format;
  format.SetFormatType(FormatType::kR64G64B64A64_SINT);
  format.AddComponent(FormatComponentType::kR, FormatMode::kSInt, 64);
  format.AddComponent(FormatComponentType::kG, FormatMode::kSInt, 64);
  format.AddComponent(FormatComponentType::kB, FormatMode::kSInt, 64);
  format.AddComponent(FormatComponentType::kA, FormatMode::kSInt, 64);

  Verifier verifier;
  Result r = verifier.Probe(&probe, &format,
                            4 * static_cast<uint32_t>(sizeof(int64_t)),
                            4 * static_cast<uint32_t>(sizeof(int64_t)), 1, 1,
                            static_cast<const void*>(&frame_buffer));
  EXPECT_TRUE(r.IsSuccess());
}

TEST_F(VerifierTest, ProbeFrameBufferFloat32) {
  ProbeCommand probe;
  probe.SetIsRGBA();
  probe.SetX(0.0f);
  probe.SetY(0.0f);
  probe.SetR(-6.0f);
  probe.SetG(14.0f);
  probe.SetB(0.1171875f);
  probe.SetA(0.8f);

  float frame_buffer[4] = {-6.0f, 14.0f, 0.1171875f, 0.8f};

  Format format;
  format.SetFormatType(FormatType::kR32G32B32A32_SFLOAT);
  format.AddComponent(FormatComponentType::kR, FormatMode::kSFloat, 32);
  format.AddComponent(FormatComponentType::kG, FormatMode::kSFloat, 32);
  format.AddComponent(FormatComponentType::kB, FormatMode::kSFloat, 32);
  format.AddComponent(FormatComponentType::kA, FormatMode::kSFloat, 32);

  Verifier verifier;
  Result r =
      verifier.Probe(&probe, &format, 4 * static_cast<uint32_t>(sizeof(float)),
                     4 * static_cast<uint32_t>(sizeof(float)), 1, 1,
                     static_cast<const void*>(&frame_buffer));
  EXPECT_TRUE(r.IsSuccess());
}

TEST_F(VerifierTest, ProbeFrameBufferFloat64) {
  ProbeCommand probe;
  probe.SetIsRGBA();
  probe.SetX(0.0f);
  probe.SetY(0.0f);
  probe.SetR(-6.0f);
  probe.SetG(14.0f);
  probe.SetB(0.1171875f);
  probe.SetA(0.8f);

  double frame_buffer[4] = {-6.0, 14.0, 0.1171875, 0.8};

  Format format;
  format.SetFormatType(FormatType::kR64G64B64A64_SFLOAT);
  format.AddComponent(FormatComponentType::kR, FormatMode::kSFloat, 64);
  format.AddComponent(FormatComponentType::kG, FormatMode::kSFloat, 64);
  format.AddComponent(FormatComponentType::kB, FormatMode::kSFloat, 64);
  format.AddComponent(FormatComponentType::kA, FormatMode::kSFloat, 64);

  Verifier verifier;
  Result r =
      verifier.Probe(&probe, &format, 4 * static_cast<uint32_t>(sizeof(double)),
                     4 * static_cast<uint32_t>(sizeof(double)), 1, 1,
                     static_cast<const void*>(&frame_buffer));
  EXPECT_TRUE(r.IsSuccess());
}

TEST_F(VerifierTest, HexFloatToFloatR16G11B10) {
  ProbeCommand probe;
  probe.SetX(0.0f);
  probe.SetY(0.0f);

  uint64_t frame_buffer = 0;

  // 16 bits float to float
  //   Sig / Exp / Mantissa     Sig / Exp / Mantissa
  //     1 /  17 /      512 -->   1 / 129 /  4194304 = -1.1(2) * 2^2 = -6
  frame_buffer = 50688UL;
  probe.SetR(-6.0f);

  // 11 bits float to float
  //   Sig / Exp / Mantissa     Sig / Exp / Mantissa
  //     0 /  18 /       48 -->   0 / 130 / 12582912 = 1.11(2) * 2^3 = 14
  frame_buffer |= 1200UL << 16UL;
  probe.SetG(14.0f);

  // 10 bits float to float
  //   Sig / Exp / Mantissa     Sig / Exp / Mantissa
  //     0 /  11 /       28 -->   1 / 123 / 14680064 = 1.111(2) * 2^-4
  //                                                 = 0.1171875
  frame_buffer |= 380UL << (16UL + 11UL);
  probe.SetB(0.1171875f);

  Format format;
  format.SetFormatType(FormatType::kB8G8R8A8_UNORM);
  format.AddComponent(FormatComponentType::kR, FormatMode::kSFloat, 16);
  format.AddComponent(FormatComponentType::kG, FormatMode::kUFloat, 11);
  format.AddComponent(FormatComponentType::kB, FormatMode::kUFloat, 10);

  Verifier verifier;
  Result r = verifier.Probe(&probe, &format, 6, 6, 1, 1,
                            static_cast<const void*>(&frame_buffer));
  EXPECT_TRUE(r.IsSuccess());
}

TEST_F(VerifierTest, HexFloatToFloatR11G16B10) {
  ProbeCommand probe;
  probe.SetX(0.0f);
  probe.SetY(0.0f);

  uint64_t frame_buffer = 0;

  // 11 bits float to float
  //   Sig / Exp / Mantissa     Sig / Exp / Mantissa
  //     0 /  18 /       48 -->   0 / 130 / 12582912 = 1.11(2) * 2^3 = 14
  frame_buffer = 1200UL;
  probe.SetR(14.0f);

  // 16 bits float to float
  //   Sig / Exp / Mantissa     Sig / Exp / Mantissa
  //     1 /  17 /      512 -->   1 / 129 /  4194304 = -1.1(2) * 2^2 = -6
  frame_buffer |= 50688UL << 11UL;
  probe.SetG(-6.0f);

  // 10 bits float to float
  //   Sig / Exp / Mantissa     Sig / Exp / Mantissa
  //     0 /  11 /       28 -->   1 / 123 / 14680064 = 1.111(2) * 2^-4
  //                                                 = 0.1171875
  frame_buffer |= 380UL << (16UL + 11UL);
  probe.SetB(0.1171875f);

  Format format;
  format.SetFormatType(FormatType::kB8G8R8A8_UNORM);
  format.AddComponent(FormatComponentType::kR, FormatMode::kSFloat, 11);
  format.AddComponent(FormatComponentType::kG, FormatMode::kUFloat, 16);
  format.AddComponent(FormatComponentType::kB, FormatMode::kUFloat, 10);

  Verifier verifier;
  Result r = verifier.Probe(&probe, &format, 6, 6, 1, 1,
                            static_cast<const void*>(&frame_buffer));
  EXPECT_TRUE(r.IsSuccess());
}

TEST_F(VerifierTest, HexFloatToFloatR10G11B16) {
  ProbeCommand probe;
  probe.SetX(0.0f);
  probe.SetY(0.0f);

  uint64_t frame_buffer = 0;

  // 10 bits float to float
  //   Sig / Exp / Mantissa     Sig / Exp / Mantissa
  //     0 /  11 /       28 -->   1 / 123 / 14680064 = 1.111(2) * 2^-4
  //                                                 = 0.1171875
  frame_buffer = 380UL;
  probe.SetR(0.1171875f);

  // 11 bits float to float
  //   Sig / Exp / Mantissa     Sig / Exp / Mantissa
  //     0 /  18 /       48 -->   0 / 130 / 12582912 = 1.11(2) * 2^3 = 14
  frame_buffer |= 1200UL << 10UL;
  probe.SetG(14.0f);

  // 16 bits float to float
  //   Sig / Exp / Mantissa     Sig / Exp / Mantissa
  //     1 /  17 /      512 -->   1 / 129 /  4194304 = -1.1(2) * 2^2 = -6
  frame_buffer |= 50688UL << (10UL + 11UL);
  probe.SetB(-6.0f);

  Format format;
  format.SetFormatType(FormatType::kB8G8R8A8_UNORM);
  format.AddComponent(FormatComponentType::kR, FormatMode::kSFloat, 10);
  format.AddComponent(FormatComponentType::kG, FormatMode::kUFloat, 11);
  format.AddComponent(FormatComponentType::kB, FormatMode::kUFloat, 16);

  Verifier verifier;
  Result r = verifier.Probe(&probe, &format, 6, 6, 1, 1,
                            static_cast<const void*>(&frame_buffer));
  EXPECT_TRUE(r.IsSuccess());
}

TEST_F(VerifierTest, ProbeFrameBufferRGB) {
  ProbeCommand probe;
  probe.SetWholeWindow();
  probe.SetR(0.5f);
  probe.SetG(0.25f);
  probe.SetB(0.2f);

  const uint8_t frame_buffer[3][3][4] = {
      {
          {128, 64, 51, 255},
          {128, 64, 51, 255},
          {128, 64, 51, 255},
      },
      {
          {128, 64, 51, 255},
          {128, 64, 51, 255},
          {128, 64, 51, 255},
      },
      {
          {128, 64, 51, 255},
          {128, 64, 51, 255},
          {128, 64, 51, 255},
      },
  };

  Verifier verifier;
  Result r = verifier.Probe(&probe, GetColorFormat(), 4, 12, 3, 3,
                            static_cast<const void*>(frame_buffer));
  EXPECT_TRUE(r.IsSuccess());
}

TEST_F(VerifierTest, ProbeFrameBufferBadRowStride) {
  ProbeCommand probe;
  probe.SetWholeWindow();

  const uint8_t frame_buffer[4] = {128, 64, 51, 255};

  Verifier verifier;
  Result r = verifier.Probe(&probe, GetColorFormat(), 4, 3, 1, 1,
                            static_cast<const void*>(frame_buffer));
  EXPECT_FALSE(r.IsSuccess());
  EXPECT_STREQ(
      "Line 1: Verifier::Probe Row stride of 3 is too small for 1 texels of 4 "
      "bytes each",
      r.Error().c_str());
}

TEST_F(VerifierTest, ProbeSSBOUint8Single) {
  ProbeSSBOCommand probe_ssbo;

  DatumType datum_type;
  datum_type.SetType(DataType::kUint8);
  probe_ssbo.SetDatumType(datum_type);

  probe_ssbo.SetComparator(ProbeSSBOCommand::Comparator::kEqual);

  std::vector<Value> values;
  values.emplace_back();
  values.back().SetIntValue(13);
  probe_ssbo.SetValues(std::move(values));

  uint8_t ssbo = 13U;

  Verifier verifier;
  Result r = verifier.ProbeSSBO(&probe_ssbo, sizeof(uint8_t),
                                static_cast<const void*>(&ssbo));
  EXPECT_TRUE(r.IsSuccess());
}

TEST_F(VerifierTest, ProbeSSBOUint8Multiple) {
  ProbeSSBOCommand probe_ssbo;

  DatumType datum_type;
  datum_type.SetType(DataType::kUint8);
  probe_ssbo.SetDatumType(datum_type);

  probe_ssbo.SetComparator(ProbeSSBOCommand::Comparator::kEqual);

  std::vector<Value> values;
  values.resize(3);
  values[0].SetIntValue(2);
  values[1].SetIntValue(0);
  values[2].SetIntValue(10);
  probe_ssbo.SetValues(std::move(values));

  const uint8_t ssbo[3] = {2U, 0U, 10U};

  Verifier verifier;
  Result r = verifier.ProbeSSBO(&probe_ssbo, sizeof(uint8_t) * 3, ssbo);
  EXPECT_TRUE(r.IsSuccess());
}

TEST_F(VerifierTest, ProbeSSBOUint8Many) {
  ProbeSSBOCommand probe_ssbo;

  DatumType datum_type;
  datum_type.SetType(DataType::kUint8);
  probe_ssbo.SetDatumType(datum_type);

  probe_ssbo.SetComparator(ProbeSSBOCommand::Comparator::kEqual);

  std::vector<Value> values;
  values.resize(200);
  for (size_t i = 0; i < values.size(); ++i) {
    values[i].SetIntValue(255 - i);
  }
  probe_ssbo.SetValues(std::move(values));

  std::vector<uint8_t> ssbo;
  ssbo.resize(200);
  for (size_t i = 0; i < ssbo.size(); ++i) {
    ssbo[i] = static_cast<uint8_t>(255U) - static_cast<uint8_t>(i);
  }

  Verifier verifier;
  Result r = verifier.ProbeSSBO(&probe_ssbo, sizeof(uint8_t) * ssbo.size(),
                                ssbo.data());
  EXPECT_TRUE(r.IsSuccess());
}

TEST_F(VerifierTest, ProbeSSBOUint32Single) {
  ProbeSSBOCommand probe_ssbo;

  DatumType datum_type;
  datum_type.SetType(DataType::kUint32);
  probe_ssbo.SetDatumType(datum_type);

  probe_ssbo.SetComparator(ProbeSSBOCommand::Comparator::kEqual);

  std::vector<Value> values;
  values.emplace_back();
  values.back().SetIntValue(13);
  probe_ssbo.SetValues(std::move(values));

  uint32_t ssbo = 13U;

  Verifier verifier;
  Result r = verifier.ProbeSSBO(&probe_ssbo, sizeof(uint32_t),
                                static_cast<const void*>(&ssbo));
  EXPECT_TRUE(r.IsSuccess());
}

TEST_F(VerifierTest, ProbeSSBOUint32Multiple) {
  ProbeSSBOCommand probe_ssbo;

  DatumType datum_type;
  datum_type.SetType(DataType::kUint32);
  probe_ssbo.SetDatumType(datum_type);

  probe_ssbo.SetComparator(ProbeSSBOCommand::Comparator::kEqual);

  std::vector<Value> values;
  values.resize(4);
  values[0].SetIntValue(2);
  values[1].SetIntValue(0);
  values[2].SetIntValue(10);
  values[3].SetIntValue(1234);
  probe_ssbo.SetValues(std::move(values));

  const uint32_t ssbo[4] = {2U, 0U, 10U, 1234U};

  Verifier verifier;
  Result r = verifier.ProbeSSBO(&probe_ssbo, sizeof(uint32_t) * 4, ssbo);
  EXPECT_TRUE(r.IsSuccess());
}

TEST_F(VerifierTest, ProbeSSBOUint32Many) {
  ProbeSSBOCommand probe_ssbo;

  DatumType datum_type;
  datum_type.SetType(DataType::kUint32);
  probe_ssbo.SetDatumType(datum_type);

  probe_ssbo.SetComparator(ProbeSSBOCommand::Comparator::kEqual);

  std::vector<Value> values;
  values.resize(200);
  for (size_t i = 0; i < values.size(); ++i) {
    values[i].SetIntValue(i * i);
  }
  probe_ssbo.SetValues(std::move(values));

  std::vector<uint32_t> ssbo;
  ssbo.resize(200);
  for (size_t i = 0; i < ssbo.size(); ++i) {
    ssbo[i] = static_cast<uint32_t>(i) * static_cast<uint32_t>(i);
  }

  Verifier verifier;
  Result r = verifier.ProbeSSBO(&probe_ssbo, sizeof(uint32_t) * ssbo.size(),
                                ssbo.data());
  EXPECT_TRUE(r.IsSuccess());
}

TEST_F(VerifierTest, ProbeSSBOFloatSingle) {
  ProbeSSBOCommand probe_ssbo;

  DatumType datum_type;
  datum_type.SetType(DataType::kFloat);
  probe_ssbo.SetDatumType(datum_type);

  probe_ssbo.SetComparator(ProbeSSBOCommand::Comparator::kEqual);

  std::vector<Value> values;
  values.emplace_back();
  values.back().SetDoubleValue(13.7);
  probe_ssbo.SetValues(std::move(values));

  float ssbo = 13.7f;

  Verifier verifier;
  Result r = verifier.ProbeSSBO(&probe_ssbo, sizeof(float),
                                static_cast<const void*>(&ssbo));
  EXPECT_TRUE(r.IsSuccess());
}

TEST_F(VerifierTest, ProbeSSBOFloatMultiple) {
  ProbeSSBOCommand probe_ssbo;

  DatumType datum_type;
  datum_type.SetType(DataType::kFloat);
  probe_ssbo.SetDatumType(datum_type);

  probe_ssbo.SetComparator(ProbeSSBOCommand::Comparator::kEqual);

  std::vector<Value> values;
  values.resize(4);
  values[0].SetDoubleValue(2.9);
  values[1].SetDoubleValue(0.73);
  values[2].SetDoubleValue(10.0);
  values[3].SetDoubleValue(1234.56);
  probe_ssbo.SetValues(std::move(values));

  const float ssbo[4] = {2.9f, 0.73f, 10.0f, 1234.56f};

  Verifier verifier;
  Result r = verifier.ProbeSSBO(&probe_ssbo, sizeof(float) * 4, ssbo);
  EXPECT_TRUE(r.IsSuccess());
}

TEST_F(VerifierTest, ProbeSSBOFloatMany) {
  ProbeSSBOCommand probe_ssbo;

  DatumType datum_type;
  datum_type.SetType(DataType::kFloat);
  probe_ssbo.SetDatumType(datum_type);

  probe_ssbo.SetComparator(ProbeSSBOCommand::Comparator::kEqual);

  std::vector<Value> values;
  values.resize(200);
  for (size_t i = 0; i < values.size(); ++i) {
    values[i].SetDoubleValue(static_cast<double>(i) / 1.7);
  }
  probe_ssbo.SetValues(std::move(values));

  std::vector<float> ssbo;
  ssbo.resize(200);
  for (size_t i = 0; i < ssbo.size(); ++i) {
    ssbo[i] = static_cast<float>(static_cast<double>(i) / 1.7);
  }

  Verifier verifier;
  Result r =
      verifier.ProbeSSBO(&probe_ssbo, sizeof(float) * ssbo.size(), ssbo.data());
  EXPECT_TRUE(r.IsSuccess());
}

TEST_F(VerifierTest, ProbeSSBODoubleSingle) {
  ProbeSSBOCommand probe_ssbo;

  DatumType datum_type;
  datum_type.SetType(DataType::kDouble);
  probe_ssbo.SetDatumType(datum_type);

  probe_ssbo.SetComparator(ProbeSSBOCommand::Comparator::kEqual);

  std::vector<Value> values;
  values.emplace_back();
  values.back().SetDoubleValue(13.7);
  probe_ssbo.SetValues(std::move(values));

  double ssbo = 13.7;

  Verifier verifier;
  Result r = verifier.ProbeSSBO(&probe_ssbo, sizeof(double),
                                static_cast<const void*>(&ssbo));
  EXPECT_TRUE(r.IsSuccess());
}

TEST_F(VerifierTest, ProbeSSBODoubleMultiple) {
  ProbeSSBOCommand probe_ssbo;

  DatumType datum_type;
  datum_type.SetType(DataType::kDouble);
  probe_ssbo.SetDatumType(datum_type);

  probe_ssbo.SetComparator(ProbeSSBOCommand::Comparator::kEqual);

  std::vector<Value> values;
  values.resize(4);
  values[0].SetDoubleValue(2.9);
  values[1].SetDoubleValue(0.73);
  values[2].SetDoubleValue(10.0);
  values[3].SetDoubleValue(1234.56);
  probe_ssbo.SetValues(std::move(values));

  const double ssbo[4] = {2.9, 0.73, 10.0, 1234.56};

  Verifier verifier;
  Result r = verifier.ProbeSSBO(&probe_ssbo, sizeof(double) * 4, ssbo);
  EXPECT_TRUE(r.IsSuccess());
}

TEST_F(VerifierTest, ProbeSSBODoubleMany) {
  ProbeSSBOCommand probe_ssbo;

  DatumType datum_type;
  datum_type.SetType(DataType::kDouble);
  probe_ssbo.SetDatumType(datum_type);

  probe_ssbo.SetComparator(ProbeSSBOCommand::Comparator::kEqual);

  std::vector<Value> values;
  values.resize(200);
  for (size_t i = 0; i < values.size(); ++i) {
    values[i].SetDoubleValue(static_cast<double>(i) / 1.7);
  }
  probe_ssbo.SetValues(std::move(values));

  std::vector<double> ssbo;
  ssbo.resize(200);
  for (size_t i = 0; i < ssbo.size(); ++i) {
    ssbo[i] = static_cast<double>(i) / 1.7;
  }

  Verifier verifier;
  Result r = verifier.ProbeSSBO(&probe_ssbo, sizeof(double) * ssbo.size(),
                                ssbo.data());
  EXPECT_TRUE(r.IsSuccess());
}

TEST_F(VerifierTest, ProbeSSBOEqualFail) {
  ProbeSSBOCommand probe_ssbo;

  DatumType datum_type;
  datum_type.SetType(DataType::kDouble);
  probe_ssbo.SetDatumType(datum_type);

  probe_ssbo.SetComparator(ProbeSSBOCommand::Comparator::kEqual);

  std::vector<Value> values;
  values.resize(4);
  values[0].SetDoubleValue(2.9);
  values[1].SetDoubleValue(0.73);
  values[2].SetDoubleValue(10.0);
  values[3].SetDoubleValue(1234.56);
  probe_ssbo.SetValues(std::move(values));

  const double ssbo[4] = {2.8, 0.72, 9.0, 1234.55};

  Verifier verifier;
  Result r = verifier.ProbeSSBO(&probe_ssbo, sizeof(double) * 4, ssbo);
  EXPECT_FALSE(r.IsSuccess());
  EXPECT_EQ("Line 1: Verifier failed: 2.800000 == 2.900000, at index 0",
            r.Error());
}

TEST_F(VerifierTest, ProbeSSBOFuzzyEqualWithAbsoluteTolerance) {
  ProbeSSBOCommand probe_ssbo;

  DatumType datum_type;
  datum_type.SetType(DataType::kDouble);
  probe_ssbo.SetDatumType(datum_type);

  probe_ssbo.SetComparator(ProbeSSBOCommand::Comparator::kFuzzyEqual);

  std::vector<Probe::Tolerance> tolerances;
  tolerances.emplace_back(false, 0.1);
  probe_ssbo.SetTolerances(tolerances);

  std::vector<Value> values;
  values.resize(4);
  values[0].SetDoubleValue(2.9);
  values[1].SetDoubleValue(0.73);
  values[2].SetDoubleValue(10.0);
  values[3].SetDoubleValue(1234.56);
  probe_ssbo.SetValues(std::move(values));

  const double ssbo_more[4] = {2.999, 0.829, 10.099, 1234.659};

  Verifier verifier;
  Result r = verifier.ProbeSSBO(&probe_ssbo, sizeof(double) * 4, ssbo_more);
  EXPECT_TRUE(r.IsSuccess());

  const double ssbo_less[4] = {2.801, 0.631, 9.901, 1234.461};

  r = verifier.ProbeSSBO(&probe_ssbo, sizeof(double) * 4, ssbo_less);
  EXPECT_TRUE(r.IsSuccess());
}

TEST_F(VerifierTest, ProbeSSBOFuzzyEqualWithAbsoluteToleranceFail) {
  ProbeSSBOCommand probe_ssbo;

  DatumType datum_type;
  datum_type.SetType(DataType::kDouble);
  probe_ssbo.SetDatumType(datum_type);

  probe_ssbo.SetComparator(ProbeSSBOCommand::Comparator::kFuzzyEqual);

  std::vector<Probe::Tolerance> tolerances;
  tolerances.emplace_back(false, 0.1);
  probe_ssbo.SetTolerances(tolerances);

  std::vector<Value> values;
  values.resize(4);
  values[0].SetDoubleValue(2.9);
  values[1].SetDoubleValue(0.73);
  values[2].SetDoubleValue(10.0);
  values[3].SetDoubleValue(1234.56);
  probe_ssbo.SetValues(std::move(values));

  const double ssbo[4] = {3.001, 0.831, 10.101, 1234.661};

  Verifier verifier;
  Result r = verifier.ProbeSSBO(&probe_ssbo, sizeof(double) * 4, ssbo);
  EXPECT_FALSE(r.IsSuccess());
  EXPECT_EQ("Line 1: Verifier failed: 3.001000 ~= 2.900000, at index 0",
            r.Error());
}

TEST_F(VerifierTest, ProbeSSBOFuzzyEqualWithRelativeTolerance) {
  ProbeSSBOCommand probe_ssbo;

  DatumType datum_type;
  datum_type.SetType(DataType::kDouble);
  probe_ssbo.SetDatumType(datum_type);

  probe_ssbo.SetComparator(ProbeSSBOCommand::Comparator::kFuzzyEqual);

  std::vector<Probe::Tolerance> tolerances;
  tolerances.emplace_back(true, 0.1);
  probe_ssbo.SetTolerances(tolerances);

  std::vector<Value> values;
  values.resize(4);
  values[0].SetDoubleValue(2.9);
  values[1].SetDoubleValue(0.73);
  values[2].SetDoubleValue(10.0);
  values[3].SetDoubleValue(1234.56);
  probe_ssbo.SetValues(std::move(values));

  const double ssbo_more[4] = {2.9028, 0.73072, 10.009, 1235.79455};

  Verifier verifier;
  Result r = verifier.ProbeSSBO(&probe_ssbo, sizeof(double) * 4, ssbo_more);
  EXPECT_TRUE(r.IsSuccess());

  const double ssbo_less[4] = {2.8972, 0.72928, 9.991, 1233.32545};

  r = verifier.ProbeSSBO(&probe_ssbo, sizeof(double) * 4, ssbo_less);
  EXPECT_TRUE(r.IsSuccess());
}

TEST_F(VerifierTest, ProbeSSBOFuzzyEqualWithRelativeToleranceFail) {
  ProbeSSBOCommand probe_ssbo;

  DatumType datum_type;
  datum_type.SetType(DataType::kDouble);
  probe_ssbo.SetDatumType(datum_type);

  probe_ssbo.SetComparator(ProbeSSBOCommand::Comparator::kFuzzyEqual);

  std::vector<Probe::Tolerance> tolerances;
  tolerances.emplace_back(true, 0.1);
  probe_ssbo.SetTolerances(tolerances);

  std::vector<Value> values;
  values.resize(4);
  values[0].SetDoubleValue(2.9);
  values[1].SetDoubleValue(0.73);
  values[2].SetDoubleValue(10.0);
  values[3].SetDoubleValue(1234.56);
  probe_ssbo.SetValues(std::move(values));

  const double ssbo[4] = {2.903, 0.73074, 10.011, 1235.79457};

  Verifier verifier;
  Result r = verifier.ProbeSSBO(&probe_ssbo, sizeof(double) * 4, ssbo);
  EXPECT_FALSE(r.IsSuccess());
  EXPECT_EQ("Line 1: Verifier failed: 2.903000 ~= 2.900000, at index 0",
            r.Error());
}

TEST_F(VerifierTest, ProbeSSBONotEqual) {
  ProbeSSBOCommand probe_ssbo;

  DatumType datum_type;
  datum_type.SetType(DataType::kDouble);
  probe_ssbo.SetDatumType(datum_type);

  probe_ssbo.SetComparator(ProbeSSBOCommand::Comparator::kNotEqual);

  std::vector<Value> values;
  values.resize(4);
  values[0].SetDoubleValue(2.9);
  values[1].SetDoubleValue(0.73);
  values[2].SetDoubleValue(10.0);
  values[3].SetDoubleValue(1234.56);
  probe_ssbo.SetValues(std::move(values));

  const double ssbo[4] = {3.9, 0.83, 10.1, 1234.57};

  Verifier verifier;
  Result r = verifier.ProbeSSBO(&probe_ssbo, sizeof(double) * 4, ssbo);
  EXPECT_TRUE(r.IsSuccess());
}

TEST_F(VerifierTest, ProbeSSBONotEqualFail) {
  ProbeSSBOCommand probe_ssbo;

  DatumType datum_type;
  datum_type.SetType(DataType::kDouble);
  probe_ssbo.SetDatumType(datum_type);

  probe_ssbo.SetComparator(ProbeSSBOCommand::Comparator::kNotEqual);

  std::vector<Value> values;
  values.resize(4);
  values[0].SetDoubleValue(2.9);
  values[1].SetDoubleValue(0.73);
  values[2].SetDoubleValue(10.0);
  values[3].SetDoubleValue(1234.56);
  probe_ssbo.SetValues(std::move(values));

  const double ssbo[4] = {2.9, 0.73, 10.0, 1234.56};

  Verifier verifier;
  Result r = verifier.ProbeSSBO(&probe_ssbo, sizeof(double) * 4, ssbo);
  EXPECT_FALSE(r.IsSuccess());
  EXPECT_EQ("Line 1: Verifier failed: 2.900000 != 2.900000, at index 0",
            r.Error());
}

TEST_F(VerifierTest, ProbeSSBOLess) {
  ProbeSSBOCommand probe_ssbo;

  DatumType datum_type;
  datum_type.SetType(DataType::kDouble);
  probe_ssbo.SetDatumType(datum_type);

  probe_ssbo.SetComparator(ProbeSSBOCommand::Comparator::kLess);

  std::vector<Value> values;
  values.resize(4);
  values[0].SetDoubleValue(2.9);
  values[1].SetDoubleValue(0.73);
  values[2].SetDoubleValue(10.0);
  values[3].SetDoubleValue(1234.56);
  probe_ssbo.SetValues(std::move(values));

  const double ssbo[4] = {1.9, 0.63, 9.99, 1234.559};

  Verifier verifier;
  Result r = verifier.ProbeSSBO(&probe_ssbo, sizeof(double) * 4, ssbo);
  EXPECT_TRUE(r.IsSuccess());
}

TEST_F(VerifierTest, ProbeSSBOLessFail) {
  ProbeSSBOCommand probe_ssbo;

  DatumType datum_type;
  datum_type.SetType(DataType::kDouble);
  probe_ssbo.SetDatumType(datum_type);

  probe_ssbo.SetComparator(ProbeSSBOCommand::Comparator::kLess);

  std::vector<Value> values;
  values.resize(4);
  values[0].SetDoubleValue(2.9);
  values[1].SetDoubleValue(0.73);
  values[2].SetDoubleValue(10.0);
  values[3].SetDoubleValue(1234.56);
  probe_ssbo.SetValues(std::move(values));

  const double ssbo[4] = {3.9, 0.83, 10.1, 1234.57};

  Verifier verifier;
  Result r = verifier.ProbeSSBO(&probe_ssbo, sizeof(double) * 4, ssbo);
  EXPECT_FALSE(r.IsSuccess());
  EXPECT_EQ("Line 1: Verifier failed: 3.900000 < 2.900000, at index 0",
            r.Error());
}

TEST_F(VerifierTest, ProbeSSBOLessOrEqual) {
  ProbeSSBOCommand probe_ssbo;

  DatumType datum_type;
  datum_type.SetType(DataType::kDouble);
  probe_ssbo.SetDatumType(datum_type);

  probe_ssbo.SetComparator(ProbeSSBOCommand::Comparator::kLessOrEqual);

  std::vector<Value> values;
  values.resize(4);
  values[0].SetDoubleValue(2.9);
  values[1].SetDoubleValue(0.73);
  values[2].SetDoubleValue(10.0);
  values[3].SetDoubleValue(1234.56);
  probe_ssbo.SetValues(std::move(values));

  const double ssbo[4] = {1.9, 0.73, 9.99, 1234.560};

  Verifier verifier;
  Result r = verifier.ProbeSSBO(&probe_ssbo, sizeof(double) * 4, ssbo);
  EXPECT_TRUE(r.IsSuccess());
}

TEST_F(VerifierTest, ProbeSSBOLessOrEqualFail) {
  ProbeSSBOCommand probe_ssbo;

  DatumType datum_type;
  datum_type.SetType(DataType::kDouble);
  probe_ssbo.SetDatumType(datum_type);

  probe_ssbo.SetComparator(ProbeSSBOCommand::Comparator::kLessOrEqual);

  std::vector<Value> values;
  values.resize(4);
  values[0].SetDoubleValue(2.9);
  values[1].SetDoubleValue(0.73);
  values[2].SetDoubleValue(10.0);
  values[3].SetDoubleValue(1234.56);
  probe_ssbo.SetValues(std::move(values));

  const double ssbo[4] = {1.9, 0.73, 9.99, 1234.561};

  Verifier verifier;
  Result r = verifier.ProbeSSBO(&probe_ssbo, sizeof(double) * 4, ssbo);
  EXPECT_FALSE(r.IsSuccess());
  EXPECT_EQ("Line 1: Verifier failed: 1234.561000 <= 1234.560000, at index 3",
            r.Error());
}

TEST_F(VerifierTest, ProbeSSBOGreater) {
  ProbeSSBOCommand probe_ssbo;

  DatumType datum_type;
  datum_type.SetType(DataType::kDouble);
  probe_ssbo.SetDatumType(datum_type);

  probe_ssbo.SetComparator(ProbeSSBOCommand::Comparator::kGreater);

  std::vector<Value> values;
  values.resize(4);
  values[0].SetDoubleValue(2.9);
  values[1].SetDoubleValue(0.73);
  values[2].SetDoubleValue(10.0);
  values[3].SetDoubleValue(1234.56);
  probe_ssbo.SetValues(std::move(values));

  const double ssbo[4] = {3.9, 0.83, 10.1, 1234.57};

  Verifier verifier;
  Result r = verifier.ProbeSSBO(&probe_ssbo, sizeof(double) * 4, ssbo);
  EXPECT_TRUE(r.IsSuccess());
}

TEST_F(VerifierTest, ProbeSSBOGreaterFail) {
  ProbeSSBOCommand probe_ssbo;

  DatumType datum_type;
  datum_type.SetType(DataType::kDouble);
  probe_ssbo.SetDatumType(datum_type);

  probe_ssbo.SetComparator(ProbeSSBOCommand::Comparator::kGreater);

  std::vector<Value> values;
  values.resize(4);
  values[0].SetDoubleValue(2.9);
  values[1].SetDoubleValue(0.73);
  values[2].SetDoubleValue(10.0);
  values[3].SetDoubleValue(1234.56);
  probe_ssbo.SetValues(std::move(values));

  const double ssbo[4] = {3.9, 0.73, 10.1, 1234.57};

  Verifier verifier;
  Result r = verifier.ProbeSSBO(&probe_ssbo, sizeof(double) * 4, ssbo);
  EXPECT_FALSE(r.IsSuccess());
  EXPECT_EQ("Line 1: Verifier failed: 0.730000 > 0.730000, at index 1",
            r.Error());
}

TEST_F(VerifierTest, ProbeSSBOGreaterOrEqual) {
  ProbeSSBOCommand probe_ssbo;

  DatumType datum_type;
  datum_type.SetType(DataType::kDouble);
  probe_ssbo.SetDatumType(datum_type);

  probe_ssbo.SetComparator(ProbeSSBOCommand::Comparator::kGreaterOrEqual);

  std::vector<Value> values;
  values.resize(4);
  values[0].SetDoubleValue(2.9);
  values[1].SetDoubleValue(0.73);
  values[2].SetDoubleValue(10.0);
  values[3].SetDoubleValue(1234.56);
  probe_ssbo.SetValues(std::move(values));

  const double ssbo[4] = {3.9, 0.73, 10.1, 1234.56};

  Verifier verifier;
  Result r = verifier.ProbeSSBO(&probe_ssbo, sizeof(double) * 4, ssbo);
  EXPECT_TRUE(r.IsSuccess());
}

TEST_F(VerifierTest, ProbeSSBOGreaterOrEqualFail) {
  ProbeSSBOCommand probe_ssbo;

  DatumType datum_type;
  datum_type.SetType(DataType::kDouble);
  probe_ssbo.SetDatumType(datum_type);

  probe_ssbo.SetComparator(ProbeSSBOCommand::Comparator::kGreaterOrEqual);

  std::vector<Value> values;
  values.resize(4);
  values[0].SetDoubleValue(2.9);
  values[1].SetDoubleValue(0.73);
  values[2].SetDoubleValue(10.0);
  values[3].SetDoubleValue(1234.56);
  probe_ssbo.SetValues(std::move(values));

  const double ssbo[4] = {3.9, 0.73, 10.1, 1234.559};

  Verifier verifier;
  Result r = verifier.ProbeSSBO(&probe_ssbo, sizeof(double) * 4, ssbo);
  EXPECT_FALSE(r.IsSuccess());
  EXPECT_EQ("Line 1: Verifier failed: 1234.559000 >= 1234.560000, at index 3",
            r.Error());
}

}  // namespace amber
