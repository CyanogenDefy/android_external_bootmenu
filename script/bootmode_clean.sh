#!/sbin/sh

source /system/bootmenu/script/_config.sh

PATH=$PATH:/sbin:/system/xbin:/system/bin

if [ -x /system/bin/mount_ext3.sh ]; then
    mount_ext3.sh cache /cache
else
    mount -t $FS_CACHE -o nosuid,nodev,noatime,nodiratime,barrier=1 $PART_CACHE /cache
fi

mv /cache/recovery/bootmode.conf /cache/recovery/last_bootmode

