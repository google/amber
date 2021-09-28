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

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE:=amber_ndk
LOCAL_CPP_EXTENSION := .cc .cpp .cxx
LOCAL_SRC_FILES:= \
    amber.cc \
    config_helper.cc \
    config_helper_vulkan.cc \
    log.cc \
    ppm.cc \
    png.cc \
    timestamp.cc
LOCAL_C_INCLUDES := $(LOCAL_PATH)/.. $(LOCAL_PATH)/../include
LOCAL_LDLIBS:=-landroid -lvulkan -llog
LOCAL_CXXFLAGS:=-std=c++11 -fno-exceptions -fno-rtti -Werror -Wno-unknown-pragmas -DAMBER_ENGINE_VULKAN=1 -DAMBER_ENABLE_LODEPNG=1
LOCAL_STATIC_LIBRARIES:=amber lodepng
include $(BUILD_EXECUTABLE)

LOCAL_MODULE:=amber_ndk_sharedlib
LOCAL_MODULE_FILENAME:=libamber_ndk
LOCAL_SRC_FILES+=android_helper.cc
LOCAL_CXXFLAGS+=-DAMBER_ANDROID_MAIN=1
include $(BUILD_SHARED_LIBRARY)

include $(LOCAL_PATH)/../Android.mk
