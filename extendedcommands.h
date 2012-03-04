/*
 * Copyright (C) 2007-2012 The Android Open Source Project
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

#ifndef EXTENDED_COMMAND_H
#define EXTENDED_COMMAND_H

#define BM_ROOTDIR "/system/bootmenu"

#ifndef BOOTMODE_CONFIG_FILE
#define BOOTMODE_CONFIG_FILE "/cache/recovery/bootmode.conf"
#endif

// normal boot (without 2nd-init)
#if STOCK_VERSION || defined(UNLOCKED_DEVICE)
#define ALLOW_BOOT_NORMAL
#endif

// one or 2 recovery binaries
#if !STOCK_VERSION
#define USE_STABLE_RECOVERY
#endif

static const char *FILE_PRE_MENU  = BM_ROOTDIR "/script/pre_bootmenu.sh";
static const char *FILE_POST_MENU = BM_ROOTDIR "/script/post_bootmenu.sh";

static const char *FILE_2NDINIT   = BM_ROOTDIR "/script/2nd-init.sh";
static const char *FILE_2NDBOOT   = BM_ROOTDIR "/script/2nd-boot.sh";
static const char *FILE_2NDSYSTEM = BM_ROOTDIR "/script/2nd-system.sh";
static const char *FILE_STOCK     = BM_ROOTDIR "/script/stock.sh";
static const char *FILE_ADBD      = BM_ROOTDIR "/script/adbd.sh";
static const char *FILE_SDCARD    = BM_ROOTDIR "/script/sdcard.sh";
static const char *FILE_CDROM     = BM_ROOTDIR "/script/cdrom.sh";
static const char *FILE_SYSTEM    = BM_ROOTDIR "/script/system.sh";
static const char *FILE_DATA      = BM_ROOTDIR "/script/data.sh";

static const char *FILE_OVERCLOCK       = BM_ROOTDIR "/script/overclock.sh";
static const char *FILE_OVERCLOCK_CONF  = BM_ROOTDIR "/config/overclock.conf";

static const char *FILE_CUSTOMRECOVERY  = BM_ROOTDIR "/script/recovery.sh";
static const char *FILE_STABLERECOVERY  = BM_ROOTDIR "/script/recovery_stable.sh";
static const char *FILE_BOOTMODE_CLEAN  = BM_ROOTDIR "/script/bootmode_clean.sh";

static const char *FILE_DEFAULTBOOTMODE = BM_ROOTDIR "/config/default_bootmode.conf";
static const char *FILE_BOOTMODE        = BOOTMODE_CONFIG_FILE;
static const char *FILE_BYPASS          = "/data/.bootmenu_bypass";

static const char *SYS_POWER_CONNECTED  = "/sys/class/power_supply/ac/online";
static const char *SYS_USB_CONNECTED    = "/sys/class/power_supply/usb/online";
static const char *SYS_BATTERY_LEVEL    = "/sys/class/power_supply/battery/charge_counter"; // content: 0 to 100

#if STOCK_VERSION

static const char *FILE_ROOT      = BM_ROOTDIR "/script/unroot.sh";
static const char *FILE_UNINSTALL = BM_ROOTDIR "/script/uninstall.sh";

int show_menu_system(void);

#endif //STOCK_VERSION

int int_mode(char* mode);
const char* str_mode(int mode);

int show_menu_boot(void);
int show_menu_overclock(void);
int show_menu_tools(void);
int show_menu_recovery(void);

int usb_connected(void);
int adb_started(void);
int battery_level(void);

int snd_init(int ui);
int snd_boot(int ui);
int snd_system(int ui);
int stk_boot(int ui);

int show_config_bootmode(void);

int get_default_bootmode(void);
int set_default_bootmode(int mode);
int get_bootmode(int clean, int log);

int bootmode_write(const char* str);
int next_bootmode_write(const char* str);

int bypass_sign(const char* mode);
int bypass_check(void);

int exec_and_wait(char** argp);
int exec_script(const char* filename, int ui);
int real_execute(int r_argc, char** r_argv);
int file_exists(char * file);

int set_usb_device_mode(const char *mode);
int mount_usb_storage(const char *part);

#endif // EXTENDED_COMMAND_H
