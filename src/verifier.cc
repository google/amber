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

#include <cassert>
#include <cmath>
#include <string>
#include <vector>

#include "src/bit_copy.h"
#include "src/command.h"

namespace amber {
namespace {

const double kEpsilon = 0.000001;
const double kDefaultTexelTolerance = 0.002;

// This is based on "18.3. sRGB transfer functions" of
// https://www.khronos.org/registry/DataFormat/specs/1.2/dataformat.1.2.html
double SRGBToLinearValue(double sRGB) {
  if (sRGB <= 0.04045)
    return sRGB / 12.92;

  return pow((sRGB + 0.055) / 1.055, 2.4);
}

// It returns true if the difference is within the given error.
// If |is_tolerance_percent| is true, the actual tolerance will be
// relative value i.e., |tolerance| / 100 * fabs(expected).
// Otherwise, this method uses the absolute value i.e., |tolerance|.
bool IsEqualWithTolerance(const double actual,
                          const double expected,
                          double tolerance,
                          const bool is_tolerance_percent = true) {
  double difference = std::fabs(actual - expected);
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
            return Result("Line " + std::to_string(command->GetLine()) +
                          ": Verifier failed: " + std::to_string(*ptr) +
                          " == " + std::to_string(val) + ", at index " +
                          std::to_string(i));
          }
        } else {
          if (!IsEqualWithTolerance(static_cast<const double>(*ptr),
                                    static_cast<const double>(val), kEpsilon)) {
            return Result("Line " + std::to_string(command->GetLine()) +
                          ": Verifier failed: " + std::to_string(*ptr) +
                          " == " + std::to_string(val) + ", at index " +
                          std::to_string(i));
          }
        }
        break;
      case ProbeSSBOCommand::Comparator::kNotEqual:
        if (values[i].IsInteger()) {
          if (static_cast<uint64_t>(*ptr) == static_cast<uint64_t>(val)) {
            return Result("Line " + std::to_string(command->GetLine()) +
                          ": Verifier failed: " + std::to_string(*ptr) +
                          " != " + std::to_string(val) + ", at index " +
                          std::to_string(i));
          }
        } else {
          if (IsEqualWithTolerance(static_cast<const double>(*ptr),
                                   static_cast<const double>(val), kEpsilon)) {
            return Result("Line " + std::to_string(command->GetLine()) +
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
          return Result("Line " + std::to_string(command->GetLine()) +
                        ": Verifier failed: " + std::to_string(*ptr) +
                        " ~= " + std::to_string(val) + ", at index " +
                        std::to_string(i));
        }
        break;
      case ProbeSSBOCommand::Comparator::kLess:
        if (*ptr >= val) {
          return Result("Line " + std::to_string(command->GetLine()) +
                        ": Verifier failed: " + std::to_string(*ptr) + " < " +
                        std::to_string(val) + ", at index " +
                        std::to_string(i));
        }
        break;
      case ProbeSSBOCommand::Comparator::kLessOrEqual:
        if (*ptr > val) {
          return Result("Line " + std::to_string(command->GetLine()) +
                        ": Verifier failed: " + std::to_string(*ptr) +
                        " <= " + std::to_string(val) + ", at index " +
                        std::to_string(i));
        }
        break;
      case ProbeSSBOCommand::Comparator::kGreater:
        if (*ptr <= val) {
          return Result("Line " + std::to_string(command->GetLine()) +
                        ": Verifier failed: " + std::to_string(*ptr) + " > " +
                        std::to_string(val) + ", at index " +
                        std::to_string(i));
        }
        break;
      case ProbeSSBOCommand::Comparator::kGreaterOrEqual:
        if (*ptr < val) {
          return Result("Line " + std::to_string(command->GetLine()) +
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

// Convert data of |texel| into double values based on the
// information given in |texel_format|.
std::vector<Value> GetActualValuesFromTexel(const uint8_t* texel,
                                            const Format* texel_format) {
  assert(texel_format && !texel_format->GetComponents().empty());

  std::vector<Value> actual_values(texel_format->GetComponents().size());
  uint8_t bit_offset = 0;

  for (size_t i = 0; i < texel_format->GetComponents().size(); ++i) {
    const auto& component = texel_format->GetComponents()[i];
    uint8_t actual[8] = {};

    BitCopy::CopyMemoryToBuffer(actual, texel, bit_offset, component.num_bits);
    if (component.mode == FormatMode::kUFloat ||
        component.mode == FormatMode::kSFloat) {
      if (component.num_bits < 32) {
        actual_values[i].SetDoubleValue(static_cast<double>(
            BitCopy::HexFloatToFloat(actual, component.num_bits)));
      } else if (component.num_bits == 32) {
        float* ptr = reinterpret_cast<float*>(actual);
        actual_values[i].SetDoubleValue(static_cast<double>(*ptr));
      } else if (component.num_bits == 64) {
        double* ptr = reinterpret_cast<double*>(actual);
        actual_values[i].SetDoubleValue(*ptr);
      } else {
        assert(false && "Bits of component is not for double nor float type");
      }
    } else {
      uint16_t* ptr16 = nullptr;
      uint32_t* ptr32 = nullptr;
      uint64_t* ptr64 = nullptr;
      switch (component.num_bits) {
        case 8:
          actual_values[i].SetDoubleValue(static_cast<double>(*actual));
          break;
        case 16:
          ptr16 = reinterpret_cast<uint16_t*>(actual);
          actual_values[i].SetDoubleValue(static_cast<double>(*ptr16));
          break;
        case 32:
          ptr32 = reinterpret_cast<uint32_t*>(actual);
          actual_values[i].SetDoubleValue(static_cast<double>(*ptr32));
          break;
        case 64:
          ptr64 = reinterpret_cast<uint64_t*>(actual);
          actual_values[i].SetDoubleValue(static_cast<double>(*ptr64));
          break;
        default:
          assert(false && "Bits of component is not for integer type");
      }
    }

    bit_offset += component.num_bits;
  }

  return actual_values;
}

// If component mode of |texel_format| is FormatMode::kUNorm or
// ::kSNorm or ::kSRGB, scale the corresponding value in |texel|.
// Note that we do not scale values with FormatMode::kUInt, ::kSInt,
// ::kUFloat, ::kSFloat.
std::vector<Value> ScaleTexelValuesIfNeeded(const std::vector<Value>& texel,
                                            const Format* texel_format) {
  std::vector<Value> scaled_texel(texel.size());
  for (size_t i = 0; i < texel_format->GetComponents().size(); ++i) {
    const auto& component = texel_format->GetComponents()[i];

    double scaled_value = texel[i].AsDouble();
    switch (component.mode) {
      case FormatMode::kUNorm:
        scaled_value /= static_cast<double>((1 << component.num_bits) - 1);
        break;
      case FormatMode::kSNorm:
        scaled_value /=
            static_cast<double>((1 << (component.num_bits - 1)) - 1);
        break;
      case FormatMode::kUInt:
      case FormatMode::kSInt:
      case FormatMode::kUFloat:
      case FormatMode::kSFloat:
        break;
      case FormatMode::kSRGB:
        scaled_value /= static_cast<double>((1 << component.num_bits) - 1);
        if (component.type != FormatComponentType::kA)
          scaled_value = SRGBToLinearValue(scaled_value);
        break;
      case FormatMode::kUScaled:
      case FormatMode::kSScaled:
        assert(false &&
               "FormatMode::kUScaled and ::kSScaled are not implemented");
        break;
    }

    scaled_texel[i].SetDoubleValue(scaled_value);
  }

  return scaled_texel;
}

}  // namespace

Verifier::Verifier() = default;

Verifier::~Verifier() = default;

bool Verifier::IsTexelEqualToExpected(const std::vector<Value>& texel,
                                      const Format* texel_format,
                                      const ProbeCommand* command,
                                      const double* tolerance,
                                      const bool* is_tolerance_percent) {
  TexelErrorInfo texel_error_info = {};

  for (size_t i = 0; i < texel_format->GetComponents().size(); ++i) {
    const auto& component = texel_format->GetComponents()[i];

    double texel_for_component = texel[i].AsDouble();
    double scale_factor_for_error_report = 1;
    if (component.mode == FormatMode::kUNorm ||
        component.mode == FormatMode::kSNorm ||
        component.mode == FormatMode::kSRGB) {
      scale_factor_for_error_report = 255.0;
    }

    double expected = 0;
    double current_tolerance = 0;
    bool is_current_tolerance_percent = false;
    switch (component.type) {
      case FormatComponentType::kA:
        if (!command->IsRGBA()) {
          continue;
        }
        expected = static_cast<double>(command->GetA());
        current_tolerance = tolerance[3];
        is_current_tolerance_percent = is_tolerance_percent[3];
        texel_error_info.a =
            scale_factor_for_error_report * texel_for_component;
        break;
      case FormatComponentType::kR:
        expected = static_cast<double>(command->GetR());
        current_tolerance = tolerance[0];
        is_current_tolerance_percent = is_tolerance_percent[0];
        texel_error_info.r =
            scale_factor_for_error_report * texel_for_component;
        break;
      case FormatComponentType::kG:
        expected = static_cast<double>(command->GetG());
        current_tolerance = tolerance[1];
        is_current_tolerance_percent = is_tolerance_percent[1];
        texel_error_info.g =
            scale_factor_for_error_report * texel_for_component;
        break;
      case FormatComponentType::kB:
        expected = static_cast<double>(command->GetB());
        current_tolerance = tolerance[2];
        is_current_tolerance_percent = is_tolerance_percent[2];
        texel_error_info.b =
            scale_factor_for_error_report * texel_for_component;
        break;
      default:
        continue;
    }

    if (!IsEqualWithTolerance(expected, texel_for_component, current_tolerance,
                              is_current_tolerance_percent)) {
      if (actual_color_texel_.set_by_error)
        return false;

      texel_error_info.set_by_error = true;
    }
  }

  if (texel_error_info.set_by_error)
    actual_color_texel_ = texel_error_info;

  return !texel_error_info.set_by_error;
}

Result Verifier::Probe(const ProbeCommand* command,
                       const Format* texel_format,
                       uint32_t texel_stride,
                       uint32_t row_stride,
                       uint32_t frame_width,
                       uint32_t frame_height,
                       const void* buf) {
  if (!command)
    return Result("Verifier::Probe given ProbeCommand is nullptr");

  if (!texel_format)
    return Result("Verifier::Probe given texel's Format is nullptr");

  if (texel_format->GetFormatType() == FormatType::kUnknown)
    return Result("Verifier::Probe given texel's Format is unknown");

  if (!buf)
    return Result("Verifier::Probe given buffer to probe is nullptr");

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
        "Line " + std::to_string(command->GetLine()) +
        ": Verifier::Probe Position(" + std::to_string(x + width - 1) + ", " +
        std::to_string(y + height - 1) + ") is out of framebuffer scope (" +
        std::to_string(frame_width) + "," + std::to_string(frame_height) + ")");
  }

  if (row_stride < frame_width * texel_stride) {
    return Result("Line " + std::to_string(command->GetLine()) +
                  ": Verifier::Probe Row stride of " +
                  std::to_string(row_stride) + " is too small for " +
                  std::to_string(frame_width) + " texels of " +
                  std::to_string(texel_stride) + " bytes each");
  }

  double tolerance[4] = {0, 0, 0, 0};
  bool is_tolerance_percent[4] = {0, 0, 0, 0};
  SetupToleranceForTexels(command, tolerance, is_tolerance_percent);

  const uint8_t* ptr = static_cast<const uint8_t*>(buf);
  uint32_t count_of_invalid_pixels = 0;
  uint32_t first_invalid_i = 0;
  uint32_t first_invalid_j = 0;
  for (uint32_t j = 0; j < height; ++j) {
    const uint8_t* p = ptr + row_stride * (j + y) + texel_stride * x;
    for (uint32_t i = 0; i < width; ++i) {
      auto actual_texel_values =
          GetActualValuesFromTexel(p + texel_stride * i, texel_format);
      actual_texel_values =
          ScaleTexelValuesIfNeeded(actual_texel_values, texel_format);
      if (!IsTexelEqualToExpected(actual_texel_values, texel_format, command,
                                  tolerance, is_tolerance_percent)) {
        if (!count_of_invalid_pixels) {
          first_invalid_i = i;
          first_invalid_j = j;
        }
        ++count_of_invalid_pixels;
      }
    }
  }

  if (count_of_invalid_pixels) {
    const auto& component = texel_format->GetComponents().back();
    float scale_factor_for_error_report = 1.0f;
    if (component.mode == FormatMode::kUNorm ||
        component.mode == FormatMode::kSNorm ||
        component.mode == FormatMode::kSRGB) {
      scale_factor_for_error_report = 255.0f;
    }

    return Result(
        "Line " + std::to_string(command->GetLine()) +
        ": Probe failed at: " + std::to_string(x + first_invalid_i) + ", " +
        std::to_string(first_invalid_j + y) + "\n" + "  Expected RGBA: " +
        std::to_string(command->GetR() * scale_factor_for_error_report) + ", " +
        std::to_string(command->GetG() * scale_factor_for_error_report) + ", " +
        std::to_string(command->GetB() * scale_factor_for_error_report) +
        (command->IsRGBA() ? ", " +
                                 std::to_string(command->GetA() *
                                                scale_factor_for_error_report) +
                                 "\n  Actual RGBA: "
                           : "\n  Actual RGB: ") +
        std::to_string(actual_color_texel_.r) + ", " +
        std::to_string(actual_color_texel_.g) + ", " +
        std::to_string(actual_color_texel_.b) +
        (command->IsRGBA() ? ", " + std::to_string(actual_color_texel_.a)
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
  if (!cpu_memory) {
    return values.empty() ? Result()
                          : Result(
                                "Verifier::ProbeSSBO actual data is empty "
                                "while expected data is not");
  }

  const auto& datum_type = command->GetDatumType();
  size_t bytes_per_elem = datum_type.SizeInBytes() / datum_type.RowCount() /
                          datum_type.ColumnCount();
  size_t offset = static_cast<size_t>(command->GetOffset());

  if (values.size() * bytes_per_elem + offset > size_in_bytes) {
    return Result(
        "Line " + std::to_string(command->GetLine()) +
        ": Verifier::ProbeSSBO has more expected values than SSBO size");
  }

  if (offset % bytes_per_elem != 0) {
    return Result(
        "Line " + std::to_string(command->GetLine()) +
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

  return Result("Line " + std::to_string(command->GetLine()) +
                ": Verifier::ProbeSSBO unknown datum type");
}

}  // namespace amber
