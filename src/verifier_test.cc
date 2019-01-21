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
  probe.SetProbeRect();
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
  Result r = verifier.Probe(&probe, 4, 40, 10, 10,
                            static_cast<const void*>(frame_buffer));
  EXPECT_TRUE(r.IsSuccess());
}

TEST_F(VerifierTest, ProbeFrameBufferRelativeSmallExpectFail) {
  ProbeCommand probe;
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
  Result r = verifier.Probe(&probe, 4, 1000, 250, 250,
                            static_cast<const void*>(frame_buffer));
  EXPECT_STREQ(
      "Line 1: Probe failed at: 225, 225\n  Expected RGBA: 25.500000, "
      "0.000000, 0.000000, 0.000000\n  Actual RGBA: 0, 0, 0, 0\nProbe failed "
      "in 625 pixels",
      r.Error().c_str());
}

TEST_F(VerifierTest, ProbeFrameBuffer) {
  ProbeCommand probe;
  probe.SetProbeRect();
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
  Result r = verifier.Probe(&probe, 4, 40, 10, 10,
                            static_cast<const void*>(frame_buffer));
  EXPECT_TRUE(r.IsSuccess());
}

TEST_F(VerifierTest, ProbeFrameBufferNotRect) {
  uint8_t frame_buffer[10][10][4] = {};

  frame_buffer[2][1][0] = 128;
  frame_buffer[2][1][1] = 64;
  frame_buffer[2][1][2] = 51;
  frame_buffer[2][1][3] = 204;

  frame_buffer[3][7][0] = 51;
  frame_buffer[3][7][1] = 204;
  frame_buffer[3][7][2] = 64;
  frame_buffer[3][7][3] = 128;

  ProbeCommand probe;
  probe.SetIsRGBA();
  probe.SetX(1.0f);
  probe.SetY(2.0f);
  probe.SetR(0.5f);
  probe.SetG(0.25f);
  probe.SetB(0.2f);
  probe.SetA(0.8f);

  Verifier verifier;
  Result r = verifier.Probe(&probe, 4, 40, 10, 10,
                            static_cast<const void*>(frame_buffer));
  EXPECT_TRUE(r.IsSuccess());

  probe.SetIsRGBA();
  probe.SetX(7.0f);
  probe.SetY(3.0f);
  probe.SetR(0.2f);
  probe.SetG(0.8f);
  probe.SetB(0.25f);
  probe.SetA(0.5f);

  r = verifier.Probe(&probe, 4, 40, 10, 10,
                     static_cast<const void*>(frame_buffer));
  EXPECT_TRUE(r.IsSuccess());

  probe.SetX(0.0f);
  probe.SetY(0.0f);

  r = verifier.Probe(&probe, 4, 40, 10, 10,
                     static_cast<const void*>(frame_buffer));
  EXPECT_FALSE(r.IsSuccess());
  EXPECT_STREQ(
      "Line 1: Probe failed at: 0, 0\n  Expected RGBA: 51.000000, 204.000000, "
      "63.750000, 127.500000\n  Actual RGBA: 0, 0, 0, 0\nProbe failed in 1 "
      "pixels",
      r.Error().c_str());
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
