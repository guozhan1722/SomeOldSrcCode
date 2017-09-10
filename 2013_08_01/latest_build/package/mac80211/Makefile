#
# Copyright (C) 2007-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=mac80211

PKG_VERSION:=microhard
PKG_RELEASE:=2
PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/compat-wireless-$(PKG_VERSION)

PKG_CONFIG_DEPENDS:= \
	CONFIG_PACKAGE_kmod-mac80211 \
	CONFIG_PACKAGE_MAC80211_DEBUGFS \
	CONFIG_PACKAGE_ATH_DEBUG \
	CONFIG_ATH_USER_REGD \

include $(INCLUDE_DIR)/package.mk

WMENU:=Wireless Drivers

define KernelPackage/mac80211/Default
  SUBMENU:=$(WMENU)
  URL:=http://linuxwireless.org/
  MAINTAINER:=Felix Fietkau <nbd@openwrt.org>
  DEPENDS:=@(!(TARGET_avr32||TARGET_ep93xx||TARGET_ps3||TARGET_pxcab)||BROKEN)
endef

define KernelPackage/cfg80211
  $(call KernelPackage/mac80211/Default)
  TITLE:=cfg80211 - wireless configuration API
  DEPENDS+= +wireless-tools +iw +crda
ifeq ($(strip $(call CompareKernelPatchVer,$(KERNEL_PATCHVER),ge,2.6.33)),1) 
  FILES:= \
	$(PKG_BUILD_DIR)/compat/compat.$(LINUX_KMOD_SUFFIX) \
	$(PKG_BUILD_DIR)/net/wireless/cfg80211.$(LINUX_KMOD_SUFFIX)
  AUTOLOAD:=$(call AutoLoad,20,compat cfg80211)
else 
  FILES:= \
	$(PKG_BUILD_DIR)/compat/compat.$(LINUX_KMOD_SUFFIX) \
	$(PKG_BUILD_DIR)/compat/compat_firmware_class.$(LINUX_KMOD_SUFFIX) \
	$(PKG_BUILD_DIR)/net/wireless/cfg80211.$(LINUX_KMOD_SUFFIX)
  AUTOLOAD:=$(call AutoLoad,20,compat compat_firmware_class cfg80211)
endif
endef

define KernelPackage/cfg80211/description
cfg80211 is the Linux wireless LAN (802.11) configuration API.
endef

define KernelPackage/mac80211
  $(call KernelPackage/mac80211/Default)
  TITLE:=Linux 802.11 Wireless Networking Stack
  DEPENDS+= +kmod-crypto-core +kmod-crypto-arc4 +kmod-crypto-aes +kmod-cfg80211
  FILES:= $(PKG_BUILD_DIR)/net/mac80211/mac80211.$(LINUX_KMOD_SUFFIX)
  AUTOLOAD:=$(call AutoLoad,21,mac80211)
  MENU:=1
endef

define KernelPackage/mac80211/config
	menu "Configuration"
		depends on PACKAGE_kmod-mac80211

	config PACKAGE_MAC80211_DEBUGFS
		bool "Export mac80211 internals in DebugFS"
		default y
		help
		  Select this to see extensive information about
		  the internal state of mac80211 in debugfs.

		  Say N unless you know you need this.

	endmenu
endef

define KernelPackage/mac80211/description
Generic IEEE 802.11 Networking Stack (mac80211)
endef

define KernelPackage/ath/config
	menu "Configuration"
		depends on PACKAGE_kmod-ath

	config ATH_USER_REGD
		bool "Force Atheros drivers to respect the user's regdomain settings"
		help
		  Atheros' idea of regulatory handling is that the EEPROM of the card defines
		  the regulatory limits and the user is only allowed to restrict the settings
		  even further, even if the country allows frequencies or power levels that
		  are forbidden by the EEPROM settings.

		  Select this option if you want the driver to respect the user's decision about
		  regulatory settings.

	config PACKAGE_ATH_DEBUG
		bool "Atheros wireless debugging"
		help
		  Say Y, if you want to debug atheros wireless drivers.
		  Right now only ath9k makes use of this.

	endmenu
endef

define KernelPackage/ath
  $(call KernelPackage/mac80211/Default)
  TITLE:=Atheros common driver part
  DEPENDS+= +kmod-mac80211
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/ath/ath.$(LINUX_KMOD_SUFFIX)
  AUTOLOAD:=$(call AutoLoad,26,ath)
  MENU:=1
endef

define KernelPackage/ath/description
 This module contains some common parts needed by Atheros Wireless drivers.
endef

