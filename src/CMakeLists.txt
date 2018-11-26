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
    amber_impl.cc
    amberscript/executor.cc
    amberscript/parser.cc
    amberscript/pipeline.cc
    amberscript/script.cc
    amberscript/shader.cc
    buffer.cc
    command.cc
    command_data.cc
    datum_type.cc
    engine.cc
    executor.cc
    format.cc
    parser.cc
    pipeline_data.cc
    result.cc
    sleep.cc
    script.cc
    shader_compiler.cc
    tokenizer.cc
    value.cc
    verifier.cc
    vkscript/command_parser.cc
    vkscript/datum_type_parser.cc
    vkscript/executor.cc
    vkscript/format_parser.cc
    vkscript/nodes.cc
    vkscript/parser.cc
    vkscript/script.cc
    vkscript/section_parser.cc
)

if (${Vulkan_FOUND})
  add_subdirectory(vulkan)
endif()
if (${Dawn_FOUND})
  add_subdirectory(dawn)
endif()

add_library(libamber ${AMBER_SOURCES})
amber_default_compile_options(libamber)
set_target_properties(libamber PROPERTIES OUTPUT_NAME "amber")
# TODO(dsinclair): Remove pthread when building on windows.
target_link_libraries(libamber SPIRV-Tools shaderc SPIRV)
if (NOT MSVC)
	target_link_libraries(libamber pthread)
endif()

if (${Vulkan_FOUND})
  target_link_libraries(libamber libamberenginevulkan)
endif()
if (${Dawn_FOUND})
  target_link_libraries(libamber libamberenginedawn)
endif()

set(TEST_SRCS
    amberscript/parser_test.cc
    amberscript/pipeline_test.cc
    amberscript/script_test.cc
    buffer_test.cc
    command_data_test.cc
    result_test.cc
    shader_compiler_test.cc
    tokenizer_test.cc
    vkscript/command_parser_test.cc
    vkscript/datum_type_parser_test.cc
    vkscript/executor_test.cc
    vkscript/format_parser_test.cc
    vkscript/parser_test.cc
    vkscript/section_parser_test.cc
)

if (${Vulkan_FOUND})
    list(APPEND TEST_SRCS vulkan/bit_copy_test.cc)
endif()
if (${Dawn_FOUND})
    list(APPEND TEST_SRCS dawn/pipeline_info_test.cc)
endif()

add_executable(amber_unittests ${TEST_SRCS})

if (NOT MSVC)
	target_compile_options(amber_unittests PRIVATE
	    -Wno-global-constructors)
endif()

target_include_directories(amber_unittests PRIVATE
    ${gmock_SOURCE_DIR}/include)
target_link_libraries(amber_unittests libamber gmock_main)
amber_default_compile_options(amber_unittests)
add_test(NAME amber_unittests COMMAND amber_unittests)
