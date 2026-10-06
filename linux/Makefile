export OUT=$(PWD)/out
export ARCH=arm64
export CROSS_COMPILE=/usr/bin/aarch64-linux-gnu-

kernel: buildkernel buildkernel2

clean:
	cd linux; \
	sudo make mrproper

buildkernel:
	cd linux; \
	sudo make mrproper; \
	make defconfig; \
	make all -j8;

buildkernel2:
	cd linux; \
	sudo \rm -rf tmp; mkdir tmp; mkdir -p tmp/lib; \
	KVER=$$(make ARCH=arm64 -s kernelrelease); \
	sudo make ARCH=arm64 INSTALL_MOD_PATH=./tmp modules_install; \
	sudo make ARCH=arm64 INSTALL_DTBS_PATH=./tmp/boot/dtb dtbs_install; \
	sudo cp System.map ./tmp/boot/System.map-$$KVER; \
	sudo cp .config ./tmp/boot/config-$$KVER; \
	gzip -c arch/arm64/boot/Image | sudo tee ./tmp/boot/vmlinuz-$$KVER > /dev/null; \
	file ./tmp/boot/vmlinuz-$$KVER
