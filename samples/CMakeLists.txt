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
    timestamp.cc
    ${CMAKE_BINARY_DIR}/src/build-versions.h.fake
)

set(AMBER_EXTRA_LIBS "")

if (${AMBER_ENABLE_LODEPNG})
  set(AMBER_SOURCES ${AMBER_SOURCES} png.cc)
  list(APPEND AMBER_EXTRA_LIBS "lodepng")
endif()

if (${Vulkan_FOUND})
  set(AMBER_SOURCES ${AMBER_SOURCES} config_helper_vulkan.cc)
  list(APPEND AMBER_EXTRA_LIBS ${VULKAN_LIB})
endif()

if (${Dawn_FOUND})
  set(AMBER_SOURCES ${AMBER_SOURCES} config_helper_dawn.cc)
  list(APPEND AMBER_EXTRA_LIBS Dawn::dawn_native Dawn::dawn)
  if (APPLE)
    add_definitions(-DAMBER_DAWN_METAL=1)
    find_library(METAL_LIB Metal)
    list(APPEND AMBER_EXTRA_LIBS ${METAL_LIB})
  else()
    add_definitions(-DAMBER_DAWN_METAL=0)
  endif()
endif()

add_executable(amber ${AMBER_SOURCES})
target_include_directories(amber PRIVATE "${CMAKE_BINARY_DIR}")

set_target_properties(amber PROPERTIES OUTPUT_NAME "amber")
target_link_libraries(amber libamber ${AMBER_EXTRA_LIBS})
amber_default_compile_options(amber)

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/src/build-versions.h.fake
    COMMAND
      ${PYTHON_EXECUTABLE}
        ${PROJECT_SOURCE_DIR}/tools/update_build_version.py
        ${CMAKE_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/third_party
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    COMMENT "Update build-versions.h in the build directory"
)

set(IMAGE_DIFF_SOURCES
    image_diff.cc
)
add_executable(image_diff ${IMAGE_DIFF_SOURCES})
target_include_directories(image_diff PRIVATE "${CMAKE_BINARY_DIR}")
target_link_libraries(image_diff libamber "lodepng")
amber_default_compile_options(image_diff)
set_target_properties(image_diff PROPERTIES OUTPUT_NAME "image_diff")
