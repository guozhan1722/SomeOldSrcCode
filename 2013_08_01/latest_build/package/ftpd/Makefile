# 
# Copyright (C) 2009 Microhard Systems 
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id: Makefile,v 1.3 2010/07/15 14:57:27 Bob Exp $

include $(TOPDIR)/rules.mk

PKG_NAME:=ftpd
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/ftpd
  SECTION:=microhard
  CATEGORY:=Microhard Packages
  DEPENDS:=
  TITLE:=Microhard modified ftpd daemon
endef

define Package/ftpd/description
 This package contains the microhard home grown ftpd server derived from IXP4xx based IP/VIP platform.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)
	find $(PKG_BUILD_DIR) -name CVS | xargs rm -rf
endef

TARGET_CFLAGS += $(FPIC)
TARGET_LDFLAGS += -lcrypt

define Package/ftpd/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/$(PKG_NAME).init $(1)/etc/init.d/$(PKG_NAME)
	$(INSTALL_DIR) $(1)/etc/radiusclient-ng
	$(CP) ./files/radiusclient-ng/* $(1)/etc/radiusclient-ng
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ftpd $(1)/bin/ftpd
endef

$(eval $(call BuildPackage,ftpd))
