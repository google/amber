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

#ifndef SRC_FORMAT_H_
#define SRC_FORMAT_H_

#include <cstdint>
#include <vector>

#include "src/format_data.h"

namespace amber {

/// The format class describes requested image formats. (eg. R8G8B8A8_UINT).
class Format {
 public:
  /// Describes an individual component of a format.
  struct Component {
    Component(FormatComponentType t, FormatMode m, uint8_t bits)
        : type(t), mode(m), num_bits(bits) {}

    FormatComponentType type;
    FormatMode mode;
    uint8_t num_bits;
  };

  Format();
  Format(const Format&);
  ~Format();

  void SetFormatType(FormatType type) { type_ = type; }
  FormatType GetFormatType() const { return type_; }

  /// Set the number of bytes this format is packed into, if provided.
  void SetPackSize(uint8_t size_in_bytes) {
    pack_size_in_bytes_ = size_in_bytes;
  }
  /// Retrieves the number of bytes this format is packed into.
  uint8_t GetPackSize() const { return pack_size_in_bytes_; }

  void AddComponent(FormatComponentType type, FormatMode mode, uint8_t bits) {
    components_.emplace_back(type, mode, bits);
  }
  const std::vector<Component>& GetComponents() const { return components_; }

  uint32_t GetByteSize() const {
    if (pack_size_in_bytes_)
      return pack_size_in_bytes_;

    uint32_t bits = 0;
    for (uint32_t j = 0; j < components_.size(); ++j) {
      bits += components_[j].num_bits;
    }
    return bits / 8;
  }

 private:
  FormatType type_;
  uint8_t pack_size_in_bytes_ = 0;
  std::vector<Component> components_;
};

}  // namespace amber

#endif  // SRC_FORMAT_H_
