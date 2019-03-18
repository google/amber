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

cmake_minimum_required(VERSION 2.8)
if (POLICY CMP0048)
  cmake_policy(SET CMP0048 NEW)
endif()
if (POLICY CMP0054)
  # Avoid dereferencing variables or interpret keywords that have been
  # quoted or bracketed.
  # https://cmake.org/cmake/help/v3.1/policy/CMP0054.html
  cmake_policy(SET CMP0054 NEW)
endif()

project(amber)
enable_testing()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_STANDARD 11)

include(CheckIncludeFile)
include(GNUInstallDirs)

option(AMBER_SKIP_TESTS
  "Skip building tests along with the library" ${AMBER_SKIP_TESTS})
option(AMBER_SKIP_SPIRV_TOOLS
  "Skip building spirv-tools into the library" ${AMBER_SKIP_SPIRV_TOOLS})
option(AMBER_SKIP_SHADERC
  "Skip building Shaderc into the library" ${AMBER_SKIP_SHADERC})
option(AMBER_SKIP_SAMPLES
  "Skip building sample application" ${AMBER_SKIP_SAMPLES})
option(AMBER_USE_LOCAL_VULKAN "Build with vulkan in third_party" OFF)

if (${AMBER_SKIP_SPIRV_TOOLS})
  set(AMBER_ENABLE_SPIRV_TOOLS FALSE)
  set(AMBER_ENABLE_SHADERC FALSE)
else()
  set(AMBER_ENABLE_SPIRV_TOOLS TRUE)

  if (${AMBER_SKIP_SHADERC})
    set(AMBER_ENABLE_SHADERC FALSE)
  else()
    set(AMBER_ENABLE_SHADERC TRUE)
  endif()
endif()
if (${AMBER_SKIP_TESTS})
  set(AMBER_ENABLE_TESTS FALSE)
else()
  set(AMBER_ENABLE_TESTS TRUE)
endif()
if (${AMBER_SKIP_SAMPLES})
  set(AMBER_ENABLE_SAMPLES FALSE)
else()
  set(AMBER_ENABLE_SAMPLES TRUE)
endif()

if (${AMBER_USE_LOCAL_VULKAN})
  message(STATUS "Using python3")
  # vulkan-loaders requires python 3
  find_package(PythonInterp 3 REQUIRED)
endif()

message(STATUS "Amber enable SPIRV-Tools: ${AMBER_ENABLE_SPIRV_TOOLS}")
message(STATUS "Amber enable Shaderc: ${AMBER_ENABLE_SHADERC}")
message(STATUS "Amber enable tests: ${AMBER_ENABLE_TESTS}")
message(STATUS "Amber enable samples: ${AMBER_ENABLE_SAMPLES}")

include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${PROJECT_SOURCE_DIR}")

if (${AMBER_ENABLE_SPIRV_TOOLS})
  include_directories("${PROJECT_SOURCE_DIR}/third_party/spirv-tools/include")
endif()

if (NOT ANDROID)
  include(src/dawn/find_dawn.cmake)
endif()

include(src/vulkan/find_vulkan.cmake)

add_definitions(-DAMBER_CTS_VULKAN_HEADER=$<BOOL:${VULKAN_CTS_HEADER}>)
add_definitions(-DAMBER_ENGINE_VULKAN=$<BOOL:${Vulkan_FOUND}>)
add_definitions(-DAMBER_ENGINE_DAWN=$<BOOL:${Dawn_FOUND}>)
add_definitions(-DAMBER_ENABLE_SPIRV_TOOLS=$<BOOL:${AMBER_ENABLE_SPIRV_TOOLS}>)
add_definitions(-DAMBER_ENABLE_SHADERC=$<BOOL:${AMBER_ENABLE_SHADERC}>)

if ("${CMAKE_BUILD_TYPE}" STREQUAL "")
  message(STATUS "No build type selected, default to Debug")
  set(CMAKE_BUILD_TYPE "Debug")
endif()

if(("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU") OR
    (("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang") AND
     (NOT CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC")))
  set(COMPILER_IS_LIKE_GNU TRUE)
endif()

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(CUSTOM_CXX_FLAGS
    ${CUSTOM_CXX_FLAGS}
    -std=c++11
    -fvisibility=hidden
    -Wall
    -Werror
    -Weverything
    -Wextra
    -Wno-c++98-compat
    -Wno-c++98-compat-pedantic
    -Wno-format-pedantic
    -Wno-padded
    -Wno-switch-enum
    -Wno-unknown-pragmas
    -Wno-unknown-warning-option)

elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(CUSTOM_CXX_FLAGS
    ${CUSTOM_CXX_FLAGS}
    -std=c++11
    -fvisibility=hidden
    -Wall
    -Werror
    -Wextra
    -Wno-unknown-pragmas
    -Wpedantic
    -pedantic-errors)
elseif(MSVC)
  # We don't want to have to copy the C Runtime DLL everywhere the executable
  # goes.  So compile code to assume the CRT is statically linked, i.e. use
  # /MT* options.  For debug builds use /MTd, and for release builds use /MT.
  if(CMAKE_BUILD_TYPE MATCHES "Debug")
    set(CUSTOM_CXX_FLAGS ${CUSTOM_CXX_FLAGS} /MTd)
    message(STATUS "Setting /MTd")
  elseif(CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo")
    set(CUSTOM_CXX_FLAGS ${CUSTOM_CXX_FLAGS} /MT)
    message(STATUS "Setting /MT")
  else()
    set(CUSTOM_CXX_FLAGS ${CUSTOM_CXX_FLAGS} /MT)
    message(STATUS "Setting /MT")
  endif()

  set(CUSTOM_CXX_FLAGS
    ${CUSTOM_CXX_FLAGS}
    /bigobj
    /EHsc
    /W3
    /WX
    /wd4068
    /wd4514
    /wd4571
    /wd4625
    /wd4626
    /wd4710
    /wd4774
    /wd4820
    /wd5026
    /wd5027
  )
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

if (${AMBER_ENABLE_SAMPLES} AND NOT ANDROID)
  add_subdirectory(samples)
endif()
