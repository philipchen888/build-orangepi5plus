export ARCH=arm64
export CROSS_COMPILE=/usr/bin/aarch64-linux-gnu-
export BOOT_UUID=95E46EA5

clean:
	cd linux; \
	sudo make mrproper

kernel: buildkernel

buildkernel:
	cp ./patches/config-6.1.0-1020-rockchip ./linux/arch/arm64/configs/defconfig
	cd linux; \
	sudo \rm -rf tmp; mkdir tmp; mkdir -p tmp/lib; \
	sudo make mrproper; \
	make ARCH=arm64 defconfig; \
	make ARCH=arm64 all -j8; \
	sudo make ARCH=arm64 INSTALL_MOD_PATH=./tmp modules_install; \
	sudo make ARCH=arm64 INSTALL_DTBS_PATH=./tmp/boot/dtb dtbs_install; \
	sudo make ARCH=arm64 INSTALL_PATH=./tmp/boot install;
	cd linux/tmp/boot; \
	sudo fdtoverlay -i ./dtb/rockchip/rk3588-orangepi-5-plus.dtb -o ./dtb/opi5plus.dtb ./dtb/rockchip/overlay/rk3588-uart3-m1.dtbo ./dtb/rockchip/overlay/rk3588-pwm14-m2.dtbo ./dtb/rockchip/overlay/rk3588-spi0-m2-cs0-spidev.dtbo ./dtb/rockchip/overlay/rk3588-i2c2-m0.dtbo; \
	sudo mv ./dtb/opi5plus.dtb ./dtb/rockchip/rk3588-orangepi-5-plus.dtb;
