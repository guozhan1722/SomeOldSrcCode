# 
# Copyright (C) 2009 Microhard Systems 
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id: Makefile,v 1.4 2011/10/12 21:10:29 Larry Exp $

include $(TOPDIR)/rules.mk

PKG_NAME:=smodem
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

KDIR:=$(KERNEL_BUILD_DIR)

define Package/smodem
  SECTION:=microhard
  CATEGORY:=Microhard Packages
  DEPENDS:=+kmod-vip-bsp
  TITLE:=Microhard smodem utility
endef

define Package/smodem/description
 This package contains the microhard home grown smodem utility.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
#	if [ -e ./src ]; then \
#		cvs update ./src; \
#	else \
#		cvs co -r $(IPX20_TAG) -d src network/IPx20/apps/mhx-utils; \
#	fi
	$(CP) ./src/* $(PKG_BUILD_DIR)
	$(CP) ./files/* $(PKG_BUILD_DIR)
	find $(PKG_BUILD_DIR) -name CVS | xargs rm -rf
endef

TARGET_CFLAGS += $(FPIC) -I$(KDIR)/vip-bsp
#TARGET_LDFLAGS += -lUICommon

define Package/smodem/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/smodem $(1)/etc/config/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/smodem.init $(1)/etc/init.d/smodem
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mtip $(1)/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mdtip $(1)/bin/
endef

$(eval $(call BuildPackage,smodem))
