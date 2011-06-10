ifneq ($(TARGET_SIMULATOR),true)
 ifeq ($(TARGET_ARCH),arm)


LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := bootmenu
LOCAL_MODULE_TAGS := eng

commands_bootmenu_local_path := $(LOCAL_PATH)

LOCAL_SRC_FILES := \
    extendedcommands.c \
    overclock.c \
    bootmenu.c \
    default_bootmenu_ui.c \
    ui.c \

LOCAL_FORCE_STATIC_EXECUTABLE := true

BOOTMENU_VERSION := 0.8.7-open
LOCAL_CFLAGS += -DBOOTMENU_VERSION="${BOOTMENU_VERSION}"

LOCAL_STATIC_LIBRARIES :=
LOCAL_STATIC_LIBRARIES += libminui_bm libpixelflinger_static libpng libz
LOCAL_STATIC_LIBRARIES += libstdc++ libc libcutils 

ifeq ($(TARGET_PRODUCT),cyanogen_jordan)
	LOCAL_MODULE_PATH := $(PRODUCT_OUT)/system/bin
endif
include $(BUILD_EXECUTABLE)



include $(CLEAR_VARS)

include $(commands_bootmenu_local_path)/minui/Android.mk
include $(commands_bootmenu_local_path)/2nd-init/Android.mk


 endif # TARGET_ARCH arm
endif  # !TARGET_SIMULATOR

