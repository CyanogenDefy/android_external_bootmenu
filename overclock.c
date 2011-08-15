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

#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "overclock.h"
#include "extendedcommands.h"
#include "minui/minui.h"
#include "bootmenu_ui.h"

#undef USE_4_CLOCK_LEVELS

struct overclock_config
{
 const char *name;
 int value;
};

struct overclock_config overclock[] = {
  { "enable", 0 },
  { "load_all", 0 },
  { "scaling", 2 },
  { "clk1", 300 },
  { "clk2", 600 },
  { "clk3", 1000 },
  { "clk4", 1100 },
  { "vsel1", 33 },
  { "vsel2", 48 },
  { "vsel3", 58 },
  { "vsel4", 62 },
  { "con_up_threshold", 80 },
  { "con_down_threshold", 20 },
  { "con_freq_step", 5 },
  { "con_sampling_rate", 160000 },
  { "int_min_sample_rate", 50000 },
  { "ond_up_threshold", 86 },
  { "ond_sampling_rate", 50000 },
  { "smt_min_cpu_load", 20 },
  { "smt_max_cpu_load", 80 },
  { "smt_awake_min_freq", 200000 },
  { "smt_sleep_max_freq", 200000 },
  { "smt_up_min_freq", 1000000 },
  { "smt_wakeup_freq", 1000000 },
  { "smt_ramp_up_step", 200000 },
  { NULL, 0 },
};


int
get_overclock_value(char* name) {
  struct overclock_config *config;

  for (config = overclock; config->name != NULL; ++config) {
      if (!strcmp(config->name, name)) {
        return config->value;
      }
    }
  return -1;
}

int
set_overclock_value(char* name, int value) {
  struct overclock_config *config;

  for (config = overclock; config->name != NULL; ++config) {
      if (!strcmp(config->name, name)) {
        config->value = value;
        return config->value;
      }
    }
  return -1;
}

int
get_overclock_config(void) {
  FILE *fp;
  char name[255];
  struct overclock_config *config;

  if ((fp = fopen(FILE_OVERCLOCK_CONF, "r")) == NULL) {
    return 1;
  }

  while((fscanf(fp, "%s", name)) != EOF) {

    for (config = overclock; config->name != NULL; ++config) {
      if (!strcmp(config->name, name)) {
        fscanf(fp, "%d", &config->value);
      }
    }
  }
  fclose(fp);
  return 0;
}

int
set_overclock_config(void) {
  FILE *fp;
  struct overclock_config *config;

  if ((fp = fopen(FILE_OVERCLOCK_CONF, "w")) == NULL) {
    return 1;
  }

  for (config = overclock; config->name != NULL; ++config) {
    fprintf(fp, "%s", config->name);
    fprintf(fp, " %d\n", config->value);
  }
  fclose(fp);
  return 0;
}


int
menu_overclock_status(int intl_value) {

#define OVERCLOCK_STATUS_DISABLE      0
#define OVERCLOCK_STATUS_ENABLE       1

  static char** title_headers = NULL;

  if (title_headers == NULL) {
    char* headers[] = { " # --> Set Enable/Disable -->",
                        "",
                        NULL };
    title_headers = prepend_title((const char**)headers);
  }

  char* items[2][2] =  {
                         { "  *[Disable]", "   [Disable]" },
                         { "  *[Enable]", "   [Enable]" },
                       };

  int mode = intl_value;

  for (;;) {
    char* options[4];
    int i;

    for (i = 0; i < 2; ++i) {
      if (mode == i)
        options[i] = items[i][0];
      else
        options[i] = items[i][1];
    }

    options[2] = "   --Go Back.";
    options[3] = NULL;

    int chosen_item = get_menu_selection(title_headers, options, 1, mode);

    switch (chosen_item) {
      case OVERCLOCK_STATUS_ENABLE:
        mode = 1;
        break;

      case OVERCLOCK_STATUS_DISABLE:
        mode = 0;
        break;

      default:
        return mode;
    }
  }

  return mode;
}

#if FULL_VERSION
# define MENU_SYSTEM " System -->"
# define MENU_OVERCLOCK " Overclock -->"
#else
# define MENU_SYSTEM ""
# define MENU_OVERCLOCK " CPU settings -->"
#endif

