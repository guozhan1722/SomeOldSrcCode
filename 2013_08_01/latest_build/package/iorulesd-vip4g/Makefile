# 
# Copyright (C) 2009 Microhard Systems 
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id: Makefile,v 1.2 2011/01/18 22:07:29 Jim Exp $

include $(TOPDIR)/rules.mk

PKG_NAME:=iorulesd-vip4g
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/iorulesd-vip4g
  SECTION:=microhard
  CATEGORY:=Microhard Packages
  DEPENDS:=+libuci +libpthread
  TITLE:=Microhard iorulesd-vip4g utility
endef

define Package/iorulesd-vip4g/description
 This package contains the microhard home grown VIP4G I/O Application utility.
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


define Package/iorulesd-vip4g/install
	$(INSTALL_DIR) $(1)/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/iorulesd $(1)/sbin/
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/iorules.config $(1)/etc/config/iorules
	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_BIN) ./files/iorules.init $(1)/etc/init.d/iorules
	$(INSTALL_DIR) $(1)/www/cgi-bin/webif/
	$(INSTALL_BIN) ./files/io-rules.sh $(1)/www/cgi-bin/webif/io-rules.sh

endef

$(eval $(call BuildPackage,iorulesd-vip4g))
