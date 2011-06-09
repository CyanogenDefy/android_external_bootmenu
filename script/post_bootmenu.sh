#!/sbin/sh

######## BootMenu Script v0.8.6
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

## sbin cleanup
rm /sbin/lsof
#rm /sbin/adbd.root

## busybox cleanup..
for cmd in $(/sbin/busybox --list); do
  [ -L "/sbin/$cmd" ] && rm "/sbin/$cmd"
done

rm /sbin/busybox

######## Don't Delete.... ########################
mount -o remount,ro rootfs /
mount -o remount,ro /dev/block/mmcblk1p21 /system
##################################################

exit
