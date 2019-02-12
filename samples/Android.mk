LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE:=amber_ndk
LOCAL_CPP_EXTENSION := .cc .cpp .cxx
LOCAL_SRC_FILES:= \
    amber.cc \
    config_helper.cc \
    config_helper_vulkan.cc \
    log.cc \
    ppm.cc
LOCAL_C_INCLUDES := $(LOCAL_PATH)/.. $(LOCAL_PATH)/../include
LOCAL_LDLIBS:=-landroid -lvulkan -llog
LOCAL_CXXFLAGS:=-std=c++11 -fno-exceptions -fno-rtti -Werror -DAMBER_ENGINE_VULKAN
LOCAL_STATIC_LIBRARIES:=amber
include $(BUILD_EXECUTABLE)

include $(LOCAL_PATH)/../Android.mk
