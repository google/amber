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

cmake_minimum_required(VERSION 3.0)
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
if (${AMBER_USE_CLSPV})
  set(CMAKE_CXX_STANDARD 14)
else()
  set(CMAKE_CXX_STANDARD 11)
endif()

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
option(AMBER_SKIP_LODEPNG
  "Skip building lodepng into the library" ${AMBER_SKIP_LODEPNG})
option(AMBER_USE_DXC "Enable DXC integration" ${AMBER_USE_DXC})
option(AMBER_USE_LOCAL_VULKAN "Build with vulkan in third_party" OFF)
option(AMBER_USE_CLSPV "Build with Clspv support" OFF)
option(AMBER_ENABLE_SWIFTSHADER
  "Build using SwiftShader" ${AMBER_ENABLE_SWIFTSHADER})

if(WIN32)
  # On Windows, CMake by default compiles with the shared CRT.
  # Default it to the static CRT.
  option(AMBER_ENABLE_SHARED_CRT
         "Amber: Use the shared CRT with MSVC instead of the static CRT"
	 ${AMBER_ENABLE_SHARED_CRT})
endif(WIN32)

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

if (${AMBER_SKIP_LODEPNG})
  set(AMBER_ENABLE_LODEPNG FALSE)
else()
  set(AMBER_ENABLE_LODEPNG TRUE)
endif()

if (${AMBER_ENABLE_SWIFTSHADER})
  # Swiftshader requires the loader to be built.
  set(AMBER_USE_LOCAL_VULKAN TRUE)
endif()

if (${AMBER_USE_DXC})
  set(AMBER_ENABLE_DXC TRUE)
else()
  set(AMBER_ENABLE_DXC FALSE)
endif()

if (${AMBER_USE_CLSPV})
  enable_language(ASM)
  set(AMBER_ENABLE_CLSPV TRUE)
  set(AMBER_ENABLE_SPIRV_TOOLS TRUE)
else()
  set(AMBER_ENABLE_CLSPV FALSE)
endif()

message(STATUS "Using python3")
find_package(PythonInterp 3 REQUIRED)

message(STATUS "Amber enable SPIRV-Tools: ${AMBER_ENABLE_SPIRV_TOOLS}")
message(STATUS "Amber enable Shaderc: ${AMBER_ENABLE_SHADERC}")
message(STATUS "Amber enable tests: ${AMBER_ENABLE_TESTS}")
message(STATUS "Amber enable samples: ${AMBER_ENABLE_SAMPLES}")
message(STATUS "Amber enable lodepng: ${AMBER_ENABLE_LODEPNG}")
message(STATUS "Amber enable SwiftShader: ${AMBER_ENABLE_SWIFTSHADER}")
message(STATUS "Amber enable DXC: ${AMBER_ENABLE_DXC}")
message(STATUS "Amber enable Clspv: ${AMBER_ENABLE_CLSPV}")

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
add_definitions(-DAMBER_ENABLE_DXC=$<BOOL:${AMBER_ENABLE_DXC}>)
add_definitions(-DAMBER_ENABLE_CLSPV=$<BOOL:${AMBER_ENABLE_CLSPV}>)
add_definitions(-DAMBER_ENABLE_LODEPNG=$<BOOL:${AMBER_ENABLE_LODEPNG}>)

set(CMAKE_DEBUG_POSTFIX "")

# This has to be done very early so the link path will get set correctly for all
# the various libraries and binaries.
if (${AMBER_ENABLE_DXC})
  link_directories("${CMAKE_BINARY_DIR}/third_party/dxc/lib")

  if (MSVC)
    # DXC turns this off all over the place so we have to do the same.
    add_definitions(/D_ITERATOR_DEBUG_LEVEL=0)
  endif()
endif()

if ("${CMAKE_BUILD_TYPE}" STREQUAL "")
  message(STATUS "No build type selected, default to Debug")
  set(CMAKE_BUILD_TYPE "Debug")
endif()

if(("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU") OR
    (("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang") AND
     (NOT CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC")))
  set(COMPILER_IS_LIKE_GNU TRUE)
endif()

if(MSVC)
  # We don't want to have to copy the C Runtime DLL everywhere the executable
  # goes.  So by default compile code to assume the CRT is statically linked,
  # i.e. use /MT* options.  For debug builds use /MTd, and for release builds
  # use /MT.  If AMBER_ENABLE_SHARED_CRT is ON, then use the shared C runtime.
  # Modify the project-wide options variables. This is ugly, but seems to be
  # the state of the art.
  if(NOT ${AMBER_ENABLE_SHARED_CRT})
    message(STATUS "Amber: Static C runtime selected: replacing /MD* with /MT*")
    foreach (flag_var
	     CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
	     CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO
	     CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
	     CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
      string(REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
    endforeach()
  endif()
endif()

function(amber_default_compile_options TARGET)
  if (${COMPILER_IS_LIKE_GNU})
    target_compile_options(${TARGET} PRIVATE
      -std=c++11
      -fno-exceptions
      -fno-rtti
      -fvisibility=hidden
      -Wall
      -Werror
      -Wextra
      -Wno-padded
      -Wno-switch-enum
      -Wno-unknown-pragmas
      -pedantic-errors
    )

    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
      target_compile_options(${TARGET} PRIVATE
        -Wno-c++98-compat
        -Wno-c++98-compat-pedantic
        -Wno-format-pedantic
        -Wno-unknown-warning-option
        -Weverything
      )
    endif()
  endif()

  if (MSVC)
    # Specify /EHs for exception handling.
    target_compile_options(${TARGET} PRIVATE
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

  if (NOT ${AMBER_ENABLE_SHARED_CRT})
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
  endif()
endfunction()

add_subdirectory(third_party)
add_subdirectory(src)

if (${AMBER_ENABLE_SAMPLES} AND NOT ANDROID)
  add_subdirectory(samples)
endif()
