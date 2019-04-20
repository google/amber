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

#ifndef SRC_FORMAT_PARSER_H_
#define SRC_FORMAT_PARSER_H_

#include <memory>
#include <string>
#include <vector>

#include "src/format.h"

namespace amber {

class Format;

/// Parses a Vulkan image string into a format object.
class FormatParser {
 public:
  FormatParser();
  ~FormatParser();

  std::unique_ptr<Format> Parse(const std::string& fmt);

 private:
  std::unique_ptr<Format> ParseGlslFormat(const std::string& fmt);
  void ProcessChunk(Format*, const std::string&);
  FormatType NameToType(const std::string& data);
  void AddPiece(FormatComponentType type, uint8_t bits);
  void FlushPieces(Format* fmt, FormatMode mode);

  struct Pieces {
    Pieces(FormatComponentType t, uint8_t bits) : type(t), num_bits(bits) {}

    FormatComponentType type;
    uint8_t num_bits;
  };

  std::vector<Pieces> pieces_;
};

}  // namespace amber

#endif  // SRC_FORMAT_PARSER_H_
