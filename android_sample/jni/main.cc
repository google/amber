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

#include <android/log.h>
#include <android_native_app_glue.h>

#include "amber/amber.h"
#include "amber/recipe.h"
#include "amber/result.h"
#include "amber_script.h"

namespace {

// TODO(jaebaek): Change this as a method rather than macro.
// Android log function wrappers
const char* kTAG = "Amber";
#define LOGE(...) \
  ((void)__android_log_print(ANDROID_LOG_ERROR, kTAG, __VA_ARGS__))

void amber_sample_main(android_app* app) {
  amber::android::AmberScriptLoader loader(app);

  amber::Result r = loader.LoadAllScriptsFromAsset();
  if (!r.IsSuccess()) {
    LOGE("%s", r.Error().c_str());
    return;
  }

  const auto& script_info = loader.GetScripts();

  std::vector<std::string> failures;
  for (const auto& info : script_info) {
    LOGE("\ncase %s: run...", info.asset_name.c_str());

    amber::Amber am;
    amber::Recipe recipe;
    amber::Result r = am.Parse(info.script_content, &recipe);
    if (!r.IsSuccess()) {
      LOGE("\ncase %s: fail\n\t%s", info.asset_name.c_str(), r.Error().c_str());
      failures.push_back(info.asset_name);
      continue;
    }

    amber::Options amber_options;
    r = am.ExecuteWithShaderData(&recipe, amber_options, info.shader_map);
    if (!r.IsSuccess()) {
      LOGE("\ncase %s: fail\n\t%s", info.asset_name.c_str(), r.Error().c_str());
      failures.push_back(info.asset_name);
      continue;
    }

    LOGE("\ncase %s: pass", info.asset_name.c_str());
  }

  if (!failures.empty()) {
    LOGE("\nSummary of Failures:");
    for (const auto& failure : failures)
      LOGE("%s", failure.c_str());
  }
  LOGE("\nsummary: %u pass, %u fail",
       static_cast<uint32_t>(script_info.size() - failures.size()),
       static_cast<uint32_t>(failures.size()));
}

// Process the next main command.
void handle_cmd(android_app* app, int32_t cmd) {
  switch (cmd) {
    case APP_CMD_INIT_WINDOW:
      amber_sample_main(app);
      break;
    case APP_CMD_TERM_WINDOW:
      break;
    default:
      break;
  }
}

}  // namespace

void android_main(struct android_app* app) {
  // Set the callback to process system events
  app->onAppCmd = handle_cmd;

  // Used to poll the events in the main loop
  int events;
  android_poll_source* source;

  // Main loop
  while (app->destroyRequested == 0) {
    if (ALooper_pollAll(1, nullptr, &events, (void**)&source) >= 0) {
      if (source != NULL)
        source->process(app, source);
    }
  }
}
