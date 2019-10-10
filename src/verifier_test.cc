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

#include "src/verifier.h"

#include <memory>
#include <utility>
#include <vector>

#include "amber/result.h"
#include "amber/value.h"
#include "gtest/gtest.h"
#include "src/command.h"
#include "src/make_unique.h"
#include "src/pipeline.h"
#include "src/type_parser.h"

namespace amber {
namespace {

class VerifierTest : public testing::Test {
 public:
  VerifierTest() = default;
  ~VerifierTest() = default;

  const Format* GetColorFormat() {
    if (color_frame_format_)
      return color_frame_format_.get();

    TypeParser parser;
    color_frame_type_ = parser.Parse("B8G8R8A8_UNORM");

    color_frame_format_ = MakeUnique<Format>(color_frame_type_.get());
    return color_frame_format_.get();
  }

 private:
  std::unique_ptr<type::Type> color_frame_type_;
  std::unique_ptr<Format> color_frame_format_;
};

}  // namespace

TEST_F(VerifierTest, ProbeFrameBufferWholeWindow) {
  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();

  ProbeCommand probe(color_buf.get());
  probe.SetWholeWindow();
  probe.SetProbeRect();
  probe.SetIsRGBA();
  probe.SetB(0.5f);
  probe.SetG(0.25f);
  probe.SetR(0.2f);
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
  EXPECT_TRUE(r.IsSuccess()) << r.Error();
}

TEST_F(VerifierTest, ProbeFrameBufferRelative) {
  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();

  ProbeCommand probe(color_buf.get());
  probe.SetProbeRect();
  probe.SetRelative();
  probe.SetIsRGBA();
  probe.SetX(0.1f);
  probe.SetY(0.2f);
  probe.SetWidth(0.4f);
  probe.SetHeight(0.6f);
  probe.SetB(0.5f);
  probe.SetG(0.25f);
  probe.SetR(0.2f);
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
  EXPECT_TRUE(r.IsSuccess()) << r.Error();
}

TEST_F(VerifierTest, ProbeFrameBufferRelativeSmallExpectFail) {
  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();

  ProbeCommand probe(color_buf.get());
  probe.SetProbeRect();
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
  EXPECT_EQ(
      "Line 1: Probe failed at: 225, 225\n  Expected: 25.500000, "
      "0.000000, 0.000000, 0.000000\n    Actual: 0.000000, 0.000000, "
      "0.000000, 0.000000\nProbe failed in 625 pixels",
      r.Error());
}

TEST_F(VerifierTest, ProbeFrameBuffer) {
  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();

  ProbeCommand probe(color_buf.get());
  probe.SetProbeRect();
  probe.SetIsRGBA();
  probe.SetX(1.0f);
  probe.SetY(2.0f);
  probe.SetWidth(4.0f);
  probe.SetHeight(6.0f);
  probe.SetB(0.5f);
  probe.SetG(0.25f);
  probe.SetR(0.2f);
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
  EXPECT_TRUE(r.IsSuccess()) << r.Error();
}

TEST_F(VerifierTest, ProbeFrameBufferUInt8) {
  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();

  ProbeCommand probe(color_buf.get());
  probe.SetIsRGBA();
  probe.SetX(0.0f);
  probe.SetY(0.0f);
  probe.SetR(255);
  probe.SetG(14);
  probe.SetB(75);
  probe.SetA(8);

  uint8_t frame_buffer[4] = {255, 14, 75, 8};

  TypeParser parser;
  auto type = parser.Parse("R8G8B8A8_UINT");
  Format fmt(type.get());

  Verifier verifier;
  Result r =
      verifier.Probe(&probe, &fmt, 4 * static_cast<uint32_t>(sizeof(uint8_t)),
                     4 * static_cast<uint32_t>(sizeof(uint8_t)), 1, 1,
                     static_cast<const void*>(&frame_buffer));
  EXPECT_TRUE(r.IsSuccess());
}

TEST_F(VerifierTest, ProbeFrameBufferUInt16) {
  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();

  ProbeCommand probe(color_buf.get());
  probe.SetIsRGBA();
  probe.SetX(0.0f);
  probe.SetY(0.0f);
  probe.SetR(65535);
  probe.SetG(14);
  probe.SetB(1875);
  probe.SetA(8);

  uint16_t frame_buffer[4] = {65535, 14, 1875, 8};

  TypeParser parser;
  auto type = parser.Parse("R16G16B16A16_UINT");
  Format fmt(type.get());

  Verifier verifier;
  Result r =
      verifier.Probe(&probe, &fmt, 4 * static_cast<uint32_t>(sizeof(uint16_t)),
                     4 * static_cast<uint32_t>(sizeof(uint16_t)), 1, 1,
                     static_cast<const void*>(&frame_buffer));
  EXPECT_TRUE(r.IsSuccess());
}

TEST_F(VerifierTest, ProbeFrameBufferUInt32) {
  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();

  ProbeCommand probe(color_buf.get());
  probe.SetIsRGBA();
  probe.SetX(0.0f);
  probe.SetY(0.0f);
  probe.SetR(6);
  probe.SetG(14);
  probe.SetB(1171875);
  probe.SetA(8);

  uint32_t frame_buffer[4] = {6, 14, 1171875, 8};

  TypeParser parser;
  auto type = parser.Parse("R32G32B32A32_UINT");
  Format fmt(type.get());

  Verifier verifier;
  Result r =
      verifier.Probe(&probe, &fmt, 4 * static_cast<uint32_t>(sizeof(uint32_t)),
                     4 * static_cast<uint32_t>(sizeof(uint32_t)), 1, 1,
                     static_cast<const void*>(&frame_buffer));
  EXPECT_TRUE(r.IsSuccess());
}

TEST_F(VerifierTest, ProbeFrameBufferUInt64) {
  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();

  ProbeCommand probe(color_buf.get());
  probe.SetIsRGBA();
  probe.SetX(0.0f);
  probe.SetY(0.0f);
  probe.SetR(6);
  probe.SetG(14);
  probe.SetB(1171875);
  probe.SetA(8);

  uint64_t frame_buffer[4] = {6, 14, 1171875, 8};

  TypeParser parser;
  auto type = parser.Parse("R64G64B64A64_UINT");
  Format fmt(type.get());

  Verifier verifier;
  Result r =
      verifier.Probe(&probe, &fmt, 4 * static_cast<uint32_t>(sizeof(uint64_t)),
                     4 * static_cast<uint32_t>(sizeof(uint64_t)), 1, 1,
                     static_cast<const void*>(&frame_buffer));
  EXPECT_TRUE(r.IsSuccess());
}

TEST_F(VerifierTest, ProbeFrameBufferSInt8) {
  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();

  ProbeCommand probe(color_buf.get());
  probe.SetIsRGBA();
  probe.SetX(0.0f);
  probe.SetY(0.0f);
  probe.SetR(-6);
  probe.SetG(14);
  probe.SetB(75);
  probe.SetA(8);

  int8_t frame_buffer[4] = {-6, 14, 75, 8};

  TypeParser parser;
  auto type = parser.Parse("R8G8B8A8_SINT");
  Format fmt(type.get());

  Verifier verifier;
  Result r =
      verifier.Probe(&probe, &fmt, 4 * static_cast<uint32_t>(sizeof(int8_t)),
                     4 * static_cast<uint32_t>(sizeof(int8_t)), 1, 1,
                     static_cast<const void*>(&frame_buffer));
  EXPECT_TRUE(r.IsSuccess());
}

TEST_F(VerifierTest, ProbeFrameBufferSInt16) {
  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();

  ProbeCommand probe(color_buf.get());
  probe.SetIsRGBA();
  probe.SetX(0.0f);
  probe.SetY(0.0f);
  probe.SetR(-6);
  probe.SetG(14);
  probe.SetB(1875);
  probe.SetA(8);

  int16_t frame_buffer[4] = {-6, 14, 1875, 8};

  TypeParser parser;
  auto type = parser.Parse("R16G16B16A16_SINT");
  Format fmt(type.get());

  Verifier verifier;
  Result r =
      verifier.Probe(&probe, &fmt, 4 * static_cast<uint32_t>(sizeof(int16_t)),
                     4 * static_cast<uint32_t>(sizeof(int16_t)), 1, 1,
                     static_cast<const void*>(&frame_buffer));
  EXPECT_TRUE(r.IsSuccess());
}

TEST_F(VerifierTest, ProbeFrameBufferSInt32) {
  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();

  ProbeCommand probe(color_buf.get());
  probe.SetIsRGBA();
  probe.SetX(0.0f);
  probe.SetY(0.0f);
  probe.SetR(-6);
  probe.SetG(14);
  probe.SetB(1171875);
  probe.SetA(8);

  int32_t frame_buffer[4] = {-6, 14, 1171875, 8};

  TypeParser parser;
  auto type = parser.Parse("R32G32B32A32_SINT");
  Format fmt(type.get());

  Verifier verifier;
  Result r =
      verifier.Probe(&probe, &fmt, 4 * static_cast<uint32_t>(sizeof(int32_t)),
                     4 * static_cast<uint32_t>(sizeof(int32_t)), 1, 1,
                     static_cast<const void*>(&frame_buffer));
  EXPECT_TRUE(r.IsSuccess());
}

TEST_F(VerifierTest, ProbeFrameBufferSInt64) {
  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();

  ProbeCommand probe(color_buf.get());
  probe.SetIsRGBA();
  probe.SetX(0.0f);
  probe.SetY(0.0f);
  probe.SetR(-6);
  probe.SetG(14);
  probe.SetB(1171875);
  probe.SetA(8);

  int64_t frame_buffer[4] = {-6, 14, 1171875, 8};

  TypeParser parser;
  auto type = parser.Parse("R64G64B64A64_SINT");
  Format fmt(type.get());

  Verifier verifier;
  Result r =
      verifier.Probe(&probe, &fmt, 4 * static_cast<uint32_t>(sizeof(int64_t)),
                     4 * static_cast<uint32_t>(sizeof(int64_t)), 1, 1,
                     static_cast<const void*>(&frame_buffer));
  EXPECT_TRUE(r.IsSuccess());
}

TEST_F(VerifierTest, ProbeFrameBufferFloat32) {
  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();

  ProbeCommand probe(color_buf.get());
  probe.SetIsRGBA();
  probe.SetX(0.0f);
  probe.SetY(0.0f);
  probe.SetR(-6.0f);
  probe.SetG(14.0f);
  probe.SetB(0.1171875f);
  probe.SetA(0.8f);

  float frame_buffer[4] = {-6.0f, 14.0f, 0.1171875f, 0.8f};

  TypeParser parser;
  auto type = parser.Parse("R32G32B32A32_SFLOAT");
  Format fmt(type.get());

  Verifier verifier;
  Result r =
      verifier.Probe(&probe, &fmt, 4 * static_cast<uint32_t>(sizeof(float)),
                     4 * static_cast<uint32_t>(sizeof(float)), 1, 1,
                     static_cast<const void*>(&frame_buffer));
  EXPECT_TRUE(r.IsSuccess());
}

TEST_F(VerifierTest, ProbeFrameBufferFloat64) {
  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();

  ProbeCommand probe(color_buf.get());
  probe.SetIsRGBA();
  probe.SetX(0.0f);
  probe.SetY(0.0f);
  probe.SetR(-6.0f);
  probe.SetG(14.0f);
  probe.SetB(0.1171875f);
  probe.SetA(0.8f);

  double frame_buffer[4] = {-6.0, 14.0, 0.1171875, 0.8};

  TypeParser parser;
  auto type = parser.Parse("R64G64B64A64_SFLOAT");
  Format fmt(type.get());

  Verifier verifier;
  Result r =
      verifier.Probe(&probe, &fmt, 4 * static_cast<uint32_t>(sizeof(double)),
                     4 * static_cast<uint32_t>(sizeof(double)), 1, 1,
                     static_cast<const void*>(&frame_buffer));
  EXPECT_TRUE(r.IsSuccess());
}

TEST_F(VerifierTest, HexFloatToFloatR16G11B10) {
  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();

  ProbeCommand probe(color_buf.get());
  probe.SetX(0.0f);
  probe.SetY(0.0f);

  uint64_t frame_buffer = 0;

  // 16 bits float to float
  //   Sig / Exp / Mantissa     Sig / Exp / Mantissa
  //     1 /  17 /      512 -->   1 / 129 /  4194304 = -1.1(2) * 2^2 = -6
  frame_buffer = 50688ULL;
  probe.SetR(-6.0f);

  // 11 bits float to float
  //   Sig / Exp / Mantissa     Sig / Exp / Mantissa
  //     0 /  18 /       48 -->   0 / 130 / 12582912 = 1.11(2) * 2^3 = 14
  frame_buffer |= 1200ULL << 16ULL;
  probe.SetG(14.0f);

  // 10 bits float to float
  //   Sig / Exp / Mantissa     Sig / Exp / Mantissa
  //     0 /  11 /       28 -->   1 / 123 / 14680064 = 1.111(2) * 2^-4
  //                                                 = 0.1171875
  frame_buffer |= 380ULL << (16ULL + 11ULL);
  probe.SetB(0.1171875f);

  auto list = MakeUnique<type::List>();
  list->AddMember(FormatComponentType::kR, FormatMode::kSFloat, 16);
  list->AddMember(FormatComponentType::kG, FormatMode::kSFloat, 11);
  list->AddMember(FormatComponentType::kB, FormatMode::kSFloat, 10);

  Format format(list.get());

  Verifier verifier;
  Result r = verifier.Probe(&probe, &format, 6, 6, 1, 1,
                            static_cast<const void*>(&frame_buffer));
  EXPECT_TRUE(r.IsSuccess()) << r.Error();
}

TEST_F(VerifierTest, HexFloatToFloatR11G16B10) {
  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();

  ProbeCommand probe(color_buf.get());
  probe.SetX(0.0f);
  probe.SetY(0.0f);

  uint64_t frame_buffer = 0;

  // 11 bits float to float
  //   Sig / Exp / Mantissa     Sig / Exp / Mantissa
  //     0 /  18 /       48 -->   0 / 130 / 12582912 = 1.11(2) * 2^3 = 14
  frame_buffer = 1200ULL;
  probe.SetR(14.0f);

  // 16 bits float to float
  //   Sig / Exp / Mantissa     Sig / Exp / Mantissa
  //     1 /  17 /      512 -->   1 / 129 /  4194304 = -1.1(2) * 2^2 = -6
  frame_buffer |= 50688ULL << 11ULL;
  probe.SetG(-6.0f);

  // 10 bits float to float
  //   Sig / Exp / Mantissa     Sig / Exp / Mantissa
  //     0 /  11 /       28 -->   1 / 123 / 14680064 = 1.111(2) * 2^-4
  //                                                 = 0.1171875
  frame_buffer |= 380ULL << (16ULL + 11ULL);
  probe.SetB(0.1171875f);

  auto list = MakeUnique<type::List>();
  list->AddMember(FormatComponentType::kR, FormatMode::kSFloat, 11);
  list->AddMember(FormatComponentType::kG, FormatMode::kSFloat, 16);
  list->AddMember(FormatComponentType::kB, FormatMode::kSFloat, 10);

  Format format(list.get());

  Verifier verifier;
  Result r = verifier.Probe(&probe, &format, 6, 6, 1, 1,
                            static_cast<const void*>(&frame_buffer));
  EXPECT_TRUE(r.IsSuccess()) << r.Error();
}

TEST_F(VerifierTest, HexFloatToFloatR10G11B16) {
  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();

  ProbeCommand probe(color_buf.get());
  probe.SetX(0.0f);
  probe.SetY(0.0f);

  uint64_t frame_buffer = 0;

  // 10 bits float to float
  //   Sig / Exp / Mantissa     Sig / Exp / Mantissa
  //     0 /  11 /       28 -->   1 / 123 / 14680064 = 1.111(2) * 2^-4
  //                                                 = 0.1171875
  frame_buffer = 380ULL;
  probe.SetR(0.1171875f);

  // 11 bits float to float
  //   Sig / Exp / Mantissa     Sig / Exp / Mantissa
  //     0 /  18 /       48 -->   0 / 130 / 12582912 = 1.11(2) * 2^3 = 14
  frame_buffer |= 1200ULL << 10ULL;
  probe.SetG(14.0f);

  // 16 bits float to float
  //   Sig / Exp / Mantissa     Sig / Exp / Mantissa
  //     1 /  17 /      512 -->   1 / 129 /  4194304 = -1.1(2) * 2^2 = -6
  frame_buffer |= 50688ULL << (10ULL + 11ULL);
  probe.SetB(-6.0f);

  auto list = MakeUnique<type::List>();
  list->AddMember(FormatComponentType::kR, FormatMode::kSFloat, 10);
  list->AddMember(FormatComponentType::kG, FormatMode::kSFloat, 11);
  list->AddMember(FormatComponentType::kB, FormatMode::kSFloat, 16);

  Format format(list.get());

  Verifier verifier;
  Result r = verifier.Probe(&probe, &format, 6, 6, 1, 1,
                            static_cast<const void*>(&frame_buffer));
  EXPECT_TRUE(r.IsSuccess()) << r.Error();
}

TEST_F(VerifierTest, ProbeFrameBufferNotRect) {
  uint8_t frame_buffer[10][10][4] = {};

  frame_buffer[2][1][0] = 128;
  frame_buffer[2][1][1] = 64;
  frame_buffer[2][1][2] = 51;
  frame_buffer[2][1][3] = 204;

  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();

  ProbeCommand probe(color_buf.get());
  probe.SetIsRGBA();
  probe.SetX(1.0f);
  probe.SetY(2.0f);
  probe.SetB(0.5f);
  probe.SetG(0.25f);
  probe.SetR(0.2f);
  probe.SetA(0.8f);

  Verifier verifier;
  Result r = verifier.Probe(&probe, GetColorFormat(), 4, 40, 10, 10,
                            static_cast<const void*>(frame_buffer));
  EXPECT_TRUE(r.IsSuccess()) << r.Error();
}

TEST_F(VerifierTest, ProbeFrameBufferRGB) {
  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();

  ProbeCommand probe(color_buf.get());
  probe.SetWholeWindow();
  probe.SetProbeRect();
  probe.SetB(0.5f);
  probe.SetG(0.25f);
  probe.SetR(0.2f);

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
  EXPECT_TRUE(r.IsSuccess()) << r.Error();
}

TEST_F(VerifierTest, ProbeFrameBufferBadRowStride) {
  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();

  ProbeCommand probe(color_buf.get());
  probe.SetWholeWindow();
  probe.SetProbeRect();

  const uint8_t frame_buffer[4] = {128, 64, 51, 255};

  Verifier verifier;
  Result r = verifier.Probe(&probe, GetColorFormat(), 4, 3, 1, 1,
                            static_cast<const void*>(frame_buffer));
  EXPECT_FALSE(r.IsSuccess());
  EXPECT_EQ(
      "Line 1: Verifier::Probe Row stride of 3 is too small for 1 texels of 4 "
      "bytes each",
      r.Error());
}

TEST_F(VerifierTest, ProbeSSBOUint8Single) {
  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();

  ProbeSSBOCommand probe_ssbo(color_buf.get());

  TypeParser parser;
  auto type = parser.Parse("R8_UINT");
  Format fmt(type.get());

  probe_ssbo.SetFormat(&fmt);
  probe_ssbo.SetComparator(ProbeSSBOCommand::Comparator::kEqual);

  std::vector<Value> values;
  values.emplace_back();
  values.back().SetIntValue(13);
  probe_ssbo.SetValues(std::move(values));

  uint8_t ssbo = 13U;

  Verifier verifier;
  Result r =
      verifier.ProbeSSBO(&probe_ssbo, 1, static_cast<const void*>(&ssbo));
  EXPECT_TRUE(r.IsSuccess()) << r.Error();
}

TEST_F(VerifierTest, ProbeSSBOUint8Multiple) {
  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();

  ProbeSSBOCommand probe_ssbo(color_buf.get());

  TypeParser parser;
  auto type = parser.Parse("R8_UINT");
  Format fmt(type.get());

  probe_ssbo.SetFormat(&fmt);
  probe_ssbo.SetComparator(ProbeSSBOCommand::Comparator::kEqual);

  std::vector<Value> values;
  values.resize(3);
  values[0].SetIntValue(2);
  values[1].SetIntValue(0);
  values[2].SetIntValue(10);
  probe_ssbo.SetValues(std::move(values));

  const uint8_t ssbo[3] = {2U, 0U, 10U};

  Verifier verifier;
  Result r = verifier.ProbeSSBO(&probe_ssbo, 3, ssbo);
  EXPECT_TRUE(r.IsSuccess()) << r.Error();
}

TEST_F(VerifierTest, ProbeSSBOUint8Many) {
  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();

  ProbeSSBOCommand probe_ssbo(color_buf.get());

  TypeParser parser;
  auto type = parser.Parse("R8_UINT");
  Format fmt(type.get());

  probe_ssbo.SetFormat(&fmt);
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
  Result r = verifier.ProbeSSBO(&probe_ssbo, 200, ssbo.data());
  EXPECT_TRUE(r.IsSuccess()) << r.Error();
}

TEST_F(VerifierTest, ProbeSSBOUint32Single) {
  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();

  ProbeSSBOCommand probe_ssbo(color_buf.get());

  TypeParser parser;
  auto type = parser.Parse("R32_UINT");
  Format fmt(type.get());

  probe_ssbo.SetFormat(&fmt);
  probe_ssbo.SetComparator(ProbeSSBOCommand::Comparator::kEqual);

  std::vector<Value> values;
  values.emplace_back();
  values.back().SetIntValue(13);
  probe_ssbo.SetValues(std::move(values));

  uint32_t ssbo = 13U;

  Verifier verifier;
  Result r =
      verifier.ProbeSSBO(&probe_ssbo, 1, static_cast<const void*>(&ssbo));
  EXPECT_TRUE(r.IsSuccess()) << r.Error();
}

TEST_F(VerifierTest, ProbeSSBOUint32Multiple) {
  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();

  ProbeSSBOCommand probe_ssbo(color_buf.get());

  TypeParser parser;
  auto type = parser.Parse("R32_UINT");
  Format fmt(type.get());

  probe_ssbo.SetFormat(&fmt);
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
  Result r = verifier.ProbeSSBO(&probe_ssbo, 4, ssbo);
  EXPECT_TRUE(r.IsSuccess()) << r.Error();
}

TEST_F(VerifierTest, ProbeSSBOUint32Many) {
  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();

  ProbeSSBOCommand probe_ssbo(color_buf.get());

  TypeParser parser;
  auto type = parser.Parse("R32_UINT");
  Format fmt(type.get());

  probe_ssbo.SetFormat(&fmt);
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
  Result r = verifier.ProbeSSBO(&probe_ssbo, 200, ssbo.data());
  EXPECT_TRUE(r.IsSuccess()) << r.Error();
}

TEST_F(VerifierTest, ProbeSSBOFloatSingle) {
  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();

  ProbeSSBOCommand probe_ssbo(color_buf.get());

  TypeParser parser;
  auto type = parser.Parse("R32_SFLOAT");
  Format fmt(type.get());

  probe_ssbo.SetFormat(&fmt);
  probe_ssbo.SetComparator(ProbeSSBOCommand::Comparator::kEqual);

  std::vector<Value> values;
  values.emplace_back();
  values.back().SetDoubleValue(13.7);
  probe_ssbo.SetValues(std::move(values));

  float ssbo = 13.7f;

  Verifier verifier;
  Result r =
      verifier.ProbeSSBO(&probe_ssbo, 1, static_cast<const void*>(&ssbo));
  EXPECT_TRUE(r.IsSuccess()) << r.Error();
}

TEST_F(VerifierTest, ProbeSSBOFloatMultiple) {
  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();

  ProbeSSBOCommand probe_ssbo(color_buf.get());

  TypeParser parser;
  auto type = parser.Parse("R32_SFLOAT");
  Format fmt(type.get());

  probe_ssbo.SetFormat(&fmt);
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
  Result r = verifier.ProbeSSBO(&probe_ssbo, 4, ssbo);
  EXPECT_TRUE(r.IsSuccess()) << r.Error();
}

TEST_F(VerifierTest, ProbeSSBOFloatMany) {
  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();

  ProbeSSBOCommand probe_ssbo(color_buf.get());

  TypeParser parser;
  auto type = parser.Parse("R32_SFLOAT");
  Format fmt(type.get());

  probe_ssbo.SetFormat(&fmt);
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
  Result r = verifier.ProbeSSBO(&probe_ssbo, 200, ssbo.data());
  EXPECT_TRUE(r.IsSuccess()) << r.Error();
}

TEST_F(VerifierTest, ProbeSSBODoubleSingle) {
  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();

  ProbeSSBOCommand probe_ssbo(color_buf.get());

  TypeParser parser;
  auto type = parser.Parse("R64_SFLOAT");
  Format fmt(type.get());

  probe_ssbo.SetFormat(&fmt);
  probe_ssbo.SetComparator(ProbeSSBOCommand::Comparator::kEqual);

  std::vector<Value> values;
  values.emplace_back();
  values.back().SetDoubleValue(13.7);
  probe_ssbo.SetValues(std::move(values));

  double ssbo = 13.7;

  Verifier verifier;
  Result r =
      verifier.ProbeSSBO(&probe_ssbo, 1, static_cast<const void*>(&ssbo));
  EXPECT_TRUE(r.IsSuccess()) << r.Error();
}

TEST_F(VerifierTest, ProbeSSBODoubleMultiple) {
  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();

  ProbeSSBOCommand probe_ssbo(color_buf.get());

  TypeParser parser;
  auto type = parser.Parse("R64_SFLOAT");
  Format fmt(type.get());

  probe_ssbo.SetFormat(&fmt);
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
  Result r = verifier.ProbeSSBO(&probe_ssbo, 4, ssbo);
  EXPECT_TRUE(r.IsSuccess()) << r.Error();
}

TEST_F(VerifierTest, ProbeSSBODoubleMany) {
  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();

  ProbeSSBOCommand probe_ssbo(color_buf.get());

  TypeParser parser;
  auto type = parser.Parse("R64_SFLOAT");
  Format fmt(type.get());

  probe_ssbo.SetFormat(&fmt);
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
  Result r = verifier.ProbeSSBO(&probe_ssbo, 200, ssbo.data());
  EXPECT_TRUE(r.IsSuccess()) << r.Error();
}

TEST_F(VerifierTest, ProbeSSBOEqualFail) {
  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();

  ProbeSSBOCommand probe_ssbo(color_buf.get());

  TypeParser parser;
  auto type = parser.Parse("R64_SFLOAT");
  Format fmt(type.get());

  probe_ssbo.SetFormat(&fmt);
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
  Result r = verifier.ProbeSSBO(&probe_ssbo, 4, ssbo);
  EXPECT_FALSE(r.IsSuccess());
  EXPECT_EQ("Line 1: Verifier failed: 2.800000 == 2.900000, at index 0",
            r.Error());
}

TEST_F(VerifierTest, ProbeSSBOFuzzyEqualWithAbsoluteTolerance) {
  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();

  ProbeSSBOCommand probe_ssbo(color_buf.get());

  TypeParser parser;
  auto type = parser.Parse("R64_SFLOAT");
  Format fmt(type.get());

  probe_ssbo.SetFormat(&fmt);
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
  Result r = verifier.ProbeSSBO(&probe_ssbo, 4, ssbo_more);
  EXPECT_TRUE(r.IsSuccess());

  const double ssbo_less[4] = {2.801, 0.631, 9.901, 1234.461};

  r = verifier.ProbeSSBO(&probe_ssbo, 4, ssbo_less);
  EXPECT_TRUE(r.IsSuccess()) << r.Error();
}

TEST_F(VerifierTest, ProbeSSBOFuzzyEqualWithAbsoluteToleranceFail) {
  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();

  ProbeSSBOCommand probe_ssbo(color_buf.get());

  TypeParser parser;
  auto type = parser.Parse("R64_SFLOAT");
  Format fmt(type.get());

  probe_ssbo.SetFormat(&fmt);
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
  Result r = verifier.ProbeSSBO(&probe_ssbo, 4, ssbo);
  EXPECT_FALSE(r.IsSuccess());
  EXPECT_EQ("Line 1: Verifier failed: 3.001000 ~= 2.900000, at index 0",
            r.Error());
}

TEST_F(VerifierTest, ProbeSSBOFuzzyEqualWithRelativeTolerance) {
  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();

  ProbeSSBOCommand probe_ssbo(color_buf.get());

  TypeParser parser;
  auto type = parser.Parse("R64_SFLOAT");
  Format fmt(type.get());

  probe_ssbo.SetFormat(&fmt);
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
  Result r = verifier.ProbeSSBO(&probe_ssbo, 4, ssbo_more);
  EXPECT_TRUE(r.IsSuccess()) << r.Error();

  const double ssbo_less[4] = {2.8972, 0.72928, 9.991, 1233.32545};

  r = verifier.ProbeSSBO(&probe_ssbo, sizeof(double) * 4, ssbo_less);
  EXPECT_TRUE(r.IsSuccess()) << r.Error();
}

TEST_F(VerifierTest, ProbeSSBOFuzzyEqualWithRelativeToleranceFail) {
  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();

  ProbeSSBOCommand probe_ssbo(color_buf.get());

  TypeParser parser;
  auto type = parser.Parse("R64_SFLOAT");
  Format fmt(type.get());

  probe_ssbo.SetFormat(&fmt);
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
  Result r = verifier.ProbeSSBO(&probe_ssbo, 4, ssbo);
  EXPECT_FALSE(r.IsSuccess());
  EXPECT_EQ("Line 1: Verifier failed: 2.903000 ~= 2.900000, at index 0",
            r.Error());
}

TEST_F(VerifierTest, ProbeSSBONotEqual) {
  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();

  ProbeSSBOCommand probe_ssbo(color_buf.get());

  TypeParser parser;
  auto type = parser.Parse("R64_SFLOAT");
  Format fmt(type.get());

  probe_ssbo.SetFormat(&fmt);
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
  Result r = verifier.ProbeSSBO(&probe_ssbo, 4, ssbo);
  EXPECT_TRUE(r.IsSuccess()) << r.Error();
}

TEST_F(VerifierTest, ProbeSSBONotEqualFail) {
  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();

  ProbeSSBOCommand probe_ssbo(color_buf.get());

  TypeParser parser;
  auto type = parser.Parse("R64_SFLOAT");
  Format fmt(type.get());

  probe_ssbo.SetFormat(&fmt);
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
  Result r = verifier.ProbeSSBO(&probe_ssbo, 4, ssbo);
  EXPECT_FALSE(r.IsSuccess());
  EXPECT_EQ("Line 1: Verifier failed: 2.900000 != 2.900000, at index 0",
            r.Error());
}

TEST_F(VerifierTest, ProbeSSBOLess) {
  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();

  ProbeSSBOCommand probe_ssbo(color_buf.get());

  TypeParser parser;
  auto type = parser.Parse("R64_SFLOAT");
  Format fmt(type.get());

  probe_ssbo.SetFormat(&fmt);
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
  Result r = verifier.ProbeSSBO(&probe_ssbo, 4, ssbo);
  EXPECT_TRUE(r.IsSuccess()) << r.Error();
}

TEST_F(VerifierTest, ProbeSSBOLessFail) {
  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();

  ProbeSSBOCommand probe_ssbo(color_buf.get());

  TypeParser parser;
  auto type = parser.Parse("R64_SFLOAT");
  Format fmt(type.get());

  probe_ssbo.SetFormat(&fmt);
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
  Result r = verifier.ProbeSSBO(&probe_ssbo, 4, ssbo);
  EXPECT_FALSE(r.IsSuccess());
  EXPECT_EQ("Line 1: Verifier failed: 3.900000 < 2.900000, at index 0",
            r.Error());
}

TEST_F(VerifierTest, ProbeSSBOLessOrEqual) {
  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();

  ProbeSSBOCommand probe_ssbo(color_buf.get());

  TypeParser parser;
  auto type = parser.Parse("R64_SFLOAT");
  Format fmt(type.get());

  probe_ssbo.SetFormat(&fmt);
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
  Result r = verifier.ProbeSSBO(&probe_ssbo, 4, ssbo);
  EXPECT_TRUE(r.IsSuccess()) << r.Error();
}

TEST_F(VerifierTest, ProbeSSBOLessOrEqualFail) {
  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();

  ProbeSSBOCommand probe_ssbo(color_buf.get());

  TypeParser parser;
  auto type = parser.Parse("R64_SFLOAT");
  Format fmt(type.get());

  probe_ssbo.SetFormat(&fmt);
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
  Result r = verifier.ProbeSSBO(&probe_ssbo, 4, ssbo);
  EXPECT_FALSE(r.IsSuccess());
  EXPECT_EQ("Line 1: Verifier failed: 1234.561000 <= 1234.560000, at index 3",
            r.Error());
}

TEST_F(VerifierTest, ProbeSSBOGreater) {
  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();

  ProbeSSBOCommand probe_ssbo(color_buf.get());

  TypeParser parser;
  auto type = parser.Parse("R64_SFLOAT");
  Format fmt(type.get());

  probe_ssbo.SetFormat(&fmt);
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
  Result r = verifier.ProbeSSBO(&probe_ssbo, 4, ssbo);
  EXPECT_TRUE(r.IsSuccess()) << r.Error();
}

TEST_F(VerifierTest, ProbeSSBOGreaterFail) {
  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();

  ProbeSSBOCommand probe_ssbo(color_buf.get());

  TypeParser parser;
  auto type = parser.Parse("R64_SFLOAT");
  Format fmt(type.get());

  probe_ssbo.SetFormat(&fmt);
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
  Result r = verifier.ProbeSSBO(&probe_ssbo, 4, ssbo);
  EXPECT_FALSE(r.IsSuccess());
  EXPECT_EQ("Line 1: Verifier failed: 0.730000 > 0.730000, at index 1",
            r.Error());
}

TEST_F(VerifierTest, ProbeSSBOGreaterOrEqual) {
  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();

  ProbeSSBOCommand probe_ssbo(color_buf.get());

  TypeParser parser;
  auto type = parser.Parse("R64_SFLOAT");
  Format fmt(type.get());

  probe_ssbo.SetFormat(&fmt);
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
  EXPECT_TRUE(r.IsSuccess()) << r.Error();
}

TEST_F(VerifierTest, ProbeSSBOGreaterOrEqualFail) {
  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();

  ProbeSSBOCommand probe_ssbo(color_buf.get());

  TypeParser parser;
  auto type = parser.Parse("R64_SFLOAT");
  Format fmt(type.get());

  probe_ssbo.SetFormat(&fmt);
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
  Result r = verifier.ProbeSSBO(&probe_ssbo, 4, ssbo);
  EXPECT_FALSE(r.IsSuccess());
  EXPECT_EQ("Line 1: Verifier failed: 1234.559000 >= 1234.560000, at index 3",
            r.Error());
}

TEST_F(VerifierTest, CheckRGBAOrderForFailure) {
  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();

  ProbeCommand probe(color_buf.get());
  probe.SetIsRGBA();
  probe.SetX(0.0f);
  probe.SetY(0.0f);
  probe.SetR(0.6f);
  probe.SetG(0.4f);
  probe.SetB(0.0f);
  probe.SetA(0.3f);

  uint8_t frame_buffer[4] = {255, 14, 75, 8};

  Verifier verifier;
  Result r = verifier.Probe(&probe, GetColorFormat(),
                            4U * static_cast<uint32_t>(sizeof(uint8_t)),
                            4U * static_cast<uint32_t>(sizeof(uint8_t)), 1U, 1U,
                            static_cast<const void*>(&frame_buffer));
  EXPECT_FALSE(r.IsSuccess());
  EXPECT_EQ(
      "Line 1: Probe failed at: 0, 0\n  Expected: 153.000000, 102.000000, "
      "0.000000, 76.500000\n    Actual: 75.000000, 14.000000, 255.000000, "
      "8.000000\nProbe failed in 1 pixels",
      r.Error());
}

TEST_F(VerifierTest, ProbeSSBOWithPadding) {
  Pipeline pipeline(PipelineType::kGraphics);
  auto color_buf = pipeline.GenerateDefaultColorAttachmentBuffer();

  ProbeSSBOCommand probe_ssbo(color_buf.get());

  TypeParser parser;
  auto type = parser.Parse("float/vec2");
  Format fmt(type.get());

  probe_ssbo.SetFormat(&fmt);
  ASSERT_TRUE(probe_ssbo.GetFormat() != nullptr);

  probe_ssbo.SetComparator(ProbeSSBOCommand::Comparator::kLessOrEqual);

  std::vector<Value> values;
  values.resize(4);
  values[0].SetDoubleValue(2.9);
  values[1].SetDoubleValue(0.73);
  values[2].SetDoubleValue(10.0);
  values[3].SetDoubleValue(1234.56);
  probe_ssbo.SetValues(std::move(values));

  // The vec2 will get padded to 4 bytes in std430.
  const float ssbo[8] = {1.9f, 0.73f, 0.0f, 0.0f, 9.99f, 1234.560f, 0.0f, 0.0f};

  Verifier verifier;
  Result r = verifier.ProbeSSBO(&probe_ssbo, 4, ssbo);
  EXPECT_TRUE(r.IsSuccess()) << r.Error();
}

}  // namespace amber
