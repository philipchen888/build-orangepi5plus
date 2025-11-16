export OUT=$(PWD)/out
export BOOT=$(OUT)/boot.img
export ARCH=arm64
export CROSS_COMPILE=/usr/bin/aarch64-linux-gnu-
export BOOT_UUID=95E46EA5

kernel: buildkernel postbuildkernel

clean:
	cd linux; \
	sudo make mrproper

buildkernel:
	cd linux; \
	sudo \rm -rf tmp; mkdir tmp; mkdir -p tmp/lib; \
	sudo make mrproper; \
	make defconfig; \
	make all -j8; \
	sudo make ARCH=arm64 INSTALL_MOD_PATH=./tmp modules_install; \
	sudo make ARCH=arm64 INSTALL_DTBS_PATH=./tmp/boot/dtb dtbs_install; \
	sudo make ARCH=arm64 INSTALL_PATH=./tmp/boot install

postbuildkernel:
	cd linux; \
	\rm -f $(BOOT); \
	mkfs.vfat -n "boot" -i ${BOOT_UUID} -S 512 -C ${BOOT} 102400; \
	mmd -i ${BOOT} ::/extlinux; \
	mcopy -i ${BOOT} -s ../rk3588.conf ::/extlinux/extlinux.conf; \
	mcopy -i ${BOOT} -s ./arch/arm64/boot/Image ::; \
	mcopy -i ${BOOT} -s ./arch/arm64/boot/dts/rockchip/rk3588-orangepi-5-plus.dtb ::
