# 
# Copyright (C) 2009 Microhard Systems 
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id: Makefile,v 1.2.4.1 2012/11/12 23:34:45 Daniel Exp $

include $(TOPDIR)/rules.mk

PKG_NAME:=mpci-utils
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

KDIR:=$(KERNEL_BUILD_DIR)

define Package/mpci-utils
  SECTION:=microhard
  CATEGORY:=Microhard Packages
  DEPENDS:=+kmod-mpci-bsp +i2c-tools
  TITLE:=Microhard mpci-utils utility
endef

define Package/mpci-utils/description
 This package contains the microhard home grown mpci-utils utility.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
#	if [ -e ./src ]; then \
#		cvs update ./src; \
#	else \
#		cvs co -r $(IPX20_TAG) -d src network/IPx20/apps/mhx-utils; \
#	fi
	$(CP) ./src/* $(PKG_BUILD_DIR)
	find $(PKG_BUILD_DIR) -name CVS | xargs rm -rf
endef

TARGET_CFLAGS += -I$(KDIR)/mpci-bsp
#TARGET_CFLAGS += $(FPIC) -I$(KDIR)/mpci-bsp
#TARGET_LDFLAGS += -lUICommon

define Package/mpci-utils/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mpci $(1)/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tip $(1)/bin/
	$(INSTALL_DIR) $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/Bin_txt $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,mpci-utils))