int
menu_overclock_scaling(void) {

#define OVERCLOCK_SCALING_Conservative   0
#define OVERCLOCK_SCALING_Interactive    1
#define OVERCLOCK_SCALING_Ondemand       2
#define OVERCLOCK_SCALING_Performance    3
#define OVERCLOCK_SCALING_Powersave      4
#define OVERCLOCK_SCALING_Smartass       5
#define OVERCLOCK_SCALING_Userspace      6

  static char** title_headers = NULL;

  if (title_headers == NULL) {
    char* headers[] = { " #" MENU_SYSTEM MENU_OVERCLOCK " Scaling -->",
                        "",
                        NULL };

    title_headers = prepend_title((const char**)headers);
  }

  char* items[7][2] = {
                        { "  *[Conservative]", "   [Conservative]" },
                        { "  *[Interactive]", "   [Interactive]" },
                        { "  *[Ondemand]", "   [Ondemand]" },
                        { "  *[Performance]", "   [Performance]" },
                        { "  *[Powersave]", "   [Powersave]" },
                        { "  *[Smartass]", "   [Smartass]" },
                        { "  *[Userspace]", "   [Userspace]" },
                      };

  for (;;) {

    char* options[9];
    int i;
    int mode = get_overclock_value("scaling");

    for (i = 0; i < 7; ++i) {
      if (mode == i)
        options[i] = items[i][0];
      else
        options[i] = items[i][1];
    }
    
    options[7] = "   --Go Back.";
    options[8] = NULL;

    int chosen_item = get_menu_selection(title_headers, options, 1, mode);

    switch (chosen_item) {
      case OVERCLOCK_SCALING_Conservative:
        set_overclock_value("scaling", 0);
        ui_print("Set Conservative.\n");
        break;

      case OVERCLOCK_SCALING_Interactive:
        set_overclock_value("scaling", 1);
        ui_print("Set Interactive.\n");
        break;

      case OVERCLOCK_SCALING_Ondemand:
        set_overclock_value("scaling", 2);
        ui_print("Set Ondemand.\n");
        break;

      case OVERCLOCK_SCALING_Performance:
        set_overclock_value("scaling", 3);
        ui_print("Set Performance.\n");
        break;

      case OVERCLOCK_SCALING_Powersave:
        set_overclock_value("scaling", 4);
        ui_print("Set Powersave.\n");
        break;

      case OVERCLOCK_SCALING_Smartass:
        set_overclock_value("scaling", 5);
        ui_print("Set Smartass.\n");
        break;

      case OVERCLOCK_SCALING_Userspace:
        set_overclock_value("scaling", 6);
        ui_print("Set Userspace.\n");
        break;

      default:
        return 0;
    }
  }

  return 0;
}


int
menu_set_value(char* name, int intl_value, int min_value, int max_value, int step) {

#define SETVALUE_TITLE     0
#define SETVALUE_SEP       1
#define SETVALUE_ADD       2
#define SETVALUE_SUB       3
#define SETVALUE_BACK      4

  static char** title_headers = NULL;
  int select = 0;

  if (title_headers == NULL) {
    char* headers[] = { " # --> Set Value -->",
                        "",
                        NULL };
    title_headers = prepend_title((const char**)headers);
  }

  char* items[6];
    items[0] = (char*)malloc(sizeof(char)*64);
    items[1] = "  ----------------------";
    items[2] = (char*)malloc(sizeof(char)*64);
    items[3] = (char*)malloc(sizeof(char)*64);
    items[4] = "  --Go Back";
    items[5] = NULL;

  int value = intl_value;

  for (;;) {
    if (value < min_value) value = min_value;
    if (value > max_value) value = max_value;

    sprintf(items[0], "  %s: [%d]", name, value);
    sprintf(items[2], "  [+%d %s]", step, name);
    sprintf(items[3], "  [-%d %s]", step, name);

    int chosen_item = get_menu_selection(title_headers, items, 1, select);

    switch (chosen_item) {
      case SETVALUE_ADD:
        value += step; break;
      case SETVALUE_SUB:
        value -= step; break;

      case SETVALUE_BACK:
        free(items[0]); free(items[2]); free(items[3]);
        return value;

      default:
        break;
    }
    select = chosen_item;
  }

  free(items[0]); free(items[2]); free(items[3]);
  return value;
}

