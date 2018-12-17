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

#include "amber_script.h"

#include "src/make_unique.h"

namespace amber {
namespace android {
namespace {

const std::string kAmberDir("amber/");
const std::string kAmberScriptExtension(".amber");
const std::string kShaderNameSignature(".vk_shader_");
const std::string kShaderExtension(".spv");

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
  size_t shader_id_pos =
      shader_name.find_last_of('.', spv_extension_pos - 1UL) + 1UL;
  return shader_name.substr(shader_id_pos, spv_extension_pos - shader_id_pos);
}

}  // namespace

AmberScriptLoader::AmberScriptLoader(android_app* app) : app_context_(app) {}

AmberScriptLoader::~AmberScriptLoader() = default;

Result AmberScriptLoader::LoadAllScriptsFromAsset() {
  FindAllScripts();
  if (script_info_.empty())
    return Result("No Amber script found");

  for (auto& info : script_info_) {
    info.script_content = ReadAssetContent(info.asset_name)->ToString();
    if (info.script_content.empty())
      return Result(info.asset_name + ":\n\tEmpty Amber script");
  }

  for (auto& info : script_info_) {
    auto shader_names = GetShaderNamesForAmberScript(info.asset_name);
    for (const auto& shader : shader_names) {
      auto shader_content = ReadAssetContent(shader)->ToVectorUint32();
      if (shader_content.empty())
        return Result(shader + ":\n\tEmpty shader");

      info.shader_map[GetShaderID(shader)] = shader_content;
    }
  }

  return {};
}

void AmberScriptLoader::FindAllScripts() {
  AAssetDir* asset = AAssetManager_openDir(app_context_->activity->assetManager,
                                           kAmberDir.c_str());
  for (const char* file_name = AAssetDir_getNextFileName(asset); file_name;
       file_name = AAssetDir_getNextFileName(asset)) {
    std::string file_name_in_string(file_name);
    if (IsEndedWith(file_name_in_string, kAmberScriptExtension)) {
      script_info_.emplace_back();
      script_info_.back().asset_name = file_name_in_string;
    }
  }
  AAssetDir_close(asset);
}

std::vector<std::string> AmberScriptLoader::GetShaderNamesForAmberScript(
    const std::string& script_name) {
  std::vector<std::string> shaders;

  AAssetDir* asset = AAssetManager_openDir(app_context_->activity->assetManager,
                                           kAmberDir.c_str());
  for (const char* file_name = AAssetDir_getNextFileName(asset); file_name;
       file_name = AAssetDir_getNextFileName(asset)) {
    std::string file_name_in_string(file_name);
    if (IsStartedWith(file_name_in_string,
                      script_name + kShaderNameSignature) &&
        IsEndedWith(file_name_in_string, kShaderExtension)) {
      shaders.push_back(file_name_in_string);
    }
  }
  AAssetDir_close(asset);

  return shaders;
}

std::unique_ptr<AmberScriptLoader::AssetContent>
AmberScriptLoader::ReadAssetContent(const std::string& asset_name,
                                    bool is_shader) {
  auto asset_path = kAmberDir + asset_name;
  AAsset* asset = AAssetManager_open(app_context_->activity->assetManager,
                                     asset_path.c_str(), AASSET_MODE_BUFFER);
  if (!asset)
    return amber::MakeUnique<AssetContent>(0);

  size_t size_in_bytes = AAsset_getLength(asset);
  if (is_shader) {
    if (size_in_bytes % sizeof(uint32_t) != 0UL) {
      size_in_bytes =
          (size_in_bytes + 3UL) / sizeof(uint32_t) * sizeof(uint32_t);
    }
  }

  auto asset_content = amber::MakeUnique<AssetContent>(size_in_bytes);
  AAsset_read(asset, asset_content->content, asset_content->size_in_bytes);
  AAsset_close(asset);

  return asset_content;
}

std::vector<uint32_t> AmberScriptLoader::AssetContent::ToVectorUint32() {
  assert(size_in_bytes % sizeof(uint32_t) == 0UL);
  const uint32_t* ptr = reinterpret_cast<const uint32_t*>(content);
  return std::vector<uint32_t>(ptr, ptr + size_in_bytes / sizeof(uint32_t));
}

}  // namespace android
}  // namespace amber
