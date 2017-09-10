#
# Copyright (C) 2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=adxl34x
PKG_RELEASE:=1
PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)


include $(INCLUDE_DIR)/package.mk

ADXL34X_AUTOLOAD:=adxl34x adxl34x-i2c

define KernelPackage/adxl34x
  SUBMENU:=Other modules
  TITLE:=Accelerometer adxl34x support
  DEPENDS:=
  FILES:=$(PKG_BUILD_DIR)/adxl34x.$(LINUX_KMOD_SUFFIX) $(PKG_BUILD_DIR)/adxl34x-i2c.$(LINUX_KMOD_SUFFIX)
  KCONFIG:=
  AUTOLOAD:=$(call AutoLoad,70,$(ADXL34X_AUTOLOAD))
endef

define KernelPackage/adxl34x/description
 Kernel module to support Accelerometer adxl34x.
endef

EXTRA_KCONFIG:= \
	CONFIG_INPUT_ADXL34X=m \
	CONFIG_INPUT_ADXL34X_I2C=m

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

$(eval $(call KernelPackage,adxl34x))

