# 
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=gd
PKG_VERSION:=2.0.35
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=http://www.libgd.org/releases
PKG_MD5SUM:=6c6c3dbb7bf079e0bb5fbbfd3bb8a71c

PKG_FIXUP:=libtool
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libgd
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+libjpeg +libpng
  TITLE:=The GD graphics library
  URL:=http://www.boutell.com/gd/
endef

define Package/libgd/description
  GD is an open source code library for the dynamic creation of images by
  programmers. GD creates PNG, JPEG and GIF images, among other formats.
endef

CONFIGURE_ARGS += \
	--enable-shared \
	--enable-static \
	--disable-rpath \
	--without-x \
	--without-freetype \
	--with-jpeg=$(STAGING_DIR)/usr \
	--with-png=$(STAGING_DIR)/usr \
	--without-xpm \
	--without-iconv

CONFIGURE_VARS += \
	LIBPNG12_CONFIG="$(STAGING_DIR)/host/bin/libpng12-config"

TARGET_CFLAGS += $(FPIC)

define Build/Configure
	( cd $(PKG_BUILD_DIR); aclocal; libtoolize; autoreconf; );
	$(call Build/Configure/Default)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/gdlib-config $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/entities.h $(1)/usr/include/
	$(CP) $(PKG_INSTALL_DIR)/usr/include/gd{,_io,cache,fontg,fontl,fontmb,fonts,fontt,fx}.h \
		$(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgd.{a,so*} $(1)/usr/lib/
	$(SED) 's,-I$$$${prefix}/include,,g' $(1)/usr/bin/gdlib-config
endef

define Package/libgd/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgd.so.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libgd))
