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

#include "src/dawn/engine_dawn.h"

#include "gtest/gtest.h"

namespace amber {
namespace dawn {

namespace {

using EngineDawnTest = testing::Test;

TEST_F(EngineDawnTest, PendingWorkDefaultIsEmpty) {
  EngineDawn e;
  auto& pending = e.GetPendingWorkForTest();
  EXPECT_TRUE(pending.empty());
}

TEST_F(EngineDawnTest, ClearAppendsToPendingWork) {
  EngineDawn e;
  {
    ClearCommand cc;
    e.DoClear(&cc);
  }
  auto& pending = e.GetPendingWorkForTest();
  EXPECT_EQ(1u, pending.size());
  EXPECT_TRUE(pending[0].IsClear());
}
}  // namespace
}  // namespace dawn
}  // namespace amber
