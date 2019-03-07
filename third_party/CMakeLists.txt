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

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  # Turn off warnings to make gtest happy
  set(GTEST_BUILD_FIXES
    -Wno-covered-switch-default
    -Wno-deprecated
    -Wno-disabled-macro-expansion
    -Wno-exit-time-constructors
    -Wno-exit-time-destructors
    -Wno-extra-semi-stmt
    -Wno-global-constructors
    -Wno-missing-field-initializers
    -Wno-format-nonliteral
    -Wno-missing-noreturn
    -Wno-missing-prototypes
    -Wno-missing-variable-declarations
    -Wno-shift-sign-overflow
    -Wno-sign-conversion
    -Wno-undef
    -Wno-unused-member-function
    -Wno-used-but-marked-unused
    -Wno-weak-vtables
    -Wno-zero-as-null-pointer-constant)

  set(GLSLANG_BUILD_FIXES
    -Wno-conversion
    -Wno-covered-switch-default
    -Wno-date-time
    -Wno-deprecated
    -Wno-disabled-macro-expansion
    -Wno-double-promotion
    -Wno-error
    -Wno-exit-time-destructors
    -Wno-extra-semi
    -Wno-extra-semi-stmt
    -Wno-float-equal
    -Wno-format-nonliteral
    -Wno-format-pedantic
    -Wno-global-constructors
    -Wno-gnu-redeclared-enum
    -Wno-implicit-fallthrough
    -Wno-inconsistent-missing-destructor-override
    -Wno-missing-field-initializers
    -Wno-missing-noreturn
    -Wno-missing-prototypes
    -Wno-missing-variable-declarations
    -Wno-newline-eof
    -Wno-old-style-cast
    -Wno-reserved-id-macro
    -Wno-shadow
    -Wno-shadow-field
    -Wno-shadow-field-in-constructor
    -Wno-shift-sign-overflow
    -Wno-sign-conversion
    -Wno-signed-enum-bitfield
    -Wno-undef
    -Wno-undefined-func-template
    -Wno-undefined-reinterpret-cast
    -Wno-unreachable-code
    -Wno-unreachable-code-break
    -Wno-unreachable-code-return
    -Wno-unused-macros
    -Wno-unused-parameter
    -Wno-unused-variable
    -Wno-used-but-marked-unused
    -Wno-weak-vtables
    -Wno-zero-as-null-pointer-constant)

  set(LODEPNG_BUILD_FIXES
    -Wno-cast-qual
    -Wno-conversion
    -Wno-covered-switch-default
    -Wno-deprecated
    -Wno-extra-semi-stmt
    -Wno-implicit-int-conversion
    -Wno-missing-prototypes
    -Wno-old-style-cast
    -Wno-shorten-64-to-32
    -Wno-weak-vtables
    -Wno-zero-as-null-pointer-constant)

  set(SPIRV_TOOLS_BUILD_FIXES
    -Wno-conditional-uninitialized
    -Wno-covered-switch-default
    -Wno-deprecated
    -Wno-documentation
    -Wno-documentation-pedantic
    -Wno-double-promotion
    -Wno-extra-semi
    -Wno-extra-semi-stmt
    -Wno-float-equal
    -Wno-format-nonliteral
    -Wno-implicit-fallthrough
    -Wno-missing-prototypes
    -Wno-old-style-cast
    -Wno-range-loop-analysis
    -Wno-shift-sign-overflow
    -Wno-unreachable-code-break
    -Wno-unreachable-code-return
    -Wno-unused-member-function
    -Wno-weak-vtables
    -Wno-zero-as-null-pointer-constant)

  set(SHADERC_BUILD_FIXES
    -Wno-comma
    -Wno-conversion
    -Wno-covered-switch
    -Wno-covered-switch-default
    -Wno-deprecated
    -Wno-disabled-macro-expansion
    -Wno-double-promotion
    -Wno-extra-semi
    -Wno-float-equal
    -Wno-global-constructors
    -Wno-inconsistent-missing-destructor-override
    -Wno-missing-field-initializers
    -Wno-missing-prototypes
    -Wno-newline-eof
    -Wno-old-style-cast
    -Wno-reserved-id-macro
    -Wno-shadow
    -Wno-shadow-field
    -Wno-shadow-field-in-constructor
    -Wno-shadow-uncaptured-local
    -Wno-sign-conversion
    -Wno-signed-enum-bitfield
    -Wno-undef
    -Wno-unreachable-code-return
    -Wno-weak-vtables
    -Wno-zero-as-null-pointer-constant
  )

  set(VULKAN_LOADER_BUILD_FIXES
    -Wno-unknown-warning-option
  )
endif()

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(GTEST_BUILD_FIXES "")

  set(GLSLANG_BUILD_FIXES
    -Wno-error
    -Wno-implicit-fallthrough
    -Wno-overflow
    -Wno-missing-field-initializers
    -Wno-pedantic
    -Wno-strict-aliasing
    -Wno-unused-parameter
    -Wno-unused-variable)

  set(LODEPNG_BUILD_FIXES "")

  set(SPIRV_TOOLS_BUILD_FIXES "")

  set(SHADERC_BUILD_FIXES
      -Wno-missing-field-initializers
      -Wno-pedantic)

  set(VULKAN_LOADER_BUILD_FIXES "")
