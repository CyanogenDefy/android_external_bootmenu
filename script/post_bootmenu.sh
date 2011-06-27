#!/sbin/sh

######## BootMenu Script
######## Execute Post BootMenu


export PATH=/sbin:/system/xbin:/system/bin

######## Main Script


## Run Init Script

######## Don't Delete.... ########################
mount -o remount,rw rootfs /
mount -o remount,rw /dev/block/mmcblk1p21 /system
##################################################

chmod 755 /system/bootmenu/init.d/*
run-parts /system/bootmenu/init.d/

# normal cleanup here (need fix in recovery first)
# disabled...


######## Don't Delete.... ########################
mount -o remount,ro rootfs /
mount -o remount,ro /dev/block/mmcblk1p21 /system
##################################################

exit
