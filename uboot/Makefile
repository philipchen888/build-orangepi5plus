export OUT=$(PWD)/out
export ARCH=arm64
export CROSS_COMPILE=/usr/local/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-
export ROCKCHIP_TPL=$(PWD)/rkbin/bin/rk35/rk3588_ddr_lp4_2112MHz_lp5_2400MHz_v1.16.bin
export BL31=$(PWD)/rkbin/bin/rk35/rk3588_bl31_v1.45.elf

u-boot: builduboot

clean:
	cd u-boot; \
	make clean

builduboot:
	\rm -rf out; mkdir out;
	cd u-boot; \
	make clean; \
	make ARCH=arm64 orangepi-5-plus-rk3588_defconfig; \
	make -j$(nproc) HOSTLDLIBS_mkimage="-lssl -lcrypto"
	cp ./u-boot/u-boot-rockchip-spi.bin ./out/u-boot-v2024.07-orangepi5-plus-spi.bin

