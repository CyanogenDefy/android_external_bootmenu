# Copyright (C) 2009 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
LOCAL_PATH := $(my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := 2nd-init
LOCAL_SRC_FILES := 2nd-init.c


LOCAL_C_INCLUDES := \
  $(KERNEL_HEADERS) \
  $(LOCAL_PATH)/linux \
  $(LOCAL_PATH)/android/arch/$(TARGET_ARCH)

LOCAL_FORCE_STATIC_EXECUTABLE := true

LOCAL_MODULE_TAGS := eng

LOCAL_STATIC_LIBRARIES :=
LOCAL_STATIC_LIBRARIES += libstdc++ libc libm


include $(BUILD_EXECUTABLE)


#include $(BUILD_SHARED_LIBRARY)
