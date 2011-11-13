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

BOOTMENU_VERSION:=1.0.7

# Variables available in BoardConfig.mk related to mount devices
EXTRA_CFLAGS :=

ifdef BOARD_DATA_DEVICE
    EXTRA_CFLAGS += -DDATA_DEVICE=\"$(BOARD_DATA_DEVICE)\"
endif
ifdef BOARD_SYSTEM_DEVICE
    EXTRA_CFLAGS += -DSYSTEM_DEVICE=\"$(BOARD_SYSTEM_DEVICE)\"
endif
ifdef BOARD_SDCARD_DEVICE_SECONDARY
    EXTRA_CFLAGS += -DSDCARD_DEVICE=\"$(BOARD_SDCARD_DEVICE_SECONDARY)\"
endif
ifdef BOARD_SDEXT_DEVICE
    EXTRA_CFLAGS += -DSDEXT_DEVICE=\"$(BOARD_SDEXT_DEVICE)\"
endif

################################

ifeq ($(BOARD_USES_BOOTMENU),true)
ifneq ($(BUILD_BOOTMENU_STANDALONE),1)

include $(CLEAR_VARS)

LOCAL_MODULE := bootmenu
LOCAL_MODULE_TAGS := eng

LOCAL_SRC_FILES := $(bootmenu_sources)

BOOTMENU_SUFFIX :=

LOCAL_CFLAGS += \
    -DBOOTMENU_VERSION="${BOOTMENU_VERSION}${BOOTMENU_SUFFIX}" -DSTOCK_VERSION=0 \
    -DMAX_ROWS=40 -DMAX_COLS=96 ${EXTRA_CFLAGS}

PRODUCT_PACKAGES += libminui_bm

LOCAL_STATIC_LIBRARIES :=
LOCAL_STATIC_LIBRARIES += libminui_bm libpixelflinger_static libpng libz
LOCAL_STATIC_LIBRARIES += libstdc++ libc libcutils 

LOCAL_FORCE_STATIC_EXECUTABLE := true

LOCAL_MODULE_PATH := $(PRODUCT_OUT)/system/bin

include $(BUILD_EXECUTABLE)

include $(call all-makefiles-under,$(bootmenu_local_path))

endif
endif # BOARD_USES_BOOTMENU


############################
# Standalone version
ifeq ($(BUILD_BOOTMENU_STANDALONE),1)

LOCAL_PATH := $(bootmenu_local_path)

include $(CLEAR_VARS)

LOCAL_MODULE := Bootmenu
LOCAL_MODULE_TAGS := eng

LOCAL_SRC_FILES := $(bootmenu_sources)

BOOTMENU_SUFFIX :=-Defy

LOCAL_CFLAGS := \
    -DBOOTMENU_VERSION="${BOOTMENU_VERSION}${BOOTMENU_SUFFIX}" -DSTOCK_VERSION=1 \
    -DMAX_ROWS=40 -DMAX_COLS=96 ${EXTRA_CFLAGS}

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
