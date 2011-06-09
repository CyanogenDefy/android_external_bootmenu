#!/system/bin/sh

######## BootMenu Script v0.8.6
######## Execute Pre BootMenu


export PATH=/sbin:/system/xbin:/system/bin

######## Main Script

mount -o remount,rw rootfs /

BUSYBOX="/sbin/busybox"

# RECOVERY tool includes busybox
cp -f /system/bootmenu/recovery/sbin/recovery $BUSYBOX

chmod 755 /sbin/*
$BUSYBOX chown 0.0 $BUSYBOX
$BUSYBOX chmod 4755 $BUSYBOX


## begin busybox sym link..

for cmd in $(/sbin/busybox --list); do
  $BUSYBOX ln -s $BUSYBOX /sbin/$cmd
done

# disable some duplicate busybox applets
[ -f /sbin/reboot ] && rm /sbin/reboot

## rootsh create.

cp -f /system/bin/sh /rootsh
chown 0.0 /rootsh
chmod 6755 /rootsh

## missing system files

[ ! -c /dev/tty0 ]  && ln -s /dev/tty /dev/tty0

## /default.prop replace..

rm -f /default.prop
cp -f /system/bootmenu/config/default.prop /default.prop

## /sbin/adbd replace..

cp -f /system/bootmenu/binary/adbd /sbin/adbd.root
chmod 755 /sbin/adbd.root

# set scaling_governor "ondemand"
echo "ondemand" > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor

exit
