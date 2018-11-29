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


# Include this file to find Dawn and and set up compilation and linking.

## Example usage:
##
## include(find_dawn.cmake)
## # Set HAVE_DAWN to 1 if we have Dawn, and 0 otherwise.
## add_definitions(-DHAVE_DAWN=$<BOOL:${Dawn_FOUND}>)
## # Set up link dependencies.
## if (${Dawn_FOUND})
##   target_link_libraries(mylib Dawn::dawn_native)
## endif()
##

# Exports these settings to the includer:
#    Boolean Dawn_FOUND indicates whether we found Dawn.
#    If Dawn was found, then library dependencies for Dawn::dawn and Dawn::dawn_native
#    will be set up.
set(Dawn_FOUND FALSE)

# Setup via CMake setting variables:
#
#   Separately specify the directory locations of the Dawn headers and
#   the dawn_native library.
#
#     -DDawn_INCLUDE_DIR=<directory containing dawn/dawn_export.h>
#     -DDawn_GEN_INCLUDE_DIR=<directory containing dawn/dawn.h>
#     -DDawn_LIBRARY_DIR=<directory containing dawn_native library
#                         e.g., libdawn_native.a>


find_path(Dawn_INCLUDE_DIR
  NAMES dawn/dawn_export.h
  PATHS
    "${Dawn_INCLUDE_DIR}"
  )
find_path(Dawn_GEN_INCLUDE_DIR
  NAMES dawn/dawn.h dawn/dawncpp.h
  PATHS
    "${Dawn_GEN_INCLUDE_DIR}"
  )
find_library(Dawn_LIBRARY
  NAMES dawn
  PATHS
    "${Dawn_LIBRARY_DIR}"
  )
find_library(Dawn_native_LIBRARY
  NAMES dawn_native
  PATHS
    "${Dawn_LIBRARY_DIR}"
  )

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Dawn
  DEFAULT_MSG
  Dawn_INCLUDE_DIR Dawn_GEN_INCLUDE_DIR
  Dawn_LIBRARY Dawn_native_LIBRARY
  )

if(${Dawn_FOUND} AND NOT TARGET Dawn::dawn)
  add_library(Dawn::dawn UNKNOWN IMPORTED)
  set_target_properties(Dawn::dawn PROPERTIES
    IMPORTED_LOCATION "${Dawn_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${Dawn_INCLUDE_DIR};${Dawn_GEN_INCLUDE_DIR}")
endif()
if(${Dawn_FOUND} AND NOT TARGET Dawn::dawn_native)
  add_library(Dawn::dawn_native UNKNOWN IMPORTED)
  set_target_properties(Dawn::dawn_native PROPERTIES
    IMPORTED_LOCATION "${Dawn_native_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${Dawn_INCLUDE_DIR};${Dawn_GEN_INCLUDE_DIR}")
endif()

if (${Dawn_FOUND})
  message(STATUS "Amber: Using Dawn headers at ${Dawn_INCLUDE_DIR}")
  message(STATUS "Amber: Using Dawn generated headers at ${Dawn_GEN_INCLUDE_DIR}")
  message(STATUS "Amber: Using Dawn library ${Dawn_LIBRARY}")
  message(STATUS "Amber: Using Dawn native library ${Dawn_native_LIBRARY}")
else()
  message(STATUS "Amber: Did not find Dawn")
endif()
