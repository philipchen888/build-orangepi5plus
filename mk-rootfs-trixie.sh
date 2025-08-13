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

if [ ! -e live-image-arm64.tar.tar.gz ]; then
	echo "\033[36m Run sudo lb build first \033[0m"
fi

finish() {
	sudo umount -lf $TARGET_ROOTFS_DIR/proc || true
	sudo umount -lf $TARGET_ROOTFS_DIR/sys || true
	sudo umount -lf $TARGET_ROOTFS_DIR/dev/pts || true
	sudo umount -lf $TARGET_ROOTFS_DIR/dev || true
	exit -1
}
trap finish ERR

echo -e "\033[36m Extract image \033[0m"
sudo tar -xpf live-image-arm64.tar.tar.gz

sudo cp -rf ../kernel/linux-rockchip/tmp/lib/modules $TARGET_ROOTFS_DIR/lib
sudo cp -rf ../kernel/linux-rockchip/tmp/boot/* $TARGET_ROOTFS_DIR/boot
export KERNEL_VERSION=$(ls $TARGET_ROOTFS_DIR/boot/vmlinuz-* 2>/dev/null | sed 's|.*/vmlinuz-||' | sort -V | tail -n 1)
echo $KERNEL_VERSION
sudo sed -e "s/6.16.0/$KERNEL_VERSION/g" < ../kernel/patches/40_custom_uuid | sudo tee $TARGET_ROOTFS_DIR/boot/40_custom_uuid > /dev/null
cat $TARGET_ROOTFS_DIR/boot/40_custom_uuid
# for key in AEBDF4819BE21867 9BDB3D89CE49EC21 8065BE1FC67AABDE F02122ECF25FB4D7; do gpg --keyserver keyserver.ubuntu.com --recv-keys $key && gpg --export $key | sudo tee -a $TARGET_ROOTFS_DIR/etc/apt/trusted.gpg.d/custom-keys.gpg > /dev/null; done

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
apt-get update
\rm -rf /etc/initramfs/post-update.d/z50-raspi-firmware
apt-get update
apt-get upgrade -y
apt-get dist-upgrade -y
apt-get install -y build-essential git wget v4l-utils grub-efi-arm64 zstd e2fsprogs

# Install and configure GRUB
mkdir -p /boot/efi
grub-install --target=arm64-efi --efi-directory=/boot/efi --bootloader-id=GRUB
update-grub

cp /boot/40_custom_uuid /etc/grub.d/
chmod +x /etc/grub.d/40_custom_uuid
rm -rf /boot/40_custom_uuid

# Fix mouse lagging issue
cat << MOUSE_EOF >> /etc/environment
MUTTER_DEBUG_ENABLE_ATOMIC_KMS=0
MUTTER_DEBUG_FORCE_KMS_MODE=simple
CLUTTER_PAINT=disable-dynamic-max-render-time
MOUSE_EOF

cat << GRUB_EOF > /etc/default/grub
GRUB_DEFAULT="Boot from UUID"
GRUB_TIMEOUT=5
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

# Create the linaro user account
/usr/sbin/useradd -d /home/linaro -G adm,sudo,video -m -N -u 29999 linaro
echo -e "linaro:linaro" | chpasswd
echo -e "linaro-alip" | tee /etc/hostname

systemctl enable rc-local
systemctl enable resize-helper
systemctl enable bluetooth.service
chsh -s /bin/bash linaro
update-initramfs -c -k $KERNEL_VERSION
sync

#---------------Clean--------------
rm -rf /var/lib/apt/lists/*
sync
EOF

sudo umount -lf $TARGET_ROOTFS_DIR/proc || true
sudo umount -lf $TARGET_ROOTFS_DIR/sys || true
sudo umount -lf $TARGET_ROOTFS_DIR/dev/pts || true
sudo umount -lf $TARGET_ROOTFS_DIR/dev || true
sync
