# 
# Copyright (C) 2009 Microhard Systems 
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id: Makefile,v 1.1.2.5 2013/07/24 15:33:11 AlexDing Exp $

include $(TOPDIR)/rules.mk

PKG_NAME:=msmscomd
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define Package/msmscomd
  SECTION:=microhard
  CATEGORY:=Microhard Packages
  DEPENDS:=+libuci +libpthread +libmserial
  TITLE:=Microhard msmscomd/sms
endef

define Package/msmscomd/description
 This package contains the microhard home grown serial/net daemon
endef

TARGET_CFLAGS += $(FPIC) -I$(PKG_BUILD_DIR)/../common-files/include/ui -I$(PKG_BUILD_DIR)/../libmserial/include -I$(PKG_BUILD_DIR)/../at_command -I$(LINUX_DIR)/../mhx-bsp\
	-I$(LINUX_DIR)/../mpci-bsp  -I$(PKG_BUILD_DIR)/../libmserial/include/phones

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)
	find $(PKG_BUILD_DIR) -name CVS | xargs rm -rf
endef

define Package/msmscomd/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/msmscom $(1)/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/msmsif $(1)/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/salert $(1)/bin/
	$(INSTALL_BIN) ./files/msmscomscript $(1)/bin/
	$(INSTALL_BIN) ./files/msmscomrun $(1)/bin/

	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/$(PKG_NAME).init $(1)/etc/init.d/$(PKG_NAME)
	$(INSTALL_BIN) ./files/salertd.init $(1)/etc/init.d/salertd

	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/$(PKG_NAME).conf $(1)/etc/config/$(PKG_NAME)
	$(INSTALL_DATA) ./files/salertd.conf $(1)/etc/config/salertd

	$(INSTALL_DIR) $(1)/www/cgi-bin/webif
	$(CP) ./files/*.sh $(1)/www/cgi-bin/webif/

endef

$(eval $(call BuildPackage,msmscomd))
