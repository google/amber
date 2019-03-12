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

#include "samples/timestamp.h"

#include <cassert>

#if defined(_WIN32) || defined(_WIN64)
#define SAMPLE_PLATFORM_WINDOWS 1
#define SAMPLE_PLATFORM_POSIX 0
#elif defined(__linux__) || defined(__APPLE__)
#define SAMPLE_PLATFORM_POSIX 1
#define SAMPLE_PLATFORM_WINDOWS 0
#endif

#if SAMPLE_PLATFORM_WINDOWS
#include <windows.h>
#elif SAMPLE_PLATFORM_POSIX
#include <time.h>
#else
#error "Unknown platform"
#endif

namespace timestamp {

uint64_t SampleGetTimestampNs() {
  uint64_t timestamp = 0;

#if SAMPLE_PLATFORM_WINDOWS

  LARGE_INTEGER tick_per_seconds;
  if (!QueryPerformanceFrequency(&tick_per_seconds)) {
    return 0;
  }
  LARGE_INTEGER ticks;
  if (!QueryPerformanceCounter(&ticks)) {
    return 0;
  }
  double tick_duration_ns = static_cast<double>(1.0e9) /
                            static_cast<double>(tick_per_seconds.QuadPart);
  timestamp = uint64_t(static_cast<double>(ticks.QuadPart) * tick_duration_ns);

#elif SAMPLE_PLATFORM_POSIX

  struct timespec time;
  if (clock_gettime(CLOCK_MONOTONIC, &time)) {
    return 0;
  }
  timestamp = static_cast<uint64_t>((time.tv_sec * 1000000000) + time.tv_nsec);

#else
#error "Implement timestamp::SampleGetTimestampNs"
#endif

  return timestamp;
}

}  // namespace timestamp
