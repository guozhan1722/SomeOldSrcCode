# 
# Copyright (C) 2009 Microhard Systems 
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id: Makefile,v 1.1.2.3 2013/07/10 16:26:38 AlexDing Exp $

include $(TOPDIR)/rules.mk

PKG_NAME:=datausemonitor
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define Package/datausemonitor
  SECTION:=microhard
  CATEGORY:=Microhard Packages
  DEPENDS:=+libuci +libpthread +libmserial
  TITLE:=Microhard carrier data usage monitor
endef

define Package/datausemonitor/description
 This package contains the microhard home grown serial/net daemon
endef

TARGET_CFLAGS += $(FPIC) -I$(PKG_BUILD_DIR)/../common-files/include/ui -I$(PKG_BUILD_DIR)/../libmserial/include -I$(PKG_BUILD_DIR)/../at_command -I$(LINUX_DIR)/../mhx-bsp\
	-I$(LINUX_DIR)/../mpci-bsp  -I$(PKG_BUILD_DIR)/../libmserial/include/phones

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)
	find $(PKG_BUILD_DIR) -name CVS | xargs rm -rf
endef

define Package/datausemonitor/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/datausemonitor $(1)/bin/
	$(INSTALL_BIN) ./files/datausagescript $(1)/bin/

	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/datausemonitord.init $(1)/etc/init.d/datausemonitord

	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/datausemonitor.conf $(1)/etc/config/datausemonitor
	$(INSTALL_DIR) $(1)/etc/carrierstat

	$(INSTALL_DIR) $(1)/www/cgi-bin/webif
	$(CP) ./files/*.sh $(1)/www/cgi-bin/webif/

endef

$(eval $(call BuildPackage,datausemonitor))
