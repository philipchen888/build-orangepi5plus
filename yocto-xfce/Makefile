apply_patches:
	cd poky/build; \
	cp -rf ../../patches/resize-helper ../meta-rockchip/recipes-bsp/; \
	cp ../../patches/orangepi-5-plus.conf ../meta-rockchip/conf/machine/; \
	cp ../../patches/rockchip-defaults.inc  ../meta-rockchip/conf/machine/include/; \
	cp ../../patches/rockchip.wks ../meta-rockchip/wic/; \
	cp ../../patches/mesa_%.bbappend ../meta-rockchip/recipes-graphics/mesa/; \
	cp ../../patches/xfce-desktop-image.bb ../meta/recipes-graphics/images/; \
	cp ../../patches/bblayers.conf ./conf/; \
	cp ../../patches/local.conf ./conf/
