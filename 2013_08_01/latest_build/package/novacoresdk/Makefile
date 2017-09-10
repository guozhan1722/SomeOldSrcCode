# 
# Copyright (C) 2009 Microhard Systems 
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id: Makefile,v 1.1.2.1 2013/01/25 00:19:45 AlexDing Exp $

include $(TOPDIR)/rules.mk

PKG_NAME:=novacoresdk
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/novacoresdk
  SECTION:=microhard
  CATEGORY:=Microhard Packages
  DEPENDS:=+kmod-mpci-bsp +i2c-tools +libpthread +VIP4G
  TITLE:=Novatel Core SDK
endef

define Package/novacoresdk/description
 This package contains the Novatel Core SDK.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	cvs update ./src; \
	$(CP) ./src/* $(PKG_BUILD_DIR)
	find $(PKG_BUILD_DIR) -name CVS | xargs rm -rf
endef

define Package/novacoresdk/install
	$(INSTALL_DIR) $(1)/bin
	$(CP) $(PKG_BUILD_DIR)/gpscarrstart $(1)/bin/

	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) ./files/bin/*s*.so $(1)/usr/lib/    	
endef

$(eval $(call BuildPackage,novacoresdk))
