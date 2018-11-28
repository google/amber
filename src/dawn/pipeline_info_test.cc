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

TEST_F(DawnComputePipelineInfoTest, DefaultValueHasNoShader) {
  ComputePipelineInfo cpi;
  EXPECT_FALSE(static_cast<bool>(cpi.compute_shader));
}

// We can't create Dawn shader modules without a Dawn device.
// So skip unit tests for those.

using DawnRenderPipelineInfoTest = testing::Test;

TEST_F(DawnRenderPipelineInfoTest, DefaultValuesForMembers) {
  RenderPipelineInfo rpi;
  EXPECT_FALSE(static_cast<bool>(rpi.vertex_shader));
  EXPECT_FALSE(static_cast<bool>(rpi.fragment_shader));
  EXPECT_FLOAT_EQ(0.0f, rpi.clear_color_value.GetR());
  EXPECT_FLOAT_EQ(0.0f, rpi.clear_color_value.GetG());
  EXPECT_FLOAT_EQ(0.0f, rpi.clear_color_value.GetB());
  EXPECT_FLOAT_EQ(0.0f, rpi.clear_color_value.GetA());
  EXPECT_FLOAT_EQ(1.0f, rpi.clear_depth_value);
  EXPECT_EQ(0u, rpi.clear_stencil_value);
  EXPECT_FALSE(static_cast<bool>(rpi.fb_texture));
  EXPECT_FALSE(static_cast<bool>(rpi.fb_buffer));
  EXPECT_EQ(0u, rpi.fb_row_stride);
  EXPECT_EQ(0u, rpi.fb_size);
  EXPECT_FALSE(rpi.fb_is_mapped);
}

}  // namespace
}  // namespace dawn
}  // namespace amber
