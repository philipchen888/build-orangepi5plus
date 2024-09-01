SUMMARY = "A free implementation of the OpenGL API"
DESCRIPTION = "Mesa is an open-source implementation of the OpenGL specification - \
a system for rendering interactive 3D graphics.  \
A variety of device drivers allows Mesa to be used in many different environments \
ranging from software emulation to complete hardware acceleration for modern GPUs. \
Mesa is used as part of the overall Direct Rendering Infrastructure and X.org \
environment."

HOMEPAGE = "http://mesa3d.org"
BUGTRACKER = "https://bugs.freedesktop.org"
SECTION = "x11"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://docs/license.rst;md5=63779ec98d78d823a9dc533a0735ef10"

PE = "2"

SRC_URI = "https://mesa.freedesktop.org/archive/mesa-${PV}.tar.xz \
"

SRC_URI[sha256sum] = "c02bb72cea290f78b11895a0c95c7c92394f180d7ff66d4a762ec6950a58addf"

UPSTREAM_CHECK_GITTAGREGEX = "mesa-(?P<pver>\d+(\.\d+)+)"

#because we cannot rely on the fact that all apps will use pkgconfig,
#make eglplatform.h independent of MESA_EGL_NO_X11_HEADER
do_install:append() {
  # sed can't find EGL/eglplatform.h as it doesn't get installed when glvnd enabled.
  # So, check if EGL/eglplatform.h exists before running sed.
  if ${@bb.utils.contains('PACKAGECONFIG', 'egl', 'true', 'false', d)} && [ -f ${D}${includedir}/EGL/eglplatform.h ]; then
      sed -i -e 's/^#elif defined(__unix__) && defined(EGL_NO_X11)$/#elif defined(__unix__) \&\& defined(EGL_NO_X11) || ${@bb.utils.contains('PACKAGECONFIG', 'x11', '0', '1', d)}/' ${D}${includedir}/EGL/eglplatform.h
  fi
}

DEPENDS = "libdrm expat makedepend-native flex-native bison-native libxml2-native zlib chrpath-replacement-native python3-pyyaml-native python3-mako-native gettext-native"
DEPENDS:append:class-target = " ${@bb.utils.contains('PACKAGECONFIG', 'opencl', 'mesa-native', '', d)}"
EXTRANATIVEPATH += "chrpath-native"
PROVIDES = " \
    ${@bb.utils.contains('PACKAGECONFIG', 'opengl', 'virtual/libgl', '', d)} \
    ${@bb.utils.contains('PACKAGECONFIG', 'glvnd', 'virtual/libglx', '', d)} \
    ${@bb.utils.contains('PACKAGECONFIG', 'gles', 'virtual/libgles1 virtual/libgles2 virtual/libgles3', '', d)} \
    ${@bb.utils.contains('PACKAGECONFIG', 'egl', 'virtual/egl', '', d)} \
    ${@bb.utils.contains('PACKAGECONFIG', 'gbm', 'virtual/libgbm', '', d)} \
    virtual/mesa \
    "

inherit meson pkgconfig python3native gettext features_check

BBCLASSEXTEND = "native nativesdk"

ANY_OF_DISTRO_FEATURES = "opengl"

PLATFORMS ??= "x11"

# set the MESA_BUILD_TYPE to either 'release' (default) or 'debug'
# by default the upstream mesa sources build a debug release
# here we assume the user will want a release build by default
MESA_BUILD_TYPE ?= "release"
MESON_BUILDTYPE = "release"

EXTRA_OEMESON = " \
    -Dgallium-drivers=panfrost \
    -Dvulkan-drivers= \
    -Dllvm=disabled \
    -Dshared-glapi=enabled \
    -Dglx-read-only-text=true \
    -Dplatforms='x11' \
"

EXTRA_OEMESON:append:class-target = " ${@bb.utils.contains('PACKAGECONFIG', 'opencl', '-Dintel-clc=system', '', d)}"
EXTRA_OEMESON:append:class-native = " ${@bb.utils.contains('PACKAGECONFIG', 'opencl', '-Dintel-clc=enabled', '', d)}"

def strip_comma(s):
    return s.strip(',')