define KernelPackage/ath5k
  $(call KernelPackage/mac80211/Default)
  TITLE:=Atheros 5xxx wireless cards support
  URL:=http://linuxwireless.org/en/users/Drivers/ath5k
  DEPENDS+= +kmod-ath
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/ath/ath5k/ath5k.$(LINUX_KMOD_SUFFIX)
  AUTOLOAD:=$(call AutoLoad,27,ath5k)
endef

define KernelPackage/ath5k/description
 This module adds support for wireless adapters based on
 Atheros 5xxx chipset.
endef

define KernelPackage/adf4350
  $(call KernelPackage/mac80211/Default)
  TITLE:=ADF4350 chip support
  URL:=http://linuxwireless.org/en/users/Drivers/ath5k
  DEPENDS+= +kmod-ath
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/ath/ath5k/ath5k.$(LINUX_KMOD_SUFFIX)
  AUTOLOAD:=$(call AutoLoad,27,ath5k)
endef

define KernelPackage/adf4350/description
 control adf4350 chipset.
endef

define KernelPackage/ath9k
  $(call KernelPackage/mac80211/Default)
  TITLE:=Atheros 802.11n wireless cards support
  URL:=http://linuxwireless.org/en/users/Drivers/ath9k
  DEPENDS+= +kmod-ath
  FILES:= \
	$(PKG_BUILD_DIR)/drivers/net/wireless/ath/ath9k/ath9k_common.$(LINUX_KMOD_SUFFIX) \
	$(PKG_BUILD_DIR)/drivers/net/wireless/ath/ath9k/ath9k_hw.$(LINUX_KMOD_SUFFIX) \
	$(PKG_BUILD_DIR)/drivers/net/wireless/ath/ath9k/ath9k.$(LINUX_KMOD_SUFFIX)
  AUTOLOAD:=$(call AutoLoad,27,ath9k_hw ath9k_common ath9k)
endef

define KernelPackage/ath9k/description
This module adds support for wireless adapters based on
Atheros IEEE 802.11n AR5008 and AR9001 family of chipsets.
endef

define KernelPackage/mac80211-hwsim
  $(call KernelPackage/mac80211/Default)
  TITLE:=mac80211 HW simulation device
  DEPENDS+= +kmod-mac80211
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/mac80211_hwsim.$(LINUX_KMOD_SUFFIX)
  AUTOLOAD:=$(call AutoLoad,60,mac80211_hwsim)
endef

