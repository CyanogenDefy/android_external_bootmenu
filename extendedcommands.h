enum {
  MODE_NORMAL,
  MODE_2NDINIT,
  MODE_2NDBOOT,
  MODE_BOOTMENU
};

static const char *FILE_2NDINIT = "/system/bootmenu/script/2nd-init.sh";
static const char *FILE_2NDBOOT = "/system/bootmenu/script/2nd-boot.sh";
static const char *FILE_ADBD = "/system/bootmenu/script/adbd.sh";
static const char *FILE_ROOT = "/system/bootmenu/script/unroot.sh";
static const char *FILE_UNINSTALL = "/system/bootmenu/script/uninstall.sh";
static const char *FILE_OVERCLOCK = "/system/bootmenu/script/overclock.sh";
static const char *FILE_CUSTOMRECOVERY = "/system/bootmenu/script/recovery.sh";

static const char *FILE_DEFAULTBOOTMODE = "/system/bootmenu/config/default_bootmode.conf";
static const char *FILE_OVERCLOCK_CONF = "/system/bootmenu/config/overclock.conf";
static const char *FILE_BYPASS = "/data/.bootmenu_bypass";
static const char *FILE_STOCKRECOVERY = "/system/bin/reboot";

int
show_menu_boot(void);

int
show_menu_system(void);

int
show_menu_tools(void);

int
show_menu_recovery(void);

int
mount_usb_storage(void);

int
snd_init(int ui);

int
snd_boot(int ui);

int
show_config_bootmode(void);

int
set_bootmode(int mode);

int
get_bootmode(void);

int
bootmode_write(const char* str);

int
led_alert(const char* color, int value);

int
bypass_sign(const char* mode);

int
bypass_check(void);

int
exec_and_wait(char** argp);

int
exec_script(const char* filename, int ui);

int
real_execute(int r_argc, char** r_argv);
