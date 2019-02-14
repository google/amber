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
LOCAL_CPP_EXTENSION := .cc .cpp .cxx
LOCAL_SRC_FILES:=test.cc
LOCAL_MODULE:=amber_ndk_test
LOCAL_LDLIBS:=-landroid
LOCAL_CXXFLAGS:=-std=c++11 -fno-exceptions -fno-rtti -Werror
LOCAL_STATIC_LIBRARIES:=amber
include $(BUILD_SHARED_LIBRARY)

include $(LOCAL_PATH)/../Android.mk
