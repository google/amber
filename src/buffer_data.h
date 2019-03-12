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

#ifndef SRC_BUFFER_DATA_H_
#define SRC_BUFFER_DATA_H_

namespace amber {

/// Types of buffers which can be created.
enum class BufferType : int8_t {
  /// Unknown buffer type
  kUnknown = -1,
  /// A color buffer.
  kColor = 0,
  /// A depth/stencil buffer.
  kDepth,
  /// An index buffer.
  kIndex,
  /// A sampled buffer.
  kSampled,
  /// A storage buffer.
  kStorage,
  /// A uniform buffer.
  kUniform,
  /// A push constant buffer.
  kPushConstant,
  /// A vertex buffer.
  kVertex
};

}  // namespace amber

#endif  // SRC_BUFFER_DATA_H_