PACKAGECONFIG = " \
	gallium \
	${@bb.utils.filter('DISTRO_FEATURES', 'x11', d)} \
	${@bb.utils.contains('DISTRO_FEATURES', 'opengl', 'opengl egl gles gbm', '', d)} \
	${@bb.utils.contains('DISTRO_FEATURES', 'x11 opengl', 'dri3', '', d)} \
"

PACKAGECONFIG:append:class-native = " gallium-llvm"

# "gbm" requires "opengl"
PACKAGECONFIG[gbm] = "-Dgbm=enabled,-Dgbm=disabled"

X11_DEPS = "xorgproto virtual/libx11 libxext libxxf86vm libxdamage libxfixes xrandr"
# "x11" requires "opengl"
PACKAGECONFIG[x11] = ",-Dglx=disabled,${X11_DEPS}"
PACKAGECONFIG[dri3] = "-Ddri3=enabled, -Ddri3=disabled, xorgproto libxshmfence"

PACKAGECONFIG[opengl] = "-Dopengl=true, -Dopengl=false"
PACKAGECONFIG[glvnd] = "-Dglvnd=true, -Dglvnd=false, libglvnd"

# "gles" requires "opengl"
PACKAGECONFIG[gles] = "-Dgles1=enabled -Dgles2=enabled, -Dgles1=disabled -Dgles2=disabled"

# "egl" requires "opengl"
PACKAGECONFIG[egl] = "-Degl=enabled, -Degl=disabled"

PACKAGECONFIG[opencl] = ""
PACKAGECONFIG[broadcom] = ""
PACKAGECONFIG[etnaviv] = ""
PACKAGECONFIG[freedreno] = ""
PACKAGECONFIG[kmsro] = ""
PACKAGECONFIG[vc4] = ""
PACKAGECONFIG[v3d] = ""
PACKAGECONFIG[zink] = ""

GALLIUMDRIVERS = ""

PACKAGECONFIG[gallium] = "-Dgallium-drivers=${@strip_comma('${GALLIUMDRIVERS}')}, -Dgallium-drivers='', libdrm"
PACKAGECONFIG[gallium-llvm] = "-Dllvm=enabled -Dshared-llvm=enabled, -Dllvm=disabled, llvm llvm-native elfutils"

PACKAGECONFIG[panfrost] = ""
GALLIUMDRIVERS:append ="${@bb.utils.contains('PACKAGECONFIG', 'panfrost', ',panfrost', '', d)}"

# llvmpipe is slow if compiled with -fomit-frame-pointer (e.g. -O2)
FULL_OPTIMIZATION:append = " -fno-omit-frame-pointer"

CFLAGS:append:armv5 = " -DMISSING_64BIT_ATOMICS"
CFLAGS:append:armv6 = " -DMISSING_64BIT_ATOMICS"

# Remove the mesa dependency on mesa-dev, as mesa is empty
DEV_PKG_DEPENDENCY = ""

# Khronos documentation says that include/GLES2/gl2ext.h can be used for
# OpenGL ES 3 specification as well as for OpenGL ES 2.
# There can be applications including GLES2/gl2ext.h instead of GLES3/gl3ext.h
# meaning we should probably bring in GLES2/gl2ext.h if someone asks for
# development package of libgles3.
RDEPENDS:libgles3-mesa-dev += "libgles2-mesa-dev"

PACKAGES =+ "libegl-mesa libegl-mesa-dev \
             libgl-mesa libgl-mesa-dev \
             libglx-mesa libglx-mesa-dev \
             libglapi libglapi-dev \
             libgbm libgbm-dev \
             libgles1-mesa libgles1-mesa-dev \
             libgles2-mesa libgles2-mesa-dev \
             libgles3-mesa libgles3-mesa-dev \
             mesa-megadriver mesa-megadriver-dev \
            "

