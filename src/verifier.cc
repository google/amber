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
#include <cstring>
#include <string>
#include <vector>

#include "src/command.h"

namespace amber {
namespace {

const uint8_t kBitsPerByte = 8;
const double kEpsilon = 0.000001;
const double kDefaultTexelTolerance = 0.002;

// Copy [src_bit_offset, src_bit_offset + bits) bits of |src| to
// [0, bits) of |dst|.
void CopyBitsOfMemoryToBuffer(uint8_t* dst,
                              const uint8_t* src,
                              uint8_t src_bit_offset,
                              uint8_t bits) {
  while (src_bit_offset > static_cast<uint8_t>(7)) {
    ++src;
    src_bit_offset = static_cast<uint8_t>(src_bit_offset - kBitsPerByte);
  }

  // Number of bytes greater than or equal to |(src_bit_offset + bits) / 8|.
  const uint8_t size_in_bytes =
      static_cast<uint8_t>((src_bit_offset + bits + 7) / kBitsPerByte);
  assert(size_in_bytes <= static_cast<uint8_t>(kBitsPerByte));

  uint64_t data = 0;
  uint8_t* ptr = reinterpret_cast<uint8_t*>(&data);
  for (uint8_t i = 0; i < size_in_bytes; ++i) {
    ptr[i] = src[i];
  }

  data >>= src_bit_offset;
  if (bits != 64)
    data &= (1ULL << bits) - 1ULL;

  std::memcpy(dst, &data, static_cast<size_t>((bits + 7) / kBitsPerByte));
}

// Convert float |value| whose size is 16 bits to 32 bits float
// based on IEEE-754.
float HexFloat16ToFloat(const uint8_t* value) {
  uint32_t sign = (static_cast<uint32_t>(value[1]) & 0x80) << 24U;
  uint32_t exponent = (((static_cast<uint32_t>(value[1]) & 0x7c) >> 2U) + 112U)
                      << 23U;
  uint32_t mantissa = ((static_cast<uint32_t>(value[1]) & 0x3) << 8U |
                       static_cast<uint32_t>(value[0]))
                      << 13U;

  uint32_t hex = sign | exponent | mantissa;
  float* hex_float = reinterpret_cast<float*>(&hex);
  return *hex_float;
}

// Convert float |value| whose size is 11 bits to 32 bits float
// based on IEEE-754.
float HexFloat11ToFloat(const uint8_t* value) {
  uint32_t exponent = (((static_cast<uint32_t>(value[1]) << 2U) |
                        ((static_cast<uint32_t>(value[0]) & 0xc0) >> 6U)) +
                       112U)
                      << 23U;
  uint32_t mantissa = (static_cast<uint32_t>(value[0]) & 0x3f) << 17U;

  uint32_t hex = exponent | mantissa;
  float* hex_float = reinterpret_cast<float*>(&hex);
  return *hex_float;
}

// Convert float |value| whose size is 10 bits to 32 bits float
// based on IEEE-754.
float HexFloat10ToFloat(const uint8_t* value) {
  uint32_t exponent = (((static_cast<uint32_t>(value[1]) << 3U) |
                        ((static_cast<uint32_t>(value[0]) & 0xe0) >> 5U)) +
                       112U)
                      << 23U;
  uint32_t mantissa = (static_cast<uint32_t>(value[0]) & 0x1f) << 18U;

  uint32_t hex = exponent | mantissa;
  float* hex_float = reinterpret_cast<float*>(&hex);
  return *hex_float;
}

// Convert float |value| whose size is |bits| bits to 32 bits float
// based on IEEE-754.
// See https://www.khronos.org/opengl/wiki/Small_Float_Formats
// and https://en.wikipedia.org/wiki/IEEE_754.
//
//    Sign Exponent Mantissa Exponent-Bias
// 16    1        5       10            15
// 11    0        5        6            15
// 10    0        5        5            15
// 32    1        8       23           127
// 64    1       11       52          1023
//
// 11 and 10 bits floats are always positive.
// 14 bits float is used only RGB9_E5 format in OpenGL but it does not exist
// in Vulkan.
float HexFloatToFloat(const uint8_t* value, uint8_t bits) {
  switch (bits) {
    case 10:
      return HexFloat10ToFloat(value);
    case 11:
      return HexFloat11ToFloat(value);
    case 16:
      return HexFloat16ToFloat(value);
  }

  assert(false && "Invalid bits");
  return 0;
}

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
// information given in |framebuffer_format|.
std::vector<double> GetActualValuesFromTexel(const uint8_t* texel,
                                             const Format* framebuffer_format) {
  assert(framebuffer_format && !framebuffer_format->GetComponents().empty());

  std::vector<double> actual_values(framebuffer_format->GetComponents().size());
  uint8_t bit_offset = 0;

  for (size_t i = 0; i < framebuffer_format->GetComponents().size(); ++i) {
    const auto& component = framebuffer_format->GetComponents()[i];
    uint8_t actual[8] = {0, 0, 0, 0, 0, 0, 0, 0};

    CopyBitsOfMemoryToBuffer(actual, texel, bit_offset, component.num_bits);
    if (component.mode == FormatMode::kUFloat ||
        component.mode == FormatMode::kSFloat) {
      if (component.num_bits < 32) {
        actual_values[i] =
            static_cast<double>(HexFloatToFloat(actual, component.num_bits));
      } else if (component.num_bits == 32) {
        float* ptr = reinterpret_cast<float*>(actual);
        actual_values[i] = static_cast<double>(*ptr);
      } else if (component.num_bits == 64) {
        double* ptr = reinterpret_cast<double*>(actual);
        actual_values[i] = *ptr;
      } else {
        assert(false && "Bits of component is not for double nor float type");
      }
    } else {
      if (component.mode == FormatMode::kSInt ||
          component.mode == FormatMode::kSNorm) {
        switch (component.num_bits) {
          case 8: {
            int8_t* ptr8 = nullptr;
            ptr8 = reinterpret_cast<int8_t*>(actual);
            actual_values[i] = static_cast<double>(*ptr8);
            break;
          }
          case 16: {
            int16_t* ptr16 = nullptr;
            ptr16 = reinterpret_cast<int16_t*>(actual);
            actual_values[i] = static_cast<double>(*ptr16);
            break;
          }
          case 32: {
            int32_t* ptr32 = nullptr;
            ptr32 = reinterpret_cast<int32_t*>(actual);
            actual_values[i] = static_cast<double>(*ptr32);
            break;
          }
          case 64: {
            int64_t* ptr64 = nullptr;
            ptr64 = reinterpret_cast<int64_t*>(actual);
            actual_values[i] = static_cast<double>(*ptr64);
            break;
          }
          default: {
            assert(false && "Bits of component is not for integer type");
          }
        }
      } else {
        switch (component.num_bits) {
          case 8: {
            actual_values[i] = static_cast<double>(*actual);
            break;
          }
          case 16: {
            uint16_t* ptr16 = nullptr;
            ptr16 = reinterpret_cast<uint16_t*>(actual);
            actual_values[i] = static_cast<double>(*ptr16);
            break;
          }
          case 32: {
            uint32_t* ptr32 = nullptr;
            ptr32 = reinterpret_cast<uint32_t*>(actual);
            actual_values[i] = static_cast<double>(*ptr32);
            break;
          }
          case 64: {
            uint64_t* ptr64 = nullptr;
            ptr64 = reinterpret_cast<uint64_t*>(actual);
            actual_values[i] = static_cast<double>(*ptr64);
            break;
          }
          default: {
            assert(false && "Bits of component is not for integer type");
          }
        }
      }
    }

    bit_offset = static_cast<uint8_t>(bit_offset + component.num_bits);
  }

  return actual_values;
}

// If component mode of |framebuffer_format| is FormatMode::kUNorm or
// ::kSNorm or ::kSRGB, scale the corresponding value in |texel|.
// Note that we do not scale values with FormatMode::kUInt, ::kSInt,
// ::kUFloat, ::kSFloat.
void ScaleTexelValuesIfNeeded(std::vector<double>* texel,
                              const Format* framebuffer_format) {
  assert(framebuffer_format->GetComponents().size() == texel->size());
  for (size_t i = 0; i < framebuffer_format->GetComponents().size(); ++i) {
    const auto& component = framebuffer_format->GetComponents()[i];

    double scaled_value = (*texel)[i];
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

    (*texel)[i] = scaled_value;
  }
}

/// Check |texel| with |texel_format| is the same with the expected
/// RGB(A) values given via |command|. This method allow error
/// smaller than |tolerance|. If an element of
/// |is_tolerance_percent| is true, we assume that the corresponding
/// |tolerance| is relative i.e., percentage allowed error.
bool IsTexelEqualToExpected(const std::vector<double>& texel,
                            const Format* framebuffer_format,
                            const ProbeCommand* command,
                            const double* tolerance,
                            const bool* is_tolerance_percent) {
  for (size_t i = 0; i < framebuffer_format->GetComponents().size(); ++i) {
    const auto& component = framebuffer_format->GetComponents()[i];

    double texel_for_component = texel[i];
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
        break;
      case FormatComponentType::kR:
        expected = static_cast<double>(command->GetR());
        current_tolerance = tolerance[0];
        is_current_tolerance_percent = is_tolerance_percent[0];
        break;
      case FormatComponentType::kG:
        expected = static_cast<double>(command->GetG());
        current_tolerance = tolerance[1];
        is_current_tolerance_percent = is_tolerance_percent[1];
        break;
      case FormatComponentType::kB:
        expected = static_cast<double>(command->GetB());
        current_tolerance = tolerance[2];
        is_current_tolerance_percent = is_tolerance_percent[2];
        break;
      default:
        continue;
    }

    if (!IsEqualWithTolerance(expected, texel_for_component, current_tolerance,
                              is_current_tolerance_percent)) {
      return false;
    }
  }

