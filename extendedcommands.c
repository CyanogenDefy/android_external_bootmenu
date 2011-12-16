/*
 * Copyright (C) 2007-2011 The Android Open Source Project
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
#include "overclock.h"
#include "minui/minui.h"
#include "bootmenu_ui.h"

//#define DEBUG_ALLOC

#define MODES_COUNT 9
const char* modes[] = {
  "bootmenu",
  "2nd-init",
  "2nd-boot",
  "normal",
  "2nd-init-adb",
  "2nd-boot-adb",
  "normal-adb",
  "recovery",
  "shell",
};

//user friendly menu labels
#define LABEL_2NDINIT    "2nd-init"
#define LABEL_2NDBOOT    "2nd-boot"
#define LABEL_NORMAL     "Stock"
#define LABEL_2NDINIT_D  "2nd-init + adb"
#define LABEL_2NDBOOT_D  "2nd-boot + adb"
#define LABEL_NORMAL_D   "Stock + adb"

/**
 * int_mode()
 *
 */
int int_mode(char * mode) {
  int m;
  for (m=0; m < MODES_COUNT; m++) {
    if (0 == strcmp(modes[m], mode)) {
      return m;
    }
  }
  return 0;
}

/**
 * str_mode()
 *
 */
const char* str_mode(int mode) {
  if (mode >= 0 && mode < MODES_COUNT) {
    return modes[mode];
  }
  return "bootmenu";
}

/**
 * show_menu_boot() 
 *
 */
int show_menu_boot(void) {

  #define BOOT_2NDINIT    1
  #define BOOT_2NDBOOT    2
  #define BOOT_NORMAL     3
  #define BOOT_2NDINIT_D  4
  #define BOOT_2NDBOOT_D  5
  #define BOOT_NORMAL_D   6

  #define BOOT_FBTEST     7
  #define BOOT_EVTTEST    8
  #define BOOT_PNGTEST    9
  #define BOOT_TEST      10

  int status, res = 0;
  const char* headers[] = {
        " # Boot -->",
        "",
        NULL
  };
  char** title_headers = prepend_title(headers);

  char* items[12] = {
        "  +Set Default: [" LABEL_2NDINIT_D "] -->",
        "  [" LABEL_2NDINIT "]",
        "  [" LABEL_2NDBOOT "]",
        "  [" LABEL_NORMAL "]",
        "  [" LABEL_2NDINIT_D "]",
        "  [" LABEL_2NDBOOT_D "]",
        "  [" LABEL_NORMAL_D "]",

#ifdef DEBUG_ALLOC
        "  [test fb]",
        "  [test evt]",
        "  [test png]",
        "  [test all]",
#endif
        "  --Go Back.",
        NULL
  };

  char lndef[64];
  int chosen_item, bootmode;

  for (;;) {
    bootmode = get_default_bootmode();

    sprintf(lndef,"  +Set Default: [%s] -->", str_mode(bootmode) );
    items[0] = lndef;

    chosen_item = get_menu_selection(title_headers, items, 1, 0);

    if (chosen_item == GO_BACK) {
        goto exit_loop;
    }

    //Submenu: select default mode
    if (chosen_item == 0) {
        show_config_bootmode();
        continue;
    }

    //Next boot modes
    else if (chosen_item < BOOT_NORMAL) {
        if (next_bootmode_write( str_mode(chosen_item) ) != 0) {
            //write error
            continue;
        }
        sync();
        reboot(RB_AUTOBOOT);
        goto exit_loop;
    }

    //Direct boot modes (with adb, but buggy overlay)
    else if (chosen_item == BOOT_2NDINIT_D) {
        free_menu_headers(title_headers);
        if (usb_connected() && !adb_started()) exec_script(FILE_ADBD, ENABLE);
        status = snd_init(ENABLE);
        return (status == 0);
    }
    else if (chosen_item == BOOT_2NDBOOT_D) {
        free_menu_headers(title_headers);
        if (usb_connected() && !adb_started()) exec_script(FILE_ADBD, ENABLE);
        status = snd_boot(ENABLE);
        return (status == 0);
    }
#if STOCK_VERSION
    else if (chosen_item == BOOT_NORMAL) {
        free_menu_headers(title_headers);
        if (next_bootmode_write( str_mode(chosen_item) ) != 0) {
            //write error
            continue;
        }
        sync();
        reboot(RB_AUTOBOOT);
        goto exit_loop;
    }
    else if (chosen_item == BOOT_NORMAL_D) {
        free_menu_headers(title_headers);
        if (usb_connected() && !adb_started()) exec_script(FILE_ADBD, ENABLE);
        status = stk_boot(ENABLE);
        return (status == 0);
    }
#endif
    else
    switch (chosen_item) {
#ifdef DEBUG_ALLOC
      case BOOT_TEST:
        led_alert("green", 1);
        ui_final();
        ui_init();
        led_alert("green", 0);
        res = 0;
        goto exit_loop;

      case BOOT_FBTEST:
        led_alert("green", 1);
        gr_fb_test();
        led_alert("green", 0);
        res = 0;
        goto exit_loop;

      case BOOT_EVTTEST:
        led_alert("green", 1);
        evt_exit();
        evt_init();
        led_alert("green", 0);
        res = 0;
        goto exit_loop;

      case BOOT_PNGTEST:
        led_alert("green", 1);
        ui_free_bitmaps();
        ui_create_bitmaps();
        led_alert("green", 0);
        res = 0;
        goto exit_loop;
#endif
      default:
        goto exit_loop;
    }

  } //for

exit_loop:

  free_menu_headers(title_headers);

  return res;
}

