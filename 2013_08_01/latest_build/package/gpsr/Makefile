# 
# Copyright (C) 2009 Microhard Systems 
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id: Makefile,v 1.1.2.13 2013/06/27 20:51:05 AlexDing Exp $

include $(TOPDIR)/rules.mk

PKG_NAME:=gpsr
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define Package/gpsr
  SECTION:=microhard
  CATEGORY:=Microhard Packages
  DEPENDS:=+libuci +libpthread +libmserial
  TITLE:=Microhard gps report and tracker sets
endef

define Package/gpsr/description
 This package contains the microhard home grown serial/net daemon
endef

TARGET_CFLAGS += $(FPIC) -I$(PKG_BUILD_DIR)/../common-files/include/ui -I$(PKG_BUILD_DIR)/../libmserial/include -I$(PKG_BUILD_DIR)/../at_command -I$(LINUX_DIR)/../mhx-bsp\
	-I$(LINUX_DIR)/../mpci-bsp -I$(PKG_BUILD_DIR)/../libmserial/include/phones

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)
	find $(PKG_BUILD_DIR) -name CVS | xargs rm -rf
endef

define Package/gpsr/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/gpsreport $(1)/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/gpsr_sms $(1)/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/gps_webget $(1)/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/trackersmsif $(1)/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/gpstracker $(1)/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/gpsrecorder $(1)/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/gpsloadrecord $(1)/bin/
	$(INSTALL_BIN) ./files/gpsrscript $(1)/bin/
	$(INSTALL_BIN) ./files/trackersms $(1)/bin/trackersms
	$(INSTALL_BIN) ./files/mylocation.init $(1)/bin/mylocation
	$(INSTALL_BIN) ./files/gpscarr $(1)/bin/

	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/$(PKG_NAME).init $(1)/etc/init.d/$(PKG_NAME)
	$(INSTALL_BIN) ./files/gpsd.init $(1)/etc/init.d/gpsd
	$(INSTALL_BIN) ./files/gpsgatetr.init $(1)/etc/init.d/gpsgatetr
	$(INSTALL_BIN) ./files/gpsrecorderd.init $(1)/etc/init.d/gpsrecorderd

	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/$(PKG_NAME).config $(1)/etc/config/$(PKG_NAME)
	$(INSTALL_DATA) ./files/gpsd.config $(1)/etc/config/gpsd
	$(INSTALL_DATA) ./files/gpsgatetr.config $(1)/etc/config/gpsgatetr
	$(INSTALL_DATA) ./files/gpsrecorderd.config $(1)/etc/config/gpsrecorderd
	$(INSTALL_DIR) $(1)/etc/gpsdata

	$(INSTALL_DIR) $(1)/www/cgi-bin/webif
	$(CP) ./files/*.sh $(1)/www/cgi-bin/webif/

	$(INSTALL_DIR) $(1)/www/js
	$(CP) ./files/*.js $(1)/www/js/

endef

$(eval $(call BuildPackage,gpsr))
