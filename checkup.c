/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/reboot.h>
#include <unistd.h>

#include "common.h"
#include "extendedcommands.h"
#include "minui/minui.h"
#include "bootmenu_ui.h"

static int checkup_error=0;

void error_detected(char * msg) {
    led_alert("red", 1);
    checkup_error = 1;

    ui_print("Error: %s\n", msg);
}

/**
 * checkup_report()
 *
 */
int checkup_report(void) {
    struct stat st;

    checkup_error=0;

    memset(&st,0,sizeof(st));
    if (stat("/sbin/busybox", &st) < 0) {
        error_detected("busybox was not created in rootfs !");
    }

    memset(&st,0,sizeof(st));
    if (stat("/system/bin/sh", &st) < 0) {
        error_detected("/system/bin/sh not found !");
    }

    memset(&st,0,sizeof(st));
    if (stat("/system/bin/cp", &st) < 0 && stat("/system/xbin/cp", &st) < 0) {
        error_detected("cp not found !");
    }

    memset(&st,0,sizeof(st));
    if (stat("/sbin/adbd", &st) < 0) {
        error_detected("adbd not found !");
    }

    if (checkup_error) {
        sleep(1);
        led_alert("red", 0);
    }
    return 0;
}

