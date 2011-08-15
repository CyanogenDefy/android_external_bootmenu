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
  INSTALL_SUCCESS,
  INSTALL_ERROR,
  INSTALL_CORRUPT
};

static const char *FILE_PRE_MENU = "/system/bootmenu/script/pre_bootmenu.sh";
static const char *FILE_POST_MENU = "/system/bootmenu/script/post_bootmenu.sh";

static char** main_headers = NULL;

char**
prepend_title(const char** headers) {
  char* title[] = { "Android BootMenu <v"
                    EXPAND(BOOTMENU_VERSION) ">",
                    "",
                    NULL };

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

int
get_menu_selection(char** headers, char** items, int menu_only,
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

static int compare_string(const void* a, const void* b) {
  return strcmp(*(const char**)a, *(const char**)b);
}

static void
prompt_and_wait() {
  int select = 0;

  for (;;) {
    ui_reset_progress();
    int chosen_item = get_menu_selection(main_headers, MENU_ITEMS, 0, select);

    // device-specific code may take some action here.  It may
    // return one of the core actions handled in the switch
    // statement below.
    chosen_item = device_perform_action(chosen_item);
    switch (chosen_item) {
      case ITEM_BOOT:
        if (show_menu_boot()) return; else break;
#if FULL_VERSION
      case ITEM_SYSTEM:
        if (show_menu_system()) return; else break;
#else
      case ITEM_OVERCLOCK:
        if (show_menu_overclock()) return; else break;
#endif
      case ITEM_RECOVERY:
        if (show_menu_recovery()) return; else break;
      case ITEM_TOOLS:
        if (show_menu_tools()) return; else break;
      case ITEM_REBOOT:
        ui_print("Reboot now....\n");
        sync();
        reboot(RB_AUTOBOOT);
        return;
    }
    select = chosen_item;
  }
}

static void
ui_finish(void) {
  LOGI("Exiting....\n");
  ui_final();
}

static int
wait_key(int key) {
  int i;
  int result = INSTALL_SUCCESS;

  evt_init();
  ui_clear_key_queue();
  for(i=0; i < 100; i++) {
    if(ui_key_pressed(key)) {
      result = INSTALL_ERROR;
    }
    else {
      usleep(15000); //15ms
    }
  }
  evt_exit();
  return result;
}

static int
run_bootmenu(void) {
  int mode, status = INSTALL_SUCCESS;
  time_t start = time(NULL);

  LOGI("Starting bootmenu on %s", ctime(&start));

  if (bypass_check()) {

    // init rootfs and mount cache
    exec_script(FILE_PRE_MENU, DISABLE);

    led_alert("blue", ENABLE);

    mode = get_bootmode();

    // dont wait if bootmenu or recovery mode asked
    if (mode != MODE_BOOTMENU && mode != MODE_RECOVERY) {
      status = wait_key(KEY_VOLUMEDOWN);
    }

    if (status != INSTALL_ERROR) {

      switch (mode) {
        case MODE_2NDINIT:
          led_alert("blue", DISABLE);
          led_alert("green", ENABLE);
          snd_init(DISABLE);
          led_alert("green", DISABLE);
          break;

        case MODE_2NDBOOT:
          led_alert("blue", DISABLE);
          led_alert("red", ENABLE);
          snd_boot(DISABLE);
          led_alert("red", DISABLE);
          break;

        case MODE_BOOTMENU:
          led_alert("blue", DISABLE);
          status = INSTALL_ERROR;
          break;

        case MODE_RECOVERY:
          led_alert("blue", DISABLE);
          exec_script(FILE_STABLERECOVERY, DISABLE);
          status = INSTALL_SUCCESS;
          break;
      }
    }

    if (status != INSTALL_SUCCESS) {
      ui_init();
      ui_set_background(BACKGROUND_DEFAULT);

      ui_show_text(ENABLE);
      LOGI("Start Android BootMenu....\n");

      main_headers = prepend_title((const char**)MENU_HEADERS);
      prompt_and_wait();
      free(main_headers);

      ui_finish();
    }

  }
  return EXIT_SUCCESS;
}

int
main(int argc, char **argv) {
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
    real_execute(argc, argv);
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
