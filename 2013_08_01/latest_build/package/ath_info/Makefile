#
# Copyright (C) 2007 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id: Makefile,v 1.1 2010/11/04 22:33:00 Jim Exp $

include $(TOPDIR)/rules.mk

PKG_NAME:=ath_info
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

KDIR:=$(KERNEL_BUILD_DIR)

define Package/ath_info
  SECTION:=net
  CATEGORY:=Network
#  DEPENDS:=kmod-madwifi
  TITLE:=Atheros wifi card utility
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
#	if [ -e ./src ]; then \
#		cvs update ./src; \
#	else \
#		cvs co -r $(IPX20_TAG) -d src network/IPx20/apps/mhx-utils; \
#	fi
	$(CP) ./src/* $(PKG_BUILD_DIR)
	find $(PKG_BUILD_DIR) -name CVS | xargs rm -rf
endef

#define Build/Compile
#	$(MAKE) -C $(PKG_BUILD_DIR) \
#               $(TARGET_CONFIGURE_OPTS) \
#               CFLAGS="$(TARGET_CFLAGS) $(TARGET_CPPFLAGS) -Wall" \
#               all
#endef

define Package/ath_info/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ath_info $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,ath_info))

