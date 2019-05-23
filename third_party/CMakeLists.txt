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

if (${AMBER_ENABLE_TESTS})
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/googletest EXCLUDE_FROM_ALL)
endif()

if (${AMBER_ENABLE_SPIRV_TOOLS})
  set(SPIRV-Headers_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/spirv-headers CACHE STRING "")
  set(SPIRV_SKIP_TESTS ON CACHE BOOL ON)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/spirv-tools EXCLUDE_FROM_ALL)
endif()

if (${AMBER_ENABLE_SHADERC})
  set(ENABLE_HLSL ON CACHE BOOL ON)
  set(BUILD_TESTING FALSE CACHE BOOL ON)
  set(ENABLE_GLSLANG_BINARIES OFF CACHE BOOL ON)
  set(ENABLE_SPVREMAPPER OFF CACHE BOOL ON)
  # Tell Glslang to statically link the C Runtime library.
  set(LLVM_USE_CRT_DEBUG MTd CACHE STRING "")
  set(LLVM_USE_CRT_RELEASE MT CACHE STRING "")
  set(LLVM_USE_CRT_RELWITHDEBINFO MT CACHE STRING "")
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/glslang)

  set(SHADERC_THIRD_PARTY_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE STRING "")
  set(SHADERC_SKIP_TESTS ON CACHE BOOL ON)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/shaderc)
endif()

if (${AMBER_ENABLE_SAMPLES})
  # Lodepng
  set(LODEPNG_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/lodepng/lodepng.cpp)
  add_library(lodepng STATIC ${LODEPNG_SOURCES})
endif()

if (${AMBER_USE_LOCAL_VULKAN})
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vulkan-headers)

  set(BUILD_TESTS FALSE)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vulkan-loader)
endif()

if (${AMBER_ENABLE_SWIFTSHADER})
  set(BUILD_EGL FALSE CACHE BOOL FALSE)
  set(BUILD_GLESv2 FALSE CACHE BOOL FALSE)
  set(BUILD_GLES_CM FALSE CACHE BOOL FALSE)
  set(BUILD_VULKAN TRUE CACHE BOOL TRUE)
  set(BUILD_SAMPLES FALSE CACHE BOOL FALSE)
  set(BUILD_TESTS FALSE CACHE BOOL FALSE)
  set(WARNINGS_AS_ERRORS FALSE CACHE BOOL FALSE)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/swiftshader)
endif()
