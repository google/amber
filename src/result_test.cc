// Copyright 2018 The Amber Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "amber/result.h"

#include "gtest/gtest.h"

namespace amber {

using ResultTest = testing::Test;

TEST_F(ResultTest, SuccessByDefault) {
  Result r;
  EXPECT_TRUE(r.IsSuccess());
}

TEST_F(ResultTest, ErrorWithString) {
  Result r("Test Failed");
  EXPECT_FALSE(r.IsSuccess());
  EXPECT_EQ("Test Failed", r.Error());
}

TEST_F(ResultTest, Copy) {
  Result r("Testing");
  Result r2(r);

  EXPECT_FALSE(r2.IsSuccess());
  EXPECT_EQ("Testing", r2.Error());
}

}  // namespace amber
