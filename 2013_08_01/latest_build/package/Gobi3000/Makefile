# 
# Copyright (C) 2009 Microhard Systems 
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id: Makefile 12225 2008-08-06 22:10:29Z bobs $

include $(TOPDIR)/rules.mk

PKG_NAME:=Gobi3000
PKG_RELEASE:=1

ifeq ($(IPX20_TAG), )
IPX20_TAG:=nano_cell
endif

include $(INCLUDE_DIR)/package.mk

define Package/Gobi3000
  SECTION:=microhard
  CATEGORY:=Microhard Packages
  DEPENDS:=+kmod-GobiNet +kmod-GobiSerial
  TITLE:=Microhard Gobi3000 utility
endef

define Package/Gobi3000/description
 This package contains the microhard home grown Gobi3000 utility.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	cvs update ./src; \
	$(CP) ./src/* $(PKG_BUILD_DIR)
	find $(PKG_BUILD_DIR) -name CVS | xargs rm -rf
endef

MH_DEF= $(if $(CONFIG_TARGET_ar71xx),-DMH_BIG_ENDIAN) 
TARGET_CFLAGS += $(FPIC) -D__GNUC__ $(MH_DEF)  


#TARGET_LDFLAGS += -lUICommon

MAKE_OPTS:= \
	ARCH="$(LINUX_KARCH)" \
	CROSS_COMPILE="$(TARGET_CROSS)" \
	SUBDIRS="$(PKG_BUILD_DIR)" \
	EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
	$(EXTRA_KCONFIG)


define Build/InstallDev

endef

define Package/Gobi3000/install
	$(INSTALL_DIR) $(1)/bin
	mkdir -p $(1)/bin
	mkdir -p $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/GobiConnectionMgmt/*.so* $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/GobiImageMgmt/*.so* $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/ConnManager/gobi-data $(1)/bin/
	$(CP) $(PKG_BUILD_DIR)/ConnManager/gobi-gps $(1)/bin/
endef

$(eval $(call BuildPackage,Gobi3000))
