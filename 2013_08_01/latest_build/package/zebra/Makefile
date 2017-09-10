# 
# Copyright (C) 2009 Microhard Systems 
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id: Makefile,v 1.3 2010/08/18 19:05:56 Bob Exp $

include $(TOPDIR)/rules.mk

PKG_NAME:=zebra
PKG_VERSION:=0.95a
PKG_RELEASE:=1

export TARGET_CROSS

include $(INCLUDE_DIR)/package.mk

define Package/zebra
  SECTION:=microhard
  CATEGORY:=Microhard Packages
  DEPENDS:=
  TITLE:=Microhard zebra
endef

define Package/zebra/description
 This package contains zebra.
endef

define Build/Prepare
	rm -r $(PKG_BUILD_DIR)
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -a ./src/* $(PKG_BUILD_DIR)
	find $(PKG_BUILD_DIR) -name CVS | xargs rm -rf
endef

define Build/Compile
	$(MAKE) -C "$(PKG_BUILD_DIR)"
endef

TARGET_CFLAGS += $(FPIC)

define Package/zebra/install
	$(INSTALL_DIR) $(1)/usr/local/zebra
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/zebra/zebra $(1)/usr/local/zebra/
	#$(INSTALL_BIN) $(PKG_BUILD_DIR)/bgpd/bgpd $(1)/usr/local/zebra/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ripd/ripd $(1)/usr/local/zebra/
	#$(INSTALL_BIN) $(PKG_BUILD_DIR)/ospfd/ospfd $(1)/usr/local/zebra/
	$(INSTALL_DIR) $(1)/usr/local/etc
	#$(INSTALL_DATA) $(PKG_BUILD_DIR)/zebra/zebra.conf.sample $(1)/usr/local/etc
	#$(INSTALL_DATA) $(PKG_BUILD_DIR)/bgpd/bgpd.conf.sample $(1)/usr/local/etc
	#$(INSTALL_DATA) $(PKG_BUILD_DIR)/bgpd/bgpd.conf.sample2 $(1)/usr/local/etc
	#$(INSTALL_DATA) $(PKG_BUILD_DIR)/ripd/ripd.conf.sample $(1)/usr/local/etc
	#$(INSTALL_DATA) $(PKG_BUILD_DIR)/ospfd/ospfd.conf.sample $(1)/usr/local/etc
endef

$(eval $(call BuildPackage,zebra))