  return true;
}

std::vector<double> GetTexelInRGBA(const std::vector<double>& texel,
                                   const Format* framebuffer_format) {
  std::vector<double> texel_in_rgba(texel.size());
  for (size_t i = 0; i < framebuffer_format->GetComponents().size(); ++i) {
    const auto& component = framebuffer_format->GetComponents()[i];
    switch (component.type) {
      case FormatComponentType::kR:
        texel_in_rgba[0] = texel[i];
        break;
      case FormatComponentType::kG:
        texel_in_rgba[1] = texel[i];
        break;
      case FormatComponentType::kB:
        texel_in_rgba[2] = texel[i];
        break;
      case FormatComponentType::kA:
        texel_in_rgba[3] = texel[i];
        break;
      default:
        continue;
    }
  }
  return texel_in_rgba;
}

}  // namespace

Verifier::Verifier() = default;

Verifier::~Verifier() = default;

Result Verifier::Probe(const ProbeCommand* command,
                       const Format* framebuffer_format,
                       uint32_t texel_stride,
                       uint32_t row_stride,
                       uint32_t frame_width,
                       uint32_t frame_height,
                       const void* buf) {
  if (!command)
    return Result("Verifier::Probe given ProbeCommand is nullptr");
  if (!framebuffer_format)
    return Result("Verifier::Probe given texel's Format is nullptr");
  if (!buf)
    return Result("Verifier::Probe given buffer to probe is nullptr");

  uint32_t x = 0;
  uint32_t y = 0;
  uint32_t width = 1;
  uint32_t height = 1;

  if (command->IsWholeWindow()) {
    width = frame_width;
    height = frame_height;
  } else if (command->IsRelative()) {
    x = static_cast<uint32_t>(static_cast<float>(frame_width) *
                              command->GetX());
    y = static_cast<uint32_t>(static_cast<float>(frame_height) *
                              command->GetY());
    if (command->IsProbeRect()) {
      width = static_cast<uint32_t>(static_cast<float>(frame_width) *
                                    command->GetWidth());
      height = static_cast<uint32_t>(static_cast<float>(frame_height) *
                                     command->GetHeight());
    }
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
  std::vector<double> actual_texel_values_on_failure;
  for (uint32_t j = 0; j < height; ++j) {
    const uint8_t* p = ptr + row_stride * (j + y) + texel_stride * x;
    for (uint32_t i = 0; i < width; ++i) {
      auto actual_texel_values =
          GetActualValuesFromTexel(p + texel_stride * i, framebuffer_format);
      ScaleTexelValuesIfNeeded(&actual_texel_values, framebuffer_format);
      if (!IsTexelEqualToExpected(actual_texel_values, framebuffer_format,
                                  command, tolerance, is_tolerance_percent)) {
        if (!count_of_invalid_pixels) {
          actual_texel_values_on_failure =
              GetTexelInRGBA(actual_texel_values, framebuffer_format);
          first_invalid_i = i;
          first_invalid_j = j;
        }
        ++count_of_invalid_pixels;
      }
    }
  }

  if (count_of_invalid_pixels) {
    const auto& component = framebuffer_format->GetComponents().back();
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
        std::to_string(static_cast<float>(actual_texel_values_on_failure[0]) *
                       scale_factor_for_error_report) +
        ", " +
        std::to_string(static_cast<float>(actual_texel_values_on_failure[1]) *
                       scale_factor_for_error_report) +
        ", " +
        std::to_string(static_cast<float>(actual_texel_values_on_failure[2]) *
                       scale_factor_for_error_report) +
        (command->IsRGBA()
             ? ", " + std::to_string(static_cast<float>(
                                         actual_texel_values_on_failure[3]) *
                                     scale_factor_for_error_report)
             : "") +
        "\n" + "Probe failed in " + std::to_string(count_of_invalid_pixels) +
        " pixels");
  }

  return {};
}