/**
 * show_config_bootmode()
 *
 */
int show_config_bootmode(void) {

  //last mode enabled for default modes (adb disabled)
  #define LAST_MODE 4

  int res = 0;
  const char* headers[3] = {
          " # Boot --> Set Default -->",
          "",
          NULL
  };
  char** title_headers = prepend_title(headers);

  static char options[8][64];
  char* menu_opts[8];
  int i, mode, chosen_item;

  for (;;) {

    mode = get_default_bootmode();

    for(i = 0; i < LAST_MODE; ++i) {
      sprintf(options[i], "   [%s]", str_mode(i));
      if(mode == i)
        options[i][2] = '*';
      menu_opts[i] = options[i];
    }

    menu_opts[LAST_MODE] = "   --Go Back.";
    menu_opts[LAST_MODE+1] = NULL;

    chosen_item = get_menu_selection(title_headers, menu_opts, 1, mode);
    if (chosen_item >= LAST_MODE || strlen(menu_opts[chosen_item]) == 0) {
      //back
      res=1;
      break;
    }
#if !STOCK_VERSION
    if (chosen_item == BOOT_NORMAL || chosen_item == BOOT_NORMAL_D) {
      //back, disable stock boot in CyanogenMod
      res=1;
      break;
    }
#endif
    if (set_default_bootmode(chosen_item) == 0) {
      ui_print("Done..\n");
      continue;
    }
    else {
      ui_print("Failed to setup default boot mode.\n");
      break;
    }
  }

  free_menu_headers(title_headers);
  return res;
}


#if STOCK_VERSION

/**
 * show_menu_system()
 *
 */
