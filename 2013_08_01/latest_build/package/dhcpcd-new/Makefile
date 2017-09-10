# 
# Copyright (C) 2009 Microhard Systems 
#
#
# $Id: Makefile,v 1.1 2010/04/07 15:27:46 Bob Exp $

include $(TOPDIR)/rules.mk

PKG_NAME:=dhcpcd-new
PKG_RELEASE:=1

ifeq ($(IPX20_TAG), )
IPX20_TAG:=openwrt_port
endif

PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(INCLUDE_DIR)/package.mk

define Package/dhcpcd-new
  SECTION:=net
  CATEGORY:=Network
  TITLE:=DHCP client for automatically configuring IPv4 networking
endef

define Package/dhcpcd-new/description
	This version of dhcpcd is based on version 1.3.20 and modified by Microhard Inc.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	if [ -e ./src ]; then \
		cvs update ./src; \
	else \
		cvs co -r $(IPX20_TAG) -d src network/IPx20/apps/dhcpcd-new; \
	fi
	$(CP) ./src/* $(PKG_BUILD_DIR)
	find $(PKG_BUILD_DIR) -name CVS | xargs rm -rf
endef

TARGET_CFLAGS += $(FPIC)

define Package/dhcpcd-new/install
	$(INSTALL_DIR) $(1)/sbin
	$(CP) $(PKG_BUILD_DIR)/dhcpcd $(1)/sbin/
endef

$(eval $(call BuildPackage,dhcpcd-new))

