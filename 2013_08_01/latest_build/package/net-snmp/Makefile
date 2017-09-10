# 
# Copyright (C) 2009 Microhard Systems 
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id: Makefile,v 1.12.4.4 2013/04/03 16:47:05 Jim Exp $

include $(TOPDIR)/rules.mk

PKG_NAME:=net-snmp
PKG_RELEASE:=5.0.9

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define Package/net-snmp
  SECTION:=microhard
  CATEGORY:=Microhard Packages
  DEPENDS:=+libuci +libpthread
  TITLE:=Microhard modified snmp package
endef

define Package/net-snmp/description
 This package contains the microhard home grown net-snmp server derived from IXP4xx based IP/VIP platform.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
#	if [ -e ./src ]; then \
#		cvs update ./src; \
#	else \
#		cvs co -r $(IPX20_TAG) -d src network/IPx20/apps/net-snmp; \
#	fi
	$(CP) ./src/* $(PKG_BUILD_DIR)
	find $(PKG_BUILD_DIR) -name CVS | xargs rm -rf
endef

ifneq ($(CONFIG_TARGET_ixp4xx_generic_VIP421EXP)$(CONFIG_TARGET_ixp4xx_generic_VIP421EXPBR)$(CONFIG_TARGET_ixp4xx_generic_VIP421EXP49G)$(CONFIG_TARGET_ixp4xx_generic_VIP421ANTEXP)$(CONFIG_TARGET_ixp4xx_generic_VIP421ANT18dBiEXP)$(CONFIG_TARGET_ixp4xx_generic_VIP421900EXP)$(CONFIG_TARGET_ixp4xx_generic_VIP4214900EXP)$(CONFIG_TARGET_ixp4xx_generic_VIP4212400XEXP)$(CONFIG_TARGET_ixp4xx_generic_VIP4215800XEXP)$(CONFIG_TARGET_ixp4xx_generic_VIP4214900XEXP), )
EXPORT_CFLAGS=-DEXPORT
endif

ifneq ($(CONFIG_TARGET_ixp4xx_generic_VIP421EXP49G)$(CONFIG_TARGET_ixp4xx_generic_VIP4214900X)$(CONFIG_TARGET_ixp4xx_generic_VIP4214900XEXP), )
EXPORT_CFLAGS=-DCONFIG_TARGET_ixp4xx_generic_VIP421EXP49G
endif

ifneq ($(CONFIG_TARGET_ar71xx), )
IPnDDL_CFLAGS=-DCONFIG_TARGET_ar71xx_IPnDDL
endif

ifneq ($(CONFIG_TARGET_ar71xx_IPnVTEXP), )
EXPORT_CFLAGS=-DEXPORT
endif

ifneq ($(CONFIG_TARGET_ar71xx_VIP4GEXP), )
EXPORT_CFLAGS=-DEXPORT
endif

ifneq ($(CONFIG_TARGET_ar71xx_IPnVTn3GEXP), )
EXPORT_CFLAGS=-DEXPORT
endif

ifneq ($(CONFIG_TARGET_ar71xx_IPn4GEXP), )
EXPORT_CFLAGS=-DEXPORT
endif

ifneq ($(CONFIG_TARGET_ar71xx_IPn4G), )
EXPORT_CFLAGS=-DAR71XX_IPn4G
endif

ifneq ($(CONFIG_TARGET_ar71xx_VIP4G), )
EXPORT_CFLAGS=-DAR71XX_VIP4G
endif

TARGET_CFLAGS += $(FPIC) -Dlinux $(EXPORT_CFLAGS) $(IPnDDL_CFLAGS)

define Package/net-snmp/install
	$(INSTALL_DIR) $(1)/lib
	if [ -n "$(CONFIG_TARGET_ar71xx_VIP4G)" ]; then \
		$(CP) ./files/snmp_item_VIP4G $(1)/lib/snmp_item ; \
	elif [ -n "$(CONFIG_TARGET_ar71xx_IPn4G)" ]; then \
		$(CP) ./files/snmp_item_IPn4G $(1)/lib/snmp_item ; \
	fi

	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/snmpd.conf $(1)/etc/config/snmpd
	$(INSTALL_DIR) $(1)/etc/snmp
	ln -sf /var/run/snmpd.conf $(1)/etc/snmp/
	$(INSTALL_DIR) $(1)/etc/default
	$(INSTALL_DATA) ./files/snmpd.default $(1)/etc/default/snmpd
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/snmpd.init $(1)/etc/init.d/snmpd
	$(INSTALL_DIR) $(1)/etc/
	$(INSTALL_BIN) ./files/snmpd.rmuci $(1)/etc/rmuci
	$(INSTALL_DIR) $(1)/etc/
	$(INSTALL_BIN) ./files/snmpd.tmpstatus $(1)/etc/tmpstatus
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_BIN) ./files/snmpd.tmpstatus_conf $(1)/etc/config/tmpstatus
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/snmpd.systemmode $(1)/etc/init.d/systemmode
	$(INSTALL_DIR) $(1)/bin/
	$(INSTALL_BIN) ./files/snmpd.mibparse $(1)/bin/mibparse

	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/agent/snmpd $(1)/usr/sbin/

	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/apps/snmptrap $(1)/bin/
endef

$(eval $(call BuildPackage,net-snmp))
