ifneq ($(TARGET_SIMULATOR),true)
 ifeq ($(TARGET_ARCH),arm)

 ifeq ($(BOARD_USES_BOOTMENU),true)


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

BOOTMENU_VERSION := 0.9.9-defy
LOCAL_CFLAGS += -DBOOTMENU_VERSION="${BOOTMENU_VERSION}" -DENABLE_MENU_SYSTEM=0

LOCAL_STATIC_LIBRARIES :=
LOCAL_STATIC_LIBRARIES += libminui_bm libpixelflinger_static libpng libz
LOCAL_STATIC_LIBRARIES += libstdc++ libc libcutils 

ifeq ($(TARGET_PRODUCT),cyanogen_jordan)
    LOCAL_MODULE_PATH := $(PRODUCT_OUT)/system/bin
endif
include $(BUILD_EXECUTABLE)


include $(call all-makefiles-under,$(LOCAL_PATH))


 endif # BOARD_USES_BOOTMENU

 endif # TARGET_ARCH arm
endif  # !TARGET_SIMULATOR

