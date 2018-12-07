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

#include <android/log.h>
#include <android_native_app_glue.h>
#include <cstring>
#include <string>
#include <vector>

#include "amber/amber.h"
#include "amber/recipe.h"

// Android log function wrappers
static const char* kTAG = "Amber";
#define LOGI(...) \
  ((void)__android_log_print(ANDROID_LOG_INFO, kTAG, __VA_ARGS__))
#define LOGW(...) \
  ((void)__android_log_print(ANDROID_LOG_WARN, kTAG, __VA_ARGS__))
#define LOGE(...) \
  ((void)__android_log_print(ANDROID_LOG_ERROR, kTAG, __VA_ARGS__))

static const char* const kAmberDir = "amber/";
static const char* const kAmberScriptExtension = ".amber";

static std::string GetAmberScriptPathFromAsset(android_app* app) {
  AAssetDir* asset =
      AAssetManager_openDir(app->activity->assetManager, kAmberDir);
  const char* amber_script = nullptr;
  for (const char* file_name = AAssetDir_getNextFileName(asset); file_name;
       file_name = AAssetDir_getNextFileName(asset)) {
    if (!strcmp(&file_name[strlen(file_name) - strlen(kAmberScriptExtension)],
                kAmberScriptExtension)) {
      amber_script = file_name;
      break;
    }
  }
  AAssetDir_close(asset);

  if (!amber_script)
    return std::string();

  return std::string(kAmberDir) + std::string(amber_script);
}

static std::string ReadFile(android_app* app, const std::string& file_path) {
  std::string content;
  AAsset* file = AAssetManager_open(app->activity->assetManager,
                                    file_path.c_str(), AASSET_MODE_BUFFER);
  if (!file)
    return std::string();

  content.resize(AAsset_getLength(file));
  void* ptr = static_cast<void*>(&content.front());
  AAsset_read(file, ptr, content.size());
  AAsset_close(file);

  return content;
}

static void amber_main(android_app* app) {
  auto amber_script_name = GetAmberScriptPathFromAsset(app);
  if (amber_script_name.empty()) {
    LOGE("%s:\n\tNo Amber script found", amber_script_name.c_str());
    return;
  }

  auto data = ReadFile(app, amber_script_name);
  if (data.empty()) {
    LOGE("%s:\n\tEmpty Amber script", amber_script_name.c_str());
    return;
  }

  amber::Amber am;
  amber::Recipe recipe;
  amber::Result result = am.Parse(data, &recipe);
  if (!result.IsSuccess()) {
    LOGE("%s:\n\t%s", amber_script_name.c_str(), result.Error().c_str());
    return;
  }

  amber::Options amber_options;
  result = am.Execute(&recipe, amber_options);
  if (!result.IsSuccess()) {
    LOGE("%s:\n\t%s", amber_script_name.c_str(), result.Error().c_str());
    return;
  }
}

// Process the next main command.
static void handle_cmd(android_app* app, int32_t cmd) {
  switch (cmd) {
    case APP_CMD_INIT_WINDOW:
      amber_main(app);
      break;
    case APP_CMD_TERM_WINDOW:
      break;
    default:
      break;
  }
}

void android_main(struct android_app* app) {
  // Set the callback to process system events
  app->onAppCmd = handle_cmd;

  // Used to poll the events in the main loop
  int events;
  android_poll_source* source;

  // Main loop
  do {
    if (ALooper_pollAll(1, nullptr, &events, (void**)&source) >= 0)
      if (source != NULL)
        source->process(app, source);
  } while (app->destroyRequested == 0);
}
