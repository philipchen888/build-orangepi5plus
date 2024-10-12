#!/bin/sh
BOOT=/dev/sda
dd if=/dev/zero of=${BOOT} bs=1M count=1024
parted ${BOOT} mklabel gpt
parted ${BOOT} unit s mkpart boot fat32 32768 1261567
parted -s ${BOOT} set 1 boot on
parted -s ${BOOT} unit s mkpart boot2 ext4 1261568 3358719
parted -s ${BOOT} -- unit s mkpart rootfs ext4 3358720 -34s
ROOT_UUID="B921B045-1DF0-41C3-AF44-4C6F280D3FAE"
gdisk ${BOOT} <<EOF
x
c
3
${ROOT_UUID}
w
y
EOF
