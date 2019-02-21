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

#include <cctype>
#include <cstdlib>
#include <memory>
#include <string>

#include "src/amberscript/parser.h"
#include "src/descriptor_set_and_binding_parser.h"
#include "src/engine.h"
#include "src/executor.h"
#include "src/make_unique.h"
#include "src/parser.h"
#include "src/vkscript/parser.h"

namespace amber {

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

  Result r = engine->Initialize(opts->config, script->GetRequiredFeatures(),
                                script->GetRequiredInstanceExtensions(),
                                script->GetRequiredDeviceExtensions());
  if (!r.IsSuccess())
    return r;

  Executor executor;
  Result executor_result = executor.Execute(
      engine.get(), script, shader_data,
      opts->pipeline_create_only ? ExecutionType::kPipelineCreateOnly
                                 : ExecutionType::kExecute);
  // Hold the executor result until the extractions are complete. This will let
  // us dump any buffers requested even on failure.

  // The dump process holds onto the results and terminates the loop if any dump
  // fails. This will allow us to validate |extractor_result| first as if the
  // extractor fails before running the pipeline that will trigger the dumps
  // to almost always fail.
  for (BufferInfo& buffer_info : opts->extractions) {
    if (buffer_info.buffer_name == "framebuffer") {
      ResourceInfo info;
      r = engine->GetFrameBufferInfo(&info);
      if (!r.IsSuccess())
        break;

      buffer_info.width = info.image_info.width;
      buffer_info.height = info.image_info.height;
      r = engine->GetFrameBuffer(&(buffer_info.values));
      if (!r.IsSuccess())
        break;

      continue;
    }

    DescriptorSetAndBindingParser desc_set_and_binding_parser;
    r = desc_set_and_binding_parser.Parse(buffer_info.buffer_name);
    if (!r.IsSuccess())
      break;

    ResourceInfo info = ResourceInfo();
    r = engine->GetDescriptorInfo(
        desc_set_and_binding_parser.GetDescriptorSet(),
        desc_set_and_binding_parser.GetBinding(), &info);
    if (!r.IsSuccess())
      break;

    const uint8_t* ptr = static_cast<const uint8_t*>(info.cpu_memory);
    auto& values = buffer_info.values;
    for (size_t i = 0; i < info.size_in_bytes; ++i) {
      values.emplace_back();
      values.back().SetIntValue(*ptr);
      ++ptr;
    }
  }

  if (!executor_result.IsSuccess()) {
    engine->Shutdown();
    return executor_result;
  }
  if (!r.IsSuccess()) {
    engine->Shutdown();
    return r;
  }
  return engine->Shutdown();
}

}  // namespace amber
