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
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/reboot.h>
//#include <sys/wait.h>
#include <time.h>

#include <unistd.h>

#include "common.h"
#include "extendedcommands.h"
#include "overclock.h"
#include "minui/minui.h"
#include "bootmenu_ui.h"

enum {
  BUTTON_ERROR,
  BUTTON_PRESSED,
  BUTTON_TIMEOUT,
};

/* Main menu items */
#define ITEM_REBOOT      0
#define ITEM_BOOT        1
#define ITEM_SYSTEM      2
#define ITEM_OVERCLOCK   2
#define ITEM_RECOVERY    3
#define ITEM_TOOLS       4
#define ITEM_POWEROFF    5

#define ITEM_LAST        5

char* MENU_ITEMS[] = {
    "  [Reboot]",
    "  +Boot -->",
#if STOCK_VERSION
    "  +System -->",
#else
    "  +CPU Settings -->",
#endif
    "  +Recovery -->",
    "  +Tools -->",
    "  [Shutdown]",
    NULL
};

static char** main_headers = NULL;

/**
 * prepend_title()
 *
 */
char** prepend_title(const char** headers) {

  char* title[] = {
      "Android Bootmenu <v"
      EXPAND(BOOTMENU_VERSION) ">",
      "",
      NULL
  };

  // count the number of lines in our title, plus the
  // caller-provided headers.
  int count = 0;
  char** p;
  for (p = title; *p; ++p, ++count);
  for (p = (char**) headers; *p; ++p, ++count);

  char** new_headers = malloc((count+1) * sizeof(char*));
  char** h = new_headers;
  for (p = title; *p; ++p, ++h) *h = *p;
  for (p = (char**) headers; *p; ++p, ++h) *h = *p;
  *h = NULL;

  return new_headers;
}

void free_menu_headers(char **headers) {
  char** p = headers;
  for (p = headers; *p; ++p) *p = NULL;
  if (headers != NULL) {
    free(headers);
    headers = NULL;
  }
}

/**
 * get_menu_selection()
 *
 */
int get_menu_selection(char** headers, char** items, int menu_only,
                       int initial_selection) {
  // throw away keys pressed previously, so user doesn't
  // accidentally trigger menu items.
  ui_clear_key_queue();

  ui_start_menu(headers, items, initial_selection);
  int selected = initial_selection;
  int chosen_item = -1;

  while (chosen_item < 0) {
    int key = ui_wait_key();
    int visible = ui_text_visible();

    int action = device_handle_key(key, visible);

      if (action < 0) {
        switch (action) {
          case HIGHLIGHT_UP:
            --selected;
            selected = ui_menu_select(selected);
            break;
          case HIGHLIGHT_DOWN:
            ++selected;
            selected = ui_menu_select(selected);
            break;
          case SELECT_ITEM:
            chosen_item = selected;
            break;
          case ACTION_CANCEL:
            chosen_item = GO_BACK;
            break;
          case NO_ACTION:
            break;
      }
    } else if (!menu_only) {
      chosen_item = action;
    }
  }

  ui_end_menu();
  return chosen_item;
}

/**
 * compare_string()
 *
 */
static int compare_string(const void* a, const void* b) {
  return strcmp(*(const char**)a, *(const char**)b);
}

/**
 * prompt_and_wait()
 *
 */
static void prompt_and_wait() {

  int select = 0;

  for (;;) {
    ui_reset_progress();
    int chosen_item = get_menu_selection(main_headers, MENU_ITEMS, 0, select);

    // device-specific code may take some action here.  It may
    // return one of the core actions handled in the switch
    // statement below.

    if (chosen_item >= 0 && chosen_item <= ITEM_LAST) {

      switch (chosen_item) {
      case ITEM_REBOOT:
        sync();
        reboot(RB_AUTOBOOT);
        return;
      case ITEM_BOOT:
        if (show_menu_boot()) return;
        break;
#if STOCK_VERSION
      case ITEM_SYSTEM:
        if (show_menu_system()) return;
        break;
#else
      case ITEM_OVERCLOCK:
        if (show_menu_overclock()) return;
        break;
#endif
      case ITEM_RECOVERY:
        if (show_menu_recovery()) return;
        break;
      case ITEM_TOOLS:
        if (show_menu_tools()) return;
        break;
      case ITEM_POWEROFF:
        sync();
        __reboot(LINUX_REBOOT_MAGIC1, LINUX_REBOOT_MAGIC2, LINUX_REBOOT_CMD_POWER_OFF, NULL);
        return;
      }

      select = chosen_item;
    }
  }
}

/**
 * ui_finish()
 *
 */
static void ui_finish(void) {
  LOGI("Exiting....\n");
  ui_final();
}

/**
 * wait_key()
 *
 */
