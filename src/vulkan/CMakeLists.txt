# Copyright 2018 The Amber Authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set(VULKAN_ENGINE_SOURCES
    bit_copy.cc
    buffer.cc
    buffer_descriptor.cc
    command.cc
    compute_pipeline.cc
    descriptor.cc
    device.cc
    engine_vulkan.cc
    format_data.cc
    frame_buffer.cc
    log.cc
    resource.cc
    image.cc
    pipeline.cc
    graphics_pipeline.cc
    vertex_buffer.cc
)

add_library(libamberenginevulkan ${VULKAN_ENGINE_SOURCES})
amber_default_compile_options(libamberenginevulkan)
set_target_properties(libamberenginevulkan PROPERTIES
    OUTPUT_NAME "amberenginevulkan"
)
target_link_libraries(libamberenginevulkan ${VULKAN_LIB})

if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
  # vulkan/vulkan.h defines VK_NULL_HANDLE as 0u and that also serves as a null pointer.
  # Disable Clang's warning that will alwaays fire on that.  This is required to build
  # with XCode 10.
  target_compile_options(libamberenginevulkan PRIVATE -Wno-zero-as-null-pointer-constant)
endif()
