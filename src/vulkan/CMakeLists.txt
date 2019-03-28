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
    buffer_descriptor.cc
    command_buffer.cc
    command_pool.cc
    compute_pipeline.cc
    device.cc
    engine_vulkan.cc
    frame_buffer.cc
    graphics_pipeline.cc
    index_buffer.cc
    pipeline.cc
    push_constant.cc
    resource.cc
    transfer_buffer.cc
    transfer_image.cc
    vertex_buffer.cc
    ${CMAKE_BINARY_DIR}/src/vk-wrappers.inc.fake
)

add_library(libamberenginevulkan ${VULKAN_ENGINE_SOURCES})
amber_default_compile_options(libamberenginevulkan)
target_include_directories(libamberenginevulkan PRIVATE "${CMAKE_BINARY_DIR}")
set_target_properties(libamberenginevulkan PROPERTIES
    OUTPUT_NAME "amberenginevulkan"
)
if (NOT ANDROID)
  target_link_libraries(libamberenginevulkan)
endif()

if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
  # vulkan/vulkan.h defines VK_NULL_HANDLE as 0u and that also serves as a null pointer.
  # Disable Clang's warning that will alwaays fire on that.  This is required to build
  # with XCode 10.
  target_compile_options(libamberenginevulkan PRIVATE -Wno-zero-as-null-pointer-constant)
endif()

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/src/vk-wrappers.inc.fake
    COMMAND
      ${PYTHON_EXECUTABLE}
        ${PROJECT_SOURCE_DIR}/tools/update_vk_wrappers.py
        ${CMAKE_BINARY_DIR}
        ${PROJECT_SOURCE_DIR}
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    COMMENT "Update vk-wrapper files in the build directory"
)
