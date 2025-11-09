export ARCH=arm64
export CROSS_COMPILE=/usr/bin/aarch64-linux-gnu-
export BOOT_UUID=95E46EA5

clean:
	cd linux; \
	sudo make mrproper

kernel: buildkernel

buildkernel:
	cd linux; \
	sudo \rm -rf tmp; mkdir tmp; mkdir -p tmp/lib; \
	sudo make mrproper; \
	make defconfig; \
	make all -j8; \
	sudo make ARCH=arm64 INSTALL_MOD_PATH=./tmp modules_install; \
	sudo make ARCH=arm64 INSTALL_DTBS_PATH=./tmp/boot/dtb dtbs_install; \
	sudo make ARCH=arm64 INSTALL_PATH=./tmp/boot install
