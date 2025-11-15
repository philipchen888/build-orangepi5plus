export OUT=$(PWD)/out
export ARCH=arm64
export CROSS_COMPILE=/usr/bin/aarch64-linux-gnu-
export ROCKCHIP_TPL=$(PWD)/rkbin/bin/rk35/rk3588_ddr_lp4_2112MHz_lp5_2736MHz_v1.11.bin
export BL31=$(PWD)/rkbin/bin/rk35/rk3588_bl31_v1.38.elf

u-boot: builduboot

clean:
	cd u-boot; \
	make clean

builduboot:
	\rm -rf out; mkdir out;
	cd u-boot; \
	make clean; \
	make ARCH=arm64 orangepi-5-plus-rk3588_defconfig; \
	make -j$(nproc)
	cp ./u-boot/idbloader.img ./out
	cp ./u-boot/u-boot.itb ./out
