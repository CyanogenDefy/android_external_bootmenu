#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/reboot.h>
#include <unistd.h>

#ifndef REBOOT_REASON_DEFAULT
#define REBOOT_REASON_DEFAULT NULL
#endif

#ifndef BOARD_BOOTMODE_CONFIG_FILE
#define BOARD_BOOTMODE_CONFIG_FILE "/cache/recovery/bootmode.conf"
#endif

#define DBG_LEVEL 0

#ifndef UNLOCKED_DEVICE
#define OVERRIDE_STOCK_RECOVERY
#endif

#ifndef SINGLE_APPLET
#define SINGLE_APPLET 0
#endif

#if (DBG_LEVEL)
#define pr_debug(fmt, ...) printf("reboot: " fmt, ##__VA_ARGS__)
#else
#define pr_debug(...)
#endif

// beware, toolbox doesn't support -f and -p
#define SH_RM    "busybox rm -f"
#define SH_MKDIR "busybox mkdir -p"

int reboot_wrapper(const char* reason) {

    int ret = 0;
    int reboot_with_reason = 0;
    int need_clear_reason = 0;
    FILE * config;

    if (reason == NULL || strlen(reason) == 0 ) {

        reason = REBOOT_REASON_DEFAULT;

        system(SH_RM " '" BOARD_BOOTMODE_CONFIG_FILE "'");
        pr_debug(SH_RM BOARD_BOOTMODE_CONFIG_FILE "\n");

    } else {

        // pass the reason to the kernel when we reboot
        reboot_with_reason = 1;

        system(SH_MKDIR " /cache/recovery");

        config = fopen(BOARD_BOOTMODE_CONFIG_FILE,"w");
        if (config == NULL) {
            fprintf(stderr, "reboot: unable to create file " BOARD_BOOTMODE_CONFIG_FILE "\n");
            reboot(RB_AUTOBOOT);
            exit(1);
        }

        // called for all reboot reasons
        if ( 0 == strncmp(reason,"bootloader",10) ) {
#ifdef OVERRIDE_STOCK_BOOTLOADER
            // override bootloader reboot mode
            ret = fputs("bootmenu", config);
            pr_debug("%s->bootmenu " BOARD_BOOTMODE_CONFIG_FILE " (%d)\n", reason, ret);
            need_clear_reason = 1;
#endif
        } else if ( 0 == strncmp(reason,"bootmenu", 8) ) {

            char ext_reason[128], dummy[10];
            if (strlen(reason) > 8 && sscanf(reason, "%s %s", dummy, ext_reason) > 0) {
                // allow "bootmenu recovery" or "bootmenu shell"
                ret = fputs(ext_reason, config);
                pr_debug("bootmenu %s " BOARD_BOOTMODE_CONFIG_FILE " (%d)\n", ext_reason, ret);
            } else {
                ret = fputs(reason, config);
                pr_debug("%s->bootmenu " BOARD_BOOTMODE_CONFIG_FILE " (%d)\n", reason, ret);
            }
            need_clear_reason = 1;

        } else if ( 0 == strncmp(reason,"shell",5) ) {

            // override bootloader reboot mode
            ret = fputs("shell", config);
            pr_debug("%s->shell " BOARD_BOOTMODE_CONFIG_FILE " (%d)\n", reason, ret);
            need_clear_reason = 1;

        } else if ( 0 == strncmp(reason,"recovery",8) ) {
#ifdef OVERRIDE_STOCK_RECOVERY
            ret = fputs(reason, config);
            pr_debug("%s->bootmenu recovery " BOARD_BOOTMODE_CONFIG_FILE " (%d)\n", reason, ret);
            need_clear_reason = 1;
#else
            system(SH_RM " '" BOARD_BOOTMODE_CONFIG_FILE "'");
            pr_debug("allowing stock recovery reboot\n");
#endif
        } else {
            pr_debug("unhandled reason:%s\n", reason);
        }

        fflush(config);
        fclose(config);
    }

    // although we have a reason, ignore it on reboot
    if (need_clear_reason) {
        reboot_with_reason = 0;
    }

    system("sync");

    #if (DBG_LEVEL)
    system("sleep 3");
    #endif

    if (reboot_with_reason)
        ret = __reboot(LINUX_REBOOT_MAGIC1, LINUX_REBOOT_MAGIC2, LINUX_REBOOT_CMD_RESTART2, (char *)reason);
    else
        //android_reboot(ANDROID_RB_RESTART, 0, 0);
        ret = reboot(RB_AUTOBOOT);

    return ret;
}

//toolbox applet entry
#if SINGLE_APPLET
int reboot_main(int argc, char *argv[])
{
    int ret = 0;
    int nosync = 0;
    int poweroff = 0;

    opterr = 0;
    do {
        int c = getopt(argc, argv, "np");
        if (c == EOF) {
            break;
        }

        switch (c) {
        case 'n':
            nosync = 1;
            break;
        case 'p':
            poweroff = 1;
            break;
        case '?':
            fprintf(stderr, "usage: %s [-n] [-p] [rebootcommand]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    } while (1);

    if(argc > optind + 1) {
        fprintf(stderr, "%s: too many arguments\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if(!nosync)
        sync();

    if(poweroff)
        ret = __reboot(LINUX_REBOOT_MAGIC1, LINUX_REBOOT_MAGIC2, LINUX_REBOOT_CMD_POWER_OFF, NULL);
    else if(argc > optind) {
        ret = reboot_wrapper(argv[optind]);
    } else
        ret = reboot_wrapper(NULL);
    if(ret < 0) {
        perror("reboot");
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "reboot returned\n");
    return 0;
}
#endif
