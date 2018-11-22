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

#include "amber/result.h"
#include "src/command.h"

namespace amber {

class Verifier {
 public:
  Verifier();
  ~Verifier();

  Result Probe(const ProbeCommand*,
               uint32_t stride,
               uint32_t frame_width,
               uint32_t frame_height,
               const void* buf);
  Result ProbeSSBO(const ProbeSSBOCommand*);
  Result Tolerance(const ToleranceCommand*);
};

}  // namespace amber

#endif  // SRC_VERIFIER_H_
