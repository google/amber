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

#ifndef SRC_VULKAN_LOG_H_
#define SRC_VULKAN_LOG_H_

// LOGE is used for Vulkan validation layer.

#ifdef __ANDROID__

#include <android/log.h>

static const char* kTAG = "Amber";
#define LOGE(...) \
  ((void)__android_log_print(ANDROID_LOG_ERROR, kTAG, __VA_ARGS__))

#else  // __ANDROID__

#include <cstdio>

#define LOGE(...)                         \
  do {                                    \
    ((void)fprintf(stderr, __VA_ARGS__)); \
    ((void)fprintf(stderr, "\n"));        \
    ((void)fflush(stderr));               \
  } while (0)

#endif  // __ANDROID__

#endif  // SRC_VULKAN_LOG_H_
