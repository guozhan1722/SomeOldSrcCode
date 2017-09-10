# 
# Copyright (C) 2009 Microhard Systems 
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id: Makefile 12225 2008-08-06 22:10:29Z bobs $

include $(TOPDIR)/rules.mk

PKG_NAME:=rmqctl
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

export TARGET_CROSS

include $(INCLUDE_DIR)/package.mk

define Package/rmqctl
  SECTION:=microhard
  CATEGORY:=Microhard Packages
  DEPENDS:=
  TITLE:=Microhard rmqctl
endef

define Package/rmqctl/description
 This package contains route mcast qos control utility.
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

define Package/rmqctl/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ripconfread $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/routerip $(1)/etc/init.d/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mconfread $(1)/etc/init.d/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/qosact $(1)/etc/init.d/
	$(INSTALL_DIR) $(1)/lib/qos
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/generate.sh $(1)/lib/qos/
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/route $(1)/etc/config/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/mcast $(1)/etc/config/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/qos $(1)/etc/config/	
endef

$(eval $(call BuildPackage,rmqctl))
