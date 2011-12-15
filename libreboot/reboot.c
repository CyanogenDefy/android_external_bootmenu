#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/reboot.h>
#include <unistd.h>

//#include "cutils/android_reboot.h"

#ifndef REBOOT_REASON_DEFAULT
#define REBOOT_REASON_DEFAULT NULL
#endif

#ifndef BOARD_BOOTMODE_CONFIG_FILE
#define BOARD_BOOTMODE_CONFIG_FILE "/cache/recovery/bootmode.conf"
#endif

#define DBG_LEVEL 0

int reboot_wrapper(const char* reason) {

	int ret;
	int reboot_with_reason = 0;
	int need_clear_reason = 0;
	FILE * config;

	if (reason == NULL || strlen(reason) == 0 ) {

		reason = REBOOT_REASON_DEFAULT;

		system("rm -f '" BOARD_BOOTMODE_CONFIG_FILE "'");

		#if (DBG_LEVEL)
		printf("reboot: rm -f " BOARD_BOOTMODE_CONFIG_FILE "\n");
		#endif

	} else {

		// pass the reason to the kernel when we reboot
		reboot_with_reason = 1;
		need_clear_reason = 1;

		system("mkdir -p /cache/recovery");

		config = fopen(BOARD_BOOTMODE_CONFIG_FILE,"w");
		if (config == NULL) {
			fprintf(stderr, "reboot: unable to create file " BOARD_BOOTMODE_CONFIG_FILE "\n");
			reboot(RB_AUTOBOOT);
			exit(1);
		}

		// called for all reboot reasons
		if ( 0 == strncmp(reason,"bootloader",10) || 0 == strncmp(reason,"bootmenu", 8) ) {

			// override bootloader reboot mode
			ret = fputs("bootmenu", config);

			#if (DBG_LEVEL)
			printf("reboot: %s->bootmenu " BOARD_BOOTMODE_CONFIG_FILE " (%d)\n", reason, ret);
			#endif

                } else if ( 0 == strncmp(reason,"shell",5) ) {

                        // override bootloader reboot mode
                        ret = fputs("shell", config);

                        #if (DBG_LEVEL)
                        printf("reboot: %s->shell " BOARD_BOOTMODE_CONFIG_FILE " (%d)\n", reason, ret);
                        #endif

		} else {

			ret = fputs(reason, config);

			#if (DBG_LEVEL)
			printf("reboot: %s " BOARD_BOOTMODE_CONFIG_FILE " (%d)\n", reason, ret);
			#endif

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

//toolbox applet entry (ics)
int reboot_main(int argc, char *argv[])
{
    int ret = 0;
    int nosync = 0;
    int poweroff = 0;

    opterr = 0;
    do {
        int c;

        c = getopt(argc, argv, "np");

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

