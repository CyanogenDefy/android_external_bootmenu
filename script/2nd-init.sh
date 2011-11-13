#!/sbin/sh

######## BootMenu Script
######## Execute [2nd-init] Menu


export PATH=/sbin:/system/xbin:/system/bin

######## Main Script

mount -o remount,rw /
rm -f /*.rc
cp -r -f /system/bootmenu/2nd-init/* /
chmod 755 /*.rc
chmod 4755 /system/bootmenu/binary/2nd-init
ln -s /init /sbin/ueventd

## unmount devices
sync
umount /acct
umount /mnt/asec
umount /dev/cpuctl
umount /dev/pts
umount /mnt/obb
umount /cache
umount /data

######## Cleanup

rm -f /sbin/lsof

## busybox cleanup..
for cmd in $(/sbin/busybox --list); do
  [ -L "/sbin/$cmd" ] && rm "/sbin/$cmd"
done

rm /sbin/busybox

## reduce lcd backlight to save battery
echo 18 > /sys/class/leds/lcd-backlight/brightness


######## Let's go

/system/bootmenu/binary/2nd-init

