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

#ifndef SRC_AMBERSCRIPT_EXECUTOR_H_
#define SRC_AMBERSCRIPT_EXECUTOR_H_

#include "amber/result.h"
#include "src/engine.h"
#include "src/executor.h"
#include "src/script.h"

namespace amber {
namespace amberscript {

class Executor : public amber::Executor {
 public:
  Executor();
  ~Executor() override;

  Result Execute(Engine*, const amber::Script*) override;
};

}  // namespace amberscript
}  // namespace amber

#endif  // SRC_AMBERSCRIPT_EXECUTOR_H_
