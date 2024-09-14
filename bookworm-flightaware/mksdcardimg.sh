#!/bin/sh
sudo parted -s /dev/sda mklabel gpt
sudo parted -s /dev/sda unit s mkpart boot fat32 2048 2869247
sudo parted -s /dev/sda -- unit s mkpart rootfs ext4 2869248 -34s
sudo parted -s /dev/sda set 1 boot on
sudo dd if=../rootfs-bookworm/linaro-esp.img of=/dev/sda bs=4096 seek=256 conv=notrunc,fsync
sudo dd if=../rootfs-bookworm/linaro-rootfs.img of=/dev/sda bs=4096 seek=358656 conv=notrunc,fsync
