export BOARD=orangepi5plus
export LB_BOOTSTRAP_INCLUDE="apt-transport-https"
export ARCH=arm64
export DISTRIBUTION := resolute
export CROSS_COMPILE=/usr/bin/aarch64-linux-gnu-

rootfs: clean prebuild buildrootfs postbuild mkimage mkespimage

clean:
	sudo lb clean --purge
	sudo \rm -rf config .build rootfs local

cleanall:
	sudo lb clean --purge
	\rm -rf config .build rootfs local
	\rm -rf linaro-rootfs.img

prebuild:
	lb config \
	--apt-indices false \
	--apt-recommends false \
	--apt-secure false \
	--architectures arm64 \
	--archive-areas 'main restricted universe multiverse' \
	--backports false \
	--binary-filesystem ext4 \
	--binary-images tar \
	--bootappend-live "hostname=linaro-alip username=linaro" \
	--bootstrap-qemu-arch arm64 \
	--bootstrap-qemu-static /usr/bin/qemu-aarch64 \
	--cache false \
	--chroot-filesystem none \
	--compression gzip \
	--distribution questing \
	--gzip-options '-9 --rsyncable' \
	--keyring-packages ubuntu-keyring \
	--linux-flavours rockchip \
	--linux-packages none \
	--parent-mirror-binary 'http://mirrors.mit.edu/ubuntu-ports/' \
	--parent-mirror-binary-security 'http://mirrors.mit.edu/ubuntu-ports/' \
	--parent-mirror-bootstrap 'http://mirrors.mit.edu/ubuntu-ports/' \
	--parent-mirror-chroot 'http://mirrors.mit.edu/ubuntu-ports/' \
	--parent-mirror-chroot-security 'http://mirrors.mit.edu/ubuntu-ports/' \
	--mode ubuntu \
	--security true \
	--system normal

buildrootfs:
	\cp -rf $(PWD)/../customization/* config/
	cp linaro.list.chroot config/package-lists/linaro.list.chroot
	sudo lb build

postbuild:
	ARCH=arm64 $(PWD)/../mk-rootfs-ubuntu.sh

mkimage:
	$(PWD)/../mk-image.sh

mkespimage:
	DISTRIBUTION=questing $(PWD)/../mk-espimage.sh
