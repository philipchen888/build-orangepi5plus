#!/bin/sh
BOOT=/dev/sda
dd if=/dev/zero of=${BOOT} bs=1M count=1024 conv=notrunc,fsync
parted ${BOOT} mklabel gpt
parted ${BOOT} unit s mkpart boot fat32 32769 3276800
parted ${BOOT} -- unit s mkpart rootfs ext4 3276801 -34s
parted ${BOOT} set 1 boot on
