#!/bin/sh
sudo parted -s /dev/sda mklabel gpt
sudo parted -s /dev/sda -- unit s mkpart rootfs 61440 -34s
sudo parted -s /dev/sda set 1 boot on
sudo dd if=../rootfs-bookworm/linaro-basicrootfs.img of=/dev/sda bs=4096 seek=7680 conv=notrunc,fsync