BUILDFLAGS:= \
	-I$(PKG_BUILD_DIR)/include \
	$(foreach opt,$(CONFOPTS),-DCONFIG_$(opt)) \
	$(if $(CONFIG_PCI),-DCONFIG_B43_PCI_AUTOSELECT -DCONFIG_B43_PCICORE_AUTOSELECT) \
	$(if $(CONFIG_LEDS_TRIGGERS), -DCONFIG_MAC80211_LEDS -DCONFIG_LEDS_TRIGGERS -DCONFIG_B43_LEDS -DCONFIG_B43LEGACY_LEDS -DCONFIG_AR9170_LEDS) \
	-DCONFIG_B43_HWRNG -DCONFIG_B43LEGACY_HWRNG \
	$(if $(CONFIG_PACKAGE_MAC80211_DEBUGFS),-DCONFIG_MAC80211_DEBUGFS -DCONFIG_ATH9K_DEBUGFS -DCONFIG_CARL9170_DEBUGFS) \
	$(if $(CONFIG_PACKAGE_ATH_DEBUG),-DCONFIG_ATH_DEBUG -DCONFIG_ATH9K_PKTLOG -DCONFIG_ATH5K_DEBUG) \
	-D__CONFIG_MAC80211_RC_DEFAULT=minstrel \
	-DCONFIG_MAC80211_RC_MINSTREL_HT \
	$(if $(CONFIG_ATH_USER_REGD),-DATH_USER_REGD=1) \
	$(if $(CONFIG_PACKAGE_RT2X00_LIB_DEBUGFS),-DCONFIG_RT2X00_LIB_DEBUGFS) \
	$(if $(CONFIG_PACKAGE_RT2X00_DEBUG),-DCONFIG_RT2X00_DEBUG) \
	$(if $(NEED_RT2X00_LIB_HT),-DCONFIG_RT2X00_LIB_HT) \
	$(if $(NEED_RT2X00_LIB_CRYPTO),-DCONFIG_RT2X00_LIB_CRYPTO) \
	$(if $(NEED_RT2X00_LIB_FIRMWARE),-DCONFIG_RT2X00_LIB_FIRMWARE) \
	$(if $(NEED_RT2X00_LIB_LEDS),-DCONFIG_RT2X00_LIB_LEDS) \
	$(if $(CONFIG_PACKAGE_kmod-rt2x00-pci),-DCONFIG_RT2X00_LIB_PCI) \
	$(if $(CONFIG_PACKAGE_kmod-rt2x00-usb),-DCONFIG_RT2X00_LIB_USB) \
	$(if $(CONFIG_PACKAGE_kmod-rt2x00-soc),-DCONFIG_RT2X00_LIB_SOC) \
	$(if $(CONFIG_TARGET_atheros),-DCONFIG_ATH5K_AHB,-DCONFIG_ATH5K_PCI) \
	$(if $(CONFIG_TARGET_atheros),-DCONFIG_ATH5K_AHB,-DCONFIG_ATH5K_PCI) \
	$(if $(CONFIG_PACKAGE_kmod-adf4350),-DCONFIG_ATH5K_ADF4350) \
	$(if $(CONFIG_TARGET_ixp4xx_generic_VIP421EXP49G),-DCONFIG_TARGET_ixp4xx_generic_VIP421EXP49G) \
	$(if $(CONFIG_TARGET_ixp4xx_generic_VIP4214900X),-DCONFIG_TARGET_ixp4xx_generic_VIP421EXP49G) \
	$(if $(CONFIG_TARGET_ixp4xx_generic_VIP4214900XEXP),-DCONFIG_TARGET_ixp4xx_generic_VIP421EXP49G) \
	$(if $(CONFIG_TARGET_ixp4xx_generic_VIP4215800X),-DCONFIG_TARGET_ixp4xx_generic_VIP421EXP49G) \
	$(if $(CONFIG_TARGET_ixp4xx_generic_VIP4215800XEXP),-DCONFIG_TARGET_ixp4xx_generic_VIP421EXP49G) \
	$(if $(CONFIG_TARGET_ixp4xx_generic_VIP4212400X),-DCONFIG_TARGET_ixp4xx_generic_VIP421EXP49G) \
	$(if $(CONFIG_TARGET_ixp4xx_generic_VIP4212400XEXP),-DCONFIG_TARGET_ixp4xx_generic_VIP421EXP49G) \
	$(if $(CONFIG_TARGET_ar71xx),-DCONFIG_TARGET_ixp4xx_generic_VIP421EXP49G -DCONFIG_TARGET_ar71xx_IPnDDL) \
	$(if $(CONFIG_TARGET_ar71xx_IPnDDL),-DCONFIG_PURE_G) \
	-DCONFIG_P54_SPI_DEFAULT_EEPROM 	

