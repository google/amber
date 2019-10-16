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

set(AMBER_SOURCES
    amber.cc
    amberscript/parser.cc
    buffer.cc
    command.cc
    command_data.cc
    descriptor_set_and_binding_parser.cc
    engine.cc
    executor.cc
    format.cc
    parser.cc
    pipeline.cc
    pipeline_data.cc
    recipe.cc
    result.cc
    sleep.cc
    script.cc
    shader.cc
    shader_compiler.cc
    tokenizer.cc
    type.cc
    type_parser.cc
    value.cc
    verifier.cc
    vkscript/command_parser.cc
    vkscript/datum_type_parser.cc
    vkscript/parser.cc
    vkscript/section_parser.cc
)

if (${Vulkan_FOUND})
  add_subdirectory(vulkan)
  list(APPEND AMBER_SOURCES vulkan_engine_config.cc)
endif()
if (${Dawn_FOUND})
  add_subdirectory(dawn)
  list(APPEND AMBER_SOURCES dawn_engine_config.cc)
endif()

if (${AMBER_ENABLE_DXC})
  list(APPEND AMBER_SOURCES dxc_helper.cc)
endif()

if (${AMBER_ENABLE_CLSPV})
  list(APPEND AMBER_SOURCES clspv_helper.cc)
endif()

add_library(libamber ${AMBER_SOURCES})
amber_default_compile_options(libamber)
target_include_directories(libamber PRIVATE "${CMAKE_BINARY_DIR}")
set_target_properties(libamber PROPERTIES OUTPUT_NAME "amber")

if (${AMBER_ENABLE_DXC})
  target_include_directories(libamber PRIVATE
    "${PROJECT_SOURCE_DIR}/third_party/dxc/include"
    "${CMAKE_BINARY_DIR}/third_party/dxc/include"
  )

  add_dependencies(libamber dxcompiler)
  target_link_libraries(libamber
    dxcompiler
    LLVMDxcSupport
    LLVMOption
    LLVMHLSL
    LLVMScalarOpts
  )
endif()

if (${AMBER_ENABLE_CLSPV})
  target_include_directories(libamber PRIVATE
    "${PROJECT_SOURCE_DIR}/third_party/clspv/include"
  )

  target_link_libraries(libamber
    clspv_core
    clspv_passes
  )
endif()

if (${AMBER_ENABLE_SPIRV_TOOLS})
  target_link_libraries(libamber SPIRV-Tools)
endif()

if (${AMBER_ENABLE_SHADERC})
  target_link_libraries(libamber shaderc SPIRV)
endif()

if (NOT MSVC AND NOT ANDROID)
  target_link_libraries(libamber pthread)
endif()

if (${Vulkan_FOUND})
  target_link_libraries(libamber libamberenginevulkan)
endif()
if (${Dawn_FOUND})
  target_link_libraries(libamber libamberenginedawn)
endif()

if (${AMBER_ENABLE_TESTS})
  set(TEST_SRCS
    amberscript/parser_attach_test.cc
    amberscript/parser_bind_test.cc
    amberscript/parser_buffer_test.cc
    amberscript/parser_clear_color_test.cc
    amberscript/parser_clear_test.cc
    amberscript/parser_compile_options_test.cc
    amberscript/parser_copy_test.cc
    amberscript/parser_device_feature_test.cc
    amberscript/parser_expect_test.cc
    amberscript/parser_extension_test.cc
    amberscript/parser_framebuffer_test.cc
    amberscript/parser_pipeline_test.cc
    amberscript/parser_pipeline_set_test.cc
    amberscript/parser_repeat_test.cc
    amberscript/parser_run_test.cc
    amberscript/parser_set_test.cc
    amberscript/parser_shader_opt_test.cc
    amberscript/parser_shader_test.cc
    amberscript/parser_struct_test.cc
    amberscript/parser_test.cc
    buffer_test.cc
    command_data_test.cc
    descriptor_set_and_binding_parser_test.cc
    executor_test.cc
    format_test.cc
    pipeline_test.cc
    result_test.cc
    script_test.cc
    shader_compiler_test.cc
    tokenizer_test.cc
    type_parser_test.cc
    type_test.cc
    verifier_test.cc
    vkscript/command_parser_test.cc
    vkscript/datum_type_parser_test.cc
    vkscript/parser_test.cc
    vkscript/section_parser_test.cc
    ../samples/ppm.cc
    ../samples/ppm_test.cc
  )

  if (${Vulkan_FOUND})
    list(APPEND TEST_SRCS vulkan/vertex_buffer_test.cc)
  endif()

  if (${Dawn_FOUND})
    list(APPEND TEST_SRCS dawn/pipeline_info_test.cc)
  endif()

  add_executable(amber_unittests ${TEST_SRCS})

  if (NOT MSVC)
    target_compile_options(amber_unittests PRIVATE
      -Wno-global-constructors
      -Wno-weak-vtables
    )
  endif()

  target_include_directories(amber_unittests PRIVATE
      ${gmock_SOURCE_DIR}/include)
  target_link_libraries(amber_unittests libamber gmock_main)
  amber_default_compile_options(amber_unittests)
  add_test(NAME amber_unittests COMMAND amber_unittests)

  if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    # vulkan/vulkan.h defines VK_NULL_HANDLE as 0u and that also serves as a null pointer.
    # Disable Clang's warning that will alwaays fire on that.  This is required to build
    # with XCode 10.
    target_compile_options(amber_unittests PRIVATE -Wno-zero-as-null-pointer-constant)
  endif()
endif()
