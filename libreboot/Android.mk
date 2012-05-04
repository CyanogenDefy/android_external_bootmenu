ifeq ($(BOARD_USES_BOOTMENU),true)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := main.c reboot.c
LOCAL_CFLAGS := -DSINGLE_APPLET=1

LOCAL_MODULE := reboot_bm
LOCAL_MODULE_STEM := reboot
LOCAL_MODULE_TAGS := eng

LOCAL_STATIC_LIBRARIES += libstdc++ libc libcutils
LOCAL_FORCE_STATIC_EXECUTABLE := true

LOCAL_MODULE_PATH := $(PRODUCT_OUT)/system/bootmenu/binary

include $(BUILD_EXECUTABLE)

endif # bootmenu
