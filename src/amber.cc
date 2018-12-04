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

#include "amber/amber.h"

#include <memory>
#include <string>

#include "src/amberscript/executor.h"
#include "src/amberscript/parser.h"
#include "src/engine.h"
#include "src/executor.h"
#include "src/make_unique.h"
#include "src/parser.h"
#include "src/vkscript/executor.h"
#include "src/vkscript/parser.h"

namespace amber {

Amber::Amber() = default;

Amber::~Amber() = default;

amber::Result Amber::Execute(const std::string& input, const Options& opts) {
  std::unique_ptr<Parser> parser;
  std::unique_ptr<Executor> executor;
  if (input.substr(0, 7) == "#!amber") {
    parser = MakeUnique<amberscript::Parser>();
    executor = MakeUnique<amberscript::Executor>();
  } else {
    parser = MakeUnique<vkscript::Parser>();
    executor = MakeUnique<vkscript::Executor>();
  }

  Result r = parser->Parse(input);
  if (!r.IsSuccess())
    return r;

  if (opts.parse_only)
    return {};

  auto engine = Engine::Create(opts.engine);
  if (!engine)
    return Result("Failed to create engine");

  const auto* script = parser->GetScript();

  if (opts.config) {
    r = engine->InitializeWithConfig(opts.config.get(),
                                     script->RequiredFeatures(),
                                     script->RequiredExtensions());
  } else {
    r = engine->Initialize(script->RequiredFeatures(),
                           script->RequiredExtensions());
  }

  if (!r.IsSuccess())
    return r;

  r = executor->Execute(engine.get(), script);
  if (!r.IsSuccess())
    return r;

  return engine->Shutdown();
}

}  // namespace amber
