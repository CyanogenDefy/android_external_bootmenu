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

#define ITEM_2NDINIT    "2nd-init"
#define ITEM_2NDBOOT    "2nd-boot"
#define ITEM_NORMAL     "Stock"
#define ITEM_2NDADB     "2nd-init + adb"

int
show_menu_boot(void) {

  #define BOOT_DEFAULT    0
  #define BOOT_2NDINIT    1
  #define BOOT_2NDBOOT    2
  #define BOOT_NORMAL     3
  #define BOOT_ADBINIT    4

  #define BOOT_TEST     5
  #define BOOT_FBTEST   6
  #define BOOT_EVTTEST  7
  #define BOOT_PNGTEST  8

  int status, res = 0;
  const char* headers[] = {
        " # Boot -->",
        "",
        NULL
  };
  char** title_headers = prepend_title(headers);

  char* items[11] =  {
        "  +Set Default: [" ITEM_2NDINIT "] -->",
        "  [" ITEM_2NDINIT "]",
        "  [" ITEM_2NDBOOT "]",
        "  [" ITEM_NORMAL "]",
        "  [" ITEM_2NDADB "]",
#ifdef DEBUG_ALLOC
        "  [test all]",
        "  [test fb]",
        "  [test evt]",
        "  [test png]",
#endif
        "  --Go Back.",
        NULL
  };

  for (;;) {
    int bootmode = get_bootmode();
    switch (bootmode) {
      case MODE_BOOTMENU: items[0] = "  +Set Default: [BootMenu] -->"; break;
      case MODE_2NDINIT:  items[0] = "  +Set Default: [" ITEM_2NDINIT "] -->"; break;
      case MODE_2NDBOOT:  items[0] = "  +Set Default: [" ITEM_2NDBOOT "] -->"; break;
      case MODE_NORMAL:   items[0] = "  +Set Default: [" ITEM_NORMAL "] -->"; break;
    }

    int chosen_item = get_menu_selection(title_headers, items, 1, 0);

    switch (chosen_item) {

      case BOOT_DEFAULT:
        status = show_config_bootmode();
        break;

      case BOOT_2NDINIT:
        next_bootmode_write("2nd-init");
        sync();
        reboot(RB_AUTOBOOT);
        goto exit_loop;

      case BOOT_2NDBOOT:
        //free(title_headers);
        //status = snd_boot(ENABLE);
        next_bootmode_write("2nd-boot");
        sync();
        reboot(RB_AUTOBOOT);
        goto exit_loop;

      case BOOT_NORMAL:
        res = -1;
        next_bootmode_write("normal");
        sync();
        reboot(RB_AUTOBOOT);
        goto exit_loop;

      case BOOT_ADBINIT:
        //2nd-init with adb enabled for
        // direct boot (overlay bug)
        free(title_headers);
        exec_script(FILE_ADBD, ENABLE);
        sync();
        status = snd_init(ENABLE);
        return (status == 0);

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
        res = 0;
        goto exit_loop;
    }
  }

exit_loop:

  free(title_headers);

  return res;
}

#if FULL_VERSION
int
show_menu_system(void) {

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

  free(title_headers);
  return 0;
}
#endif //#if FULL_VERSION

int
show_menu_tools(void) {

#define TOOL_ADB     0
#define TOOL_USB     1
#define TOOL_CDROM   2
#define TOOL_SYSTEM  3
#define TOOL_DATA    4

  int status;

  const char* headers[] = {
        " # Tools -->",
        "",
        NULL
  };
  char** title_headers = prepend_title(headers);

  char* items[] =  {
        "  [ADB Daemon]",
        "  [USB Mass Storage]",
        "  [USB Drivers]",
        "  [USB Mount /system]",
        "  [USB Mount /data]",
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

    case TOOL_USB:
      ui_print("USB Mass Storage....");
      //mount_usb_storage();
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

    default:
      break;
  }

  free(title_headers);
  return 0;
}


int
show_menu_recovery(void) {

  #define RECOVERY_CUSTOM     0
  #define RECOVERY_STABLE     1
  #define RECOVERY_STOCK      2

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
        "  [Latest Recovery]",
        "  [Stable Recovery]",
        "  [Stock Recovery]",
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

    case RECOVERY_STABLE:
      ui_print("Starting Stable Recovery..\n");
      ui_print("This can take a couple of seconds.\n");
      status = exec_script(FILE_STABLERECOVERY, ENABLE);
      if (!status) res = 1;
      break;

    case RECOVERY_STOCK:
      ui_print("Rebooting to Stock Recovery..\n");

      //optional
      f = fopen("/data/.recovery_mode", "w");
      if (f != NULL) {
        fprintf(f, "%s", "1");
        fclose(f);
      }

      sync();
      __reboot(LINUX_REBOOT_MAGIC1, LINUX_REBOOT_MAGIC2, LINUX_REBOOT_CMD_RESTART2, "recovery");
/*
      args = malloc(sizeof(char*) * 3);
      args[0] = (char *) FILE_STOCKRECOVERY;
      args[1] = "recovery";
      args[2] = NULL;

      sync();
      status = exec_and_wait(args);
      if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        LOGE("Error in %s\n(Status %d)\n", FILE_STOCKRECOVERY, WEXITSTATUS(status));
      }
      res = 2;
      break;
*/

    default:
      break;
  }

  free(title_headers);
  return res;
}

