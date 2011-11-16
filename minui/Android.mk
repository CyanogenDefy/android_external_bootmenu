ifeq ($(BOARD_USES_BOOTMENU),true)

ifeq ($(TARGET_ARCH),arm)
ifneq ($(TARGET_SIMULATOR),true)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := graphics.c events.c resources.c

LOCAL_C_INCLUDES +=\
    external/libpng\
    external/zlib

LOCAL_MODULE := libminui_bm
LOCAL_MODULE_TAGS := eng

include $(BUILD_STATIC_LIBRARY)

endif
endif

endif #BOARD_USES_BOOTMENU
