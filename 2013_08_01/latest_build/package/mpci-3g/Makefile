# 
# Copyright (C) 2009 Microhard Systems 
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id: Makefile,v 1.2 2011/01/18 22:07:29 Jim Exp $

include $(TOPDIR)/rules.mk

PKG_NAME:=mpci-3g
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

KDIR:=$(KERNEL_BUILD_DIR)

define Package/mpci-3g
  SECTION:=microhard
  CATEGORY:=Microhard Packages
  DEPENDS:=+kmod-mpci-bsp +i2c-tools
  TITLE:=Microhard mpci-3g utility
endef

define Package/mpci-3g/description
 This package contains the microhard home grown mpci-3g utility.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
#	if [ -e ./src ]; then \
#		cvs update ./src; \
#	else \
#		cvs co -r $(IPX20_TAG) -d src network/IPx20/apps/mhx-3g; \
#	fi
	$(CP) ./src/* $(PKG_BUILD_DIR)
	find $(PKG_BUILD_DIR) -name CVS | xargs rm -rf
endef

TARGET_CFLAGS += -I$(KDIR)/mpci-bsp
#TARGET_CFLAGS += $(FPIC) -I$(KDIR)/mpci-bsp
#TARGET_LDFLAGS += -lUICommon

define Package/mpci-3g/install
	$(INSTALL_DIR) $(1)/etc/ppp
	$(INSTALL_DATA) ./files/etc/ppp/options.l2tpd.client $(1)/etc/ppp/
	$(INSTALL_DATA) ./files/etc/ppp/options.pptpd $(1)/etc/ppp/
	$(INSTALL_DATA) ./files/etc/ppp/options.ttyS1 $(1)/etc/ppp/
	$(INSTALL_DATA) ./files/etc/ppp/options.xl2tpd $(1)/etc/ppp/
	$(INSTALL_DATA) ./files/etc/ppp/pap-secrets $(1)/etc/ppp/
	$(INSTALL_DIR) $(1)/etc/ppp/chatscripts
	$(INSTALL_DATA) ./files/etc/ppp/chatscripts/client $(1)/etc/ppp/chatscripts/
	$(INSTALL_DATA) ./files/etc/ppp/chatscripts/isp $(1)/etc/ppp/chatscripts/
	$(INSTALL_DATA) ./files/etc/ppp/chatscripts/isp_disconnect $(1)/etc/ppp/chatscripts/
	$(INSTALL_DATA) ./files/etc/ppp/chatscripts/rogers $(1)/etc/ppp/chatscripts/
	$(INSTALL_DATA) ./files/etc/ppp/chatscripts/rogers_public $(1)/etc/ppp/chatscripts/
	$(INSTALL_DATA) ./files/etc/ppp/chatscripts/windows $(1)/etc/ppp/chatscripts/
	$(INSTALL_DIR) $(1)/etc/ppp/peers
	$(INSTALL_DATA) ./files/etc/ppp/peers/isp $(1)/etc/ppp/peers/
	$(INSTALL_DATA) ./files/etc/ppp/peers/rogers $(1)/etc/ppp/peers/
	$(INSTALL_DATA) ./files/etc/ppp/peers/windows $(1)/etc/ppp/peers/
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/mpci-3g.config $(1)/etc/config/mpci-3g
	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_BIN) ./files/mpci-3g.init $(1)/etc/init.d/mpci-3g
endef

$(eval $(call BuildPackage,mpci-3g))
