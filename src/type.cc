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

#include "src/type.h"

#include <cassert>
#include <memory>

#include "src/make_unique.h"

namespace amber {
namespace type {

Type::Type() = default;

Type::~Type() = default;

List* Type::AsList() {
  return static_cast<List*>(this);
}

Number* Type::AsNumber() {
  return static_cast<Number*>(this);
}

Struct* Type::AsStruct() {
  return static_cast<Struct*>(this);
}

const List* Type::AsList() const {
  return static_cast<const List*>(this);
}

const Number* Type::AsNumber() const {
  return static_cast<const Number*>(this);
}

const Struct* Type::AsStruct() const {
  return static_cast<const Struct*>(this);
}

// static
std::unique_ptr<Number> Number::Int(uint32_t bits) {
  return MakeUnique<Number>(FormatMode::kSInt, bits);
}

// static
std::unique_ptr<Number> Number::Uint(uint32_t bits) {
  return MakeUnique<Number>(FormatMode::kUInt, bits);
}

// static
std::unique_ptr<Number> Number::Float(uint32_t bits) {
  return MakeUnique<Number>(FormatMode::kSFloat, bits);
}

Number::Number(FormatMode format_mode) : format_mode_(format_mode) {}

Number::Number(FormatMode format_mode, uint32_t bits)
    : format_mode_(format_mode), bits_(bits) {}

Number::~Number() = default;

List::List() = default;

List::~List() = default;

uint32_t List::SizeInBytes() const {
  if (pack_size_in_bits_ > 0)
    return pack_size_in_bits_;

  uint32_t size = 0;
  for (const auto& member : members_)
    size += member.SizeInBytes();

  return size;
}

Struct::Struct() = default;

Struct::~Struct() = default;

// Struct side is dependent on the layout we're currently in ....
uint32_t Struct::SizeInBytes() const {
  assert(false && "Not reached");
  return 0;
}

}  // namespace type
}  // namespace amber
