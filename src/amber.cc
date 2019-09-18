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
namespace {

const FormatType kDefaultFramebufferFormat = FormatType::kB8G8R8A8_UNORM;

Result GetFrameBuffer(Buffer* buffer, std::vector<Value>* values) {
  values->clear();

  // TODO(jaebaek): Support other formats
  if (buffer->GetFormat()->GetFormatType() != kDefaultFramebufferFormat)
    return Result("GetFrameBuffer Unsupported buffer format");

  const uint8_t* cpu_memory = buffer->ValuePtr()->data();
  if (!cpu_memory)
    return Result("GetFrameBuffer missing memory pointer");

  const auto texel_stride = buffer->GetElementStride();
  const auto row_stride = buffer->GetRowStride();

  for (uint32_t y = 0; y < buffer->GetHeight(); ++y) {
    for (uint32_t x = 0; x < buffer->GetWidth(); ++x) {
      Value pixel;

      const uint8_t* ptr_8 = cpu_memory + (row_stride * y) + (texel_stride * x);
      const uint32_t* ptr_32 = reinterpret_cast<const uint32_t*>(ptr_8);
      pixel.SetIntValue(*ptr_32);
      values->push_back(pixel);
    }
  }

  return {};
}

}  // namespace

EngineConfig::~EngineConfig() = default;

Options::Options()
    : engine(amber::EngineType::kEngineTypeVulkan),
      config(nullptr),
      execution_type(ExecutionType::kExecute),
      disable_spirv_validation(false),
      delegate(nullptr) {}

Options::~Options() = default;

BufferInfo::BufferInfo() : is_image_buffer(false), width(0), height(0) {}

BufferInfo::BufferInfo(const BufferInfo&) = default;

BufferInfo::~BufferInfo() = default;

BufferInfo& BufferInfo::operator=(const BufferInfo&) = default;

Delegate::~Delegate() = default;

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

namespace {

// Create an engine initialize it, and check the recipe's requirements.
// Returns a failing result if anything fails.  Otherwise pass the created
// engine out through |engine_ptr| and the script via |script|.  The |script|
// pointer is borrowed, and should not be freed.
Result CreateEngineAndCheckRequirements(const Recipe* recipe,
                                        Options* opts,
                                        std::unique_ptr<Engine>* engine_ptr,
                                        Script** script_ptr) {
  if (!recipe)
    return Result("Attempting to check an invalid recipe");

  Script* script = static_cast<Script*>(recipe->GetImpl());
  if (!script)
    return Result("Recipe must contain a parsed script");

  script->SetSpvTargetEnv(opts->spv_env);

  auto engine = Engine::Create(opts->engine);
  if (!engine) {
    return Result("Failed to create engine");
  }

  // Engine initialization checks requirements.  Current backends don't do
  // much else.  Refactor this if they end up doing to much here.
  Result r = engine->Initialize(opts->config, opts->delegate,
                                script->GetRequiredFeatures(),
                                script->GetRequiredInstanceExtensions(),
                                script->GetRequiredDeviceExtensions());
  if (!r.IsSuccess())
    return r;

  *engine_ptr = std::move(engine);
  *script_ptr = script;

  return r;
}
}  // namespace

amber::Result Amber::AreAllRequirementsSupported(const amber::Recipe* recipe,
                                                 Options* opts) {
  std::unique_ptr<Engine> engine;
  Script* script = nullptr;

  return CreateEngineAndCheckRequirements(recipe, opts, &engine, &script);
}

amber::Result Amber::Execute(const amber::Recipe* recipe, Options* opts) {
  ShaderMap map;
  return ExecuteWithShaderData(recipe, opts, map);
}

amber::Result Amber::ExecuteWithShaderData(const amber::Recipe* recipe,
                                           Options* opts,
                                           const ShaderMap& shader_data) {
  std::unique_ptr<Engine> engine;
  Script* script = nullptr;
  Result r = CreateEngineAndCheckRequirements(recipe, opts, &engine, &script);
  if (!r.IsSuccess())
    return r;
  script->SetSpvTargetEnv(opts->spv_env);

  Executor executor;
  Result executor_result =
      executor.Execute(engine.get(), script, shader_data, opts);
  // Hold the executor result until the extractions are complete. This will let
  // us dump any buffers requested even on failure.

  if (script->GetPipelines().empty()) {
    if (!executor_result.IsSuccess())
      return executor_result;
    return {};
  }

  // TODO(dsinclair): Figure out how extractions work with multiple pipelines.
  auto* pipeline = script->GetPipelines()[0].get();

  // The dump process holds onto the results and terminates the loop if any dump
  // fails. This will allow us to validate |extractor_result| first as if the
  // extractor fails before running the pipeline that will trigger the dumps
  // to almost always fail.
  for (BufferInfo& buffer_info : opts->extractions) {
    if (buffer_info.is_image_buffer) {
      auto* buffer = script->GetBuffer(buffer_info.buffer_name);
      if (!buffer)
        break;

      buffer_info.width = buffer->GetWidth();
      buffer_info.height = buffer->GetHeight();
      r = GetFrameBuffer(buffer, &(buffer_info.values));
      if (!r.IsSuccess())
        break;

      continue;
    }

    DescriptorSetAndBindingParser desc_set_and_binding_parser;
    r = desc_set_and_binding_parser.Parse(buffer_info.buffer_name);
    if (!r.IsSuccess())
      break;

    const auto* buffer = pipeline->GetBufferForBinding(
        desc_set_and_binding_parser.GetDescriptorSet(),
        desc_set_and_binding_parser.GetBinding());
    if (!buffer)
      break;

    const uint8_t* ptr = buffer->ValuePtr()->data();
    auto& values = buffer_info.values;
    for (size_t i = 0; i < buffer->GetSizeInBytes(); ++i) {
      values.emplace_back();
      values.back().SetIntValue(*ptr);
      ++ptr;
    }
  }

  if (!executor_result.IsSuccess())
    return executor_result;
  if (!r.IsSuccess())
    return r;

  return {};
}

}  // namespace amber
