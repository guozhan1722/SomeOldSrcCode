#
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libcli
PKG_VERSION:=1.9.4
PKG_RELEASE:=1

#PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
#PKG_SOURCE_URL:=http://libcli.googlecode.com/files
#PKG_MD5SUM:=b917617d21b90db214971efe64a33416

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define Package/libcli
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=A Cisco-like CLI library
  FILES:=$(PKG_BUILD_DIR)/libcli.$(PKG_VERSION)
  DEPENDS:=+libuci +libpthread 
 #  URL:=http://code.google.com/p/libcli/
endef

define Package/libcli/description
	Libcli provides a shared library for including a Cisco-like
	command-line interface into other software. It's a telnet
	interface which supports command-line editing, history,
	authentication and callbacks for a user-definable function tree.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

TARGET_CFLAGS += $(FPIC) -I$(PKG_BUILD_DIR)/../libmserial/include -I$(LINUX_DIR)/../mpci-bsp  -I$(PKG_BUILD_DIR)/../libmserial/include/phones -I$(STAGING_DIR)/include  -I$(STAGING_DIR)/usr/include 
MAKE_FLAGS += \
	DEBUG="" \
	OPTIM="$(TARGET_CFLAGS)" \
	CFLAGS="$(TARGET_CFLAGS)" \
	PREFIX="/usr" \
	DESTDIR="$(PKG_INSTALL_DIR)" \
	all install

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/libcli.h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libcli.so* $(1)/usr/lib/
endef

define Package/libcli/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libcli.so.* $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/clitest $(1)/bin/

	$(INSTALL_DIR) $(1)/etc/m_cli
	$(INSTALL_BIN) ./src/common_at.sh $(1)/etc/m_cli/common_at.sh
endef

$(eval $(call BuildPackage,libcli))
