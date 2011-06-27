#!/sbin/sh

######## BootMenu Script v0.8.6
######## Execute [Custom Recovery] Menu


export PATH=/sbin:/system/xbin:/system/bin

######## Main Script

# Moto 2.3.3 /tmp is a link to /data/tmp, bad thing ! &&
[ -L /tmp ] && rm /tmp
[ -L /etc ] && rm /etc

mkdir /tmp
mkdir /etc
mkdir /res

# hijack mke2fs & tune2fs CWM3
rm -f /sbin/mke2fs
rm -f /sbin/tune2fs
rm -f /sbin/e2fsck

cp -r -f /system/bootmenu/recovery/res/* /res/
cp -r -f /system/bootmenu/recovery/sbin/* /sbin/
chmod 755 /sbin/*
cp /system/bootmenu/recovery/recovery.fstab /etc/recovery.fstab

## adbd start

rm -f /sbin/adbd
ps | grep -v grep | grep adbd
ret=$?

if [ ! $ret -eq 0 ]; then
  chmod 755 /system/bootmenu/script/adbd.sh
  /system/bootmenu/script/adbd.sh
fi


#############################
## mount in /sbin/postrecoveryboot.sh
umount -l /system
umount -l /data
umount -l /cache
#############################

# turn on button backlight (back button is used in CWM Recovery 3.x)
echo 1 > /sys/class/leds/button-backlight/brightness

/sbin/recovery &

exit
