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

#ifndef SRC_PLATFORM_H_
#define SRC_PLATFORM_H_

namespace amber {

#if defined(_WIN32) || defined(_WIN64)
#define AMBER_PLATFORM_WINDOWS 1
#elif defined(__linux__)
#define AMBER_PLATFORM_LINUX 1
#define AMBER_PLATFORM_POSIX 1
#elif defined(__APPLE__)
#define AMBER_PLATFORM_APPLE 1
#define AMBER_PLATFORM_POSIX 1
#else
#error "Unknown platform."
#endif

#if !defined(AMBER_PLATFORM_WINDOWS)
#define AMBER_PLATFORM_WINDOWS 0
#endif

#if !defined(AMBER_PLATFORM_LINUX)
#define AMBER_PLATFORM_LINUX 0
#endif

#if !defined(AMBER_PLATFORM_APPLE)
#define AMBER_PLATFORM_APPLE 0
#endif

#if !defined(AMBER_PLATFORM_POSIX)
#define AMBER_PLATFORM_POSIX 0
#endif

}  // namespace amber

#endif  // SRC_PLATFORM_H_
