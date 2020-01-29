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

TEST_F(ResultTest, ErrorWithEmptyString) {
  Result r("");
  EXPECT_FALSE(r.IsSuccess());
  EXPECT_EQ("<no error message given>", r.Error());
}

TEST_F(ResultTest, Copy) {
  Result r("Testing");
  Result r2(r);

  EXPECT_FALSE(r2.IsSuccess());
  EXPECT_EQ("Testing", r2.Error());
}

TEST_F(ResultTest, Append1String) {
  Result r;
  r += "Test Failed";
  EXPECT_EQ("Test Failed", r.Error());
}

TEST_F(ResultTest, Append3Strings) {
  Result r;
  r += "Error one";
  r += "Error two";
  r += "Error three";
  EXPECT_EQ(R"(3 errors:
 (1) Error one
 (2) Error two
 (3) Error three)",
            r.Error());
}

TEST_F(ResultTest, Append1SingleErrorResult) {
  Result r;
  r += Result("Test Failed");
  EXPECT_EQ("Test Failed", r.Error());
}

TEST_F(ResultTest, Append3SingleErrorResults) {
  Result r;
  r += Result("Error one");
  r += Result("Error two");
  r += Result("Error three");
  EXPECT_EQ(R"(3 errors:
 (1) Error one
 (2) Error two
 (3) Error three)",
            r.Error());
}

TEST_F(ResultTest, Append3MixedResults) {
  Result r;
  r += Result("Error one");
  r += Result();  // success
  r += Result("Error two");
  EXPECT_EQ(R"(2 errors:
 (1) Error one
 (2) Error two)",
            r.Error());
}

TEST_F(ResultTest, AppendMultipleErrorResults) {
  Result r1;
  r1 += Result("r1 error one");
  r1 += Result("r1 error two");
  r1 += Result("r1 error three");
  r1 += Result("");
  Result r2;
  r2 += Result("r2 error one");
  r2 += r1;
  r2 += Result("r2 error two");
  EXPECT_EQ(R"(6 errors:
 (1) r2 error one
 (2) r1 error one
 (3) r1 error two
 (4) r1 error three
 (5) <no error message given>
 (6) r2 error two)",
            r2.Error());
}

}  // namespace amber
