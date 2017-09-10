#
# Copyright (C) 2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk

PKG_NAME:=gre-tunnel

PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/gre-tunnel
  SECTION:=microhard
  CATEGORY:=Microhard Packages
  DEPENDS:=+libuci
  TITLE:=Microhard Gre Tunnel Utility
endef

define Package/gre-tunnel/description
 Microhard gre tunnel utility
endef

define Build/Compile
	true
endef

define Package/gre-tunnel/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) ./files/bin/gre $(1)/bin/
	$(INSTALL_BIN) ./files/bin/gredpd $(1)/bin/

	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_BIN) ./files/etc/config/gre-tunnels $(1)/etc/config/

	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/etc/init.d/gred $(1)/etc/init.d/

	$(INSTALL_DIR) $(1)/www/cgi-bin/webif
	$(INSTALL_BIN) ./files/www/cgi-bin/webif/*.sh $(1)/www/cgi-bin/webif/

endef

$(eval $(call BuildPackage,gre-tunnel))