do_install:append () {
    # Drivers never need libtool .la files
    rm -f ${D}${libdir}/dri/*.la
    rm -f ${D}${libdir}/egl/*.la
    rm -f ${D}${libdir}/gallium-pipe/*.la
    rm -f ${D}${libdir}/gbm/*.la

    # libwayland-egl has been moved to wayland 1.15+
    rm -f ${D}${libdir}/libwayland-egl*
    rm -f ${D}${libdir}/pkgconfig/wayland-egl.pc
}

# For the packages that make up the OpenGL interfaces, inject variables so that
# they don't get Debian-renamed (which would remove the -mesa suffix), and
# RPROVIDEs/RCONFLICTs on the generic libgl name.
python __anonymous() {
    pkgconfig = (d.getVar('PACKAGECONFIG') or "").split()
    suffix = ""
    if "-native" in d.getVar("PN"):
        suffix = "-native"
    for p in (("egl", "libegl", "libegl1"),
              ("opengl", "libgl", "libgl1"),
              ("gles", "libgles1", "libglesv1-cm1"),
              ("gles", "libgles2", "libglesv2-2"),
              ("gles", "libgles3",)):
        if not p[0] in pkgconfig:
            continue
        mlprefix = d.getVar("MLPREFIX")
        fullp = mlprefix + p[1] + "-mesa" + suffix
        mlprefix = d.getVar("MLPREFIX")
        pkgs = " " + " ".join(mlprefix + x + suffix for x in p[1:])
        d.setVar("DEBIAN_NOAUTONAME:" + fullp, "1")
        d.appendVar("RREPLACES:" + fullp, pkgs)
        d.appendVar("RPROVIDES:" + fullp, pkgs)
        d.appendVar("RCONFLICTS:" + fullp, pkgs)

        d.appendVar("RRECOMMENDS:" + fullp, " ${MLPREFIX}mesa-megadriver" + suffix)

        # For -dev, the first element is both the Debian and original name
        fullp = mlprefix + p[1] + "-mesa-dev" + suffix
        pkgs = " " + mlprefix + p[1] + "-dev" + suffix
        d.setVar("DEBIAN_NOAUTONAME:" + fullp, "1")
        d.appendVar("RREPLACES:" + fullp, pkgs)
        d.appendVar("RPROVIDES:" + fullp, pkgs)
        d.appendVar("RCONFLICTS:" + fullp, pkgs)
}

python mesa_populate_packages() {
    pkgs = ['mesa', 'mesa-dev', 'mesa-dbg']
    for pkg in pkgs:
        d.setVar("RPROVIDES:%s" % pkg, pkg.replace("mesa", "mesa-dri", 1))
        d.setVar("RCONFLICTS:%s" % pkg, pkg.replace("mesa", "mesa-dri", 1))
        d.setVar("RREPLACES:%s" % pkg, pkg.replace("mesa", "mesa-dri", 1))

    import re
    dri_drivers_root = oe.path.join(d.getVar('PKGD'), d.getVar('libdir'), "dri")
    if os.path.isdir(dri_drivers_root):
        dri_pkgs = sorted(os.listdir(dri_drivers_root))
        lib_name = d.expand("${MLPREFIX}mesa-megadriver")
        for p in dri_pkgs:
            m = re.match(r'^(.*)_dri\.so$', p)
            if m:
                pkg_name = " ${MLPREFIX}mesa-driver-%s" % legitimize_package_name(m.group(1))
                d.appendVar("RPROVIDES:%s" % lib_name, pkg_name)
                d.appendVar("RCONFLICTS:%s" % lib_name, pkg_name)
                d.appendVar("RREPLACES:%s" % lib_name, pkg_name)

    pipe_drivers_root = os.path.join(d.getVar('libdir'), "gallium-pipe")
    do_split_packages(d, pipe_drivers_root, r'^pipe_(.*)\.so$', 'mesa-driver-pipe-%s', 'Mesa %s pipe driver', extra_depends='')
}

PACKAGESPLITFUNCS =+ "mesa_populate_packages"

PACKAGES_DYNAMIC += "^mesa-driver-.*"
PACKAGES_DYNAMIC:class-native = "^mesa-driver-.*-native"

FILES:${PN} = "${libdir}/libgallium-24.2.0.so"
FILES:mesa-megadriver = "${libdir}/dri/libdril_dri.so ${datadir}/drirc.d"
FILES:mesa-vulkan-drivers = "${libdir}/libvulkan_*.so ${libdir}/libpowervr_rogue.so ${datadir}/vulkan"
FILES:libegl-mesa = "${libdir}/libEGL*.so.* ${datadir}/glvnd/egl_vendor.d"
FILES:libgbm = "${libdir}/libgbm.so.*"
FILES:libgles1-mesa = "${libdir}/libGLESv1*.so.*"
FILES:libgles2-mesa = "${libdir}/libGLESv2.so.*"
FILES:libgl-mesa = "${libdir}/libGL.so.*"
FILES:libglx-mesa = "${libdir}/libGLX*.so.*"
FILES:libglapi = "${libdir}/libglapi.so.*"

FILES:${PN}-dev = "${libdir}/pkgconfig/dri.pc ${includedir}/vulkan ${libdir}/vdpau/*.so"
FILES:libegl-mesa-dev = "${libdir}/libEGL*.* ${includedir}/EGL ${includedir}/KHR ${libdir}/pkgconfig/egl.pc"
FILES:libgbm-dev = "${libdir}/libgbm.* ${libdir}/pkgconfig/gbm.pc ${includedir}/gbm.h"
FILES:libgl-mesa-dev = "${libdir}/libGL.* ${includedir}/GL ${libdir}/pkgconfig/gl.pc"
FILES:libglx-mesa-dev = "${libdir}/libGLX*.*"
FILES:libglapi-dev = "${libdir}/libglapi.*"
FILES:libgles1-mesa-dev = "${libdir}/libGLESv1*.* ${includedir}/GLES ${libdir}/pkgconfig/glesv1*.pc"
FILES:libgles2-mesa-dev = "${libdir}/libGLESv2.* ${includedir}/GLES2 ${libdir}/pkgconfig/glesv2.pc"
FILES:libgles3-mesa-dev = "${includedir}/GLES3"
FILES:mesa-megadriver-dev = " ${libdir}/dri/stm_dri.so ${libdir}/dri/st7735r_dri.so ${libdir}/dri/ingenic-drm_dri.so ${libdir}/dri/mali-dp_dri.so ${libdir}/dri/ili9225_dri.so ${libdir}/dri/panel-mipi-dbi_dri.so ${libdir}/dri/rcar-du_dri.so ${libdir}/dri/ssd130x_dri.so ${libdir}/dri/udl_dri.so ${libdir}/dri/sun4i-drm_dri.so ${libdir}/dri/mcde_dri.so ${libdir}/dri/rzg2l-du_dri.so ${libdir}/dri/vkms_dri.so ${libdir}/dri/panfrost_dri.so ${libdir}/dri/imx-lcdif_dri.so ${libdir}/dri/hx8357d_dri.so ${libdir}/dri/armada-drm_dri.so ${libdir}/dri/imx-dcss_dri.so ${libdir}/dri/ili9486_dri.so ${libdir}/dri/st7586_dri.so ${libdir}/dri/exynos_dri.so ${libdir}/dri/mi0283qt_dri.so ${libdir}/dri/ili9341_dri.so ${libdir}/dri/ili9163_dri.so ${libdir}/dri/sti_dri.so ${libdir}/dri/meson_dri.so ${libdir}/dri/imx-drm_dri.so ${libdir}/dri/panthor_dri.so ${libdir}/dri/rockchip_dri.so ${libdir}/dri/repaper_dri.so ${libdir}/dri/mediatek_dri.so ${libdir}/dri/kirin_dri.so ${libdir}/dri/mxsfb-drm_dri.so ${libdir}/dri/pl111_dri.so ${libdir}/dri/zynqmp-dpsub_dri.so ${libdir}/dri/gm12u320_dri.so ${libdir}/dri/hdlcd_dri.so ${libdir}/dri/komeda_dri.so"

# catch all to get all the tools and data
FILES:${PN}-tools = "${bindir} ${datadir}"
ALLOW_EMPTY:${PN}-tools = "1"

# Fix upgrade path from mesa to mesa-megadriver
RREPLACES:mesa-megadriver = "mesa"
RCONFLICTS:mesa-megadriver = "mesa"
RPROVIDES:mesa-megadriver = "mesa"