MAKE_OPTS:= \
	CROSS_COMPILE="$(KERNEL_CROSS)" \
	ARCH="$(LINUX_KARCH)" \
	EXTRA_CFLAGS="$(BUILDFLAGS)" \
	$(foreach opt,$(CONFOPTS),CONFIG_$(opt)=m) \
	CONFIG_MAC80211=$(if $(CONFIG_PACKAGE_kmod-mac80211),m) \
	CONFIG_MAC80211_RC_MINSTREL=y \
	CONFIG_MAC80211_LEDS=$(CONFIG_LEDS_TRIGGERS) \
	CONFIG_MAC80211_DEBUGFS=$(if $(CONFIG_PACKAGE_MAC80211_DEBUGFS),y) \
	CONFIG_B43_PCMCIA=n CONFIG_B43_PIO=n \
	CONFIG_B43_PCI_AUTOSELECT=$(if $(CONFIG_PCI),y) \
	CONFIG_B43_PCICORE_AUTOSELECT=$(if $(CONFIG_PCI),y) \
	CONFIG_B43LEGACY_LEDS=$(CONFIG_LEDS_TRIGGERS) \
	CONFIG_B43_LEDS=$(CONFIG_LEDS_TRIGGERS) \
	CONFIG_B43_HWRNG=$(if $(CONFIG_HW_RANDOM),y) \
	CONFIG_B43LEGACY_HWRNG=$(if $(CONFIG_HW_RANDOM),y) \
	CONFIG_B43=$(if $(CONFIG_PACKAGE_kmod-b43),m) \
	CONFIG_B43LEGACY=$(if $(CONFIG_PACKAGE_kmod-b43legacy),m) \
	CONFIG_ATH_COMMON=$(if $(CONFIG_PACKAGE_kmod-ath),m) \
	CONFIG_ATH_DEBUG=$(if $(CONFIG_PACKAGE_ATH_DEBUG),y) \
	CONFIG_ATH5K_DEBUG=$(if $(CONFIG_PACKAGE_ATH_DEBUG),y) \
	CONFIG_ATH9K_PKTLOG=$(if $(CONFIG_PACKAGE_ATH_DEBUG),y) \
	CONFIG_ATH5K=$(if $(CONFIG_PACKAGE_kmod-ath5k),m) \
	CONFIG_ATH5K_PCI=$(if $(CONFIG_TARGET_atheros),,y) \
	CONFIG_ATH5K_ADF4350=$(if $(CONFIG_PACKAGE_kmod-adf4350),y) \
	CONFIG_ATH5K_AHB=$(if $(CONFIG_TARGET_atheros),y) \
	CONFIG_ATH9K=$(if $(CONFIG_PACKAGE_kmod-ath9k),m) \
	CONFIG_ATH9K_HW=$(if $(CONFIG_PACKAGE_kmod-ath9k),m) \
	CONFIG_ATH9K_COMMON=$(if $(CONFIG_PACKAGE_kmod-ath9k),m) \
	CONFIG_ATH9K_DEBUGFS=$(if $(CONFIG_PACKAGE_MAC80211_DEBUGFS),y) \
	CONFIG_ZD1211RW=$(if $(CONFIG_PACKAGE_kmod-zd1211rw),m) \
	CONFIG_P54_COMMON=$(if $(CONFIG_PACKAGE_kmod-p54-common),m) \
	CONFIG_P54_PCI=$(if $(CONFIG_PACKAGE_kmod-p54-pci),m) \
	CONFIG_P54_USB=$(if $(CONFIG_PACKAGE_kmod-p54-usb),m) \
	CONFIG_P54_SPI= \
	CONFIG_RT2X00=$(if $(CONFIG_PACKAGE_kmod-rt2x00-lib),m) \
	CONFIG_RT2X00_LIB=$(if $(CONFIG_PACKAGE_kmod-rt2x00-lib),m) \
	CONFIG_RT2X00_LIB_PCI=$(if $(CONFIG_PACKAGE_kmod-rt2x00-pci),m) \
	CONFIG_RT2X00_LIB_USB=$(if $(CONFIG_PACKAGE_kmod-rt2x00-usb),m) \
	CONFIG_RT2400PCI=$(if $(CONFIG_PACKAGE_kmod-rt2400-pci),m) \
	CONFIG_RT2500PCI=$(if $(CONFIG_PACKAGE_kmod-rt2500-pci),m) \
	CONFIG_RT2500USB=$(if $(CONFIG_PACKAGE_kmod-rt2500-usb),m) \
	CONFIG_RT61PCI=$(if $(CONFIG_PACKAGE_kmod-rt61-pci),m) \
	CONFIG_RT73USB=$(if $(CONFIG_PACKAGE_kmod-rt73-usb),m) \
	CONFIG_RT2800_LIB=$(if $(CONFIG_PACKAGE_kmod-rt2800-lib),m) \
	CONFIG_RT2800PCI=$(if $(CONFIG_PACKAGE_kmod-rt2800-pci),m) \
	CONFIG_RT2800USB=$(if $(CONFIG_PACKAGE_kmod-rt2800-usb),m) \
	CONFIG_RTL8180=$(if $(CONFIG_PACKAGE_kmod-rtl8180),m) \
	CONFIG_RTL8187=$(if $(CONFIG_PACKAGE_kmod-rtl8187),m) \
	CONFIG_MAC80211_HWSIM=$(if $(CONFIG_PACKAGE_kmod-mac80211-hwsim),m) \
	CONFIG_PCMCIA= \
	CONFIG_LIBIPW=$(if $(CONFIG_PACKAGE_kmod-net-libipw),m) \
	CONFIG_LIBERTAS=$(if $(CONFIG_PACKAGE_kmod-libertas),m) \
	CONFIG_LIBERTAS_CS= \
	CONFIG_LIBERTAS_SDIO= \
	CONFIG_LIBERTAS_THINFIRM= \
	CONFIG_LIBERTAS_USB=$(if $(CONFIG_PACKAGE_kmod-libertas),m) \
	CONFIG_IPW2100=$(if $(CONFIG_PACKAGE_kmod-net-ipw2100),m) \
	CONFIG_IPW2200=$(if $(CONFIG_PACKAGE_kmod-net-ipw2200),m) \
	CONFIG_NL80211=y \
	CONFIG_LIB80211= \
	CONFIG_LIB80211_CRYPT_WEP= \
	CONFIG_LIB80211_CRYPT_CCMP= \
	CONFIG_LIB80211_CRYPT_TKIP= \
	CONFIG_IWLWIFI= \
	CONFIG_IWLAGN= \
	CONFIG_MWL8K= \
	CONFIG_ATMEL= \
	CONFIG_PCMCIA_ATMEL= \
	CONFIG_ADM8211= \
	CONFIG_USB_NET_RNDIS_HOST= \
	CONFIG_USB_NET_RNDIS_WLAN= \
	CONFIG_USB_NET_CDCETHER= \
	CONFIG_USB_USBNET= \
	CONFIG_AT76C50X_USB= \
	CONFIG_WL12XX= \
	CONFIG_EEPROM_93CX6= \
	CONFIG_HERMES= \
	CONFIG_AR9170_USB=$(if $(CONFIG_PACKAGE_kmod-ar9170),m) \
	CONFIG_AR9170_LEDS=$(CONFIG_LEDS_TRIGGERS) \
	CONFIG_IWM= \
	CONFIG_ATH9K_HTC= \
	CONFIG_MAC80211_RC_MINSTREL_HT=y \
	MADWIFI= \
	OLD_IWL= \
	KLIB_BUILD="$(LINUX_DIR)" \
	MODPROBE=: \
	KLIB=$(TARGET_MODULES_DIR) \
	KERNEL_SUBLEVEL=$(lastword $(subst ., ,$(KERNEL_PATCHVER))) \
	KBUILD_LDFLAGS_MODULE_PREREQ=

