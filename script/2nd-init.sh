#!/sbin/sh

######## BootMenu Script v0.8.6
######## Execute [2nd-init] Menu


export PATH=/sbin:/system/xbin:/system/bin

######## Main Script

rm /*.rc
cp -r -f /system/bootmenu/2nd-init/* /
chmod 755 /*.rc
chmod 755 /system/bootmenu/binary/2nd-init


umount /acct
umount /mnt/asec
umount /dev/cpuctl
umount /dev/pts
umount /mnt/obb

## sbin cleanup
## move post_bootmenu.sh


/system/bootmenu/binary/2nd-init

exit
