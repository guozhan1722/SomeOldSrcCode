#
# Copyright (C) 2006-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libmserial
PKG_RELEASE:=3

PKG_FIXUP:=libtool
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libmserial
  SECTION:=microhard
  CATEGORY:=Microhard Packages
  TITLE:=Microhard library for communication and operation with the H24 module
endef

define Package/libmserial/description
	libmserial is a suite of programs linked against a backend
	library that allows communication with the H24 module. 
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)
endef

define Build/InstallDev
	mkdir -p $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libmserial.so.3.5.0 $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libmserial.so.3 $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libmserial.so $(1)/usr/lib
endef

define Package/libmserial/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/gnokii $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/gnokiid $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/mgnokiidev $(1)/usr/sbin
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libmserial.so.3.5.0 $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libmserial.so.3 $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libmserial.so $(1)/usr/lib
endef

$(eval $(call BuildPackage,libmserial))
