export BOARD=orangepi5plus
export LB_BOOTSTRAP_INCLUDE="apt-transport-https gnupg"
export ARCH=arm64
export CROSS_COMPILE=/usr/local/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-

rootfs: clean prebuild buildrootfs postbuild mkimage mkespimage

clean:
	sudo lb clean --purge
	sudo \rm -rf config .build rootfs local

cleanall:
	sudo lb clean --purge
	\rm -rf config .build rootfs local
	\rm -rf linaro-rootfs.img linaro-basicrootfs.img

prebuild:
	lb config \
	--apt-indices false \
	--apt-recommends false \
	--apt-secure false \
	--architectures arm64 \
	--archive-areas 'main contrib non-free-firmware' \
	--backports false \
	--binary-filesystem ext4 \
	--binary-images tar \
	--bootappend-live "hostname=linaro-alip username=linaro" \
	--bootstrap-qemu-arch arm64 \
	--bootstrap-qemu-static /usr/bin/qemu-aarch64-static \
	--cache false \
	--chroot-filesystem none \
	--compression gzip \
	--debootstrap-options "--variant=minbase --include=apt-transport-https,gnupg" \
	--distribution bookworm \
	--gzip-options '-9 --rsyncable' \
	--linux-flavours none \
	--linux-packages none \
	--parent-mirror-binary 'http://ftp.us.debian.org/debian/' \
	--parent-mirror-binary-security 'http://security.debian.org/debian-security/' \
	--parent-mirror-bootstrap 'http://ftp.us.debian.org/debian/' \
	--parent-mirror-chroot 'http://ftp.us.debian.org/debian/' \
	--parent-mirror-chroot-security 'http://security.debian.org/debian-security/' \
	--mode debian \
	--security true \
	--system normal

buildrootfs:
	\cp -rf $(PWD)/../customization/* config/
	cp linaro.list.chroot config/package-lists/linaro.list.chroot
	cp extra-ppas.list.chroot config/archives/extra-ppas.list.chroot
	cp extra-ppas.list.chroot config/archives/extra-ppas.list.binary
	cp extra-ppas.key.chroot  config/archives/extra-ppas.key.chroot
	cp extra-ppas.key.chroot  config/archives/extra-ppas.key.binary
	cp extra-ppas.pref.chroot config/archives/extra-ppas.pref.chroot
	cp config/hooks/normal/* config/hooks/
	sudo lb build

postbuild:
	ARCH=arm64 $(PWD)/../mk-rootfs-bookworm.sh

mkimage:
	$(PWD)/../mk-image.sh

mkespimage:
	$(PWD)/../mk-espimage.sh
