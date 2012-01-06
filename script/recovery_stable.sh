#!/sbin/sh

######## BootMenu Script
######## Execute [Stable Recovery] Menu

source /system/bootmenu/script/_config.sh

######## Main Script

## /tmp folder can be a link to /data/tmp, bad thing !
[ -L /tmp ] && rm /tmp
mkdir -p /tmp
mkdir -p /res

rm -f /etc
mkdir /etc

# hijack mke2fs & tune2fs CWM3
rm -f /sbin/mke2fs
rm -f /sbin/tune2fs
rm -f /sbin/e2fsck

rm -f /sdcard
mkdir /sdcard

chmod 755 /sbin
chmod 755 /res

cp -r -f $BM_ROOTDIR/recovery/res/* /res/
cp -p -f $BM_ROOTDIR/recovery/sbin/* /sbin/
cp -p -f $BM_ROOTDIR/script/recoveryexit.sh /sbin/

if [ ! -f /sbin/recovery_stable ]; then
    ln -s /sbin/recovery /sbin/recovery_stable
fi

cd /sbin
ln -s recovery edify
ln -s recovery setprop
ln -s recovery dump_image
ln -s recovery erase_image
ln -s recovery flash_image
ln -s recovery mkyaffs2image
ln -s recovery unyaffs
ln -s recovery nandroid
ln -s recovery volume
ln -s recovery reboot

chmod +rx /sbin/*

rm -f /sbin/postrecoveryboot.sh

if [ ! -e /etc/recovery.fstab ]; then
    cp $BM_ROOTDIR/recovery/recovery.fstab /etc/recovery.fstab
fi

# for ext3 format
cp $BM_ROOTDIR/config/mke2fs.conf /etc/

mkdir -p /cache/recovery
touch /cache/recovery/command
touch /cache/recovery/log
touch /cache/recovery/last_log
touch /tmp/recovery.log

killall adbd

# mount fake image of pds, for backup purpose (4MB)
[ ! -d /data/data ] && mount -t $FS_DATA -o rw,noatime,nodiratime,errors=continue $PART_DATA /data
if [ ! -f /data/pds.img ]; then
    /system/etc/init.d/04pdsbackup
    umount /pds
    losetup -d /dev/block/loop7
fi
cp /data/pds.img /tmp/pds.img
if [ -f /tmp/pds.img ] ; then
    mkdir -p /pds
    umount /pds 2>/dev/null
    losetup -d /dev/block/loop7 2>/dev/null
    losetup /dev/block/loop7 /tmp/pds.img
    busybox mount -o rw,nosuid,nodev,noatime,nodiratime,barrier=1 /dev/block/loop7 /pds
fi

ps | grep -v grep | grep adbd
ret=$?

if [ ! $ret -eq 0 ]; then
   # $BM_ROOTDIR/script/adbd.sh

   # don't use adbd here, will load many android process which locks /system
   killall adbd
   killall adbd.root
fi

#############################
# mount in /sbin/postrecoveryboot.sh
umount /system

usleep 50000
mount -t $FS_SYSTEM -o rw,noatime,nodiratime $PART_SYSTEM /system

# retry without type & options if not mounted
[ ! -f /system/build.prop ] && mount -o rw $PART_SYSTEM /system

# set red led if problem with system
echo 0 > /sys/class/leds/red/brightness
echo 0 > /sys/class/leds/green/brightness
echo 0 > /sys/class/leds/blue/brightness
[ ! -f /system/build.prop ] && echo 1 > /sys/class/leds/red/brightness

#############################

/sbin/recovery_stable


# Post Recovery (back to bootmenu)

# remount system & data if unmounted
[ ! -d /data/data ] &&         mount -t $FS_DATA -o rw,noatime,nodiratime,errors=continue $PART_DATA /data
[ ! -f /system/build.prop ] && mount -t $FS_SYSTEM -o rw,noatime,nodiratime,errors=continue $PART_SYSTEM /system

if [ -f /system/build.prop ] ; then
	echo 0 > /sys/class/leds/red/brightness
	echo 0 > /sys/class/leds/green/brightness
	echo 1 > /sys/class/leds/blue/brightness
else
	echo 1 > /sys/class/leds/red/brightness
	echo 0 > /sys/class/leds/green/brightness
	echo 0 > /sys/class/leds/blue/brightness
fi

exit
