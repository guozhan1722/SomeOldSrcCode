# 
# Copyright (C) 2009 Microhard Systems 
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id: Makefile,v 1.2 2011/01/18 22:07:29 Jim Exp $

include $(TOPDIR)/rules.mk

PKG_NAME:=twatchdog
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

KDIR:=$(KERNEL_BUILD_DIR)

define Package/twatchdog
  SECTION:=microhard
  CATEGORY:=Microhard Packages
  DEPENDS:=+kmod-mpci-bsp +i2c-tools +libpthread +VIP4G
  TITLE:=Microhard trafficwatchdog utility
endef

define Package/twatchdog/description
 This package contains the microhard home grown traffic watchdog utility.
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

define Package/twatchdog/install
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/twatchdog.conf $(1)/etc/config/twatchdog
	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_BIN) ./files/twatchdog.init $(1)/etc/init.d/twatchdog
	$(INSTALL_DIR) $(1)/www/cgi-bin/webif/
	$(INSTALL_BIN) ./files/www/cgi-bin/webif/carrier-trafficwatchdog.sh $(1)/www/cgi-bin/webif/carrier-trafficwatchdog.sh
	$(INSTALL_DIR) $(1)/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/t_watchdog $(1)/sbin/
endef

$(eval $(call BuildPackage,twatchdog))
