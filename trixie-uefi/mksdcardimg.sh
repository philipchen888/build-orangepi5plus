#!/bin/sh
BOOT=/dev/sda
dd if=/dev/zero of=${BOOT} bs=1M count=1024 conv=notrunc,fsync
parted -s ${BOOT} mklabel gpt
parted -s ${BOOT} unit s mkpart boot fat32 34816  1058815
parted -s ${BOOT} set 1 boot on
parted -s ${BOOT} unit s mkpart boot2 ext4 1058816  25774079
