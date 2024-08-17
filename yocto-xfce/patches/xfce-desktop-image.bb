SUMMARY = "Basic desktop image"

IMAGE_FEATURES += "splash package-management ssh-server-openssh hwcodecs x11"

LICENSE = "MIT"

inherit core-image features_check extrausers

# let's make sure we have a good image..
REQUIRED_DISTRO_FEATURES = "pam systemd"

# make sure we boot to desktop
# by default and without x11-base in IMAGE_FEATURES we default to multi-user.target
SYSTEMD_DEFAULT_TARGET = "graphical.target"

CORE_IMAGE_BASE_INSTALL += " \
    kernel-modules \
    openbox \
    packagegroup-xfce-extended \
    lxdm \
"
