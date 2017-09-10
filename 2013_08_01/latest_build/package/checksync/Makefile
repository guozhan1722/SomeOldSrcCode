# 
# Copyright (C) 2009 Microhard Systems 
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id: Makefile,v 1.1.4.2 2013/05/15 19:53:23 AlexDing Exp $

include $(TOPDIR)/rules.mk

PKG_NAME:=checksync
PKG_RELEASE:=1

#ifeq ($(IPX20_TAG), )
#IPX20_TAG:=nano_cell
#endif

include $(INCLUDE_DIR)/package.mk

define Package/checksync
  SECTION:=microhard
  CATEGORY:=Microhard Packages
  DEPENDS:=+libuci
  TITLE:=Microhard checksync utility
endef

define Package/checksync/description
 This package contains the microhard home grown checksync utility.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
#	if [ -e ./src ]; then \
#		cvs update ./src; \
#	else \
#		cvs co -r $(IPX20_TAG) -d src network/IPx20/apps/checksync; \
#	fi
	$(CP) ./src/* $(PKG_BUILD_DIR)
	find $(PKG_BUILD_DIR) -name CVS | xargs rm -rf
endef

TARGET_CFLAGS += $(FPIC) -I$(PKG_BUILD_DIR)/../common-files/include/ui \
	$(if $(CONFIG_TARGET_ar71xx_VIP4G),-DVIP4G_SERVER) \
	$(if $(CONFIG_TARGET_ar71xx_VIP4GEXP),-DVIP4G_SERVER) \
	$(if $(CONFIG_TARGET_ar71xx_IPn4G),-DIPn4G_SERVER) \
	$(if $(CONFIG_TARGET_ar71xx_IPn4GEXP),-DIPn4G_SERVER)

#TARGET_LDFLAGS += -lUICommon

define Build/InstallDev
	mkdir -p $(1)/bin
	$(CP) $(PKG_BUILD_DIR)/checksync $(1)/bin/
	$(CP) $(PKG_BUILD_DIR)/snmptrap.sh $(1)/bin/
endef

define Package/checksync/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/checksync $(1)/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/checktimesync $(1)/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/snmptrap.sh $(1)/bin/

	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/checksync.init $(1)/etc/init.d/checksync
endef

$(eval $(call BuildPackage,checksync))
