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
#include "src/value.h"
#include "src/verifier.h"

namespace amber {

using VerifierTest = testing::Test;

TEST_F(VerifierTest, ProbeFrameBufferWholeWindow) {
  ProbeCommand probe;
  probe.SetWholeWindow();
  probe.SetIsRGBA();
  probe.SetR(0.5f);
  probe.SetG(0.25f);
  probe.SetB(0.2f);
  probe.SetA(0.8f);

  EXPECT_TRUE(probe.IsWholeWindow());
  EXPECT_TRUE(probe.IsRGBA());
  EXPECT_FLOAT_EQ(0.5f, probe.GetR());
  EXPECT_FLOAT_EQ(0.25f, probe.GetG());
  EXPECT_FLOAT_EQ(0.2f, probe.GetB());
  EXPECT_FLOAT_EQ(0.8f, probe.GetA());

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
  Result r = verifier.Probe(&probe, 4, 12, 3, 3,
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

  EXPECT_FALSE(probe.IsWholeWindow());
  EXPECT_TRUE(probe.IsRelative());
  EXPECT_TRUE(probe.IsRGBA());
  EXPECT_FLOAT_EQ(0.1f, probe.GetX());
  EXPECT_FLOAT_EQ(0.2f, probe.GetY());
  EXPECT_FLOAT_EQ(0.4f, probe.GetWidth());
  EXPECT_FLOAT_EQ(0.6f, probe.GetHeight());
  EXPECT_FLOAT_EQ(0.5f, probe.GetR());
  EXPECT_FLOAT_EQ(0.25f, probe.GetG());
  EXPECT_FLOAT_EQ(0.2f, probe.GetB());
  EXPECT_FLOAT_EQ(0.8f, probe.GetA());

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
  Result r = verifier.Probe(&probe, 4, 40, 10, 10,
                            static_cast<const void*>(frame_buffer));
  EXPECT_TRUE(r.IsSuccess());
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

  EXPECT_FALSE(probe.IsWholeWindow());
  EXPECT_FALSE(probe.IsRelative());
  EXPECT_TRUE(probe.IsRGBA());
  EXPECT_FLOAT_EQ(1.0f, probe.GetX());
  EXPECT_FLOAT_EQ(2.0f, probe.GetY());
  EXPECT_FLOAT_EQ(4.0f, probe.GetWidth());
  EXPECT_FLOAT_EQ(6.0f, probe.GetHeight());
  EXPECT_FLOAT_EQ(0.5f, probe.GetR());
  EXPECT_FLOAT_EQ(0.25f, probe.GetG());
  EXPECT_FLOAT_EQ(0.2f, probe.GetB());
  EXPECT_FLOAT_EQ(0.8f, probe.GetA());

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
  Result r = verifier.Probe(&probe, 4, 40, 10, 10,
                            static_cast<const void*>(frame_buffer));
  EXPECT_TRUE(r.IsSuccess());
}

TEST_F(VerifierTest, ProbeFrameBufferRGB) {
  ProbeCommand probe;
  probe.SetWholeWindow();
  probe.SetR(0.5f);
  probe.SetG(0.25f);
  probe.SetB(0.2f);

  EXPECT_TRUE(probe.IsWholeWindow());
  EXPECT_FALSE(probe.IsRelative());
  EXPECT_FALSE(probe.IsRGBA());
  EXPECT_FLOAT_EQ(0.5f, probe.GetR());
  EXPECT_FLOAT_EQ(0.25f, probe.GetG());
  EXPECT_FLOAT_EQ(0.2f, probe.GetB());
  EXPECT_FLOAT_EQ(0.0f, probe.GetA());

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
  Result r = verifier.Probe(&probe, 4, 12, 3, 3,
                            static_cast<const void*>(frame_buffer));
  EXPECT_TRUE(r.IsSuccess());
}

TEST_F(VerifierTest, ProbeFrameBufferBadRowStride) {
  ProbeCommand probe;
  probe.SetWholeWindow();

  const uint8_t frame_buffer[4] = {128, 64, 51, 255};

  Verifier verifier;
  Result r = verifier.Probe(&probe, 4, 3, 1, 1,
                            static_cast<const void*>(frame_buffer));
  EXPECT_FALSE(r.IsSuccess());
  EXPECT_STREQ(
      "Verifier::Probe Row stride of 3 is too small for 1 texels of 4 bytes "
      "each",
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

// TODO(jaebaek): Test all ProbeSSBOCommand::Comparator and
//                all primitive types and tolerance.

}  // namespace amber
