# 
# Copyright (C) 2009 Microhard Systems 
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id: Makefile,v 1.3 2010/08/18 19:05:56 Bob Exp $

include $(TOPDIR)/rules.mk

PKG_NAME:=mrouted
PKG_VERSION:=3.9.5
PKG_RELEASE:=1

export TARGET_CROSS

include $(INCLUDE_DIR)/package.mk

define Package/mrouted
  SECTION:=microhard
  CATEGORY:=Microhard Packages
  DEPENDS:=
  TITLE:=Microhard mrouted
endef

define Package/mrouted/description
 This package contains mrouted.
endef

define Build/Prepare
	rm -r $(PKG_BUILD_DIR)
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -a ./src/* $(PKG_BUILD_DIR)
	mkdir -p $(PKG_BUILD_DIR)/bin
	find $(PKG_BUILD_DIR) -name CVS | xargs rm -rf
endef

define Build/Compile
	$(MAKE) -C "$(PKG_BUILD_DIR)"
endef

TARGET_CFLAGS += $(FPIC)

define Package/mrouted/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/* $(1)/usr/bin/
endef

$(eval $(call BuildPackage,mrouted))
