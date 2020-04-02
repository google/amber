// Copyright 2020 The Amber Authors.
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

#include "src/virtual_file_store.h"

#include "gtest/gtest.h"

namespace amber {

TEST(VirtualFileStore, Canonical) {
  ASSERT_EQ("a/b/c.e", VirtualFileStore::GetCanonical("a/b/c.e"));
  ASSERT_EQ("a/b.c.e", VirtualFileStore::GetCanonical("a/b.c.e"));
  ASSERT_EQ("a/b/c.e", VirtualFileStore::GetCanonical("a\\b\\c.e"));
  ASSERT_EQ("a/b/c.e", VirtualFileStore::GetCanonical("./a/b/c.e"));
}

TEST(VirtualFileStore, AddGet) {
  VirtualFileStore store;
  store.Add("a/file.1", "File 1");
  store.Add("./file.2", "File 2");
  store.Add("b\\file.3", "File 3");

  std::string content;
  ASSERT_TRUE(store.Get("a/file.1", &content).IsSuccess());
  ASSERT_EQ("File 1", content);

  ASSERT_TRUE(store.Get("./file.2", &content).IsSuccess());
  ASSERT_EQ("File 2", content);

  ASSERT_TRUE(store.Get("b\\file.3", &content).IsSuccess());
  ASSERT_EQ("File 3", content);

  content = "<not-assigned>";
  ASSERT_FALSE(store.Get("missing.file", &content).IsSuccess());
  ASSERT_EQ("<not-assigned>", content);
}

}  // namespace amber
