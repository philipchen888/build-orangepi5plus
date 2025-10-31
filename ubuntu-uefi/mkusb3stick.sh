#!/bin/sh
BOOT=/dev/sda
parted ${BOOT} mklabel gpt
parted ${BOOT} unit s mkpart boot fat32 2048 9830399
parted ${BOOT} set 1 boot on