/* made in sdcard.sh

#ifndef BOARD_UMS_LUNFILE
#define BOARD_UMS_LUNFILE  "/sys/devices/platform/usb_mass_storage/lun0/file"
#endif

int
mount_usb_storage(void) {

  char usb_storage[] = BOARD_UMS_LUNFILE;
  char part[] = "/dev/block/mmcblk0p1";

  FILE* f = fopen(usb_storage, "w");

  if (f != NULL) {
    fprintf(f, "%s", part);
    fclose(f);
    return 0;
  } else {
    ui_print("Unable to write to usb_mass_storage lun file (%s)", strerror(errno));
  }
  return 1;
}

*/

int
snd_init(int ui) {
  int status;
  int i;

  bypass_sign("yes");

  if (ui)
    ui_print("Start " ITEM_2NDINIT " boot....\n");
  else
    LOGI("Start " ITEM_2NDINIT " boot....\n");

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

int
snd_boot(int ui) {
  int status;
  int i;

  bypass_sign("yes");

  if (ui)
    ui_print("Start " ITEM_2NDBOOT " boot....\n");
  else
    LOGI("Start " ITEM_2NDBOOT " boot....\n");

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

int
show_config_bootmode(void) {

  int res = -1;
  const char* headers[] = {
          " # Boot --> Set Default -->",
          "",
          NULL
  };
  char** title_headers = prepend_title(headers);

  char* items[4][2] = {
        { "   [" ITEM_2NDINIT "]", "  *[" ITEM_2NDINIT "]" },
        { "   [" ITEM_2NDBOOT "]", "  *[" ITEM_2NDBOOT "]" },
        { "   [" ITEM_NORMAL "]", "  *[" ITEM_NORMAL "]" },
        { "   [BootMenu]", "  *[BootMenu]" }
  };

  for (;;) {
    static char* options[6];

    int i;
    int mode = get_bootmode();

    for(i = 0; i < 4; ++i) {
      if(mode == i)
        options[i] = items[i][1];
      else
        options[i] = items[i][0];
    }

    options[4] = "   --Go Back.";
    options[5] = NULL;
    
    int chosen_item = get_menu_selection(title_headers, options, 1, mode);
    if (chosen_item == 4) {
      res=0;
      break;
    }
    if (set_bootmode(chosen_item) == 0) {
      ui_print("Done..\n");
      continue;
    }
    else {
      ui_print("Failed to setup default boot mode.\n");
      break;
    }
  }

  free(title_headers);
  return res;
}

int
set_bootmode(int mode) {
  switch (mode) {
    case MODE_2NDINIT:
      ui_print("Set " ITEM_2NDINIT "....");
      return bootmode_write("2nd-init");
    case MODE_2NDBOOT:
      ui_print("Set " ITEM_2NDBOOT "....");
      return bootmode_write("2nd-boot");
    case MODE_NORMAL:
      ui_print("Set " ITEM_NORMAL "....");
      return bootmode_write("normal");
    case MODE_BOOTMENU:
      ui_print("Set BootMenu....");
      return bootmode_write("bootmenu");
  }
  return 1;
}

int
get_bootmode(void) {
  FILE* f = fopen(FILE_BOOTMODE, "r");
  char mode[30];

  if (f != NULL) {

    // One-shot bootmode, bootmode.conf is deleted after

    fscanf(f, "%s", mode);
    fclose(f);

    //unlink(FILE_BOOTMODE);
    exec_script(FILE_BOOTMODE_CLEAN,DISABLE);

    if (0 == strcmp(mode, "normal"))
      return MODE_NORMAL;
    else if (0 == strcmp(mode, "2nd-init"))
      return MODE_2NDINIT;
    else if (0 == strcmp(mode, "2nd-boot"))
      return MODE_2NDBOOT;
    else if (0 == strcmp(mode, "bootmenu"))
      return MODE_BOOTMENU;
    else if (0 == strcmp(mode, "recovery"))
      return MODE_RECOVERY;
  }

  f = fopen(FILE_DEFAULTBOOTMODE, "r");

  if (f != NULL) {
    fscanf(f, "%s", mode);
    fclose(f);

    if (0 == strcmp(mode, ITEM_NORMAL))
      return MODE_NORMAL;
    else if (0 == strcmp(mode, ITEM_2NDINIT))
      return MODE_2NDINIT;
    else if (0 == strcmp(mode, ITEM_2NDBOOT))
      return MODE_2NDBOOT;
    else if (0 == strcmp(mode, ITEM_2NDADB))
      return MODE_2NDADB;
    else
      return MODE_BOOTMENU;
  }
  return -1;
}

int
bootmode_write(const char* str) {
  FILE* f = fopen(FILE_DEFAULTBOOTMODE, "w");

  if (f != NULL) {
    fprintf(f, "%s", str);
    fclose(f);
    return 0;
  }
  return 1;
}

int
next_bootmode_write(const char* str) {
  FILE* f = fopen(FILE_BOOTMODE, "w");

  if (f != NULL) {
    fprintf(f, "%s", str);
    fclose(f);
    return 0;
  }
  ui_print("Next boot mode set to %s\n\nRebooting...\n", str);

  return 1;
}

int
led_alert(const char* color, int value) {
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

int
bypass_sign(const char* mode) {
  FILE* f = fopen(FILE_BYPASS, "w");

  if (f != NULL) {
    fprintf(f, "%s",  mode);
    fclose(f);
    return 0;
  }
  return 1;
}

int
bypass_check(void) {
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

int
exec_and_wait(char** argp) {
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

int
exec_script(const char* filename, int ui) {
  int status;
  char** args;

  chmod(filename, 0755);

  args = malloc(sizeof(char*) * 2);
  args[0] = (char *) filename;
  args[1] = NULL;

  status = exec_and_wait(args);

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

int
real_execute(int r_argc, char** r_argv) {
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