int
show_menu_overclock(void) {

#define OVERCLOCK_STATUS                  0
#define OVERCLOCK_LOAD_ALL                1
#define OVERCLOCK_SCALING                 2
#define OVERCLOCK_CLOCK1                  3
#define OVERCLOCK_CLOCK2                  4
#define OVERCLOCK_CLOCK3                  5
#define OVERCLOCK_CLOCK4                  6
#define OVERCLOCK_VSEL1                   7
#define OVERCLOCK_VSEL2                   8
#define OVERCLOCK_VSEL3                   9
#define OVERCLOCK_VSEL4                  10
#define OVERCLOCK_CON_UP_THRESHOLD       11
#define OVERCLOCK_CON_DOWN_THRESHOLD     12
#define OVERCLOCK_CON_FREQ_STEP          13
#define OVERCLOCK_CON_SAMPLING_RATE      14
#define OVERCLOCK_INT_MIN_SAMPLE_RATE    15
#define OVERCLOCK_OND_UP_THRESHOLD       16
#define OVERCLOCK_OND_SAMPLING_RATE      17
#define OVERCLOCK_smt_min_cpu_load       18
#define OVERCLOCK_smt_max_cpu_load       19
#define OVERCLOCK_smt_awake_min_freq     20
#define OVERCLOCK_smt_sleep_max_freq     21
#define OVERCLOCK_smt_up_min_freq        22
#define OVERCLOCK_smt_wakeup_freq        23
#define OVERCLOCK_smt_ramp_up_step       24
#define OVERCLOCK_DEFAULT                25
#define OVERCLOCK_SAVE                   26
#define OVERCLOCK_GOBACK                 27

  static char** title_headers = NULL;
  int select = 0;

  if (title_headers == NULL) {
    char* headers[] = { " #" MENU_SYSTEM MENU_OVERCLOCK,
                        "",
                        NULL };
    title_headers = prepend_title((const char**)headers);
  }

  get_overclock_config();
  char* items[29];
    #define OC_MALLOC_FIRST 3
    items[3] = (char*)malloc(sizeof(char)*64);
    items[4] = (char*)malloc(sizeof(char)*64);
    items[5] = (char*)malloc(sizeof(char)*64);
    items[6] = (char*)malloc(sizeof(char)*64);
    items[7] = (char*)malloc(sizeof(char)*64);
    items[8] = (char*)malloc(sizeof(char)*64);
    items[9] = (char*)malloc(sizeof(char)*64);
    items[10] = (char*)malloc(sizeof(char)*64);
    items[11] = (char*)malloc(sizeof(char)*64);
    items[12] = (char*)malloc(sizeof(char)*64);
    items[13] = (char*)malloc(sizeof(char)*64);
    items[14] = (char*)malloc(sizeof(char)*64);
    items[15] = (char*)malloc(sizeof(char)*64);
    items[16] = (char*)malloc(sizeof(char)*64);
    items[17] = (char*)malloc(sizeof(char)*64);
    items[18] = (char*)malloc(sizeof(char)*64);
    items[19] = (char*)malloc(sizeof(char)*64);
    items[20] = (char*)malloc(sizeof(char)*64);
    items[21] = (char*)malloc(sizeof(char)*64);
    items[22] = (char*)malloc(sizeof(char)*64);
    items[23] = (char*)malloc(sizeof(char)*64);
    items[24] = (char*)malloc(sizeof(char)*64);
    #define OC_MALLOC_LAST 24
    items[25] = "  [Set defaults(*req reboot/don't save!!)]";
    items[26] = "  [Save]";
    items[27] = "  --Go Back";
    items[28] = NULL;

  for (;;) {

    switch (get_overclock_value("enable")) {
      case 0: items[0] = "  +Status: [Disable] -->"; break;
      case 1: items[0] = "  +Status: [Enable] -->"; break;

      default: items[0] = "  +Status: [Unknown] -->"; break;
    }

    switch (get_overclock_value("load_all")) {
      case 0: items[1] = "  +Load all modules: [Disable] -->"; break;
      case 1: items[1] = "  +Load all modules: [Enable] -->"; break;

      default: items[1] = "  +Load all modules: [Unknown] -->"; break;
    }

    switch (get_overclock_value("scaling")) {
      case 0: items[2] = "  +Scaling: [Conservative] -->"; break;
      case 1: items[2] = "  +Scaling: [Interactive] -->"; break;
      case 2: items[2] = "  +Scaling: [Ondemand] -->"; break;
      case 3: items[2] = "  +Scaling: [Performance] -->"; break;
      case 4: items[2] = "  +Scaling: [Powersave] -->"; break;
      case 5: items[2] = "  +Scaling: [Smartass] -->"; break;
      case 6: items[2] = "  +Scaling: [Userspace] -->"; break;

      default: items[2] = "  +Scaling: [Unknown] -->"; break;
    }
    
    sprintf(items[3], "  +Clk1: [%d] -->", get_overclock_value("clk1"));
    sprintf(items[4], "  +Clk2: [%d] -->", get_overclock_value("clk2"));
    sprintf(items[5], "  +Clk3: [%d] -->", get_overclock_value("clk3"));
#ifdef USE_4_CLOCK_LEVELS
    sprintf(items[6], "  +Clk4: [%d] --> (*req 2.3.3 kernel)", get_overclock_value("clk4"));
    sprintf(items[10], "  +Vsel4: [%d] --> (*req 2.3.3 kernel)", get_overclock_value("vsel4"));
#else
    strcpy(items[6], "  ----------------------");
    strcpy(items[10], "  ----------------------");
#endif
    sprintf(items[7], "  +Vsel1: [%d] -->", get_overclock_value("vsel1"));
    sprintf(items[8], "  +Vsel2: [%d] -->", get_overclock_value("vsel2"));
    sprintf(items[9], "  +Vsel3: [%d] -->", get_overclock_value("vsel3"));
    sprintf(items[11], "  +con_up_threshold: [%d] -->", get_overclock_value("con_up_threshold"));
    sprintf(items[12], "  +con_down_threshold: [%d] -->", get_overclock_value("con_down_threshold"));
    sprintf(items[13], "  +con_freq_step: [%d] -->", get_overclock_value("con_freq_step"));
    sprintf(items[14], "  +con_sampling_rate: [%d] -->", get_overclock_value("con_sampling_rate"));
    sprintf(items[15], "  +int_min_sample_rate: [%d] -->", get_overclock_value("int_min_sample_rate"));
    sprintf(items[16], "  +ond_up_threshold: [%d] -->", get_overclock_value("ond_up_threshold"));
    sprintf(items[17], "  +ond_sampling_rate: [%d] -->", get_overclock_value("ond_sampling_rate"));
    sprintf(items[18], "  +smt_min_cpu_load: [%d] -->", get_overclock_value("smt_min_cpu_load"));
    sprintf(items[19], "  +smt_max_cpu_load: [%d] -->", get_overclock_value("smt_max_cpu_load"));
    sprintf(items[20], "  +smt_awake_min_freq: [%d] -->", get_overclock_value("smt_awake_min_freq"));
    sprintf(items[21], "  +smt_sleep_max_freq: [%d] -->", get_overclock_value("smt_sleep_max_freq"));
    sprintf(items[22], "  +smt_up_min_freq: [%d] -->", get_overclock_value("smt_up_min_freq"));
    sprintf(items[23], "  +smt_wakeup_freq: [%d] -->", get_overclock_value("smt_wakeup_freq"));
    sprintf(items[24], "  +smt_ramp_up_step: [%d] -->", get_overclock_value("smt_ramp_up_step"));

    int chosen_item = get_menu_selection(title_headers, items, 1, select);

    switch (chosen_item) {
      case OVERCLOCK_STATUS:
        set_overclock_value("enable", menu_overclock_status(get_overclock_value("enable"))); break;

      case OVERCLOCK_LOAD_ALL:
        set_overclock_value("load_all", menu_overclock_status(get_overclock_value("load_all"))); break;

      case OVERCLOCK_SCALING:
        menu_overclock_scaling(); break;

      case OVERCLOCK_CLOCK1:
        set_overclock_value("clk1", menu_set_value("Clk1", get_overclock_value("clk1"), 200, 2000, 10)); break;

      case OVERCLOCK_CLOCK2:
        set_overclock_value("clk2", menu_set_value("Clk2", get_overclock_value("clk2"), 200, 2000, 10)); break;

      case OVERCLOCK_CLOCK3:
        set_overclock_value("clk3", menu_set_value("Clk3", get_overclock_value("clk3"), 200, 2000, 10)); break;

#ifdef USE_4_CLOCK_LEVELS
      case OVERCLOCK_CLOCK4:
        set_overclock_value("clk4", menu_set_value("Clk4", get_overclock_value("clk4"), 200, 2000, 10)); break;

      case OVERCLOCK_VSEL4:
        set_overclock_value("vsel4", menu_set_value("Vsel4", get_overclock_value("vsel4"), 10, 100, 1)); break;
#endif

      case OVERCLOCK_VSEL1:
        set_overclock_value("vsel1", menu_set_value("Vsel1", get_overclock_value("vsel1"), 10, 100, 1)); break;

      case OVERCLOCK_VSEL2:
        set_overclock_value("vsel2", menu_set_value("Vsel2", get_overclock_value("vsel2"), 10, 100, 1)); break;

      case OVERCLOCK_VSEL3:
        set_overclock_value("vsel3", menu_set_value("Vsel3", get_overclock_value("vsel3"), 10, 100, 1)); break;

      case OVERCLOCK_CON_UP_THRESHOLD:
        set_overclock_value("con_up_threshold", menu_set_value("con_up_threshold", get_overclock_value("con_up_threshold"), 1, 100, 1)); break;

      case OVERCLOCK_CON_DOWN_THRESHOLD:
        set_overclock_value("con_down_threshold", menu_set_value("con_down_threshold", get_overclock_value("con_down_threshold"), 1, 100, 1)); break;

      case OVERCLOCK_CON_FREQ_STEP:
        set_overclock_value("con_freq_step", menu_set_value("con_freq_step", get_overclock_value("con_freq_step"), 1, 100, 1)); break;

      case OVERCLOCK_CON_SAMPLING_RATE:
        set_overclock_value("con_sampling_rate", menu_set_value("con_sampling_rate", get_overclock_value("con_sampling_rate"), 160000, 500000, 1000)); break;

      case OVERCLOCK_INT_MIN_SAMPLE_RATE:
        set_overclock_value("int_min_sample_rate", menu_set_value("int_min_sample_rate", get_overclock_value("int_min_sample_rate"), 5000, 500000, 1000)); break;

      case OVERCLOCK_OND_UP_THRESHOLD:
        set_overclock_value("ond_up_threshold", menu_set_value("ond_up_threshold", get_overclock_value("ond_up_threshold"), 1, 100, 1)); break;

      case OVERCLOCK_OND_SAMPLING_RATE:
        set_overclock_value("ond_sampling_rate", menu_set_value("ond_sampling_rate", get_overclock_value("ond_sampling_rate"), 10000, 100000, 1000)); break;

      case OVERCLOCK_smt_min_cpu_load:
        set_overclock_value("smt_min_cpu_load", menu_set_value("smt_min_cpu_load", get_overclock_value("smt_min_cpu_load"), 1, 100, 1)); break;

      case OVERCLOCK_smt_max_cpu_load:
        set_overclock_value("smt_max_cpu_load", menu_set_value("smt_max_cpu_load", get_overclock_value("smt_max_cpu_load"), 1, 100, 1)); break;

      case OVERCLOCK_smt_awake_min_freq:
        set_overclock_value("smt_awake_min_freq", menu_set_value("smt_awake_min_freq", get_overclock_value("smt_awake_min_freq"), 200000, 2000000, 10000)); break;

      case OVERCLOCK_smt_sleep_max_freq:
        set_overclock_value("smt_sleep_max_freq", menu_set_value("smt_sleep_max_freq", get_overclock_value("smt_sleep_max_freq"), 200000, 2000000, 10000)); break;

      case OVERCLOCK_smt_up_min_freq:
        set_overclock_value("smt_up_min_freq", menu_set_value("smt_up_min_freq", get_overclock_value("smt_up_min_freq"), 200000, 2000000, 10000)); break;

      case OVERCLOCK_smt_wakeup_freq:
        set_overclock_value("smt_wakeup_freq", menu_set_value("smt_wakeup_freq", get_overclock_value("smt_wakeup_freq"), 200000, 2000000, 10000)); break;

      case OVERCLOCK_smt_ramp_up_step:
        set_overclock_value("smt_ramp_up_step", menu_set_value("smt_ramp_up_step", get_overclock_value("smt_ramp_up_step"), 100000, 500000, 10000)); break;

      case OVERCLOCK_SAVE:
        ui_print("Saving.... ");
        set_overclock_config();
        ui_print("Done.\n");
        break;

      case OVERCLOCK_DEFAULT:
        ui_print("Set defaults...");
        remove(FILE_OVERCLOCK_CONF);
        ui_print("Done. Please reboot.\n");
        break;

      default:
        return 0;
    }
    select = chosen_item;
  }

  //release mallocs
  for (select=OC_MALLOC_FIRST; select<=OC_MALLOC_LAST; select++) {
    free(items[select]);
  }

  return 0;
}