int show_menu_system(void) {

  #define SYSTEM_OVERCLOCK  0
  #define SYSTEM_ROOT       1
  #define SYSTEM_UNINSTALL  2

  int status;
  int select = 0;
  struct stat buf;

  const char* headers[] = {
        " # System -->",
        "",
        NULL
  };
  char** title_headers = prepend_title(headers);

  char* items[] =  {
        "  +Overclock -->",
        "  [UnRooting]",
        "  [Uninstall BootMenu]",
        "  --Go Back.",
        NULL
  };

  for (;;) {

    if ((stat("/system/app/Superuser.apk", &buf) < 0) && (stat("/system/app/superuser.apk", &buf) < 0))
      items[1] = "  [Rooting]";
    else
      items[1] = "  [UnRooting]";

    int chosen_item = get_menu_selection(title_headers, items, 1, select);

    switch (chosen_item) {

      case SYSTEM_OVERCLOCK:
        status = show_menu_overclock();
        break;
      case SYSTEM_ROOT:
        ui_print("[Un]Rooting....");
        status = exec_script(FILE_ROOT, ENABLE);
        ui_print("Done..\n");
        break;
      case SYSTEM_UNINSTALL:
        ui_print("Uninstall BootMenu....");
        status = exec_script(FILE_UNINSTALL, ENABLE);
        ui_print("Done..\n");
        ui_print("******** Plz reboot now.. ********\n");
        break;
      default:
        return 0;
    }
    select = chosen_item;
  }

  free_menu_headers(title_headers);
  return 0;
}
#endif //#if STOCK_VERSION


/**
 * show_menu_tools()
 *
 */
int show_menu_tools(void) {

#define TOOL_ADB     0
#define TOOL_USB     2
#define TOOL_CDROM   3
#define TOOL_SYSTEM  4
#define TOOL_DATA    5
#define TOOL_NATIVE  6

#define TOOL_UMOUNT  8

#ifndef BOARD_MMC_DEVICE
#define BOARD_MMC_DEVICE "/dev/block/mmcblk1"
#endif

  int status;

  const char* headers[] = {
        " Don't forget to stop the share after use !",
        "",
        " # Tools -->",
        "",
        NULL
  };
  char** title_headers = prepend_title(headers);

  char* items[] =  {
        "  [ADB Daemon]",
        "",
        "  [Share SD Card]",
        "  [Share Drivers]",
        "  [Share system]",
        "  [Share data]",
        "  [Share MMC - Dangerous!]",
        "",
        "  [Stop USB Share]",
        "  --Go Back.",
        NULL
  };

  int chosen_item = get_menu_selection(title_headers, items, 1, 0);

  switch (chosen_item) {
    case TOOL_ADB:
      ui_print("ADB Deamon....");
      status = exec_script(FILE_ADBD, ENABLE);
      ui_print("Done..\n");
      break;

    case TOOL_UMOUNT:
      ui_print("Stopping USB share...");
      sync();
      mount_usb_storage("");
      status = set_usb_device_mode("acm");
      ui_print("Done..\n");
      break;

    case TOOL_USB:
      ui_print("USB Mass Storage....");
      status = exec_script(FILE_SDCARD, ENABLE);
      ui_print("Done..\n");
      break;

    case TOOL_CDROM:
      ui_print("USB Drivers....");
      status = exec_script(FILE_CDROM, ENABLE);
      ui_print("Done..\n");
      break;

    case TOOL_SYSTEM:
      ui_print("Sharing System Partition....");
      status = exec_script(FILE_SYSTEM, ENABLE);
      ui_print("Done..\n");
      break;

    case TOOL_DATA:
      ui_print("Sharing Data Partition....");
      status = exec_script(FILE_DATA, ENABLE);
      ui_print("Done..\n");
      break;

    case TOOL_NATIVE:
      ui_print("Set USB device mode...");
      sync();
      mount_usb_storage(BOARD_MMC_DEVICE);
      usleep(500*1000);
      status = set_usb_device_mode("msc_adb");
      usleep(500*1000);
      mount_usb_storage(BOARD_MMC_DEVICE);
      ui_print("Done..\n");
      break;

    default:
      break;
  }

  free_menu_headers(title_headers);
  return 0;
}

/**
 * show_menu_recovery()
 *
 */
