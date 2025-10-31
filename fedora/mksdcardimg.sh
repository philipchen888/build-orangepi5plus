#!/bin/sh
BOOT=/dev/sda
parted ${BOOT} mklabel gpt
parted ${BOOT} unit s mkpart boot fat32 34816  1058815
parted ${BOOT} set 1 boot on
parted ${BOOT} unit s mkpart boot2 ext4 1058816  5154815
parted ${BOOT} -- unit s mkpart rootfs ext4 5154816 -34s
