# 
# Copyright (C) 2009 Microhard Systems 
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id: Makefile,v 1.5 2011/10/19 15:41:57 Bin Exp $

include $(TOPDIR)/rules.mk

PKG_NAME:=vip-utils
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

KDIR:=$(KERNEL_BUILD_DIR)

define Package/vip-utils
  SECTION:=microhard
  CATEGORY:=Microhard Packages
  DEPENDS:=+kmod-vip-bsp
  TITLE:=Microhard vip-utils utility
endef

define Package/vip-utils/description
 This package contains the microhard home grown vip-utils utility.
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

TARGET_CFLAGS += $(FPIC) -I$(KDIR)/vip-bsp
#TARGET_LDFLAGS += -lUICommon

define Package/vip-utils/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/vip-config $(1)/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/vip $(1)/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tip $(1)/bin/
endef

$(eval $(call BuildPackage,vip-utils))
