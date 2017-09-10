# 
# Copyright (C) 2009 Microhard Systems 
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id: Makefile,v 1.2 2011/01/18 22:07:29 Jim Exp $

include $(TOPDIR)/rules.mk

PKG_NAME:=VIP4G
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

KDIR:=$(KERNEL_BUILD_DIR)

define Package/VIP4G
  SECTION:=microhard
  CATEGORY:=Microhard Packages
  DEPENDS:=+kmod-mpci-bsp +i2c-tools +libuci +libpthread
  TITLE:=Microhard VIP4G utility
endef

define Package/VIP4G/description
 This package contains the microhard home grown VIP4G utility.
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

TARGET_CFLAGS += -I$(KDIR)/mpci-bsp \
	$(if $(CONFIG_TARGET_ar71xx_VIP4G),-DVIP4G_SERVER) \
	$(if $(CONFIG_TARGET_ar71xx_VIP4GEXP),-DVIP4G_SERVER) \
	$(if $(CONFIG_TARGET_ar71xx_IPn4G),-DIPn4G_SERVER) \
	$(if $(CONFIG_TARGET_ar71xx_IPn4GEXP),-DIPn4G_SERVER)

#TARGET_CFLAGS += $(FPIC) -I$(KDIR)/mpci-bsp
#TARGET_LDFLAGS += -lUICommon

define Package/VIP4G/install
	$(INSTALL_DIR) $(1)/lib/
	$(INSTALL_DATA) ./files/lib/atcmd.sh $(1)/lib/
	$(INSTALL_DATA) ./files/lib/status_conf $(1)/lib/
	$(INSTALL_DATA) ./files/lib/atcmd362.sh $(1)/lib/
	$(INSTALL_DATA) ./files/lib/status_conf362 $(1)/lib/
	chmod 0755 $(1)/lib/atcmd.sh
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/lte.config $(1)/etc/config/lte
	$(INSTALL_DATA) ./files/apntable $(1)/etc/apntable
	$(INSTALL_DATA) ./files/lte362.config $(1)/etc/config/lte362
	$(INSTALL_DATA) ./files/ioports.config $(1)/etc/config/ioports
	$(INSTALL_DATA) ./files/keepalive.conf $(1)/etc/config/keepalive
	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_BIN) ./files/lte.init $(1)/etc/init.d/lte
	$(INSTALL_BIN) ./files/lte_rssi.init $(1)/etc/init.d/lte_rssi
	$(INSTALL_BIN) ./files/ioports.init $(1)/etc/init.d/ioports
	$(INSTALL_BIN) ./files/keepalive.init $(1)/etc/init.d/keepalive
	$(INSTALL_BIN) ./files/dhcp_client.init $(1)/etc/init.d/dhcp_client
	$(INSTALL_DIR) $(1)/www/cgi-bin/webif/
	$(INSTALL_BIN) ./files/www/cgi-bin/webif/carrier.sh $(1)/www/cgi-bin/webif/carrier.sh
	$(INSTALL_BIN) ./files/www/cgi-bin/webif/carrier-status.sh $(1)/www/cgi-bin/webif/carrier-status.sh
	$(INSTALL_BIN) ./files/www/cgi-bin/webif/io.sh $(1)/www/cgi-bin/webif/io.sh
	$(INSTALL_BIN) ./files/www/cgi-bin/webif/io-status.sh $(1)/www/cgi-bin/webif/io-status.sh
	$(INSTALL_BIN) ./files/www/cgi-bin/webif/carrier-keepalive.sh $(1)/www/cgi-bin/webif/carrier-keepalive.sh
	$(INSTALL_DIR) $(1)/etc/webpages/
	$(INSTALL_BIN) ./files/categories_VIP4G $(1)/etc/webpages/categories_VIP4G
	$(INSTALL_DIR) $(1)/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mpci-4g_server $(1)/sbin/
	$(INSTALL_DIR) $(1)/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/setnv_4g_default $(1)/bin/.
	$(INSTALL_BIN) ./files/bin/parse_lte_status.sh $(1)/bin/.
	$(INSTALL_BIN) ./files/bin/keepalivebootup $(1)/bin/keepalivebootup
	$(INSTALL_BIN) ./files/bin/ippassthrough $(1)/bin/.
	$(INSTALL_DIR) $(1)/etc/hotplug.d/iface
	$(INSTALL_DATA) ./files/ippassthrough.hotplug $(1)/etc/hotplug.d/iface/50-ippassthrough
endef

$(eval $(call BuildPackage,VIP4G))
