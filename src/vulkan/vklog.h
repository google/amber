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

#ifndef SRC_VULKAN_VKLOG_H_
#define SRC_VULKAN_VKLOG_H_

#ifdef AMBER_LOG_VULKAN

#define VKLOG(expr) (amber::vulkan::vklog(__FILE__, __LINE__, #expr), (expr))

namespace amber {
namespace vulkan {

void vklog(const char* filepath, int line, const char* expr);

}  // namespace vulkan
}  // namespace amber

#else  // AMBER_LOG_VULKAN

#define VKLOG(expr) (expr)

#endif  // AMBER_LOG_VULKAN

#endif  // SRC_VULKAN_VKLOG_H_
