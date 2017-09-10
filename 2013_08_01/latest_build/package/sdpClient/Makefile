# 
# Copyright (C) 2009 Microhard Systems 
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id: Makefile,v 1.2 2011/03/03 16:55:56 Jim Exp $

include $(TOPDIR)/rules.mk

PKG_NAME:=sdpClient
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/sdpClient
  SECTION:=microhard
  CATEGORY:=Microhard Packages
  DEPENDS:=+libuci +libpthread
  TITLE:=Microhard sdpClient utility
endef

define Package/sdpClient/description
 This package contains the microhard home grown sdpClient utility.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	if [ -e ./src ]; then \
		cvs update ./src; \
	else \
		cvs co -r $(IPX20_TAG) -d src network/IPx20/apps/sdpClient; \
	fi
	$(CP) ./src/* $(PKG_BUILD_DIR)
	find $(PKG_BUILD_DIR) -name CVS | xargs rm -rf
endef

TARGET_LDFLAGS += -lpthread

define Build/InstallDev
	mkdir -p $(1)/bin
	$(CP) $(PKG_BUILD_DIR)/sdpClient $(1)/bin/
endef

define Package/sdpClient/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/sdpClient $(1)/bin/
endef

$(eval $(call BuildPackage,sdpClient))
