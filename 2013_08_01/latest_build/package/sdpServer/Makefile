# 
# Copyright (C) 2009 Microhard Systems 
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id: Makefile,v 1.7 2011/05/10 17:23:34 Jim Exp $

include $(TOPDIR)/rules.mk

PKG_NAME:=sdpServer
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/sdpServer
  SECTION:=microhard
  CATEGORY:=Microhard Packages
  DEPENDS:=+libuci +libpthread
  TITLE:=Microhard sdpServer utility
endef

define Package/sdpServer/description
 This package contains the microhard home grown sdpServer utility.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	if [ -e ./src ]; then \
		cvs update ./src; \
	else \
		cvs co -r $(IPX20_TAG) -d src network/IPx20/apps/sdpServer; \
	fi
	$(CP) ./src/* $(PKG_BUILD_DIR)
	find $(PKG_BUILD_DIR) -name CVS | xargs rm -rf
endef

TARGET_LDFLAGS += -lpthread

define Build/InstallDev
	mkdir -p $(1)/bin
	$(CP) $(PKG_BUILD_DIR)/sdpServer $(1)/bin/
endef

define Package/sdpServer/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/sdpServer $(1)/bin/
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/sdpServer.config $(1)/etc/config/sdpServer
	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_BIN) ./files/sdpServer.init $(1)/etc/init.d/sdpServer
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DATA) ./files/sdpServer.br_maclist $(1)/etc/br_maclist
	chmod 0755 $(1)/etc/br_maclist
	$(INSTALL_DATA) ./files/sdpServer.br_meshlist $(1)/etc/br_meshlist
	chmod 0755 $(1)/etc/br_meshlist
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DATA) ./files/sdpServer.ledflash $(1)/etc/ledflash
	chmod 0755 $(1)/etc/ledflash
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DATA) ./files/sdpServer.exdiscovery $(1)/etc/exdiscovery
	chmod 0755 $(1)/etc/exdiscovery
endef

$(eval $(call BuildPackage,sdpServer))
