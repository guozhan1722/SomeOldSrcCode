#
# Copyright (C) 2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=vip-bsp
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

BSP_AUTOLOAD:=vip-bsp

define KernelPackage/vip-bsp
  SUBMENU:=Other modules
  TITLE:=VIP2.0 BSP support
  DEPENDS:=
  FILES:=$(PKG_BUILD_DIR)/vip-bsp.$(LINUX_KMOD_SUFFIX)
  KCONFIG:=
  AUTOLOAD:=$(call AutoLoad,10,$(BSP_AUTOLOAD))
endef

define KernelPackage/vip-bsp/description
 Kernel module to support BSP for VIP2.0.
endef

EXTRA_KCONFIG:= \
	CONFIG_VIP_BSP=m

EXTRA_CFLAGS:= \
	$(patsubst CONFIG_%, -DCONFIG_%=1, $(patsubst %=m,%,$(filter %=m,$(EXTRA_KCONFIG)))) \
	$(patsubst CONFIG_%, -DCONFIG_%=1, $(patsubst %=y,%,$(filter %=y,$(EXTRA_KCONFIG))))

MAKE_OPTS:= \
	ARCH="$(LINUX_KARCH)" \
	CROSS_COMPILE="$(TARGET_CROSS)" \
	SUBDIRS="$(PKG_BUILD_DIR)" \
	EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
	$(EXTRA_KCONFIG)

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		$(MAKE_OPTS) \
		modules
endef

$(eval $(call KernelPackage,vip-bsp))