static int wait_key(int key) {
  int i;
  int result = 0;

  evt_init();
  //ui_clear_key_queue();
  for(i=0; i < 100; i++) {
    if(ui_key_pressed(key)) {
      led_alert("blue", DISABLE);
      result = 1;
      break;
    }
    else {
      usleep(15000); //15ms * 100
    }
  }
  evt_exit();
  return result;
}

/**
 * run_bootmenu()
 *
 */
static int run_bootmenu(void) {
  int defmode, mode, status = BUTTON_ERROR;
  int adb_started = 0;
  time_t start = time(NULL);

  LOGI("Starting bootmenu on %s", ctime(&start));

  if (bypass_check()) {

    // init rootfs and mount cache
    exec_script(FILE_PRE_MENU, DISABLE);

    led_alert("blue", ENABLE);

    defmode = get_default_bootmode();

    // get and clean one shot bootmode (or default)
    mode = get_bootmode(1);

    // dont wait if bootmenu or recovery mode asked
    if (mode != int_mode("bootmenu") && mode != int_mode("recovery")) {
        status = (wait_key(KEY_VOLUMEDOWN) ? BUTTON_PRESSED : BUTTON_TIMEOUT);
    }

    // only start adb if usb is connected
    if (usb_connected()) {
      if (mode == int_mode("2nd-init-adb") || mode == int_mode("2nd-boot-adb")) {
         exec_script(FILE_ADBD, DISABLE);
         adb_started = 1;
      }
    }

    // on timeout
    if (status != BUTTON_PRESSED) {

      if (mode == int_mode("bootmenu")) {
          led_alert("blue", DISABLE);
          status = BUTTON_PRESSED;
      }
      else if (mode == int_mode("2nd-init") || mode == int_mode("2nd-init-adb")) {
          led_alert("blue", DISABLE);
          led_alert("green", ENABLE);
          snd_init(DISABLE);
          led_alert("green", DISABLE);
          status = BUTTON_TIMEOUT;
      }
      else if (mode == int_mode("2nd-boot") || mode == int_mode("2nd-boot-adb")) {
          led_alert("blue", DISABLE);
          led_alert("red", ENABLE);
          snd_boot(DISABLE);
          led_alert("red", DISABLE);
          status = BUTTON_TIMEOUT;
      }
      else if (mode == int_mode("recovery")) {
          led_alert("blue", DISABLE);
          exec_script(FILE_STABLERECOVERY, DISABLE);
          status = BUTTON_TIMEOUT;
      }
#if STOCK_VERSION
      else if (mode == int_mode("normal") || mode == int_mode("normal-adb")) {
          led_alert("blue", DISABLE);
          stk_boot(DISABLE);
          status = BUTTON_TIMEOUT;
      }
#endif

    }

    if (status == BUTTON_PRESSED ) {

        ui_init();
        ui_set_background(BACKGROUND_DEFAULT);
        ui_show_text(ENABLE);
        LOGI("Start Android BootMenu....\n");

        main_headers = prepend_title((const char**)MENU_HEADERS);

        /* can be buggy

        if (!adb_started && usb_connected()) {
            ui_print("Usb connected, starting adb...\n\n");
            //ui_print("Battery level: %d %%\n", battery_level());
            exec_script(FILE_ADBD, DISABLE);
        }
        */

        //ui_print("Current mode: %s\n", str_mode(mode));
        ui_print("Default mode: %s\n", str_mode(defmode));

        checkup_report();

        prompt_and_wait();
        free_menu_headers(main_headers);

        ui_finish();
    }

  }
  return EXIT_SUCCESS;
}


/**
 * main()
 *
 * Here is the hijack part, logwrapper is linked to bootmenu
 * we trap some of logged commands from init.rc
 *
 */
int main(int argc, char **argv) {
  char* hijacked_executable = argv[0];
  int result;

  if (NULL != strstr(hijacked_executable, "bootmenu")) {
    fprintf(stdout, "Run BootMenu..\n");
    result = run_bootmenu();
    sync();
    return result;
  }
  else if (argc >= 3 && 0 == strcmp(argv[2], "userdata")) {
    //real_execute(argc, argv);
    result = run_bootmenu();
    real_execute(argc, argv);
    bypass_sign("no");
    sync();
    return result;
  }
  else if (argc >= 3 && 0 == strcmp(argv[2], "pds")) {
    //kept for stock rom compatibility, cyanogen use postbootmenu
    real_execute(argc, argv);
    exec_script(FILE_OVERCLOCK, DISABLE);
    result = exec_script(FILE_POST_MENU, DISABLE);
    bypass_sign("no");
    sync();
    return result;
  }
  else if (argc == 2 && 0 == strcmp(argv[1], "postbootmenu")) {
    exec_script(FILE_OVERCLOCK, DISABLE);
    result = exec_script(FILE_POST_MENU, DISABLE);
    bypass_sign("no");
    sync();
    return result;
  }
  else {
    return real_execute(argc, argv);
  }
}

