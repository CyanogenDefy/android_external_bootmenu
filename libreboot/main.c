#include <stdio.h>

int reboot_main(int argc, char *argv[]);

// single applet entry point
int main(int argc, char *argv[])
{
    return reboot_main(argc, argv);
}

