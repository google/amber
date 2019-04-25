# Copyright 2019 The Amber Authors.
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

add_library(libamber_dxc_util dxc_util.cc)

target_link_libraries(libamber_dxc_util clangSPIRV dxcompiler)

# This is necessary so that the linked dxcompiler is loaded into memory space
# first, and in case dxcompiler.so is loaded via 'dlopen', it resolves to the
# same memory space, and prevents getting two copies of global static variables.
add_dependencies(libamber_dxc_util clangSPIRV dxcompiler)

target_include_directories(libamber_dxc_util
    PRIVATE "${PROJECT_SOURCE_DIR}/third_party/dxc/include"
    PRIVATE "${CMAKE_BINARY_DIR}/third_party/dxc/include")
