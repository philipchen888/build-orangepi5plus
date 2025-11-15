#!/bin/sh

BOARD_DIR="$(dirname "$0")"

install -m 0644 -D $BOARD_DIR/../../../../../linux/linux/arch/arm64/boot/Image $BINARIES_DIR/Image
install -m 0644 -D $BOARD_DIR/../../../../../linux/linux/arch/arm64/boot/dts/rockchip/rk3588-orangepi-5-plus.dtb $BINARIES_DIR/rk3588-orangepi-5-plus.dtb
install -m 0644 -D ${BOARD_DIR}/extlinux.conf $BINARIES_DIR/extlinux/extlinux.conf
install -m 0644 -D $BOARD_DIR/../../../../../uboot/out/u-boot.itb $BINARIES_DIR/u-boot.itb
install -m 0644 -D $BOARD_DIR/../../../../../uboot/out/idbloader.img $BINARIES_DIR/idbloader.img
