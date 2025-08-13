export OUT=$(PWD)/out
export BOOT=$(OUT)/boot.img
export ARCH=arm64
export CROSS_COMPILE=/usr/bin/aarch64-linux-gnu-
export BOOT_UUID=95E46EA5

clean:
	cd linux; \
	sudo make mrproper

kernel: buildkernel postbuildkernel

buildkernel:
	cd linux; \
	sudo \rm -rf tmp; mkdir tmp; mkdir -p tmp/lib; \
	sudo make mrproper; \
	make ARCH=arm64 defconfig; \
	make ARCH=arm64 all -j8; \
	sudo make ARCH=arm64 INSTALL_MOD_PATH=./tmp modules_install; \
	sudo make ARCH=arm64 INSTALL_DTBS_PATH=./tmp/boot/dtb dtbs_install; \
	sudo make ARCH=arm64 INSTALL_PATH=./tmp/boot install

postbuildkernel:
	cd linux; \
	\rm -f $(BOOT); \
	mkfs.vfat -n "boot" -i ${BOOT_UUID} -S 512 -C ${BOOT} 512000; \
	mmd -i ${BOOT} ::/dtb; \
	mcopy -i ${BOOT} -s ./tmp/boot/* ::; \
