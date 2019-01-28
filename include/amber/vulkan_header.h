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

#ifndef AMBER_VULKAN_HEADER_H_
#define AMBER_VULKAN_HEADER_H_

#if AMBER_CTS_VULKAN_HEADER

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#include "vkDefs.h"
#pragma clang diagnostic pop

#else  // AMBER_CTS_VULKAN_HEADER

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"
// Note, VK_NO_PROTOTYPES is used by vulkan/vulkan.h to disable defining
// all of the vk* methods.
#define VK_NO_PROTOTYPES
#include "vulkan/vulkan.h"
#undef VK_NO_PROTOTYPES
#pragma clang diagnostic pop

#endif  // AMBER_CTS_VULKAN_HEADER

#endif  // AMBER_VULKAN_HEADER_H_
