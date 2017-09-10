#
# Copyright (C) 2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=GobiSerial
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

GOBI_AUTOLOAD:= \
	usbserial \
	GobiSerial

define KernelPackage/GobiSerial
  SUBMENU:=Other modules
  TITLE:=Radio kernel driver modules support
  DEPENDS:=
  FILES:= \
	$(PKG_BUILD_DIR)/GobiSerial.$(LINUX_KMOD_SUFFIX) \

  KCONFIG:=
  AUTOLOAD:=$(call AutoLoad,70,$(GOBI_AUTOLOAD))
endef

define KernelPackage/GobiSerial/description
 Kernel modules to support GobiSerial radio network.
endef

EXTRA_KCONFIG:= \
	CONFIG_NANOCPU_RADIO=m

EXTRA_CFLAGS:= \
	$(patsubst CONFIG_%, -DCONFIG_%=1, -DCCMP_REKEY=1, $(patsubst %=m,%,$(filter %=m,$(EXTRA_KCONFIG)))) \
	$(patsubst CONFIG_%, -DCONFIG_%=1, -DCCMP_REKEY=1, $(patsubst %=y,%,$(filter %=y,$(EXTRA_KCONFIG))))

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

$(eval $(call KernelPackage,GobiSerial))

