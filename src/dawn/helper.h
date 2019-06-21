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

#ifndef SRC_HELPER_H_
#define SRC_HELPER_H_

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

// Creates a bind group.
// Helpers to make creating bind groups look nicer:
//
//   utils::MakeBindGroup(device, layout, {
//       {0, mySampler},
//       {1, myBuffer, offset, size},
//       {3, myTexture}
//   });

// Structure with one constructor per-type of bindings, so that the
// initializer_list accepts bindings with the right type and no extra
// information.
struct BindingInitializationHelper {
  BindingInitializationHelper(uint32_t binding,
                              const ::dawn::Buffer& buffer,
                              uint64_t offset,
                              uint64_t size)
      : binding(binding), buffer(buffer), offset(offset), size(size) {}

  ::dawn::BindGroupBinding GetAsBinding() const {
    ::dawn::BindGroupBinding result;
    result.binding = binding;
    result.sampler = sampler;
    result.textureView = textureView;
    result.buffer = buffer;
    result.offset = offset;
    result.size = size;
    return result;
  }

  uint32_t binding;
  ::dawn::Sampler sampler;
  ::dawn::TextureView textureView;
  ::dawn::Buffer buffer;
  uint64_t offset = 0;
  uint64_t size = 0;
};
