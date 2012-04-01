ifeq ($(BOARD_USES_BOOTMENU),true)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := events.c resources.c

ifneq ($(BOARD_CUSTOM_BOOTMENU_GRAPHICS),)
  LOCAL_SRC_FILES += $(BOARD_CUSTOM_BOOTMENU_GRAPHICS)
else
  LOCAL_SRC_FILES += graphics.c
endif

LOCAL_C_INCLUDES +=\
    external/libpng\
    external/zlib

LOCAL_MODULE := libminui_bm
LOCAL_MODULE_TAGS := eng debug

# Defy use this :
ifeq ($(TARGET_RECOVERY_PIXEL_FORMAT),"BGRA_8888")
    LOCAL_CFLAGS += -DPIXELS_BGRA
endif

ifeq ($(TARGET_RECOVERY_PIXEL_FORMAT),"RGBA_8888")
    LOCAL_CFLAGS += -DPIXELS_RGBA
endif

ifeq ($(TARGET_RECOVERY_PIXEL_FORMAT),"RGBX_8888")
    LOCAL_CFLAGS += -DPIXELS_RGBX
endif

ifeq ($(TARGET_RECOVERY_PIXEL_FORMAT),"RGB_565")
    LOCAL_CFLAGS += -DPIXELS_RGB565
endif

# Reversed 16bits RGB (ics software gralloc)
#LOCAL_CFLAGS += -DPIXELS_BGR_16BPP

include $(BUILD_STATIC_LIBRARY)

endif #BOARD_USES_BOOTMENU
