#!/bin/sh
BOOT=/dev/sda
dd if=/dev/zero of=${BOOT} bs=1M count=1024
parted ${BOOT} mklabel gpt
parted ${BOOT} unit s mkpart boot fat32 2048 2869247
parted ${BOOT} set 1 boot on