Result Verifier::ProbeSSBO(const ProbeSSBOCommand* command,
                           uint32_t buffer_element_count,
                           const void* buffer) {
  const auto& values = command->GetValues();
  if (!buffer) {
    if (values.empty())
      return {};
    return Result(
        "Verifier::ProbeSSBO actual data is empty while expected "
        "data is not");
  }

  auto* fmt = command->GetFormat();
  size_t elem_count = values.size() / fmt->ValuesPerElement();
  size_t offset = static_cast<size_t>(command->GetOffset());
  size_t size_in_bytes = buffer_element_count * fmt->SizeInBytes();
  if ((elem_count * fmt->SizeInBytes()) + offset > size_in_bytes) {
    return Result("Line " + std::to_string(command->GetLine()) +
                  ": Verifier::ProbeSSBO request to access to byte " +
                  std::to_string((elem_count * fmt->SizeInBytes()) + offset) +
                  " would read outside buffer of size " +
                  std::to_string(size_in_bytes) + " bytes");
  }

  if (offset % fmt->SizeInBytes() != 0) {
    return Result("Line " + std::to_string(command->GetLine()) +
                  ": Verifier::ProbeSSBO given offset (" +
                  std::to_string(offset) + ") " +
                  "is not multiple of element size (" +
                  std::to_string(fmt->SizeInBytes()) + ")");
  }

  const uint8_t* ptr = static_cast<const uint8_t*>(buffer) + offset;
  if (fmt->IsInt8())
    return CheckValue<int8_t>(command, ptr, values);
  if (fmt->IsUint8())
    return CheckValue<uint8_t>(command, ptr, values);
  if (fmt->IsInt16())
    return CheckValue<int16_t>(command, ptr, values);
  if (fmt->IsUint16())
    return CheckValue<uint16_t>(command, ptr, values);
  if (fmt->IsInt32())
    return CheckValue<int32_t>(command, ptr, values);
  if (fmt->IsUint32())
    return CheckValue<uint32_t>(command, ptr, values);
  if (fmt->IsInt64())
    return CheckValue<int64_t>(command, ptr, values);
  if (fmt->IsUint64())
    return CheckValue<uint64_t>(command, ptr, values);
  if (fmt->IsFloat())
    return CheckValue<float>(command, ptr, values);
  if (fmt->IsDouble())
    return CheckValue<double>(command, ptr, values);

  return Result("Line " + std::to_string(command->GetLine()) +
                ": Verifier::ProbeSSBO unknown datum type");
}

}  // namespace amber
