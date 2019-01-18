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

#ifndef SRC_VERIFIER_H_
#define SRC_VERIFIER_H_

#include <vector>

#include "amber/result.h"
#include "src/command.h"
#include "src/format.h"

namespace amber {

/// The verifier is used to validate if a probe command is successful or not.
class Verifier {
 public:
  /// Create a verifier.
  Verifier();
  ~Verifier();

  /// Check |command| against |buf|. The result will be success if the probe
  /// passes correctly.
  Result Probe(const ProbeCommand* command,
               const Format* texel_format,
               uint32_t texel_stride,
               uint32_t row_stride,
               uint32_t frame_width,
               uint32_t frame_height,
               const void* buf);

  /// Check |command| against |cpu_memory|. The result will be success if the
  /// probe passes correctly.
  Result ProbeSSBO(const ProbeSSBOCommand* command,
                   size_t size,
                   const void* cpu_memory);

 private:
  struct TexelErrorInfo {
    bool set_by_error = false;
    double r = 0;
    double g = 0;
    double b = 0;
    double a = 0;
  };

  /// Check texel pointed by |texel| with |texel_format| is the same with
  /// the expected RGB(A) values given via |command|. This method allow
  /// error smaller than |tolerance|. If an element of
  /// |is_tolerance_percent| is true, we assume that the corresponding
  /// |tolerance| is relative i.e., percentage allowed error.
  bool IsTexelEqualToExpected(const std::vector<Value>& texel,
                              const Format* texel_format,
                              const ProbeCommand* command,
                              const double* tolerance,
                              const bool* is_tolerance_percent);

  /// When IsTexelEqualToExpected() returns false because the actual color
  /// texel is different from the expected one, it keeps the actual color
  /// texel values in |actual_color_texel_| and set
  /// |actual_color_texel_.set_by_error| as true to provide users with
  /// this information later.
  TexelErrorInfo actual_color_texel_ = {};
};

}  // namespace amber

#endif  // SRC_VERIFIER_H_
