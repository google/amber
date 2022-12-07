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
  if (${AMBER_ENABLE_SHARED_CRT})
    set(gtest_force_shared_crt ON)
  endif()
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
  set(SHADERC_ENABLE_WERROR_COMPILE OFF CACHE BOOL "disable werror")

  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/shaderc)
endif()

if (${AMBER_ENABLE_SAMPLES} AND ${AMBER_ENABLE_LODEPNG})
  # Lodepng
  set(LODEPNG_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/lodepng/lodepng.cpp)
  add_library(lodepng STATIC ${LODEPNG_SOURCES})
endif()

if (${AMBER_USE_LOCAL_VULKAN})
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vulkan-headers)

  # Skip adding the validation layers and the Vulkan loader on Android.
  if (NOT ANDROID)

    set(BUILD_TESTS FALSE)

    # The vulkan-loader CMake file assumes that directory exists if
    # Wayland support is to be built.
    if(NOT EXISTS ${WAYLAND_CLIENT_INCLUDE_DIR})
      message(STATUS "Amber: Disabling Wayland support in Vulkan-Loader")
      set(BUILD_WSI_WAYLAND_SUPPORT OFF CACHE BOOL "" FORCE)
    endif()
    message(STATUS "Amber: Disabling X11 support in Vulkan-Loader")
    set(BUILD_WSI_XLIB_SUPPORT OFF CACHE BOOL "" FORCE)

    set(ROBIN_HOOD_HASHING_INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/robin-hood-hashing" CACHE STRING "" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/robin-hood-hashing)
    set(SPIRV_HEADERS_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/spirv-headers/include" CACHE STRING "" FORCE)

    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vulkan-loader)
    if (MSVC)
      option(BUILD_WERROR "Treat compiler warnings as errors" OFF)
    endif()
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vulkan-validationlayers)
  endif()
endif()

if (${AMBER_ENABLE_SWIFTSHADER})
  set(SWIFTSHADER_BUILD_EGL FALSE)
  set(SWIFTSHADER_BUILD_GLESv2 FALSE)
  set(SWIFTSHADER_BUILD_GLES_CM FALSE)
  set(SWIFTSHADER_BUILD_VULKAN TRUE)
  set(SWIFTSHADER_BUILD_SAMPLES FALSE)
  set(SWIFTSHADER_BUILD_TESTS FALSE)
  set(SWIFTSHADER_WARNINGS_AS_ERRORS FALSE)
  set(SWIFTSHADER_LOGGING_LEVEL "Error")

  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/swiftshader)
endif()

