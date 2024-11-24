#!/bin/sh
BOOT=/dev/sda
dd if=/dev/zero of=${BOOT} bs=1M count=0 seek=6144
parted -s ${BOOT} mklabel gpt
parted -s ${BOOT} unit s mkpart boot fat32 2048 2869247
parted -s ${BOOT} -- unit s mkpart rootfs ext4 2869248 -34s
parted -s ${BOOT} set 1 boot on
dd if=../rootfs-bookworm/linaro-esp.img of=${BOOT} bs=4096 seek=256 conv=notrunc,fsync
dd if=../rootfs-bookworm/linaro-rootfs.img of=${BOOT} bs=4096 seek=358656 conv=notrunc,fsync
