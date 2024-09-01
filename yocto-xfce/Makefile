apply_patches:
	cd poky/build; \
	cp -rf ../../patches/resize-helper ../meta-rockchip/recipes-bsp/; \
	cp ../../patches/orangepi-5-plus.conf ../meta-rockchip/conf/machine/; \
	cp ../../patches/rockchip-defaults.inc  ../meta-rockchip/conf/machine/include/; \
	cp ../../patches/rockchip.wks ../meta-rockchip/wic/; \
	cp ../../patches/xfce-desktop-image.bb ../meta/recipes-graphics/images/; \
	cp ../../patches/mesa_%.bbappend ../meta-rockchip/recipes-graphics/mesa/; \
	\rm -rf ../meta/recipes-graphics/mesa/mesa_24.0.7.bb; \
	\rm -rf ../meta/recipes-graphics/mesa/mesa-gl_24.0.7.bb; \
	cp ../../patches/mesa_24.2.0.bb ../meta/recipes-graphics/mesa/; \
	cp ../../patches/mesa-gl_24.2.0.bb ../meta/recipes-graphics/mesa/; \
	cp ../../patches/mesa.inc ../meta/recipes-graphics/mesa/; \
	cp ../../patches/vulkan-loader_1.3.275.0.bb ../meta/recipes-graphics/vulkan/; \
	cp ../../patches/bblayers.conf ./conf/; \
	cp ../../patches/local.conf ./conf/
