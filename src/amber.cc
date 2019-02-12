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
#include "src/engine.h"
#include "src/executor.h"
#include "src/make_unique.h"
#include "src/parser.h"
#include "src/vkscript/parser.h"

namespace amber {
namespace {

// Parse |buffer_id| and return desciptor set and binding using |desc_set|
// and |binding|. |buffer_id| must be non-empty string. It must be a single
// non-negative integer or two those integers separated by ':'. For example,
// ":0", "1", and "2:3" are valid strings for |buffer_id|, but "", "-4",
// ":-5", ":", "a", and "b:c" are invalid strings for |buffer_id|.
amber::Result ParseDescriptorSetAndBinding(const std::string& buffer_id,
                                           uint32_t* desc_set,
                                           uint32_t* binding) {
  if (buffer_id.empty())
    return Result("Given buffer to dump has an empty name.");

  size_t pos = 0;
  while (pos < buffer_id.size() && std::isdigit(buffer_id[pos]))
    ++pos;

  // |buffer_id| is a single integer. It must be non-negative.
  if (pos >= buffer_id.size()) {
    *desc_set = 0;
    int32_t binding_before_validation =
        static_cast<int32_t>(std::strtoull(buffer_id.c_str(), nullptr, 10));
    if (binding_before_validation < 0) {
      return Result("binding for buffer must be positive, got: " +
                    std::to_string(binding_before_validation));
    }
    *binding = binding_before_validation;
    return {};
  }

  // |buffer_id| contains a character that is not ':'.
  if (buffer_id[pos] != ':')
    return Result("Invalid name of buffer '" + buffer_id + "' to dump.");

  // |buffer_id| is ":".
  if (pos + 1 >= buffer_id.size())
    return Result("Invalid name of buffer '" + buffer_id + "' to dump.");

  // |buffer_id| has two integers. They must be non-negative.
  int32_t number_before_validation = static_cast<int32_t>(
      std::strtoull(buffer_id.substr(0, pos).c_str(), nullptr, 10));
  if (number_before_validation < 0) {
    return Result("binding for buffer must be positive, got: " +
                  std::to_string(number_before_validation));
  }
  *desc_set = static_cast<uint32_t>(number_before_validation);

  number_before_validation = static_cast<int32_t>(
      std::strtoull(buffer_id.substr(pos + 1).c_str(), nullptr, 10));
  if (number_before_validation < 0) {
    return Result("binding for buffer must be positive, got: " +
                  std::to_string(number_before_validation));
  }
  *binding = static_cast<uint32_t>(number_before_validation);
  return {};
}

}  // namespace

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

      continue;
    }

    uint32_t desc_set = 0;
    uint32_t binding = 0;
    r = ParseDescriptorSetAndBinding(buffer_info.buffer_name, &desc_set,
                                     &binding);
    if (!r.IsSuccess()) {
      engine->Shutdown();
      return r;
    }

    ResourceInfo info = ResourceInfo();
    r = engine->GetDescriptorInfo(desc_set, binding, &info);
    if (!r.IsSuccess()) {
      engine->Shutdown();
      return r;
    }

    const uint8_t* ptr = static_cast<const uint8_t*>(info.cpu_memory);
    auto& values = buffer_info.values;
    for (size_t i = 0; i < info.size_in_bytes; ++i) {
      values.emplace_back();
      values.back().SetIntValue(*ptr);
      ++ptr;
    }
  }

  return engine->Shutdown();
}

}  // namespace amber
