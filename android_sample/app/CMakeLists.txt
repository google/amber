cmake_minimum_required(VERSION 3.4.1)

# build native_app_glue as a static lib
set(APP_GLUE_DIR ${ANDROID_NDK}/sources/android/native_app_glue)
include_directories(${APP_GLUE_DIR})
add_library( app-glue STATIC ${APP_GLUE_DIR}/android_native_app_glue.c)

# build vulkan app
set(SRC_DIR src/main/jni)

add_library(amber_android SHARED
            ${SRC_DIR}/AndroidMain.cpp)

include_directories(${SRC_DIR})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Werror \
                     -DVK_USE_PLATFORM_ANDROID_KHR")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} \
                               -u ANativeActivity_onCreate")

# Begin: include Amber
set(AMBER_DIR ${CMAKE_SOURCE_DIR}/../..)
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
