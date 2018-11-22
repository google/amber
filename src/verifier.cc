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

namespace amber {
namespace {

const float kEpsilon = 0.002f;

bool IsFloatPixelEqualInt(float pixel, uint8_t expected) {
  // TODO(jaebaek): Change kEpsilon to tolerance.
  return std::fabs(pixel - static_cast<float>(expected) / 255.0f) < kEpsilon;
}

}  // namespace

Verifier::Verifier() = default;

Verifier::~Verifier() = default;

Result Verifier::Probe(const ProbeCommand* command,
                       uint32_t stride,
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
    x = static_cast<uint32_t>(frame_width * command->GetX());
    y = static_cast<uint32_t>(frame_height * command->GetY());
    width = static_cast<uint32_t>(frame_width * command->GetWidth());
    height = static_cast<uint32_t>(frame_height * command->GetHeight());
  } else {
    x = static_cast<uint32_t>(command->GetX());
    y = static_cast<uint32_t>(command->GetY());
    width = static_cast<uint32_t>(command->GetWidth());
    height = static_cast<uint32_t>(command->GetHeight());
  }

  if (x + width > frame_width || y + height > frame_height) {
    return Result(
        "Vulkan::Probe Position(" + std::to_string(x + width - 1) + ", " +
        std::to_string(y + height - 1) + ") is out of framebuffer scope (" +
        std::to_string(frame_width) + "," + std::to_string(frame_height) + ")");
  }

  // TODO(jaebaek): Support all VkFormat
  const uint8_t* ptr = static_cast<const uint8_t*>(buf);
  uint32_t count_of_invalid_pixels = 0;
  uint32_t first_invalid_i = 0;
  uint32_t first_invalid_j = 0;
  for (uint32_t j = 0; j < height; ++j) {
    const uint8_t* p = ptr + stride * frame_width * (j + y) + stride * x;
    for (uint32_t i = 0; i < width; ++i) {
      // TODO(jaebaek): Get actual pixel values based on frame buffer formats.
      if (!IsFloatPixelEqualInt(command->GetR(), p[stride * i]) ||
          !IsFloatPixelEqualInt(command->GetG(), p[stride * i + 1]) ||
          !IsFloatPixelEqualInt(command->GetB(), p[stride * i + 2]) ||
          (command->IsRGBA() &&
           !IsFloatPixelEqualInt(command->GetA(), p[stride * i + 3]))) {
        if (!count_of_invalid_pixels) {
          first_invalid_i = i;
          first_invalid_j = j;
        }
        ++count_of_invalid_pixels;
      }
    }
  }

  if (count_of_invalid_pixels) {
    const uint8_t* p =
        ptr + stride * frame_width * (first_invalid_j + y) + stride * x;
    return Result(
        "Probe failed at: " + std::to_string(first_invalid_i + x) + ", " +
        std::to_string(first_invalid_j + y) + "\n" +
        "  Expected RGBA: " + std::to_string(command->GetR() * 255) + ", " +
        std::to_string(command->GetG() * 255) + ", " +
        std::to_string(command->GetB() * 255) +
        (command->IsRGBA() ? ", " + std::to_string(command->GetA() * 255) +
                                 "\n  Actual RGBA: "
                           : "\n  Actual RGB: ") +
        std::to_string(static_cast<int>(p[stride * first_invalid_i])) + ", " +
        std::to_string(static_cast<int>(p[stride * first_invalid_i + 1])) +
        ", " +
        std::to_string(static_cast<int>(p[stride * first_invalid_i + 2])) +
        (command->IsRGBA() ? ", " + std::to_string(static_cast<int>(
                                        p[stride * first_invalid_i + 3]))
                           : "") +
        "\n" + "Probe failed in " + std::to_string(count_of_invalid_pixels) +
        " pixels");
  }

  return {};
}

Result Verifier::ProbeSSBO(const ProbeSSBOCommand*) {
  return Result("Verifier::ProbeSSBO Not Implemented");
}

Result Verifier::Tolerance(const ToleranceCommand*) {
  return Result("Verifier::Tolerance Not Implemented");
}

}  // namespace amber