ifeq ($(CONFIG_PACKAGE_kmod-libertas-sd)$(CONFIG_PACKAGE_kmod-libertas-usb)$(CONFIG_PACKAGE_kmod-rt2x00-lib),)
  MAKE_OPTS += CONFIG_COMPAT_KFIFO=
endif

define Build/Prepare
	rm -rf $(PKG_BUILD_DIR)
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src_2/* $(PKG_BUILD_DIR)/
#	$(PKG_UNPACK)
#	$(Build/Patch)
#	rm -rf $(PKG_BUILD_DIR)/include/linux/ssb
#	rm -f $(PKG_BUILD_DIR)/include/net/ieee80211.h
#	rm $(PKG_BUILD_DIR)/include/linux/eeprom_93cx6.h
endef

ifneq ($(CONFIG_PACKAGE_kmod-cfg80211),)
 define Build/Compile/kmod
	rm -rf $(PKG_BUILD_DIR)/modules
	$(MAKE) $(PKG_JOBS) -C "$(PKG_BUILD_DIR)" $(MAKE_OPTS) all
 endef
endif

define Build/Compile
	$(call Build/Compile/kmod)
endef

define Build/InstallDev
	mkdir -p \
		$(1)/usr/include/mac80211 \
		$(1)/usr/include/mac80211/ath \
		$(1)/usr/include/net/mac80211
	$(CP) $(PKG_BUILD_DIR)/net/mac80211/*.h $(PKG_BUILD_DIR)/include/* $(1)/usr/include/mac80211/
	$(CP) $(PKG_BUILD_DIR)/net/mac80211/rate.h $(1)/usr/include/net/mac80211/
	$(CP) $(PKG_BUILD_DIR)/drivers/net/wireless/ath/*.h $(1)/usr/include/mac80211/ath/
endef

define KernelPackage/cfg80211/install
	$(INSTALL_DIR) $(1)/lib/wifi
	$(CP) ./files/etc/* $(1)/etc
	$(INSTALL_DATA) ./files/lib/wifi/mac80211.sh $(1)/lib/wifi
	mkdir -p $(1)/lib/mdf
	$(INSTALL_DATA) ./files/lib/wifi/mdfcfg $(1)/lib/mdf
	$(INSTALL_DATA) ./files/lib/wifi/cardtype $(1)/lib/mdf
	$(INSTALL_DATA) ./files/lib/wifi/channellist.24g $(1)/lib/mdf
	$(INSTALL_DATA) ./files/lib/wifi/channellist.49g $(1)/lib/mdf
	$(INSTALL_DATA) ./files/lib/wifi/channellist.58g $(1)/lib/mdf
	$(INSTALL_DATA) ./files/lib/wifi/xchannellist.24g $(1)/lib/mdf
	$(INSTALL_DATA) ./files/lib/wifi/xchannellist.49g $(1)/lib/mdf
	$(INSTALL_DATA) ./files/lib/wifi/xchannellist.58g $(1)/lib/mdf
endef

$(eval $(call KernelPackage,ath5k))
$(eval $(call KernelPackage,cfg80211))
$(eval $(call KernelPackage,mac80211))
$(eval $(call KernelPackage,mac80211-hwsim))
$(eval $(call KernelPackage,ath9k))
$(eval $(call KernelPackage,ath))
$(eval $(call KernelPackage,adf4350))

