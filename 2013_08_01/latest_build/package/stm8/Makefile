#
# Copyright (C) 2010-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=stm8
PKG_VERSION:=0.0.1
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/stm8
  SECTION:=microhard
  CATEGORY:=Microhard Packages
  DEPENDS:=+kmod-mpci-bsp +i2c-tools +libuci +libpthread
  TITLE:=Microhard STM8 I2C utility
endef

define Package/stm8/description
An agent script that makes STM8 simple,easy and manageable.
endef

define Build/Compile
endef

define Package/stm8/install
	$(CP) ./files/* $(1)
endef

define Package/stm8/postinst
[ -n "$${IPKG_INSTROOT}" ] || /etc/init.d/stm8 enable
exit 0
endef

$(eval $(call BuildPackage,stm8))
