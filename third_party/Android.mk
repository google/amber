# Copyright 2020 The Amber Authors.
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

THIRD_PARTY_PATH := $(call my-dir)

LOCAL_PATH := $(call my-dir)

# Lodepng
include $(CLEAR_VARS)
LOCAL_MODULE:=lodepng
LOCAL_CXXFLAGS:=-std=c++11 -fno-exceptions -fno-rtti
LOCAL_SRC_FILES:= lodepng/lodepng.cpp
include $(BUILD_STATIC_LIBRARY)

ifeq ($(GLSLANG_LOCAL_PATH),)
	GLSLANG_LOCAL_PATH:=$(THIRD_PARTY_PATH)/glslang
endif
ifeq ($(SPVTOOLS_LOCAL_PATH),)
  SPVTOOLS_LOCAL_PATH:=$(THIRD_PARTY_PATH)/spirv-tools
endif
ifeq ($(SPVHEADERS_LOCAL_PATH),)
	SPVHEADERS_LOCAL_PATH:=$(THIRD_PARTY_PATH)/spirv-headers
endif

# The Shaderc Android.mk file will include the glslang and the spirv-tools
# build files.
ifeq ($(SHADERC_LOCAL_PATH),)
	SHADERC_LOCAL_PATH:=$(THIRD_PARTY_PATH)/shaderc
endif
include $(SHADERC_LOCAL_PATH)/Android.mk
