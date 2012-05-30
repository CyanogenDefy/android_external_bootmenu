ifeq ($(BOARD_USES_BOOTMENU),true)

################################

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

bootmenu_local_path := $(LOCAL_PATH)

bootmenu_sources := \
    extendedcommands.c \
    overclock.c \
    bootmenu.c \
    checkup.c \
    default_bootmenu_ui.c \
    ui.c \

BOOTMENU_VERSION:=1.3.0

# Variables available in BoardConfig.mk related to mount devices

ifeq ($(BOARD_WITH_CPCAP),true)
    bootmenu_sources += battery/batt_cpcap.c
    EXTRA_CFLAGS += -DBOARD_WITH_CPCAP
endif

ifeq ($(TARGET_CPU_SMP),true)
    EXTRA_CFLAGS += -DUSE_DUALCORE_DIRTY_HACK
endif
ifneq ($(BOARD_DATA_DEVICE),)
    EXTRA_CFLAGS += -DDATA_DEVICE="\"$(BOARD_DATA_DEVICE)\""
endif
ifneq ($(BOARD_SYSTEM_DEVICE),)
    EXTRA_CFLAGS += -DSYSTEM_DEVICE="\"$(BOARD_SYSTEM_DEVICE)\""
endif
ifneq ($(BOARD_MMC_DEVICE),)
    EXTRA_CFLAGS += -DBOARD_MMC_DEVICE="\"$(BOARD_MMC_DEVICE)\""
endif
ifneq ($(BOARD_SDCARD_DEVICE_SECONDARY),)
    EXTRA_CFLAGS += -DSDCARD_DEVICE="\"$(BOARD_SDCARD_DEVICE_SECONDARY)\""
endif
ifneq ($(BOARD_SDEXT_DEVICE),)
    EXTRA_CFLAGS += -DSDEXT_DEVICE="\"$(BOARD_SDEXT_DEVICE)\""
endif

# ics var first, used in vold too, else previous one
ifneq ($(TARGET_USE_CUSTOM_LUN_FILE_PATH),)
    EXTRA_CFLAGS += -DBOARD_UMS_LUNFILE="\"$(TARGET_USE_CUSTOM_LUN_FILE_PATH)\""
else
 ifneq ($(BOARD_MASS_STORAGE_FILE_PATH),)
    EXTRA_CFLAGS += -DBOARD_UMS_LUNFILE="\"$(BOARD_MASS_STORAGE_FILE_PATH)\""
 endif
endif

# one-shot reboot mode file location
ifneq ($(BOARD_BOOTMODE_CONFIG_FILE),)
    EXTRA_CFLAGS += -DBOOTMODE_CONFIG_FILE="\"$(BOARD_BOOTMODE_CONFIG_FILE)\""
endif

# Special flag for unlocked devices (do not override libreboot for recovery)
ifeq ($(TARGET_BOOTLOADER_BOARD_NAME),olympus)
    EXTRA_CFLAGS += -DUNLOCKED_DEVICE -DNO_OVERCLOCK
endif

ifneq ($(BOARD_DEFY_MODEL),DEFY_FROYO)
    EXTRA_CFLAGS += -DUSE_4_CLOCK_LEVELS
endif

######################################
# Cyanogen version

ifneq ($(BUILD_BOOTMENU_STANDALONE),1)

LOCAL_MODULE := bootmenu
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(bootmenu_sources)

BOOTMENU_SUFFIX :=

LOCAL_CFLAGS += \
    -DBOOTMENU_VERSION="\"${BOOTMENU_VERSION}${BOOTMENU_SUFFIX}\"" -DSTOCK_VERSION=0 \
    -DMAX_ROWS=44 -DMAX_COLS=96 ${EXTRA_CFLAGS}

LOCAL_STATIC_LIBRARIES := libminui_bm libpixelflinger_static libpng libz
LOCAL_STATIC_LIBRARIES += libstdc++ libc libcutils 

LOCAL_FORCE_STATIC_EXECUTABLE := true

LOCAL_MODULE_PATH := $(PRODUCT_OUT)/system/bin

include $(BUILD_EXECUTABLE)

endif # !BUILD_BOOTMENU_STANDALONE

#####################################
# Standalone version for stock roms

ifeq ($(BUILD_BOOTMENU_STANDALONE),1)

LOCAL_PATH := $(bootmenu_local_path)

include $(CLEAR_VARS)

LOCAL_MODULE := Bootmenu
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(bootmenu_sources)

BOOTMENU_SUFFIX := -$(TARGET_BOOTLOADER_BOARD_NAME)

LOCAL_CFLAGS := \
    -DBOOTMENU_VERSION="\"${BOOTMENU_VERSION}${BOOTMENU_SUFFIX}\"" -DSTOCK_VERSION=1 \
    -DMAX_ROWS=44 -DMAX_COLS=96 ${EXTRA_CFLAGS}

LOCAL_STATIC_LIBRARIES := libminui_bm libpixelflinger_static libpng libz
LOCAL_STATIC_LIBRARIES += libstdc++ libc libcutils

LOCAL_FORCE_STATIC_EXECUTABLE := true

LOCAL_MODULE_PATH := $(PRODUCT_OUT)/system/bootmenu/binary
LOCAL_MODULE_STEM := bootmenu-standalone

include $(BUILD_EXECUTABLE)

endif #BUILD_BOOTMENU_STANDALONE

#####################################
# Include minui

include $(call all-makefiles-under,$(bootmenu_local_path))

#####################################

endif #BOARD_USES_BOOTMENU

