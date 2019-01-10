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

#include "samples/log.h"

#include <string>

#ifdef __ANDROID__
#include <android/log.h>
#else  // __ANDROID__
#include <iostream>
#endif  // __ANDROID__

namespace sample {
namespace {

#ifdef __ANDROID__
const char* kTAG = "Amber";
#endif  // __ANDROID__

}  // namespace

void LogError(const std::string& msg) {
#ifdef __ANDROID__
  ((void)__android_log_print(ANDROID_LOG_ERROR, kTAG, "%s", msg.c_str()));
#else   // __ANDROID__
  std::cerr << msg << std::endl << std::flush;
#endif  // __ANDROID__
}

}  // namespace sample
