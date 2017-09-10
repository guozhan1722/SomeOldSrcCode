#
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id: Makefile,v 1.1 2010/09/24 21:44:23 Bob Exp $

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=u-boot
PKG_VERSION:=1.1.4-ar71xx
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
#PKG_SOURCE_URL:=ftp://ftp.denx.de/pub/u-boot
PKG_SOURCE_URL:=
PKG_MD5SUM:=
PKG_CAT:=bzcat

include $(INCLUDE_DIR)/package.mk

UBOOT_CONFIG=$(strip $(subst ",, $(CONFIG_UBOOT_TARGET)))
LAN_IP=$(strip $(CONFIG_UBOOT_IPADDR))
LAN_SERVERIP=$(strip $(subst ",, $(CONFIG_UBOOT_SERVERIP)))
CONFIG_SERIAL_NUM=$(strip $(subst ",, $(CONFIG_FACTORY_SERIAL_NUM)))

define Build/Compile
if [ "$(CONFIG_NANOPCI_UBOOT)" = "y" ]; then \
		$(MAKE) -C $(PKG_BUILD_DIR) $(UBOOT_CONFIG)_config; \
		export CROSS_COMPILE=$(TARGET_CROSS); \
		export LAN_IP=$(LAN_IP); \
		export LAN_SERVERIP=$(LAN_SERVERIP); \
		export CONFIG_SERIAL_NUM=$(CONFIG_SERIAL_NUM); \
		export CONFIG_PRODUCTNAME=$(PROFILE); \
		$(MAKE) -C $(PKG_BUILD_DIR); \
		$(PKG_BUILD_DIR)/tools/ubparams; \
		cp params $(KDIR); \
fi
endef

define Build/InstallDev
if [ "$(CONFIG_NANOPCI_UBOOT)" = "y" ]; then \
	dd if=$(PKG_BUILD_DIR)/u-boot.bin of=$(PKG_BUILD_DIR)/u-boot.block bs=232k count=1 conv=sync; \
	cp $(PKG_BUILD_DIR)/u-boot.bin $(KDIR); \
fi
endef

$(eval $(call Build/DefaultTargets))
