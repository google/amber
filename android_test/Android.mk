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
