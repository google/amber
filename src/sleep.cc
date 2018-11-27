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

#include "src/sleep.h"

#include "src/platform.h"

#if AMBER_PLATFORM_WINDOWS
#include <windows.h>
#elif AMBER_PLATFORM_POSIX
#include <unistd.h>
#else
#error "Unknown platform"
#endif

namespace amber {

void USleep(uint32_t microseconds) {
#if AMBER_PLATFORM_WINDOWS
  // The Windows call uses milliseconds.
  Sleep(static_cast<DWORD>((microseconds + 999) / 1000));
#elif AMBER_PLATFORM_POSIX
  usleep(microseconds);
#else
#error "Implement amber::USleep"
#endif
}

}  // namespace amber
