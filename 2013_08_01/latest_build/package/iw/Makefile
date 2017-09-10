#
# Copyright (C) 2007-2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=iw
#PKG_VERSION:=0.9.22
PKG_VERSION:=microhard
PKG_RELEASE:=1

#PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
#PKG_SOURCE_URL:=http://wireless.kernel.org/download/iw/
#PKG_MD5SUM:=561c2602c5ce7c65a590118286c0892a
PKG_BUILD_DEPENDS:=mac80211

include $(INCLUDE_DIR)/package.mk

define Package/iw
  SECTION:=net
  CATEGORY:=Network
  TITLE:=cfg80211 interface configuration utility
  URL:=http://wireless.kernel.org/en/users/Documentation/iw
  DEPENDS:= +libnl-tiny @(!(TARGET_avr32||TARGET_etrax)||BROKEN)
endef

define Build/Prepare
	rm -rf $(PKG_BUILD_DIR)
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Configure
	echo "const char iw_version[] = \"$(PKG_VERSION)\";" > $(PKG_BUILD_DIR)/version.c
	rm -f $(PKG_BUILD_DIR)/version.sh
	touch $(PKG_BUILD_DIR)/version.sh
	chmod +x $(PKG_BUILD_DIR)/version.sh
endef

TARGET_CPPFLAGS:= \
	-I$(STAGING_DIR)/usr/include/libnl-tiny \
	-I$(STAGING_DIR)/usr/include/mac80211 \
	$(TARGET_CPPFLAGS) \
	-DCONFIG_LIBNL20 \
	$(if $(CONFIG_TARGET_ixp4xx_generic_VIP421EXP49G),-DCONFIG_TARGET_ixp4xx_generic_VIP421EXP49G) \
	$(if $(CONFIG_TARGET_ixp4xx_generic_VIP4214900X),-DCONFIG_TARGET_ixp4xx_generic_VIP421EXP49G) \
	$(if $(CONFIG_TARGET_ixp4xx_generic_VIP4214900XEXP),-DCONFIG_TARGET_ixp4xx_generic_VIP421EXP49G) \
	$(if $(CONFIG_TARGET_ixp4xx_generic_VIP4215800X),-DCONFIG_TARGET_ixp4xx_generic_VIP421EXP49G) \
	$(if $(CONFIG_TARGET_ixp4xx_generic_VIP4215800XEXP),-DCONFIG_TARGET_ixp4xx_generic_VIP421EXP49G) \
	$(if $(CONFIG_TARGET_ixp4xx_generic_VIP4212400X),-DCONFIG_TARGET_ixp4xx_generic_VIP421EXP49G) \
	$(if $(CONFIG_TARGET_ixp4xx_generic_VIP4212400XEXP),-DCONFIG_TARGET_ixp4xx_generic_VIP421EXP49G) \
	$(if $(CONFIG_TARGET_ar71xx),-DCONFIG_TARGET_ixp4xx_generic_VIP421EXP49G) \
	-D_GNU_SOURCE

MAKE_FLAGS += \
	CFLAGS="$(TARGET_CPPFLAGS) $(TARGET_CFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS)" \
	NL1FOUND="" NL2FOUND=Y \
	NLLIBNAME="libnl-tiny" \
	LIBS="-lm -lnl-tiny" \
	V=1

define Package/iw/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/iw $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/testmode $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mdf $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,iw))
