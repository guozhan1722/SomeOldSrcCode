# 
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

#$(ARCH)

ifdef OPENWRTVERSION

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=at-legacy-art
PKG_VERSION:=5.4b7
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define KernelPackage/at-legacy-art
  SUBMENU:=Wireless Drivers
  TITLE:=Atheros Radio Test for Linux for 802.11a/b/g (legacy)
  DEPENDS:= +libpthread
  URL:=http://www.cardaccess-inc.com/
  VERSION:=$(LINUX_VERSION)+$(PKG_VERSION)-$(BOARD)-$(PKG_RELEASE)
  FILES:=$(PKG_BUILD_DIR)/art_module/modules/art_legacy.$(LINUX_KMOD_SUFFIX)
endef

define KernelPackage/at-legacy-art/description
 Atheros Radio Test for Linux (legacy).  Builds ART for an PB44-ish platform.
endef

define Build/Configure
endef

define Build/Prepare
	mkdir -p "$(PKG_BUILD_DIR)"
	$(CP) ./* "$(PKG_BUILD_DIR)"
	mkdir -p "$(PKG_BUILD_DIR)"/art_host/bin
	mkdir -p "$(PKG_BUILD_DIR)"/art_host/lib
endef

#$(ARCH)

#--------------------------------------------------------
ifneq ($(CONFIG_TARGET_ar71xx),)
define ART_MAKEOPTS
	LINUX_KARCH=mips \
	LINUX_SUBARCH= \
	CROSS_COMPILE=$(TARGET_CROSS) \
	LINUX_DIR=$(LINUX_DIR) 
endef
else
define ART_MAKEOPTS
	LINUX_KARCH=arm \
	LINUX_SUBARCH= \
	CROSS_COMPILE=$(TARGET_CROSS) \
	LINUX_DIR=$(LINUX_DIR) 
endef
endif
#	AR6000_COMM=$(CONFIG_AT_LEGACY_ART_AR6000_COMM)  //daniel: are we going to use this
#----------------------------------------------------------

define Build/Compile
	$(MAKE) $(MAKEPARALLEL) -C "$(PKG_BUILD_DIR)"/art/src/dk/mdk -f makefile.soc.linux $(ART_MAKEOPTS)
	$(MAKE) -C "$(PKG_BUILD_DIR)"/art_host $(ART_MAKEOPTS)
	$(MAKE) $(MAKEPARALLEL) -C "$(PKG_BUILD_DIR)"/art_module -f makefile.soc.linux.mips hydra MB62=1 $(ART_MAKEOPTS)
endef

define KernelPackage/at-legacy-art/install
	$(INSTALL_DIR) $(1)/lib	
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/local/art_host
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/art/src/dk/mdk/devlib/libdevlib.so $(1)/lib
	$(INSTALL_BIN) \
		$(PKG_BUILD_DIR)/./art/src/dk/mdk/client/soc_linux/obj/mdk_client-legacy \
		$(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/scripts/art-legacy $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/scripts/art-linux $(1)/usr/bin
	rm -r -f $(PKG_BUILD_DIR)/art_host/config/CVS
	cp $(PKG_BUILD_DIR)/art_host/config/* $(1)/usr/local/art_host
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/art_host/bin/art_host $(1)/usr/local/art_host
endef

define KernelPackage/at-legacy-art/config
	source $(SOURCE)/Config.in
endef

$(eval $(call KernelPackage,at-legacy-art))

else

### Non-OpenWRT
-include .makefile
all clean: do_client do_module
ifneq ($(POSTBUILD),)
	$(POSTBUILD)
endif

.PHONY: FORCE

do_client: FORCE
	$(MAKE) -C art/src/dk/mdk -f makefile.soc.linux $(MAKECMDGOALS)

ifeq ($(MAKECMDGOALS),)
module_target=hydra
else
module_target=$(MAKECMDGOALS)
endif

do_module: FORCE
	$(MAKE) -C art_module -f makefile.soc.linux.mips $(module_target) MB62=1

endif
