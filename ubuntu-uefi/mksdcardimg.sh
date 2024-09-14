#!/bin/sh
BOOT=/dev/sda
dd if=/dev/zero of=${BOOT} bs=1M count=1024 conv=notrunc,fsync
parted ${BOOT} mklabel gpt
parted ${BOOT} unit s mkpart boot1 fat32 32768 9830399
parted ${BOOT} set 1 boot on
