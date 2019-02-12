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

include_directories("${PROJECT_SOURCE_DIR}/include")

set(AMBER_SOURCES
    amber.cc
    config_helper.cc
    log.cc
    ppm.cc
    ${CMAKE_BINARY_DIR}/src/build-versions.h.fake
)

set(AMBER_EXTRA_LIBS "")

if (${Vulkan_FOUND})
  set(AMBER_SOURCES ${AMBER_SOURCES} config_helper_vulkan.cc)
  list(APPEND AMBER_EXTRA_LIBS ${VULKAN_LIB})
endif()

if (${Dawn_FOUND})
  set(AMBER_SOURCES ${AMBER_SOURCES} config_helper_dawn.cc)
  list(APPEND AMBER_EXTRA_LIBS Dawn::dawn_native Dawn::dawn)
  if (APPLE)
    add_definitions(-DAMBER_DAWN_METAL=1)
    set(AMBER_SOURCES ${AMBER_SOURCES} dawn_device_metal.mm)
    find_library(METAL_LIB Metal)
    list(APPEND AMBER_EXTRA_LIBS ${METAL_LIB})
  else()
    add_definitions(-DAMBER_DAWN_METAL=0)
  endif()
endif()

add_executable(amber ${AMBER_SOURCES})
target_include_directories(amber PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/..")
set_target_properties(amber PROPERTIES OUTPUT_NAME "amber")
target_link_libraries(amber libamber ${AMBER_EXTRA_LIBS})

if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
  # vulkan/vulkan.h defines VK_NULL_HANDLE as 0u and that also serves as a null pointer.
  # Disable Clang's warning that will alwaays fire on that.  This is required to build
  # with XCode 10.
  target_compile_options(amber PRIVATE -Wno-zero-as-null-pointer-constant)
endif()

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/src/build-versions.h.fake
    COMMAND
      ${PYTHON_EXE}
        ${CMAKE_CURRENT_SOURCE_DIR}/../tools/update_build_version.py
        ${CMAKE_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../third_party
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/.."
    COMMENT "Update build-versions.h in the build directory"
)
