# 
# Copyright (C) 2009 Microhard Systems 
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id: Makefile,v 1.1.2.4 2012/10/25 18:27:29 AlexDing Exp $

include $(TOPDIR)/rules.mk

PKG_NAME:=eurd
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define Package/eurd
  SECTION:=microhard
  CATEGORY:=Microhard Packages
  DEPENDS:=+libuci +libpthread +zlib
  TITLE:=Microhard eurd/udp report
endef

define Package/eurd/description
 This package contains the microhard home grown serial/net daemon
endef

TARGET_CFLAGS += $(FPIC) -I$(PKG_BUILD_DIR)/../common-files/include/ui -I$(PKG_BUILD_DIR)/../libmserial/include -I$(PKG_BUILD_DIR)/../at_command -I$(LINUX_DIR)/../mhx-bsp\
	-I$(LINUX_DIR)/../mpci-bsp


define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)
	find $(PKG_BUILD_DIR) -name CVS | xargs rm -rf
endef

#TARGET_LDFLAGS += -lUICommon -lpthread

define Package/eurd/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/eurd_report0 $(1)/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/eurd_report1 $(1)/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/eurd_report2 $(1)/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/eurd_report3 $(1)/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/eurd_sms $(1)/bin/
	$(INSTALL_BIN) ./files/eurdscript $(1)/bin/

	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/$(PKG_NAME).init $(1)/etc/init.d/$(PKG_NAME)
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/$(PKG_NAME).config $(1)/etc/config/$(PKG_NAME)

	$(INSTALL_DIR) $(1)/www/cgi-bin/webif
	$(CP) ./files/*.sh $(1)/www/cgi-bin/webif/
endef

$(eval $(call BuildPackage,eurd))
