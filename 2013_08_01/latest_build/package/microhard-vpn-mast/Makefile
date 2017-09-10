#
# Copyright (C) 2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk

PKG_NAME:=microhard-vpn-mast

PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/microhard-vpn-mast
  SECTION:=microhard
  CATEGORY:=Microhard Packages
  DEPENDS:=+libuci +openswan
  TITLE:=Microhard VPN Utility
endef

define Package/microhard-vpn-mast/description
 Microhard VPN utility
endef

define Build/Compile
	true
endef

define Package/microhard-vpn-mast/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) ./files/sbin/m* $(1)/sbin/
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) ./files/lib/mipsec $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/etc/init.d/ipsec* $(1)/etc/init.d/
	$(INSTALL_BIN) ./files/etc/init.d/vcad $(1)/etc/init.d/
	$(INSTALL_BIN) ./files/etc/init.d/x2sadd $(1)/etc/init.d/

	$(INSTALL_DIR) $(1)/etc/hotplug.d/iface
	$(CP) ./files/etc/hotplug.d/iface/50-mvpn $(1)/etc/hotplug.d/iface/

	$(INSTALL_DIR) $(1)/www/cgi-bin/webif
	$(CP) ./files/www/*.sh $(1)/www/cgi-bin/webif/

endef

$(eval $(call BuildPackage,microhard-vpn-mast))
