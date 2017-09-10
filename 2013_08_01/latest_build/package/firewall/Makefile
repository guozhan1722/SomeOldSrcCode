#
# Copyright (C) 2008-2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk

PKG_NAME:=firewall

PKG_VERSION:=1
PKG_RELEASE:=10

include $(INCLUDE_DIR)/package.mk

define Package/firewall
  SECTION:=net
  CATEGORY:=Base system
  URL:=http://openwrt.org/
  TITLE:=OpenWrt firewall
  DEPENDS:=+libuci +libpthread +iptables +iptables-mod-conntrack +iptables-mod-nat
  PKGARCH:=all
endef

define Package/firewall/description
 uci based firewall for openwrt
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)
	find $(PKG_BUILD_DIR) -name CVS | xargs rm -rf
endef

TARGET_CFLAGS += $(FPIC) 
#TARGET_LDFLAGS += -lUICommon -lpthread

#define Build/Compile
#	true
#endef

define Package/firewall/conffiles
	/etc/config/firewall
endef

define Package/firewall/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mh_firewall $(1)/sbin/
	$(INSTALL_DIR) $(1)/lib/firewall
	$(INSTALL_DATA) ./files/uci_firewall.sh $(1)/lib/firewall
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/firewall.config $(1)/etc/config/firewall
	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_BIN) ./files/firewall.init $(1)/etc/init.d/firewall
	$(INSTALL_DIR) $(1)/etc/hotplug.d/iface
	$(INSTALL_DATA) ./files/20-firewall $(1)/etc/hotplug.d/iface
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DATA) ./files/firewall.user $(1)/etc
endef

$(eval $(call BuildPackage,firewall))
