# Copyright (C) 2010 Commotion
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
include $(TOPDIR)/rules.mk

PKG_NAME:=websockserver
PKG_RELEASE:=1
#PKG_VERSION:=0.0.1

#PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

#PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
#PKG_SOURCE_URL:=git://git.warmcat.com/libwebsockets
#PKG_SOURCE_VERSION:=71e53691756fd420c409538c71b010997f06f414
#PKG_SOURCE_SUBDIR:=libwebsockets
#PKG_SOURCE_PROTO:=git

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

#TARGET_CFLAGS += $(FPIC) -I$(PKG_BUILD_DIR)/../common-files/include/ui -I$(PKG_BUILD_DIR)/../libmserial/include -I$(PKG_BUILD_DIR)/../at_command -I$(LINUX_DIR)/../mhx-bsp\
#	-I$(LINUX_DIR)/../mpci-bsp -I$(PKG_BUILD_DIR)/../libmserial/include/phones

define Package/websockserver
  SECTION:=microhard
  CATEGORY:=Microhard Packages
  DEPENDS:=+libuci +libpthread +libmserial
  TITLE:=Microhard websocket service
endef

#define Build/Configure
#	$(call Build/Configure/Default,--with-linux-headers=$(LINUX_DIR) --datadir=/usr/share)
#endef

define Package/websockserver/description
  C Websockets Server Library and service, this package only installs the library and a test application.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)
	find $(PKG_BUILD_DIR) -name CVS | xargs rm -rf
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/lib/libwebsockets.h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/lib/.libs/libwebsockets.{a,so*} $(1)/usr/lib/	
endef

define Package/websockserver/install
	$(INSTALL_DIR) $(1)/bin
	if [ -f $(PKG_BUILD_DIR)/test-server/.libs/websockserverbin ]; then \
		$(CP)  $(PKG_BUILD_DIR)/test-server/.libs/websockserverbin $(PKG_BUILD_DIR)/test-server/websockserverbin ;\
	fi;
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/test-server/websockserverbin $(1)/bin/websockserver
	$(INSTALL_BIN) ./files/websockscript $(1)/bin/websockscript

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/lib/.libs/libwebsockets.{a,so*} $(1)/usr/lib/

	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/websockserverd.init $(1)/etc/init.d/websockserverd
		
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/lib/libwebsockets.h $(1)/usr/include

	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/websockserver.conf $(1)/etc/config/websockserver

	$(INSTALL_DIR) $(1)/www/cgi-bin/webif
	$(CP) ./files/*.sh $(1)/www/cgi-bin/webif/

endef

$(eval $(call BuildPackage,websockserver))
