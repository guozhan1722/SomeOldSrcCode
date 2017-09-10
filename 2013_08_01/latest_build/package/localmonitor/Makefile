# 
# Copyright (C) 2009 Microhard Systems 
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id: Makefile,v 1.1.2.1 2013/06/11 16:06:37 AlexDing Exp $

include $(TOPDIR)/rules.mk

PKG_NAME:=localmonitor
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define Package/localmonitor
  SECTION:=microhard
  CATEGORY:=Microhard Packages
#  DEPENDS:=+libuci 
  TITLE:=Microhard local device monitor
endef

define Package/localmonitor/description
 This package contains the microhard home grown serial/net daemon
endef

TARGET_CFLAGS += $(FPIC) 

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)
	find $(PKG_BUILD_DIR) -name CVS | xargs rm -rf
endef

define Package/localmonitor/install
	$(INSTALL_DIR) $(1)/bin
#	$(INSTALL_BIN) $(PKG_BUILD_DIR)/localmonitor $(1)/bin/
	$(INSTALL_BIN) ./files/localmoncheck $(1)/bin/

	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/localmonitord.init $(1)/etc/init.d/localmonitord

	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/localmonitor.config $(1)/etc/config/localmonitor

	$(INSTALL_DIR) $(1)/www/cgi-bin/webif
	$(CP) ./files/*.sh $(1)/www/cgi-bin/webif/

endef

$(eval $(call BuildPackage,localmonitor))
