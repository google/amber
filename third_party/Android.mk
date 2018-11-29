THIRD_PARTY_PATH := $(call my-dir)

ifeq ($(GLSLANG_LOCAL_PATH),)
	GLSLANG_LOCAL_PATH:=$(THIRD_PARTY_PATH)/glslang
endif
ifeq ($(SPVTOOLS_LOCAL_PATH),)
  SPVTOOLS_LOCAL_PATH:=$(THIRD_PARTY_PATH)/spirv-tools
endif
ifeq ($(SPVHEADERS_LOCAL_PATH),)
	SPVHEADERS_LOCAL_PATH:=$(THIRD_PARTY_PATH)/spirv-headers
endif

# The shaderc Android.mk file will include the glslang and the spirv-tools
# build files.
ifeq ($(SHADERC_LOCAL_PATH),)
	SHADERC_LOCAL_PATH:=$(THIRD_PARTY_PATH)/shaderc
endif
include $(SHADERC_LOCAL_PATH)/Android.mk
