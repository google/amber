// Copyright 2019 The Amber Authors.
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

#ifndef SRC_TYPE_PARSER_H_
#define SRC_TYPE_PARSER_H_

#include <memory>
#include <string>
#include <vector>

#include "src/type.h"

namespace amber {

/// Parses a Vulkan image string into a type object.
class TypeParser {
 public:
  static FormatType NameToFormatType(const std::string& data);

  TypeParser();
  ~TypeParser();

  /// Parses the |fmt| string and returns the associated type, nullptr if the
  /// conversion is not possible.
  ///
  /// The format string can come in two different forms, it can be a vulkan
  /// style format string (e.g. R32G32B32A32_SFLOAT) or it can be in the type
  /// format (gl_type/glsl_type) specified by VkScript (e.g. byte/vec4).
  std::unique_ptr<type::Type> Parse(const std::string& fmt);

 private:
  std::unique_ptr<type::Type> ParseGlslFormat(const std::string& fmt);
  void ProcessChunk(const std::string&);
  void AddPiece(FormatComponentType type, FormatMode mode, uint8_t bits);
  void FlushPieces(type::Type* type, FormatMode mode);

  struct Pieces {
    Pieces(FormatComponentType t, FormatMode m, uint8_t bits)
        : type(t), mode(m), num_bits(bits) {}

    FormatComponentType type;
    FormatMode mode;
    uint8_t num_bits;
  };

  FormatMode mode_ = FormatMode::kSInt;
  uint32_t pack_size_ = 0;
  std::vector<Pieces> pieces_;
};

}  // namespace amber

#endif  // SRC_TYPE_PARSER_H_