int show_menu_recovery(void) {

#if STOCK_VERSION
  #define RECOVERY_CUSTOM     0
  #define RECOVERY_STOCK      1
#else
  #define RECOVERY_CUSTOM     0
  #define RECOVERY_STABLE     1
  #define RECOVERY_STOCK      2
#endif

  int status, res=0;
  char** args;
  FILE* f;

  const char* headers[] = {
        " # Recovery -->",
        "",
        NULL
  };
  char** title_headers = prepend_title(headers);

  char* items[] =  {
        "  [Custom Recovery]",
#if STOCK_VERSION
        "  [Stock Recovery]",
#else
        "  [Stable Recovery]",
        "  [Stock Recovery]",
#endif
        "  --Go Back.",
        NULL
  };

  int chosen_item = get_menu_selection(title_headers, items, 1, 0);

  switch (chosen_item) {
    case RECOVERY_CUSTOM:
      ui_print("Starting Recovery..\n");
      ui_print("This can take a couple of seconds.\n");
      status = exec_script(FILE_CUSTOMRECOVERY, ENABLE);
      if (!status) res = 1;
      break;

#if !STOCK_VERSION
    case RECOVERY_STABLE:
      ui_print("Starting Recovery..\n");
      ui_print("This can take a couple of seconds.\n");
      status = exec_script(FILE_STABLERECOVERY, ENABLE);
      if (!status) res = 1;
      break;
#endif

    case RECOVERY_STOCK:
      ui_print("Rebooting to Stock Recovery..\n");

      sync();
      __reboot(LINUX_REBOOT_MAGIC1, LINUX_REBOOT_MAGIC2, LINUX_REBOOT_CMD_RESTART2, "recovery");

    default:
      break;
  }

  free_menu_headers(title_headers);
  return res;
}

/**
 * snd_init()
 *
 */
int snd_init(int ui) {
  int status;
  int i;

  bypass_sign("yes");

  if (ui)
    ui_print("Start " LABEL_2NDINIT " boot....\n");
  else
    LOGI("Start " LABEL_2NDINIT " boot....\n");

  status = exec_script(FILE_2NDINIT, ui);
  if (status) {
    return -1;
    bypass_sign("no");
  }

  if (ui)
    ui_print("Wait 2 seconds....\n");
  else 
    LOGI("Wait 2 seconds....\n");

  for(i = 2; i > 0; --i) {
    if (ui)
      ui_print("%d.\n", i);
    else
      LOGI("%d..\n", i);
    usleep(1000000);
  }

  bypass_sign("no");
  return 0;
}

/**
 * snd_boot()
 *
 */
int snd_boot(int ui) {
  int status;
  int i;

  bypass_sign("yes");

  if (ui)
    ui_print("Start " LABEL_2NDBOOT " boot....\n");
  else
    LOGI("Start " LABEL_2NDBOOT " boot....\n");

  status = exec_script(FILE_2NDBOOT, ui);
  if (status) {
    bypass_sign("no");
    return -1;
  }

  if (ui)
    ui_print("Wait 2 seconds....\n");
  else
    LOGI("Wait 2 seconds....\n");

  for(i = 2; i > 0; --i) {
    if (ui)
      ui_print("%d.\n", i);
    else
      LOGI("%d..\n", i);
    usleep(1000000);
  }

  bypass_sign("no");
  return 0;
}

#if STOCK_VERSION
/**
 * stk_boot()
 *
 */
int stk_boot(int ui) {
  int status;
  int i;

  bypass_sign("yes");

  if (ui)
    ui_print("Start " LABEL_NORMAL " boot....\n");
  else
    LOGI("Start " LABEL_NORMAL " boot....\n");

  status = exec_script(FILE_STOCK, ui);
  if (status) {
    return -1;
    bypass_sign("no");
  }

  usleep(1000000);

  bypass_sign("no");
  return 0;
}
#endif

// --------------------------------------------------------

/**
 * get_default_bootmode()
 *
 */
int get_default_bootmode() {
  char mode[32];
  int m;
  FILE* f = fopen(FILE_DEFAULTBOOTMODE, "r");
  if (f != NULL) {
      fscanf(f, "%s", mode);
      fclose(f);

      m = int_mode(mode);
      LOGI("default_bootmode=%d\n", m);

      if (m >=0) return m;
      else return int_mode("bootmenu");

  }
  return -1;
}

