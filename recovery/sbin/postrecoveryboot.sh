#!/sbin/sh

for i in $(seq 1 20); do
  TMP=$(mount | grep /tmp)
  if [ -n "$TMP" ]; then
    umount -l /tmp
    break
  fi

  sleep 0.5
done

mount -o remount,rw /

[ -L /sdcard ] && rm /sdcard
[ -L /sdcard-ext ] && rm /sdcard-ext
mkdir /sdcard
mkdir /sdcard-ext

touch /tmp/recovery.log


mount -t ext3 -o rw,noatime,nodiratime /dev/block/mmcblk1p24 /cache
mount -t ext3 -o rw,noatime,nodiratime /dev/block/mmcblk1p25 /data
mount -t ext3 -o rw,noatime,nodiratime /dev/block/mmcblk1p21 /system

sync
