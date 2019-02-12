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

#include "src/amberscript/parser.h"
#include "src/engine.h"
#include "src/executor.h"
#include "src/make_unique.h"
#include "src/parser.h"
#include "src/vkscript/parser.h"

namespace amber {

EngineConfig::~EngineConfig() = default;

Amber::Amber() = default;

Amber::~Amber() = default;

amber::Result Amber::Parse(const std::string& input, amber::Recipe* recipe) {
  if (!recipe)
    return Result("Recipe must be provided to Parse.");

  std::unique_ptr<Parser> parser;
  if (input.substr(0, 7) == "#!amber")
    parser = MakeUnique<amberscript::Parser>();
  else
    parser = MakeUnique<vkscript::Parser>();

  Result r = parser->Parse(input);
  if (!r.IsSuccess())
    return r;

  recipe->SetImpl(parser->GetScript().release());
  return {};
}

amber::Result Amber::Execute(const amber::Recipe* recipe, Options* opts) {
  ShaderMap map;
  return ExecuteWithShaderData(recipe, opts, map);
}

amber::Result Amber::ExecuteWithShaderData(const amber::Recipe* recipe,
                                           Options* opts,
                                           const ShaderMap& shader_data) {
  if (!recipe)
    return Result("Attempting to execute and invalid recipe");

  Script* script = static_cast<Script*>(recipe->GetImpl());
  if (!script)
    return Result("Recipe must contain a parsed script");

  script->SetSpvTargetEnv(opts->spv_env);

  auto engine = Engine::Create(opts->engine);
  if (!engine)
    return Result("Failed to create engine");

  Result r = engine->Initialize(opts->config, script->RequiredFeatures(),
                                script->RequiredExtensions());
  if (!r.IsSuccess())
    return r;

  Executor executor;
  r = executor.Execute(engine.get(), script, shader_data);
  if (!r.IsSuccess()) {
    // Clean up Vulkan/Dawn objects
    engine->Shutdown();
    return r;
  }

  for (BufferInfo& buffer_info : opts->extractions) {
    if (buffer_info.buffer_name == "framebuffer") {
      ResourceInfo info;
      r = engine->GetFrameBufferInfo(&info);
      if (!r.IsSuccess()) {
        engine->Shutdown();
        return r;
      }
      buffer_info.width = info.image_info.width;
      buffer_info.height = info.image_info.height;
      r = engine->GetFrameBuffer(&(buffer_info.values));
      if (!r.IsSuccess()) {
        engine->Shutdown();
        return r;
      }
    }
  }

  return engine->Shutdown();
}

}  // namespace amber
