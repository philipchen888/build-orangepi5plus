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
	sudo ln -s vmlinuz-6.1.75 vmlinuz

postbuildkernel:
	cd linux-rockchip; \
	\rm -f $(BOOT); \
	mkfs.vfat -n "boot" -S 512 -C ${BOOT} 1024000; \
	mmd -i ${BOOT} ::/dtb; \
	mcopy -i ${BOOT} -s ./tmp/boot/* ::; \
