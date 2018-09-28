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

cmake_minimum_required(VERSION 2.8.12)

project(amber)
enable_testing()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

include(CheckIncludeFile)
include(GNUInstallDirs)

include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/third_party/spirv-tools/include")

include(src/dawn/find_dawn.cmake)
include(src/vulkan/find_vulkan.cmake)

add_definitions(-DAMBER_ENGINE_VULKAN=$<BOOL:${Vulkan_FOUND}>)
add_definitions(-DAMBER_ENGINE_DAWN=$<BOOL:${Dawn_FOUND}>)


if ("${CMAKE_BUILD_TYPE}" STREQUAL "")
  message(STATUS "No build type selected, default to Debug")
  set(CMAKE_BUILD_TYPE "Debug")
endif()

set(CUSTOM_CXX_FLAGS
    -std=c++11
    -Wall
    -Werror
    -Wextra)

if(("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU") OR
    (("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang") AND
     (NOT CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC")))
  set(COMPILER_IS_LIKE_GNU TRUE)
endif()

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(CUSTOM_CXX_FLAGS
      ${CUSTOM_CXX_FLAGS}
      -Weverything
      -Wno-c++98-compat
      -Wno-c++98-compat-pedantic
      -Wno-padded
      -Wno-switch-enum
      -Wno-unknown-pragmas
      -Wno-unknown-warning-option)

elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(CUSTOM_CXX_FLAGS
      ${CUSTOM_CXX_FLAGS}
      -Wno-unknown-pragmas
      -Wpedantic
      -pedantic-errors)
elseif(MSVC)
  set(CUSTOM_CXX_FLAGS
      ${CUSTOM_CXX_FLAGS}
      /WX)
endif()

SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "${CUSTOM_CXX_FLAGS}")
STRING(REGEX REPLACE ";" " " CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

function(amber_default_compile_options TARGET)
  if (${COMPILER_IS_LIKE_GNU})
    target_compile_options(${TARGET} PRIVATE
        -fno-exceptions
        -fno-rtti)
  endif()

  if (MSVC)
    # Specify /EHs for exception handling.
    target_compile_options(${TARGET} PRIVATE /EHs)
  endif()

  # For MinGW cross compile, statically link to the C++ runtime.
  # But it still depends on MSVCRT.dll.
  if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
      set_target_properties(${TARGET} PROPERTIES LINK_FLAGS
          -static
          -static-libgcc
          -static-libstdc++)
    endif()
  endif()
endfunction()

add_subdirectory(third_party)
add_subdirectory(src)
add_subdirectory(samples)
