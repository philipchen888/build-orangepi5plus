export OUT=$(PWD)/out
export BOOT=$(OUT)/boot.img
export ARCH=arm64
export CROSS_COMPILE=/usr/local/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-

clean:
	cd linux-rockchip; \
	sudo make mrproper

kernel: buildkernel postbuildkernel

buildkernel:
	cp ./patches/config-6.1.0-1020-rockchip ./linux-rockchip/arch/arm64/configs/defconfig
	cd linux-rockchip; \
	sudo \rm -rf tmp; mkdir tmp; mkdir -p tmp/lib; \
	sudo make mrproper; \
	make ARCH=arm64 defconfig; \
	make ARCH=arm64 -j8; \
	sudo make ARCH=arm64 INSTALL_MOD_PATH=./tmp modules_install; \
	sudo make ARCH=arm64 INSTALL_DTBS_PATH=./tmp/boot/dtb dtbs_install; \
	sudo make ARCH=arm64 INSTALL_PATH=./tmp/boot install; \
	sudo cp -r ../patches/extlinux ./tmp/boot
	cd linux-rockchip/tmp/boot; \
	sudo touch initrd.img-6.1.75; \
	sudo ln -s initrd.img-6.1.75 initrd.img; \
	sudo ln -s vmlinuz-6.1.75 vmlinuz;

postbuildkernel:
	cd linux-rockchip/tmp/boot; \
	sudo fdtoverlay -i ./dtb/rockchip/rk3588-orangepi-5-plus.dtb -o ./dtb/opi5plus.dtb ./dtb/rockchip/overlay/rk3588-uart3-m1.dtbo ./dtb/rockchip/overlay/rk3588-pwm14-m2.dtbo ./dtb/rockchip/overlay/rk3588-spi0-m2-cs0-spidev.dtbo ./dtb/rockchip/overlay/rk3588-i2c2-m0.dtbo;
	cd linux-rockchip; \
	\rm -f $(BOOT); \
	mkfs.vfat -n "boot" -S 512 -C ${BOOT} 1024000; \
	mmd -i ${BOOT} ::/dtb; \
	mcopy -i ${BOOT} -s ./tmp/boot/* ::; \
