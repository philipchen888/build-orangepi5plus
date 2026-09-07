#!/bin/sh
BOOT=/dev/sda
dd if=/dev/zero of=${BOOT} bs=1M count=1024 conv=notrunc,fsync
parted -s ${BOOT} mklabel gpt
parted -s ${BOOT} unit s mkpart loader1 64 8063
parted -s ${BOOT} unit s mkpart loader2 16384 24575
parted -s ${BOOT} unit s mkpart trust 24576 32767
dd if=./out/idbloader.img of=${BOOT} bs=4096 seek=8 conv=notrunc,fsync
dd if=./out/uboot.img of=${BOOT} bs=4096 seek=2048 conv=notrunc,fsync
dd if=./out/trust.img of=${BOOT} bs=4096 seek=3072 conv=notrunc,fsync
