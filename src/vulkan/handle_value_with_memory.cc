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

#include "src/vulkan/handle_value_with_memory.h"

namespace amber {
namespace vulkan {
namespace {

template <typename T>
void SetValueForBuffer(void* memory, const std::vector<Value>& values) {
  T* ptr = static_cast<T*>(memory);
  for (const auto& v : values) {
    *ptr = static_cast<T>(v.IsInteger() ? v.AsUint64() : v.AsDouble());
    ++ptr;
  }
}

}  // namespace

HandleValueWithMemory::HandleValueWithMemory() = default;

HandleValueWithMemory::~HandleValueWithMemory() = default;

void HandleValueWithMemory::UpdateMemoryWithData(void* memory,
                                                 const BufferData& data) {
  uint8_t* ptr = static_cast<uint8_t*>(memory) + data.offset;
  switch (data.type) {
    case DataType::kInt8:
    case DataType::kUint8:
      SetValueForBuffer<uint8_t>(ptr, data.values);
      break;
    case DataType::kInt16:
    case DataType::kUint16:
      SetValueForBuffer<uint16_t>(ptr, data.values);
      break;
    case DataType::kInt32:
    case DataType::kUint32:
      SetValueForBuffer<uint32_t>(ptr, data.values);
      break;
    case DataType::kInt64:
    case DataType::kUint64:
      SetValueForBuffer<uint64_t>(ptr, data.values);
      break;
    case DataType::kFloat:
      SetValueForBuffer<float>(ptr, data.values);
      break;
    case DataType::kDouble:
      SetValueForBuffer<double>(ptr, data.values);
      break;
  }
}

}  // namespace vulkan
}  // namespace amber
