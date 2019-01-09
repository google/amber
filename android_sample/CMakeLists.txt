# Copyright 2019 The Amber Authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 3.1)

# build native_app_glue as a static lib
set(APP_GLUE_DIR ${ANDROID_NDK}/sources/android/native_app_glue)
include_directories(${APP_GLUE_DIR})
add_library(app-glue STATIC
  ${APP_GLUE_DIR}/android_native_app_glue.c)

# build vulkan app
set(SRC_DIR jni)

add_library(amber_android SHARED
  ${SRC_DIR}/main.cc
  ${SRC_DIR}/amber_script.cc)

include_directories(${SRC_DIR})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror \
                     -DVK_USE_PLATFORM_ANDROID_KHR")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} \
                               -u ANativeActivity_onCreate")

# Begin: include Amber
set(AMBER_DIR ${CMAKE_SOURCE_DIR}/..)
set(AMBER_SKIP_TESTS TRUE)
set(AMBER_SKIP_SPIRV_TOOLS TRUE)
set(Vulkan_FOUND TRUE)
set(Dawn_FOUND FALSE)

include_directories(${AMBER_DIR}/include)
message("Android Amber adds header path: ${AMBER_DIR}/include")
include_directories(${AMBER_DIR})
message("Android Amber adds header path: ${AMBER_DIR}")

add_subdirectory(${AMBER_DIR} amber_vulkan)
target_link_libraries(amber_android libamber vulkan)
# End: include Amber

target_link_libraries(amber_android app-glue log android)
