LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE:=amber
LOCAL_CXXFLAGS:=-std=c++11 -fno-exceptions -fno-rtti
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
    src/vkscript/format_parser.cc \
    src/vkscript/parser.cc \
    src/vkscript/section_parser.cc
LOCAL_STATIC_LIBRARIES:=glslang SPIRV-Tools shaderc
LOCAL_C_INCLUDES:=$(LOCAL_PATH)/include
LOCAL_EXPORT_C_INCLUDES:=$(LOCAL_PATH)/include
include $(BUILD_STATIC_LIBRARY)

include $(LOCAL_PATH)/third_party/Android.mk
