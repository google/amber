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

#include "src/verifier.h"

#include <cmath>
#include <string>
#include <vector>

#include "src/command.h"

namespace amber {
namespace {

const double kEpsilon = 0.000001;
const double kDefaultTexelTolerance = 0.002;

// It returns true if the difference is within the given error.
// If |is_tolerance_percent| is true, the actual tolerance will be
// relative value i.e., |tolerance| / 100 * fabs(expected).
// Otherwise, this method uses the absolute value i.e., |tolerance|.
bool IsEqualWithTolerance(const double real,
                          const double expected,
                          double tolerance,
                          const bool is_tolerance_percent = true) {
  double difference = std::fabs(real - expected);
  if (is_tolerance_percent) {
    if (difference > ((tolerance / 100.0) * std::fabs(expected))) {
      return false;
    }
  } else if (difference > tolerance) {
    return false;
  }
  return true;
}

template <typename T>
Result CheckValue(const ProbeSSBOCommand* command,
                  const uint8_t* memory,
                  const std::vector<Value>& values) {
  const auto comp = command->GetComparator();
  const auto& tolerance = command->GetTolerances();
  const T* ptr = reinterpret_cast<const T*>(memory);
  for (size_t i = 0; i < values.size(); ++i) {
    const T val = values[i].IsInteger() ? static_cast<T>(values[i].AsUint64())
                                        : static_cast<T>(values[i].AsDouble());
    switch (comp) {
      case ProbeSSBOCommand::Comparator::kEqual:
        if (values[i].IsInteger()) {
          if (static_cast<uint64_t>(*ptr) != static_cast<uint64_t>(val)) {
            return Result("Line: " + std::to_string(command->GetLine()) +
                          ": Verifier failed: " + std::to_string(*ptr) +
                          " == " + std::to_string(val) + ", at index " +
                          std::to_string(i));
          }
        } else {
          if (!IsEqualWithTolerance(static_cast<const double>(*ptr),
                                    static_cast<const double>(val), kEpsilon)) {
            return Result("Line: " + std::to_string(command->GetLine()) +
                          ": Verifier failed: " + std::to_string(*ptr) +
                          " == " + std::to_string(val) + ", at index " +
                          std::to_string(i));
          }
        }
        break;
      case ProbeSSBOCommand::Comparator::kNotEqual:
        if (values[i].IsInteger()) {
          if (static_cast<uint64_t>(*ptr) == static_cast<uint64_t>(val)) {
            return Result("Line: " + std::to_string(command->GetLine()) +
                          ": Verifier failed: " + std::to_string(*ptr) +
                          " != " + std::to_string(val) + ", at index " +
                          std::to_string(i));
          }
        } else {
          if (IsEqualWithTolerance(static_cast<const double>(*ptr),
                                   static_cast<const double>(val), kEpsilon)) {
            return Result("Line: " + std::to_string(command->GetLine()) +
                          ": Verifier failed: " + std::to_string(*ptr) +
                          " != " + std::to_string(val) + ", at index " +
                          std::to_string(i));
          }
        }
        break;
      case ProbeSSBOCommand::Comparator::kFuzzyEqual:
        if (!IsEqualWithTolerance(
                static_cast<const double>(*ptr), static_cast<const double>(val),
                command->HasTolerances() ? tolerance[0].value : kEpsilon,
                command->HasTolerances() ? tolerance[0].is_percent : true)) {
          return Result("Line: " + std::to_string(command->GetLine()) +
                        ": Verifier failed: " + std::to_string(*ptr) +
                        " ~= " + std::to_string(val) + ", at index " +
                        std::to_string(i));
        }
        break;
      case ProbeSSBOCommand::Comparator::kLess:
        if (*ptr >= val) {
          return Result("Line: " + std::to_string(command->GetLine()) +
                        ": Verifier failed: " + std::to_string(*ptr) + " < " +
                        std::to_string(val) + ", at index " +
                        std::to_string(i));
        }
        break;
      case ProbeSSBOCommand::Comparator::kLessOrEqual:
        if (*ptr > val) {
          return Result("Line: " + std::to_string(command->GetLine()) +
                        ": Verifier failed: " + std::to_string(*ptr) +
                        " <= " + std::to_string(val) + ", at index " +
                        std::to_string(i));
        }
        break;
      case ProbeSSBOCommand::Comparator::kGreater:
        if (*ptr <= val) {
          return Result("Line: " + std::to_string(command->GetLine()) +
                        ": Verifier failed: " + std::to_string(*ptr) + " > " +
                        std::to_string(val) + ", at index " +
                        std::to_string(i));
        }
        break;
      case ProbeSSBOCommand::Comparator::kGreaterOrEqual:
        if (*ptr < val) {
          return Result("Line: " + std::to_string(command->GetLine()) +
                        ": Verifier failed: " + std::to_string(*ptr) +
                        " >= " + std::to_string(val) + ", at index " +
                        std::to_string(i));
        }
        break;
    }
    ++ptr;
  }
  return {};
}

void SetupToleranceForTexels(const ProbeCommand* command,
                             double* tolerance,
                             bool* is_tolerance_percent) {
  if (command->HasTolerances()) {
    const auto& tol = command->GetTolerances();
    if (tol.size() == 4) {
      tolerance[0] = tol[0].value;
      tolerance[1] = tol[1].value;
      tolerance[2] = tol[2].value;
      tolerance[3] = tol[3].value;
      is_tolerance_percent[0] = tol[0].is_percent;
      is_tolerance_percent[1] = tol[1].is_percent;
      is_tolerance_percent[2] = tol[2].is_percent;
      is_tolerance_percent[3] = tol[3].is_percent;
    } else {
      tolerance[0] = tol[0].value;
      tolerance[1] = tol[0].value;
      tolerance[2] = tol[0].value;
      tolerance[3] = tol[0].value;
      is_tolerance_percent[0] = tol[0].is_percent;
      is_tolerance_percent[1] = tol[0].is_percent;
      is_tolerance_percent[2] = tol[0].is_percent;
      is_tolerance_percent[3] = tol[0].is_percent;
    }
  } else {
    tolerance[0] = kDefaultTexelTolerance;
    tolerance[1] = kDefaultTexelTolerance;
    tolerance[2] = kDefaultTexelTolerance;
    tolerance[3] = kDefaultTexelTolerance;
    is_tolerance_percent[0] = false;
    is_tolerance_percent[1] = false;
    is_tolerance_percent[2] = false;
    is_tolerance_percent[3] = false;
  }
}

}  // namespace

Verifier::Verifier() = default;

Verifier::~Verifier() = default;

Result Verifier::Probe(const ProbeCommand* command,
                       uint32_t texel_stride,
                       uint32_t row_stride,
                       uint32_t frame_width,
                       uint32_t frame_height,
                       const void* buf) {
  uint32_t x = 0;
  uint32_t y = 0;
  uint32_t width = 0;
  uint32_t height = 0;

  if (command->IsWholeWindow()) {
    width = frame_width;
    height = frame_height;
  } else if (command->IsRelative()) {
    x = static_cast<uint32_t>(static_cast<float>(frame_width) *
                              command->GetX());
    y = static_cast<uint32_t>(static_cast<float>(frame_height) *
                              command->GetY());
    width = static_cast<uint32_t>(static_cast<float>(frame_width) *
                                  command->GetWidth());
    height = static_cast<uint32_t>(static_cast<float>(frame_height) *
                                   command->GetHeight());
  } else {
    x = static_cast<uint32_t>(command->GetX());
    y = static_cast<uint32_t>(command->GetY());
    width = static_cast<uint32_t>(command->GetWidth());
    height = static_cast<uint32_t>(command->GetHeight());
  }

  if (x + width > frame_width || y + height > frame_height) {
    return Result(
        "Line: " + std::to_string(command->GetLine()) +
        ": Verifier::Probe Position(" + std::to_string(x + width - 1) + ", " +
        std::to_string(y + height - 1) + ") is out of framebuffer scope (" +
        std::to_string(frame_width) + "," + std::to_string(frame_height) + ")");
  }
  if (row_stride < frame_width * texel_stride) {
    return Result("Line: " + std::to_string(command->GetLine()) +
                  ": Verifier::Probe Row stride of " +
                  std::to_string(row_stride) + " is too small for " +
                  std::to_string(frame_width) + " texels of " +
                  std::to_string(texel_stride) + " bytes each");
  }

  double tolerance[4] = {};
  bool is_tolerance_percent[4] = {};
  SetupToleranceForTexels(command, tolerance, is_tolerance_percent);

  // TODO(jaebaek): Support all VkFormat
  const uint8_t* ptr = static_cast<const uint8_t*>(buf);
  uint32_t count_of_invalid_pixels = 0;
  uint32_t first_invalid_i = 0;
  uint32_t first_invalid_j = 0;
  for (uint32_t j = 0; j < height; ++j) {
    const uint8_t* p = ptr + row_stride * (j + y) + texel_stride * x;
    for (uint32_t i = 0; i < width; ++i) {
      // TODO(jaebaek): Get actual pixel values based on frame buffer formats.
      if (!IsEqualWithTolerance(
              static_cast<const double>(command->GetR()),
              static_cast<const double>(p[texel_stride * i]) / 255.0,
              tolerance[0], is_tolerance_percent[0]) ||
          !IsEqualWithTolerance(
              static_cast<const double>(command->GetG()),
              static_cast<const double>(p[texel_stride * i + 1]) / 255.0,
              tolerance[1], is_tolerance_percent[1]) ||
          !IsEqualWithTolerance(
              static_cast<const double>(command->GetB()),
              static_cast<const double>(p[texel_stride * i + 2]) / 255.0,
              tolerance[2], is_tolerance_percent[2]) ||
          (command->IsRGBA() &&
           !IsEqualWithTolerance(
               static_cast<const double>(command->GetA()),
               static_cast<const double>(p[texel_stride * i + 3]) / 255.0,
               tolerance[3], is_tolerance_percent[3]))) {
        if (!count_of_invalid_pixels) {
          first_invalid_i = i;
          first_invalid_j = j;
        }
        ++count_of_invalid_pixels;
      }
    }
  }

  if (count_of_invalid_pixels) {
    const uint8_t* p = ptr + row_stride * (first_invalid_j + y) +
                       texel_stride * (x + first_invalid_i);
    return Result(
        "Line: " + std::to_string(command->GetLine()) +
        ": Probe failed at: " + std::to_string(x + first_invalid_i) + ", " +
        std::to_string(first_invalid_j + y) + "\n" +
        "  Expected RGBA: " + std::to_string(command->GetR() * 255) + ", " +
        std::to_string(command->GetG() * 255) + ", " +
        std::to_string(command->GetB() * 255) +
        (command->IsRGBA() ? ", " + std::to_string(command->GetA() * 255) +
                                 "\n  Actual RGBA: "
                           : "\n  Actual RGB: ") +
        std::to_string(static_cast<int>(p[0])) + ", " +
        std::to_string(static_cast<int>(p[1])) + ", " +
        std::to_string(static_cast<int>(p[2])) +
        (command->IsRGBA() ? ", " + std::to_string(static_cast<int>(p[3]))
                           : "") +
        "\n" + "Probe failed in " + std::to_string(count_of_invalid_pixels) +
        " pixels");
  }

  return {};
}

Result Verifier::ProbeSSBO(const ProbeSSBOCommand* command,
                           size_t size_in_bytes,
                           const void* cpu_memory) {
  const auto& values = command->GetValues();
  const auto& datum_type = command->GetDatumType();
  size_t bytes_per_elem = datum_type.SizeInBytes() / datum_type.RowCount() /
                          datum_type.ColumnCount();
  size_t offset = static_cast<size_t>(command->GetOffset());

  if (values.size() * bytes_per_elem + offset > size_in_bytes) {
    return Result(
        "Line: " + std::to_string(command->GetLine()) +
        ": Verifier::ProbeSSBO has more expected values than SSBO size");
  }

  if (offset % bytes_per_elem != 0) {
    return Result(
        "Line: " + std::to_string(command->GetLine()) +
        ": Verifier::ProbeSSBO given offset is not multiple of bytes_per_elem");
  }

  const uint8_t* ptr = static_cast<const uint8_t*>(cpu_memory) + offset;
  if (datum_type.IsInt8())
    return CheckValue<int8_t>(command, ptr, values);
  if (datum_type.IsUint8())
    return CheckValue<uint8_t>(command, ptr, values);
  if (datum_type.IsInt16())
    return CheckValue<int16_t>(command, ptr, values);
  if (datum_type.IsUint16())
    return CheckValue<uint16_t>(command, ptr, values);
  if (datum_type.IsInt32())
    return CheckValue<int32_t>(command, ptr, values);
  if (datum_type.IsUint32())
    return CheckValue<uint32_t>(command, ptr, values);
  if (datum_type.IsInt64())
    return CheckValue<int64_t>(command, ptr, values);
  if (datum_type.IsUint64())
    return CheckValue<uint64_t>(command, ptr, values);
  if (datum_type.IsFloat())
    return CheckValue<float>(command, ptr, values);
  if (datum_type.IsDouble())
    return CheckValue<double>(command, ptr, values);

  return Result("Line: " + std::to_string(command->GetLine()) +
                ": Verifier::ProbeSSBO unknown datum type");
}

}  // namespace amber
