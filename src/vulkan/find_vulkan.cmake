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

# Include this file to find Vulkan and and set up compilation and linking.

# Export these settings to the includer.
set(Vulkan_FOUND FALSE)
set(VULKAN_CTS_HEADER FALSE)
set(VULKAN_LIB "")

if (NOT ${Vulkan_FOUND})
  if (${AMBER_USE_LOCAL_VULKAN})
    set(Vulkan_FOUND TRUE)
    set(VulkanHeaders_INCLUDE_DIR
      ${PROJECT_SOURCE_DIR}/third_party/vulkan-headers/include
      CACHE PATH "vk headers dir" FORCE)
    set(VulkanHeaders_INCLUDE_DIRS ${VulkanHeaders_INCLUDE_DIR}
      CACHE PATH "vk headers dir" FORCE)
    set(VulkanRegistry_DIR
      ${PROJECT_SOURCE_DIR}/third_party/vulkan-headers/registry
      CACHE PATH "vk_registry_dir" FORCE)
    set(VulkanRegistry_DIRS ${VulkanRegistry_DIR}
      CACHE PATH "vk_registry_dir" FORCE)
    include_directories(BEFORE "${VulkanHeaders_INCLUDE_DIR}")
    set(VULKAN_LIB vulkan)
    message(STATUS "Amber: using local vulkan")
  endif()
endif()

if (NOT ${Vulkan_FOUND})
  # Our first choice is to pick up the Vulkan headers from an enclosing project.
  # And if that's the case, then use Vulkan libraries as specified by
  # Vulkan_LIBRARIES, with a default library of "vulkan".
  set(X "${Vulkan-Headers_SOURCE_DIR}/include")
  if (IS_DIRECTORY "${X}")
    message(STATUS "Amber: Using Vulkan header dir ${X}")
    list(APPEND CMAKE_REQUIRED_INCLUDES "${X}")

    # Add the directory to the list of include paths, before any others.
    include_directories(BEFORE "${X}")
    CHECK_INCLUDE_FILE(vulkan/vulkan.h HAVE_VULKAN_HEADER)

    if (${HAVE_VULKAN_HEADER})
      if ("${Vulkan_LIBRARIES}" STREQUAL "")
        message(STATUS "Amber: Defaulting to Vulkan library: vulkan")
        set(VULKAN_LIB vulkan)
      else()
        message(STATUS "Amber: Using specified Vulkan libraries: ${Vulkan_LIBRARIES}")
        set(VULKAN_LIB "${Vulkan_LIBRARIES}")
      endif()

      # For now assume we have Vulkan.  We have its header, but we haven't checked
      # for the library.
      # TODO(dneto): Actually check for the libraries.
      set(Vulkan_FOUND TRUE)
    endif()
  endif()
  unset(X)
endif()

# Check if we're in the CTS
if (NOT ${Vulkan_FOUND})
  message(STATUS "Amber: Checking for CTS Vulkan header")
  set(X "${Vulkan-Headers_SOURCE_DIR}")
  if (IS_DIRECTORY "${X}")
    message(STATUS "Amber: Using Vulkan header dir ${X}")
    list(APPEND CMAKE_REQUIRED_INCLUDES "${X}")

    # Add the directory to the list of include paths, before any others.
    include_directories(BEFORE "${X}")

    if (EXISTS "${X}/vkDefs.h")
      set(VULKAN_CTS_HEADER TRUE)
      set(Vulkan_FOUND TRUE)
    endif()
  endif()
  unset(X)
endif()

if (NOT ${Vulkan_FOUND})
  # If we aren't already building a Vulkan library, then use CMake to find it.
  if(NOT ${CMAKE_VERSION} VERSION_LESS "3.7")
    # LunarG added FindVulkan support to CMake 3.7.  If you have the Vulkan SDK
    # published by LunarG, then set environment variables:
    #  VULKAN_SDK should point to the platform-specific SDK directory containing
    #    the include and lib directories.
    #  VK_ICD_FILENAMES should point to ICD JSON file.

    # Example, with the LunarG SDK macOS edition with MoltenVK:
    #  export VULKAN_SDK="$HOME/vulkan-macos-1.1.85.0/macOS"
    #  export VK_ICD_FILENAMES="$VULKAN_SDK/etc/vulkan/icd/MoltenVK_icd.json"
    # See https://cmake.org/cmake/help/v3.7/module/FindVulkan.html
    find_package(Vulkan)
    if(${Vulkan_FOUND})
      message(STATUS "Amber: Using Vulkan from Vulkan SDK at $ENV{VULKAN_SDK}")
      # Use the imported library target set up by find_package.
      set(VULKAN_LIB Vulkan::Vulkan)
      # Add the Vulkan include directory to the list of include paths.
      include_directories("${Vulkan_INCLUDE_DIRS}")
    endif()
  endif()
endif()

if (NOT ${Vulkan_FOUND})
  message(STATUS "Amber: Did not find Vulkan")
endif()
