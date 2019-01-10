// Copyright 2019 The Amber Authors.
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

#include "amber_script.h"

#include "src/make_unique.h"

namespace amber {
namespace android {
namespace {

const char kAmberDir[] = "amber/";
const char kAmberScriptExtension[] = ".amber";
const char kShaderNameSignature[] = ".vk_shader_";
const char kShaderExtension[] = ".spv";

bool IsEndedWith(const std::string& path, const std::string& end) {
  const size_t path_size = path.size();
  const size_t end_size = end.size();
  if (path_size < end_size)
    return false;

  return path.compare(path_size - end_size, end_size, end) == 0;
}

bool IsStartedWith(const std::string& path, const std::string& start) {
  const size_t path_size = path.size();
  const size_t start_size = start.size();
  if (path_size < start_size)
    return false;

  return path.compare(0, start_size, start) == 0;
}

std::string GetShaderID(const std::string& shader_name) {
  size_t spv_extension_pos = shader_name.find_last_of('.');
  if (spv_extension_pos == std::string::npos)
    return std::string();

  size_t shader_id_pos =
      shader_name.find_last_of('.', spv_extension_pos - 1UL) + 1UL;
  if (shader_id_pos == std::string::npos)
    return std::string();

  if (shader_id_pos >= spv_extension_pos || shader_name.size() <= shader_id_pos)
    return std::string();

  return shader_name.substr(shader_id_pos, spv_extension_pos - shader_id_pos);
}

}  // namespace

AmberScriptLoader::AmberScriptLoader(android_app* app) : app_context_(app) {}

AmberScriptLoader::~AmberScriptLoader() = default;

Result AmberScriptLoader::LoadAllScriptsFromAsset() {
  auto shader_names = FindAllScriptsAndReturnShaderNames();
  if (script_info_.empty())
    return Result("No Amber script found");

  for (auto& info : script_info_) {
    info.script_content = ReadScript(info.asset_name);
    if (info.script_content.empty())
      return Result(info.asset_name + ":\n\tEmpty Amber script");
  }

  for (auto& info : script_info_) {
    for (const auto& shader : shader_names) {
      if (!IsStartedWith(shader, info.asset_name + kShaderNameSignature))
        continue;

      auto shader_content = ReadSpvShader(shader);
      if (shader_content.empty())
        return Result(shader + ":\n\tEmpty shader");

      auto id = GetShaderID(shader);
      if (id.empty())
        return Result(shader + ":\n\tFail to get shader ID");

      info.shader_map[id] = shader_content;
    }
  }

  return {};
}

std::vector<std::string>
AmberScriptLoader::FindAllScriptsAndReturnShaderNames() {
  std::vector<std::string> shaders;

  AAssetDir* asset =
      AAssetManager_openDir(app_context_->activity->assetManager, kAmberDir);
  for (const char* file_name = AAssetDir_getNextFileName(asset); file_name;
       file_name = AAssetDir_getNextFileName(asset)) {
    std::string file_name_in_string(file_name);
    if (IsEndedWith(file_name_in_string, kAmberScriptExtension)) {
      script_info_.emplace_back();
      script_info_.back().asset_name = file_name_in_string;
    }

    if (IsEndedWith(file_name_in_string, kShaderExtension))
      shaders.push_back(file_name_in_string);
  }
  AAssetDir_close(asset);

  return shaders;
}

std::vector<uint8_t> AmberScriptLoader::ReadContent(
    const std::string& asset_name) {
  auto asset_path = kAmberDir + asset_name;
  AAsset* asset = AAssetManager_open(app_context_->activity->assetManager,
                                     asset_path.c_str(), AASSET_MODE_BUFFER);
  if (!asset)
    return std::vector<uint8_t>();

  size_t size_in_bytes = AAsset_getLength(asset);

  // Allocate a memory chunk whose size in bytes is |size_in_bytes|.
  std::vector<uint8_t> content(size_in_bytes);

  AAsset_read(asset, content.data(), size_in_bytes);
  AAsset_close(asset);

  return content;
}

std::string AmberScriptLoader::ReadScript(const std::string& script_name) {
  auto content = ReadContent(script_name);
  return std::string(reinterpret_cast<char*>(content.data()));
}

std::vector<uint32_t> AmberScriptLoader::ReadSpvShader(
    const std::string& shader_name) {
  auto content = ReadContent(shader_name);
  if (content.size() % sizeof(uint32_t) != 0)
    return std::vector<uint32_t>();

  return std::vector<uint32_t>(
      reinterpret_cast<uint32_t*>(content.data()),
      reinterpret_cast<uint32_t*>(content.data() + content.size()));
}

}  // namespace android
}  // namespace amber
