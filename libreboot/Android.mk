LOCAL_PATH:= $(call my-dir)

ifeq ($(BOARD_USES_BOOTMENU),true)

# build static library
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	reboot.c

LOCAL_MODULE := libreboot
LOCAL_MODULE_TAGS := eng

ifneq ($(TARGET_REBOOT_REASON_DEFAULT),)
    LOCAL_CFLAGS += -DREBOOT_REASON_DEFAULT='$(TARGET_REBOOT_REASON_DEFAULT)'
endif

# disabled
# include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

endif
