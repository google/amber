# Copyright 2018 The Amber Authors.
# Copyright (C) 2024 Advanced Micro Devices, Inc. All rights reserved.
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
    blas.cc
    buffer_descriptor.cc
    buffer_backed_descriptor.cc
    command_buffer.cc
    command_pool.cc
    compute_pipeline.cc
    device.cc
    descriptor.cc
    engine_vulkan.cc
    frame_buffer.cc
    graphics_pipeline.cc
    image_descriptor.cc
    index_buffer.cc
    pipeline.cc
    push_constant.cc
    raytracing_pipeline.cc
    resource.cc
    sampler.cc
    sampler_descriptor.cc
    sbt.cc
    tlas.cc
    tlas_descriptor.cc
    transfer_buffer.cc
    transfer_image.cc
    vertex_buffer.cc
    ${CMAKE_BINARY_DIR}/src/vk-wrappers.inc.fake
)

add_library(libamberenginevulkan ${VULKAN_ENGINE_SOURCES})
amber_default_compile_options(libamberenginevulkan)
target_include_directories(libamberenginevulkan PRIVATE "${CMAKE_BINARY_DIR}")

# Add the Vulkan include directory to the list of include paths.
target_include_directories(libamberenginevulkan PUBLIC "${VulkanHeaders_INCLUDE_DIR}")

# When building with dEQP Vulkan CTS the inl files needs to be included and a dependency
# must be added to the target `deqp-vk-inl` that generates the inl files.
if (${VULKAN_CTS_HEADER} AND DEFINED AMBER_CTS_INL_DIR)
    target_include_directories(libamberenginevulkan PRIVATE "${AMBER_CTS_INL_DIR}")
    add_dependencies(libamberenginevulkan deqp-vk-inl)
endif()

set_target_properties(libamberenginevulkan PROPERTIES
    OUTPUT_NAME "amberenginevulkan"
)
if (NOT ANDROID)
  target_link_libraries(libamberenginevulkan)
endif()

if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
  # vulkan/vulkan.h defines VK_NULL_HANDLE as 0u and that also serves as a null pointer.
  # Disable Clang's warning that will always fire on that. This is required to build
  # with XCode 10.
  target_compile_options(libamberenginevulkan PRIVATE -Wno-zero-as-null-pointer-constant)
endif()

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/src/vk-wrappers.inc.fake
    COMMAND
      ${Python3_EXECUTABLE}
        ${PROJECT_SOURCE_DIR}/tools/update_vk_wrappers.py
        ${CMAKE_BINARY_DIR}
        ${PROJECT_SOURCE_DIR}
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    COMMENT "Update vk-wrapper files in the build directory"
)
