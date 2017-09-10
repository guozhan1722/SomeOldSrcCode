# 
# Copyright (C) 2009 Microhard Systems 
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id: Makefile,v 1.1 2010/04/14 23:00:42 Bob Exp $

include $(TOPDIR)/rules.mk

PKG_NAME:=unpackfw
PKG_RELEASE:=1

TOOLS_PATH:=$(TOPDIR)/tools
include $(INCLUDE_DIR)/package.mk

define Package/unpackfw
  SECTION:=microhard
  CATEGORY:=Microhard Packages
  DEPENDS:=
  TITLE:=Unpack Firmware utility
endef

define Package/unpackfw/description
 This package contains the utility to unpack microhard firmware
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) $(TOOLS_PATH)/firmware-utils/src/myloader.h $(PKG_BUILD_DIR)
	$(CP) $(TOOLS_PATH)/firmware-utils/src/extractmylofw.c $(PKG_BUILD_DIR)
	find $(PKG_BUILD_DIR) -name CVS | xargs rm -rf
endef

TARGET_CFLAGS += $(FPIC)

define Build/Compile
	$(TARGET_CC) $(TARGET_CFLAGS) -I$(PKG_BUILD_DIR) -o $(PKG_BUILD_DIR)/extractmylofw $(PKG_BUILD_DIR)/extractmylofw.c
endef

define Package/unpackfw/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/extractmylofw $(1)/bin/
endef

$(eval $(call BuildPackage,unpackfw))
