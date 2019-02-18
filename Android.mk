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

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE:=amber
LOCAL_CXXFLAGS:=-std=c++11 -fno-exceptions -fno-rtti \
    -DAMBER_ENABLE_SPIRV_TOOLS=1 \
    -DAMBER_ENABLE_SHADERC=1 \
    -DAMBER_ENGINE_VULKAN=1
LOCAL_SRC_FILES:= \
    src/amber.cc \
    src/amberscript/parser.cc \
    src/buffer.cc \
    src/command.cc \
    src/command_data.cc \
    src/datum_type.cc \
    src/engine.cc \
    src/executor.cc \
    src/format.cc \
    src/format_parser.cc \
    src/parser.cc \
    src/pipeline.cc \
    src/pipeline_data.cc \
    src/recipe.cc \
    src/result.cc \
    src/script.cc \
    src/shader.cc \
    src/shader_compiler.cc \
    src/tokenizer.cc \
    src/value.cc \
    src/verifier.cc \
    src/vkscript/command_parser.cc \
    src/vkscript/datum_type_parser.cc \
    src/vkscript/parser.cc \
    src/vkscript/section_parser.cc \
    src/vulkan/buffer.cc \
    src/vulkan/buffer_descriptor.cc \
    src/vulkan/command_buffer.cc \
    src/vulkan/command_pool.cc \
    src/vulkan/compute_pipeline.cc \
    src/vulkan/descriptor.cc \
    src/vulkan/device.cc \
    src/vulkan/engine_vulkan.cc \
    src/vulkan/format_data.cc \
    src/vulkan/frame_buffer.cc \
    src/vulkan/graphics_pipeline.cc \
    src/vulkan/image.cc \
    src/vulkan/index_buffer.cc \
    src/vulkan/pipeline.cc \
    src/vulkan/push_constant.cc \
    src/vulkan/resource.cc \
    src/vulkan/vertex_buffer.cc
LOCAL_STATIC_LIBRARIES:=glslang SPIRV-Tools shaderc
LOCAL_C_INCLUDES:=$(LOCAL_PATH)/include
LOCAL_EXPORT_C_INCLUDES:=$(LOCAL_PATH)/include
include $(BUILD_STATIC_LIBRARY)

include $(LOCAL_PATH)/third_party/Android.mk
