#
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id: Makefile,v 1.1 2010/04/07 15:27:46 Bob Exp $

include $(TOPDIR)/rules.mk

PKG_NAME:=ntp
PKG_VERSION:=4.2.4p4
PKG_RELEASE:=3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://www.eecis.udel.edu/~ntp/ntp_spool/ntp4/
PKG_MD5SUM:=fcb32a01e1ae3f9ed5bac86b12c7a1cc

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_BUILD_DEPENDS:=libelf

include $(INCLUDE_DIR)/package.mk

define Package/ntpd
  SUBMENU:=Time Synchronization
  SECTION:=net
  CATEGORY:=Network
  TITLE:=full featured NTP server
  DEPENDS:=+libreadline
  URL:=http://www.ntp.org/
endef

define Package/ntpd/description
	The full featured original ntpd
endef

#define Package/ntpd/conffiles
#/etc/ntp.conf
#endef

define Package/ntpd/postinst
#!/bin/sh
echo "ntp             123/udp     # Network Time Protocol
ntp             123/tcp     # Network Time Protocol" >> $${IPKG_INSTROOT}/etc/services
endef

TARGET_CPPFLAGS += -I$(STAGING_DIR)/usr/include/libelf

define Build/Configure
	$(call Build/Configure/Default, \
		--disable-all-clocks \
		--disable-parse-clocks \
		--enable-NMEA \
		--enable-LOCAL-CLOCK \
		--enable-SHM \
		--enable-linuxcaps \
	)
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CFLAGS="$(TARGET_CFLAGS)"
endef

define Package/ntpd/install	
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ntpd/ntpd $(1)/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ntpdate/ntpdate $(1)/bin/
	$(INSTALL_DIR) $(1)/etc/ntp
	$(INSTALL_DATA) ./files/step-tickers $(1)/etc/ntp
#	$(INSTALL_DIR) $(1)/usr/sbin/
#	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ntpd/ntpd $(1)/usr/sbin/
#	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ntpdate/ntpdate $(1)/usr/sbin/
#	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ntpdc/ntpdc $(1)/usr/sbin/
#	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ntpq/ntpq $(1)/usr/sbin/
#	$(INSTALL_DIR) $(1)/etc
#	$(INSTALL_DATA) ./files/ntp.conf $(1)/etc/
#	$(INSTALL_DIR) $(1)/etc/init.d
#	$(INSTALL_BIN) ./files/ntpd.init $(1)/etc/init.d/ntpd
#	$(INSTALL_DIR) $(1)/etc/hotplug.d/iface
#	$(INSTALL_BIN) ./files/ntpd.hotplug $(1)/etc/hotplug.d/iface/20-ntpd
endef

$(eval $(call BuildPackage,ntpd))
