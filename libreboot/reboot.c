#include <reboot/reboot.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/reboot.h>

#ifdef RECOVERY_SHELL
#include "../../../bootable/recovery/libcrecovery/common.h"
#define system __system
#endif

#ifndef REBOOT_REASON_DEFAULT
#define REBOOT_REASON_DEFAULT NULL
#endif

#ifndef BOARD_BOOTMODE_CONFIG_FILE
#define BOARD_BOOTMODE_CONFIG_FILE "/cache/recovery/bootmode.conf"
#endif

#define DBG_LEVEL 0
//#define OVERRIDE_STOCK_BOOTLOADER

//defined in system/core/libreboot
#ifndef UNLOCKED_DEVICE
#define OVERRIDE_STOCK_RECOVERY
#endif

#if (DBG_LEVEL)
#define pr_debug(fmt, ...) printf("reboot: " fmt, ##__VA_ARGS__)
#else
#define pr_debug(...)
#endif

int reboot_wrapper(const char* reason) {

	int ret = 0;
	int reboot_with_reason = 0;
	int need_clear_reason = 0;
	FILE * config;

	if (reason == NULL || strlen(reason) == 0 ) {

		reason = REBOOT_REASON_DEFAULT;

		system("rm -f '" BOARD_BOOTMODE_CONFIG_FILE "'");
		pr_debug("rm -f " BOARD_BOOTMODE_CONFIG_FILE "\n");

	} else {

		// pass the reason to the kernel when we reboot
		reboot_with_reason = 1;

		system("mkdir -p /cache/recovery");

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
				#if (DBG_LEVEL)
				printf("reboot: %s->bootmenu " BOARD_BOOTMODE_CONFIG_FILE " (%d)\n", ext_reason, ret);
				#endif
			} else {
				ret = fputs(reason, config);
				#if (DBG_LEVEL)
				printf("reboot: %s->bootmenu " BOARD_BOOTMODE_CONFIG_FILE " (%d)\n", reason, ret);
				#endif
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
			system("rm -f '" BOARD_BOOTMODE_CONFIG_FILE "'");
			pr_debug("allowing stock recovery reboot\n");
#endif
		} else {
			//hmm...
			pr_debug("unhandled reason:%s\n", reason);
			//need_clear_reason = 1;
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
		ret = reboot(RB_AUTOBOOT);

	return ret;
}
