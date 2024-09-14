#!/bin/bash -e

# Directory contains the target rootfs
TARGET_ROOTFS_DIR="binary"

if [ -e $TARGET_ROOTFS_DIR ]; then
	sudo rm -rf $TARGET_ROOTFS_DIR
fi

if [ "$ARCH" == "armhf" ]; then
	ARCH='armhf'
elif [ "$ARCH" == "arm64" ]; then
	ARCH='arm64'
else
    echo -e "\033[36m please input is: armhf or arm64...... \033[0m"
fi

if [ ! $VERSION ]; then
	VERSION="release"
fi

if [ ! -e binary-tar.tar.gz ]; then
	echo "\033[36m Run sudo lb build first \033[0m"
fi

finish() {
	sudo umount $TARGET_ROOTFS_DIR/dev
	exit -1
}
trap finish ERR

echo -e "\033[36m Extract image \033[0m"
sudo tar -xpf binary-tar.tar.gz

sudo cp -rf ../linux/linux-rockchip/tmp/lib/modules $TARGET_ROOTFS_DIR/lib
sudo cp -rf ../linux/linux-rockchip/tmp/boot/* $TARGET_ROOTFS_DIR/boot
sudo cp ../linux/patches/40_custom_uuid $TARGET_ROOTFS_DIR/boot

# overlay folder
sudo cp -rf ../overlay/* $TARGET_ROOTFS_DIR/

echo -e "\033[36m Change root.....................\033[0m"
if [ "$ARCH" == "armhf" ]; then
	sudo cp /usr/bin/qemu-arm-static $TARGET_ROOTFS_DIR/usr/bin/
elif [ "$ARCH" == "arm64"  ]; then
	sudo cp /usr/bin/qemu-aarch64-static $TARGET_ROOTFS_DIR/usr/bin/
fi
sudo mount -o bind /proc $TARGET_ROOTFS_DIR/proc
sudo mount -o bind /sys $TARGET_ROOTFS_DIR/sys
sudo mount -o bind /dev $TARGET_ROOTFS_DIR/dev
sudo mount -o bind /dev/pts $TARGET_ROOTFS_DIR/dev/pts

cat << EOF | sudo chroot $TARGET_ROOTFS_DIR

rm -rf /etc/resolv.conf
echo -e "nameserver 8.8.8.8\nnameserver 8.8.4.4" > /etc/resolv.conf
resolvconf -u
add-apt-repository -y ppa:jjriek/panfork-mesa
apt-get update
apt-get -y install mali-g610-firmware
apt-get -y dist-upgrade
apt-get -y install libmali-g610-x11
apt-get update
apt-get upgrade -y
apt-get -y dist-upgrade
apt-get install -y build-essential git wget camera-engine-rkaiq-rk3588 wiringpi-opi libwiringpi2-opi libwiringpi-opi-dev grub-efi-arm64

# Install and configure GRUB
mkdir -p /boot/efi
grub-install --target=arm64-efi --efi-directory=/boot/efi --bootloader-id=GRUB
update-grub

cp /boot/40_custom_uuid /etc/grub.d/
chmod +x /etc/grub.d/40_custom_uuid
rm -rf /boot/40_custom_uuid

# Migrate extlinux.conf to GRUB
rm -rf /boot/extlinux
cat << GRUB_EOF > /etc/default/grub
GRUB_DEFAULT="Boot from UUID"
GRUB_TIMEOUT=20
GRUB_CMDLINE_LINUX_DEFAULT="quiet"
GRUB_CMDLINE_LINUX=""
GRUB_EOF

cat << FSTAB_EOF > /etc/fstab
UUID=0c21ae25-58aa-4153-87f6-ea31b7a8e0f5 /  ext4    errors=remount-ro   0   1
UUID=95E4-6EA5  /boot/efi  vfat    umask=0077      0       1
FSTAB_EOF

update-grub

chmod o+x /usr/lib/dbus-1.0/dbus-daemon-launch-helper
chmod +x /etc/rc.local

systemctl enable rc-local
systemctl enable resize-helper
update-initramfs -c -k 6.1.75

#---------------Clean--------------
rm -rf /var/lib/apt/lists/*

EOF

sudo umount $TARGET_ROOTFS_DIR/proc
sudo umount $TARGET_ROOTFS_DIR/sys
sudo umount $TARGET_ROOTFS_DIR/dev/pts
sudo umount $TARGET_ROOTFS_DIR/dev
