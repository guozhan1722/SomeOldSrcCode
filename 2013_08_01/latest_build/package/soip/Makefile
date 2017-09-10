# 
# Copyright (C) 2009 Microhard Systems 
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id: Makefile,v 1.8.4.1 2012/06/28 20:36:39 Bin Exp $

include $(TOPDIR)/rules.mk

PKG_NAME:=soip
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define Package/soip
  SECTION:=microhard
  CATEGORY:=Microhard Packages
  DEPENDS:=+libuci +libpthread +libmserial
  TITLE:=Microhard serial/net daemon
endef

define Package/soip/description
 This package contains the microhard home grown serial/net daemon
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)
	find $(PKG_BUILD_DIR) -name CVS | xargs rm -rf
endef

TARGET_CFLAGS += $(FPIC) -I$(PKG_BUILD_DIR)/../common-files/include/ui -I$(PKG_BUILD_DIR)/../libmserial/include -I$(PKG_BUILD_DIR)/../at_command -I$(LINUX_DIR)/../mhx-bsp\
	-I$(LINUX_DIR)/../mpci-bsp
#TARGET_LDFLAGS += -lUICommon -lpthread

define Package/soip/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/$(PKG_NAME).init $(1)/etc/init.d/$(PKG_NAME)

	$(INSTALL_DIR) $(1)/bin
	if [ -n "$(CONFIG_TARGET_ar71xx)" ]; then\
		$(INSTALL_BIN) ./files/soip2.init $(1)/etc/init.d/soip2; \
		$(INSTALL_BIN) ./files/soip2.getty $(1)/etc/init.d/soip2.getty; \
		$(INSTALL_BIN) $(PKG_BUILD_DIR)/soipdmpci $(1)/bin/; \
		$(INSTALL_BIN) $(PKG_BUILD_DIR)/soipdmpci2 $(1)/bin/; \
	elif [ -n "$(CONFIG_TARGET_ixp4xx)" ]; then\
		$(INSTALL_BIN) ./files/soip2.init $(1)/etc/init.d/soip2; \
		$(INSTALL_BIN) ./files/soip2.getty $(1)/etc/init.d/soip2.getty; \
		$(INSTALL_BIN) $(PKG_BUILD_DIR)/soipd1 $(1)/bin/; \
		$(INSTALL_BIN) $(PKG_BUILD_DIR)/soipd2 $(1)/bin/; \
		$(INSTALL_BIN) $(PKG_BUILD_DIR)/soipdusb $(1)/bin/; \
	else \
		$(INSTALL_BIN) $(PKG_BUILD_DIR)/soipd1 $(1)/bin/; \
		$(INSTALL_BIN) $(PKG_BUILD_DIR)/soipd2 $(1)/bin/; \
		$(INSTALL_BIN) $(PKG_BUILD_DIR)/soipdusb $(1)/bin/; \
		$(INSTALL_BIN) $(PKG_BUILD_DIR)/soipdmpci $(1)/bin/; \
	fi
endef

$(eval $(call BuildPackage,soip))