if (${AMBER_ENABLE_DXC})
  set(DXC_EXTERNAL_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}" CACHE STRING "" FORCE)
  set(DXC_SPIRV_HEADERS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/spirv-headers" CACHE PATH "" FORCE)
  set(DXC_SPIRV_TOOLS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/spirv-tools" CACHE PATH "" FORCE)

  set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "")
  set(LLVM_APPEND_VC_REV ON CACHE BOOL "")
  set(LLVM_DEFAULT_TARGET_TRIPLE "dxil-ms-dx" CACHE STRING "")
  set(LLVM_ENABLE_EH ON CACHE BOOL "")
  set(LLVM_ENABLE_RTTI ON CACHE BOOL "")
  set(LLVM_BUILD_EXAMPLES OFF CACHE BOOL "")
  set(LLVM_BUILD_TESTS ON CACHE BOOL "")
  set(LLVM_INCLUDE_DOCS OFF CACHE BOOL "")
  set(LLVM_INCLUDE_EXAMPLES OFF CACHE BOOL "")
  set(LLVM_INCLUDE_TESTS OFF CACHE BOOL "")
  set(LLVM_INCLUDE_TOOLS ON CACHE BOOL "")
  set(LLVM_INCLUDE_UTILS OFF CACHE BOOL "")
  set(LLVM_USE_CRT_DEBUG MTd CACHE STRING "")
  set(LLVM_USE_CRT_RELEASE MT CACHE STRING "")
  set(LLVM_USE_CRT_RELWITHDEBINFO MT CACHE STRING "")

  set(LLVM_OPTIMIZED_TABLEGEN OFF CACHE BOOL "")
  set(LLVM_REQUIRES_EH ON CACHE BOOL "")
  set(LLVM_REQUIRES_RTTI ON CACHE BOOL "")
  set(LLVM_TARGETS_TO_BUILD None CACHE STRING "")
  set(LIBCLANG_BUILD_STATIC ON CACHE BOOL "")
  set(CLANG_BUILD_EXAMPLES OFF CACHE BOOL "")
  set(CLANG_CL OFF CACHE BOOL "")
  set(CLANG_ENABLE_ARCMT OFF CACHE BOOL "")
  set(CLANG_ENABLE_STATIC_ANALYZER OFF CACHE BOOL "")
  set(CLANG_INCLUDE_TESTS OFF CACHE BOOL "")
  set(HLSL_INCLUDE_TESTS OFF CACHE BOOL "")
  set(ENABLE_SPIRV_CODEGEN ON CACHE BOOL "enable spirv codegen")
  set(SPIRV_BUILD_TESTS OFF CACHE BOOL "Enable dxc spirv tests")

  set(LLVM_BUILD_STATIC ON CACHE BOOL "")
  set(BUILD_SHARED_LIBS OFF CACHE BOOL "")

  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/dxc EXCLUDE_FROM_ALL)

  if (MSVC)
    add_custom_target(dxc-binary ALL
      DEPENDS "${CMAKE_BINARY_DIR}/third_party/dxc/bin/dxcompiler.dll"
      COMMENT "dxc-binary custom target")

      ADD_CUSTOM_COMMAND(
        TARGET dxc-binary
        POST_BUILD
        COMMAND ${CMAKE_COMMAND}
        ARGS -E copy ${CMAKE_BINARY_DIR}/third_party/dxc/bin/dxcompiler.dll ${CMAKE_BINARY_DIR}
      )
  endif()
endif()

if (${AMBER_ENABLE_CLSPV})
  if (NOT DEFINED AMBER_CLSPV_DIR)
    set(AMBER_CLSPV_DIR "${CMAKE_CURRENT_SOURCE_DIR}/clspv")
  endif()
  if (NOT DEFINED AMBER_CLSPV_LLVM_DIR)
    set(AMBER_CLSPV_LLVM_DIR "${CMAKE_CURRENT_SOURCE_DIR}/clspv-llvm")
  endif()

  list(APPEND CMAKE_MODULE_PATH ${AMBER_CLSPV_LLVM_DIR}/llvm/cmake/modules/)

  set(CLSPV_LLVM_SOURCE_DIR "${AMBER_CLSPV_LLVM_DIR}/llvm" CACHE STRING "")
  set(CLSPV_CLANG_SOURCE_DIR "${AMBER_CLSPV_LLVM_DIR}/clang" CACHE STRING "")
  set(SPIRV_HEADERS_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/spirv-headers" CACHE STRING "")
  set(SPIRV_TOOLS_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/spirv-tools" CACHE STRING "")
  set(LINKER_SUPPORTS_COLOR_DIAGNOSTICS 0 CACHE INTERNAL 0 FORCE)

  set(LLVM_TEMPORARILY_ALLOW_OLD_TOOLCHAIN ON CACHE STRING "")
  set(LLVM_BUILD_STATIC OFF CACHE BOOL "")
  set(LLVM_INCLUDE_BENCHMARKS OFF CACHE BOOL "")
  set(LLVM_INCLUDE_DOCS OFF CACHE BOOL "")
  set(LLVM_INCLUDE_EXAMPLES OFF CACHE BOOL "")
  set(LLVM_INCLUDE_GO_TESTS OFF CACHE BOOL "")
  set(LLVM_INCLUDE_TESTS OFF CACHE BOOL "")
  set(LLVM_INCLUDE_UTILS OFF CACHE BOOL "")

  add_subdirectory(${AMBER_CLSPV_DIR} ${CMAKE_CURRENT_BINARY_DIR}/clspv)

  # TODO(asuonpaa): Using the latest LLVM gives multiple warnings of using deprecated
  # arguments. Remove this when CLSPV doesn't use them.
  add_compile_options(clspv, NO_DEPRECATED_FLAGS)
endif()
