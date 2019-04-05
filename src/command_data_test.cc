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

#include "src/command_data.h"

#include "gtest/gtest.h"

namespace amber {

using CommandDataTest = testing::Test;

TEST_F(CommandDataTest, TopologyConversions) {
  const struct {
    const char* name;
    Topology val;
  } topologies[] = {
      {"PATCH_LIST", Topology::kPatchList},
      {"POINT_LIST", Topology::kPointList},
      {"GL_LINE_STRIP_ADJACENCY", Topology::kLineStripWithAdjacency},
      {"GL_LINE_STRIP", Topology::kLineStrip},
      {"GL_LINES", Topology::kLineList},
      {"GL_LINES_ADJACENCY", Topology::kLineListWithAdjacency},
      {"GL_PATCHES", Topology::kPatchList},
      {"GL_POINTS", Topology::kPointList},
      {"GL_TRIANGLE_STRIP", Topology::kTriangleStrip},
      {"GL_TRIANGLE_FAN", Topology::kTriangleFan},
      {"GL_TRIANGLES", Topology::kTriangleList},
      {"GL_TRIANGLES_ADJACENCY", Topology::kTriangleListWithAdjacency},
      {"GL_TRIANGLE_STRIP_ADJACENCY", Topology::kTriangleStripWithAdjacency},
      {"LINE_LIST", Topology::kLineList},
      {"LINE_LIST_WITH_ADJACENCY", Topology::kLineListWithAdjacency},
      {"LINE_STRIP", Topology::kLineStrip},
      {"LINE_STRIP_WITH_ADJACENCY", Topology::kLineStripWithAdjacency},
      {"TRIANGLE_FAN", Topology::kTriangleFan},
      {"TRIANGLE_LIST", Topology::kTriangleList},
      {"TRIANGLE_LIST_WITH_ADJACENCY", Topology::kTriangleListWithAdjacency},
      {"TRIANGLE_STRIP", Topology::kTriangleStrip},
      {"TRIANGLE_STRIP_WITH_ADJACENCY", Topology::kTriangleStripWithAdjacency},
  };

  for (const auto& topo : topologies) {
    EXPECT_EQ(topo.val, NameToTopology(topo.name));
  }
}

TEST_F(CommandDataTest, TopologyInvalid) {
  EXPECT_EQ(Topology::kUnknown, NameToTopology(""));
  EXPECT_EQ(Topology::kUnknown, NameToTopology("Invalid_Topology"));
}

}  // namespace amber
