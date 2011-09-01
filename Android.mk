ifeq ($(TARGET_ARCH),arm)
ifneq ($(TARGET_SIMULATOR),true)

################################

LOCAL_PATH := $(call my-dir)
bootmenu_local_path := $(LOCAL_PATH)

bootmenu_sources := \
    extendedcommands.c \
    overclock.c \
    bootmenu.c \
    default_bootmenu_ui.c \
    ui.c \

BOOTMENU_VERSION:=1.0.3

ifeq ($(BOARD_USES_BOOTMENU),true)

include $(CLEAR_VARS)

LOCAL_MODULE := bootmenu
LOCAL_MODULE_TAGS := eng

LOCAL_SRC_FILES := $(bootmenu_sources)

BOOTMENU_SUFFIX :=

LOCAL_CFLAGS += -DBOOTMENU_VERSION="${BOOTMENU_VERSION}${BOOTMENU_SUFFIX}" -DFULL_VERSION=0

# LOCAL_REQUIRED_MODULES += libminui_bm
PRODUCT_PACKAGES += libminui_bm

LOCAL_STATIC_LIBRARIES :=
LOCAL_STATIC_LIBRARIES += libminui_bm libpixelflinger_static libpng libz
LOCAL_STATIC_LIBRARIES += libstdc++ libc libcutils 

LOCAL_FORCE_STATIC_EXECUTABLE := true

LOCAL_MODULE_PATH := $(PRODUCT_OUT)/system/bin

include $(BUILD_EXECUTABLE)

include $(call all-makefiles-under,$(bootmenu_local_path))

endif # BOARD_USES_BOOTMENU


############################
# Standalone version
ifeq ($(BUILD_BOOTMENU_STANDALONE),1)

LOCAL_PATH := $(bootmenu_local_path)

include $(CLEAR_VARS)

LOCAL_MODULE := Bootmenu
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(bootmenu_sources)

BOOTMENU_SUFFIX :=-full

LOCAL_CFLAGS := -DBOOTMENU_VERSION="${BOOTMENU_VERSION}${BOOTMENU_SUFFIX}" -DFULL_VERSION=1

# LOCAL_REQUIRED_MODULES += libminui_bm

LOCAL_STATIC_LIBRARIES :=
LOCAL_STATIC_LIBRARIES += libminui_bm libpixelflinger_static libpng libz
LOCAL_STATIC_LIBRARIES += libstdc++ libc libcutils

LOCAL_FORCE_STATIC_EXECUTABLE := true

LOCAL_MODULE_PATH := $(PRODUCT_OUT)/system/bootmenu/binary
LOCAL_MODULE_STEM := bootmenu-standalone

include $(BUILD_EXECUTABLE)

include $(call all-makefiles-under,$(bootmenu_local_path))

endif
###########################

endif # !TARGET_SIMULATOR
endif # TARGET_ARCH arm