/**
 * get_bootmode()
 *
 */
int get_bootmode(int clean) {
  char mode[32];
  int m;
  FILE* f = fopen(FILE_BOOTMODE, "r");
  if (f != NULL) {

      // One-shot bootmode, bootmode.conf is deleted after
      fscanf(f, "%s", mode);
      fclose(f);

      if (clean) {
          // unlink(FILE_BOOTMODE); //buggy unlink ?
          exec_script(FILE_BOOTMODE_CLEAN,DISABLE);
      }

      m = int_mode(mode);
      LOGI("bootmode=%d\n", m);
      if (m >= 0) return m;
  }

  return get_default_bootmode();
}

/**
 * set_default_bootmode()
 *
 */
int set_default_bootmode(int mode) {

  char log[64];
  char* str = (char*) str_mode(mode);

  if (mode < MODES_COUNT) {

      ui_print("Set %s...\n", str);
      return bootmode_write(str);
  }

  ui_print("ERROR: bad mode %d\n", mode);
  return 1;
}

/**
 * bootmode_write()
 *
 * write default boot mode in config file
 */
int bootmode_write(const char* str) {
  FILE* f = fopen(FILE_DEFAULTBOOTMODE, "w");

  if (f != NULL) {
    fprintf(f, "%s", str);
    fflush(f);
    fclose(f);
    sync();
    //double check
    if (get_bootmode(0) == int_mode( (char*)str) ) {
      return 0;
    }
  }

  ui_print("ERROR: unable to write mode %s\n", str);
  return 1;
}

/**
 * next_bootmode_write()
 *
 * write next boot mode in config file
 */
int next_bootmode_write(const char* str) {
  FILE* f = fopen(FILE_BOOTMODE, "w");

  if (f != NULL) {
    fprintf(f, "%s", str);
    fflush(f);
    fclose(f);
    sync();
    ui_print("Next boot mode set to %s\n\nRebooting...\n", str);
    return 0;
  }

  return 1;
}

// --------------------------------------------------------

/**
 * led_alert()
 *
 */
int led_alert(const char* color, int value) {
  char led_path[PATH_MAX];
  sprintf(led_path, "/sys/class/leds/%s/brightness", color);
  FILE* f = fopen(led_path, "w");

  if (f != NULL) {
    fprintf(f, "%d", value);
    fclose(f);
    return 0;
  }
  return 1;
}

/**
 * bypass_sign()
 *
 */
int bypass_sign(const char* mode) {
  FILE* f = fopen(FILE_BYPASS, "w");

  if (f != NULL) {
    fprintf(f, "%s",  mode);
    fclose(f);
    return 0;
  }
  return 1;
}

/**
 * bypass_check()
 *
 */
int bypass_check(void) {
   FILE* f = fopen(FILE_BYPASS, "r");
   char bypass[30];

  if (f != NULL) {
    fscanf(f, "%s", bypass);
    fclose(f);
    if (0 == strcmp(bypass, "yes")) {
      return 0;
    }
  }
  return 1;
}

/**
 * exec_and_wait()
 *
 */
int exec_and_wait(char** argp) {
  pid_t pid;
  sig_t intsave, quitsave;
  sigset_t mask, omask;
  int pstat;

  sigemptyset(&mask);
  sigaddset(&mask, SIGCHLD);
  sigprocmask(SIG_BLOCK, &mask, &omask);
  switch (pid = vfork()) {
  case -1:            /* error */
    sigprocmask(SIG_SETMASK, &omask, NULL);
    return(-1);
  case 0:                /* child */
    sigprocmask(SIG_SETMASK, &omask, NULL);
    execve(argp[0], argp, environ);

    fprintf(stdout, "E:Can't run %s (%s)\n", argp[0], strerror(errno));
    _exit(127);
  }

  intsave = (sig_t)  bsd_signal(SIGINT, SIG_IGN);
  quitsave = (sig_t) bsd_signal(SIGQUIT, SIG_IGN);
  pid = waitpid(pid, (int *)&pstat, 0);
  sigprocmask(SIG_SETMASK, &omask, NULL);
  (void)bsd_signal(SIGINT, intsave);
  (void)bsd_signal(SIGQUIT, quitsave);
  return (pid == -1 ? -1 : pstat);
}

