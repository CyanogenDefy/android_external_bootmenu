ifeq ($(TARGET_ARCH),arm)
ifneq ($(TARGET_SIMULATOR),true)

################################

LOCAL_PATH := $(call my-dir)
commands_bootmenu_local_path := $(LOCAL_PATH)

bootmenu_sources := \
    extendedcommands.c \
    overclock.c \
    bootmenu.c \
    default_bootmenu_ui.c \
    ui.c \

include $(CLEAR_VARS)

BOOTMENU_VERSION:=1.0.2

ifeq ($(BOARD_USES_BOOTMENU),true)

LOCAL_MODULE := bootmenu
LOCAL_MODULE_TAGS := eng

LOCAL_SRC_FILES := $(bootmenu_sources)

BOOTMENU_SUFFIX :=

LOCAL_CFLAGS += -DBOOTMENU_VERSION="${BOOTMENU_VERSION}${BOOTMENU_SUFFIX}" -DFULL_VERSION=0

LOCAL_STATIC_LIBRARIES :=
LOCAL_STATIC_LIBRARIES += libminui_bm libpixelflinger_static libpng libz
LOCAL_STATIC_LIBRARIES += libstdc++ libc libcutils 

LOCAL_FORCE_STATIC_EXECUTABLE := true

LOCAL_MODULE_PATH := $(PRODUCT_OUT)/system/bin

include $(BUILD_EXECUTABLE)

include $(call all-makefiles-under,$(commands_bootmenu_local_path))

include $(CLEAR_VARS)

endif # BOARD_USES_BOOTMENU


############################
# Standalone version

LOCAL_PATH := $(commands_bootmenu_local_path)

LOCAL_MODULE := Bootmenu
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(bootmenu_sources)

BOOTMENU_SUFFIX :=-full

LOCAL_CFLAGS := -DBOOTMENU_VERSION="${BOOTMENU_VERSION}${BOOTMENU_SUFFIX}" -DFULL_VERSION=1

LOCAL_STATIC_LIBRARIES :=
LOCAL_STATIC_LIBRARIES += libminui_bm libpixelflinger_static libpng libz
LOCAL_STATIC_LIBRARIES += libstdc++ libc libcutils

LOCAL_FORCE_STATIC_EXECUTABLE := true

LOCAL_MODULE_PATH := $(PRODUCT_OUT)/system/bootmenu/binary/

include $(call all-makefiles-under,$(commands_bootmenu_local_path)/minui)

include $(BUILD_EXECUTABLE)

###########################

endif # !TARGET_SIMULATOR
endif # TARGET_ARCH arm