endif()

if (MSVC)
  set(GTEST_BUILD_FIXES
    /W3
    /wd4061
    /wd4365
    /wd4514
    /wd4571
    /wd4623
    /wd4625
    /wd4626
    /wd4668
    /wd4710
    /wd4774
    /wd4820
    /wd5026
    /wd5027
    /wd5039
    /wd5045)

  set(GLSLANG_BUILD_FIXES
    /W3
    /wd4242
    /wd4244
    /wd4571
    /wd4623
    /wd4625
    /wd4626
    /wd4702
    /wd4774
    /wd4820
    /wd4946
    /wd4996
    /wd5026
    /wd5027
    /wd5045)

  set(LODEPNG_BUILD_FIXES
    /W3
    /wd4267)

  set(SPIRV_TOOLS_BUILD_FIXES
    /W3
    /wd4365
    /wd4389
    /wd4571
    /wd4623
    /wd4625
    /wd4626
    /wd4702
    /wd4706
    /wd4710
    /wd4774
    /wd4820
    /wd4826
    /wd4868
    /wd5026
    /wd5027
    /wd5045)

  set(SHADERC_BUILD_FIXES
    /W3
    /wd4365
    /wd4571
    /wd4623
    /wd4625
    /wd4626
    /wd4702
    /wd4774
    /wd4820
    /wd4996
    /wd5026
    /wd5027
    /wd5039
    /wd5045)

  set(VULKAN_LOADER_BUILD_FIXES
    /W3
  )
endif()

if (${AMBER_ENABLE_TESTS})
  set(CXX_BACK ${CMAKE_CXX_FLAGS})
  SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "${GTEST_BUILD_FIXES}")
  STRING(REGEX REPLACE ";" " " CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/googletest EXCLUDE_FROM_ALL)
  set(CMAKE_CXX_FLAGS ${CXX_BACK})
endif()

if (${AMBER_ENABLE_SPIRV_TOOLS})
  set(CXX_BACK ${CMAKE_CXX_FLAGS})
  SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "${SPIRV_TOOLS_BUILD_FIXES}")
  STRING(REGEX REPLACE ";" " " CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  set(SPIRV-Headers_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/spirv-headers CACHE STRING "")
  set(SPIRV_SKIP_TESTS ON CACHE BOOL ON)
  set(SPIRV_SKIP_EXECUTABLES ON CACHE BOOL ON)
  set(SPIRV_HEADERS_SKIP_EXAMPLES ON CACHE BOOL ON)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/spirv-tools)
  set(CMAKE_CXX_FLAGS ${CXX_BACK})
endif()

if (${AMBER_ENABLE_SHADERC})
  set(CXX_BACK ${CMAKE_CXX_FLAGS})
  SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "${GLSLANG_BUILD_FIXES}")
  STRING(REGEX REPLACE ";" " " CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  set(ENABLE_HLSL ON CACHE BOOL ON)
  set(BUILD_TESTING FALSE CACHE BOOL ON)
  set(ENABLE_GLSLANG_BINARIES OFF CACHE BOOL ON)
  set(ENABLE_SPVREMAPPER OFF CACHE BOOL ON)
  # Tell Glslang to statically link the C Runtime library.
  set(LLVM_USE_CRT_DEBUG MTd CACHE STRING "")
  set(LLVM_USE_CRT_RELEASE MT CACHE STRING "")
  set(LLVM_USE_CRT_RELWITHDEBINFO MT CACHE STRING "")
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/glslang)
  set(CMAKE_CXX_FLAGS ${CXX_BACK})

  set(CXX_BACK ${CMAKE_CXX_FLAGS})
  SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "${SHADERC_BUILD_FIXES}")
  STRING(REGEX REPLACE ";" " " CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  set(SHADERC_THIRD_PARTY_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE STRING "")
  set(SHADERC_SKIP_TESTS ON CACHE BOOL ON)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/shaderc)
  set(CMAKE_CXX_FLAGS ${CXX_BACK})
endif()

if (${AMBER_ENABLE_SAMPLES})
  # Lodepng
  set(LODEPNG_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/lodepng/lodepng.cpp)
  STRING(REGEX REPLACE ";" " " LODEPNG_BUILD_FIXES "${LODEPNG_BUILD_FIXES}")

  set_source_files_properties(
    ${LODEPNG_SOURCES}
    PROPERTIES
    COMPILE_FLAGS "${LODEPNG_BUILD_FIXES}")

  add_library(lodepng STATIC ${LODEPNG_SOURCES})
endif()

if (${AMBER_USE_LOCAL_VULKAN})
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vulkan-headers)

  set(CXX_BACK ${CMAKE_CXX_FLAGS})
  SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "${VULKAN_LOADER_BUILD_FIXES}")
  STRING(REGEX REPLACE ";" " " CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  set(BUILD_TESTS FALSE)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vulkan-loader)
  set(CMAKE_CXX_FLAGS ${CXX_BACK})
endif()