/**
 * exec_script()
 *
 */
int exec_script(const char* filename, int ui) {
  int status;
  char** args;

  LOGI("exec %s\n", filename);

  chmod(filename, 0755);

  args = malloc(sizeof(char*) * 2);
  args[0] = (char *) filename;
  args[1] = NULL;

  status = exec_and_wait(args);

  free(args);

  if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
    if (ui) {
      LOGE("Error in %s\n(Result: %s)\n", filename, strerror(errno));
    }
    else {
      LOGI("E:Error in %s\n(Result: %s)\n", filename, strerror(errno));
    }
    return -1;
  }

  return 0;
}

/**
 * real_execute()
 *
 */
int real_execute(int r_argc, char** r_argv) {
  char* hijacked_executable = r_argv[0];
  int result = 0;
  int i;

  char real_executable[PATH_MAX];
  sprintf(real_executable, "%s.bin", hijacked_executable);
  char ** argp = (char **)malloc(sizeof(char *) * (r_argc + 1));
  for (i = 0; i < r_argc; i++) {
      argp[i] = r_argv[i];
  }
  argp[r_argc] = NULL;

  argp[0] = real_executable;

  result = exec_and_wait(argp);

  free(argp);

  if (!WIFEXITED(result) || WEXITSTATUS(result) != 0)
    return -1;
  else
    return 0;
}

/**
 * usb_connected()
 *
 */
int usb_connected() {
  int state;
  FILE* f;

  //usb should be 1 and ac 0
  f = fopen(SYS_USB_CONNECTED, "r");
  if (f != NULL) {
    fscanf(f, "%d", &state);
    fclose(f);
    if (state) {
      f = fopen(SYS_POWER_CONNECTED, "r");
      if (f != NULL) {
        fscanf(f, "%d", &state);
        fclose(f);
        return (state == 0);
      }
    }
  }
  return 0;
}

int adb_started() {
  int res=0;
  FILE* f;

  

  return res;
}

/**
 * bettery_level()
 *
 */
int battery_level() {
  int state = 0;
  FILE* f = fopen(SYS_BATTERY_LEVEL, "r");
  if (f != NULL) {
    fscanf(f, "%d", &state);
    fclose(f);
  }
  return state;
}


/**
 * Native USB ADB Mode Switch
 *
 */
int set_usb_device_mode(const char* mode) {

  #ifndef BOARD_USB_MODESWITCH
  #define BOARD_USB_MODESWITCH  "/dev/usb_device_mode"
  #endif

  FILE* f = fopen(BOARD_USB_MODESWITCH, "w");
  if (f != NULL) {

    fprintf(f, "%s", mode);
    fclose(f);

    LOGI("set usb mode=%s\n", mode);
    return 0;

  } else {
    fprintf(stdout, "E:Can't open " BOARD_USB_MODESWITCH " (%s)\n", strerror(errno));
    return errno;
  }
}

int mount_usb_storage(const char* part) {

  #ifndef BOARD_UMS_LUNFILE
  #define BOARD_UMS_LUNFILE  "/sys/devices/platform/usb_mass_storage/lun0/file"
  #endif

  FILE* f = fopen(BOARD_UMS_LUNFILE, "w");
  if (f != NULL) {

    fprintf(f, "%s", part);
    fclose(f);
    return 0;

  } else {
    ui_print("E:Unable to write to lun file (%s)", strerror(errno));
    return errno;
  }
}

