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
#include <exception>

#include "amber/amber.h"
#include "amber/recipe.h"
#include "amber/result.h"
#include "amber_script.h"

// Android log function wrappers
static const char* kTAG = "Amber";
#define LOGE(...) \
  ((void)__android_log_print(ANDROID_LOG_ERROR, kTAG, __VA_ARGS__))

static void amber_main(android_app* app) {
  amber::android::AmberScriptLoader loader(app);

  amber::Result r = loader.LoadAllScriptsFromAsset();
  if (!r.IsSuccess()) {
    LOGE("%s", r.Error().c_str());
    return;
  }

  const auto& script_info = loader.GetScripts();

  uint32_t n_passes = 0;
  uint32_t n_failures = 0;
  for (const auto& info : script_info) {
    amber::Amber am;
    amber::Recipe recipe;
    amber::Result r = am.Parse(info.script_content, &recipe);
    if (!r.IsSuccess()) {
      LOGE("case %s: fail\n\t%s", info.asset_name.c_str(), r.Error().c_str());
      ++n_failures;
      continue;
    }

    amber::Options amber_options;
    try {
      r = am.ExecuteWithShaderData(&recipe, amber_options, info.shader_map);
    } catch (const std::exception& e) {
      LOGE("case %s: exception\n\t", e.what());
      ++n_failures;
      continue;
    }

    if (!r.IsSuccess()) {
      LOGE("case %s: fail\n\t%s", info.asset_name.c_str(), r.Error().c_str());
      ++n_failures;
      continue;
    }

    LOGE("case %s: pass", info.asset_name.c_str());
    ++n_passes;
  }

  LOGE("summary: %u pass, %u fail", n_passes, n_failures);
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
