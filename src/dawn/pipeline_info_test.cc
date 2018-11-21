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

#include "src/dawn/pipeline_info.h"

#include <cstdint>
#include "dawn/dawncpp.h"
#include "gmock/gmock.h"

namespace amber {
namespace dawn {

namespace {

using DawnComputePipelineInfoTest = testing::Test;

TEST_F(DawnComputePipelineInfoTest, DefaultConvertsToFalse) {
  ComputePipelineInfo cpi;
  EXPECT_EQ(false, cpi.IsConfigured());
}

// We can't create Dawn shader modules without a Dawn device.
// So skip unit tests for those.

using DawnRenderPipelineInfoTest = testing::Test;

TEST_F(DawnRenderPipelineInfoTest, DefaultConvertsToFalse) {
  RenderPipelineInfo rpi;
  EXPECT_EQ(false, rpi.IsConfigured());
}

TEST_F(DawnRenderPipelineInfoTest, DefaultClearColorIsAllZeroes) {
  RenderPipelineInfo rpi;
  const ClearColorCommand& color = rpi.GetClearColorValue();
  EXPECT_FLOAT_EQ(color.GetR(), 0.0f);
  EXPECT_FLOAT_EQ(color.GetG(), 0.0f);
  EXPECT_FLOAT_EQ(color.GetB(), 0.0f);
  EXPECT_FLOAT_EQ(color.GetA(), 0.0f);
}

TEST_F(DawnRenderPipelineInfoTest, DefaultClearDepthIsOne) {
  RenderPipelineInfo rpi;
  EXPECT_FLOAT_EQ(1.0f, rpi.GetClearDepthValue());
}

TEST_F(DawnRenderPipelineInfoTest, DefaultClearStencilIsZero) {
  RenderPipelineInfo rpi;
  EXPECT_EQ(uint32_t(0), rpi.GetClearStencilValue());
}

TEST_F(DawnRenderPipelineInfoTest, SetClearColor) {
  RenderPipelineInfo rpi;
  ClearColorCommand ccc;
  ccc.SetR(1.0f);
  ccc.SetG(2.0f);
  ccc.SetB(3.0f);
  ccc.SetA(-4.0f);
  rpi.SetClearColorValue(ccc);
  const ClearColorCommand& color = rpi.GetClearColorValue();
  EXPECT_FLOAT_EQ(1.0f, color.GetR());
  EXPECT_FLOAT_EQ(2.0f, color.GetG());
  EXPECT_FLOAT_EQ(3.0f, color.GetB());
  EXPECT_FLOAT_EQ(-4.0f, color.GetA());
}

TEST_F(DawnRenderPipelineInfoTest, SetClearDepth) {
  RenderPipelineInfo rpi;
  rpi.SetClearDepthValue(-12.0f);
  EXPECT_FLOAT_EQ(-12.0f, rpi.GetClearDepthValue());
}

TEST_F(DawnRenderPipelineInfoTest, SetClearStencil) {
  RenderPipelineInfo rpi;
  rpi.SetClearStencilValue(2172);
  EXPECT_EQ(2172, rpi.GetClearStencilValue());
}

}  // namespace
}  // namespace dawn
}  // namespace amber
