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

#ifndef SRC_CAST_HASH_H_
#define SRC_CAST_HASH_H_

namespace amber {

/// A hash implementation for types that can trivially be up-cast to a size_t.
/// For example, use this as a hasher for an enum whose underlying type is
/// an integer no wider than size_t.
///
/// The need for this was a defect in the C++11 library, and has been fixed
/// in C++14.  http://www.open-std.org/jtc1/sc22/wg21/docs/lwg-defects.html#2148
template <typename T>
struct CastHash {
  size_t operator()(const T& value) const { return static_cast<size_t>(value); }
};

}  // namespace amber

#endif  // SRC_CAST_HASH_H_
