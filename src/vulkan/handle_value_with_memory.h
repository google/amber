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

#ifndef SRC_VULKAN_HANDLE_VALUE_WITH_MEMORY_H_
#define SRC_VULKAN_HANDLE_VALUE_WITH_MEMORY_H_

#include <vector>

#include "src/datum_type.h"
#include "src/value.h"

namespace amber {
namespace vulkan {

// Contain information of updating memory
// [|offset|, |offset| + |size_in_bytes|) with |values| whose data
// type is |type|.
struct BufferData {
  DataType type;
  uint32_t offset;
  size_t size_in_bytes;
  std::vector<Value> values;
};

class HandleValueWithMemory {
 public:
  HandleValueWithMemory();
  ~HandleValueWithMemory();

  // Update |memory| from |offset| of |data| to |offset| + |size_in_bytes| of
  // |data| with |values| of |data|.
  void UpdateMemoryWithData(void* memory, const BufferData& data);
};

}  // namespace vulkan
}  // namespace amber

#endif  // SRC_VULKAN_HANDLE_VALUE_WITH_MEMORY_H_
