
include $(TOPDIR)/rules.mk

PKG_NAME:=stunnel
PKG_VERSION:=4.29
PKG_RELEASE:=4

PKG_MD5SUM:=14dc3f8412947f0548975cbce74d6863
PKG_SOURCE_URL:=http://www.stunnel.org/download/stunnel/src/
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz

PKG_FIXUP:=libtool
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/stunnel
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+libopenssl +libwrap
  TITLE:=SSL TCP Wrapper
  URL:=http://www.stunnel.org/
endef

define Package/stunnel/description
	Stunnel is a program that allows you to encrypt arbitrary TCP
	connections inside SSL (Secure Sockets Layer) available on both Unix
	and Windows. Stunnel can allow you to secure non-SSL aware daemons and
	protocols (like POP, IMAP, LDAP, etc) by having Stunnel provide the
	encryption, requiring no changes to the daemon's code.
endef

define Package/stunnel/conffiles
/etc/stunnel/stunnel.conf
endef

define Build/Configure
	$(call Build/Configure/Default, \
		--with-random=/dev/urandom \
		--with-threads=fork \
		--with-ssl=$(STAGING_DIR)/usr \
	)
endef

define Build/Compile
	mkdir -p $(PKG_INSTALL_DIR)/etc/stunnel
	echo '#dummy' > $(PKG_INSTALL_DIR)/etc/stunnel/stunnel.pem
	$(call Build/Compile/Default)
endef

define Package/stunnel/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/stunnel $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/lib/stunnel
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/stunnel/libstunnel.so $(1)/usr/lib/stunnel/
	$(INSTALL_DIR) $(1)/etc/stunnel
	$(INSTALL_BIN) ./files/stunnel.cert $(1)/etc/stunnel/
	$(INSTALL_BIN) ./files/stunnel.conf $(1)/etc/stunnel/
	$(INSTALL_BIN) ./files/stunnel.csr $(1)/etc/stunnel/
	$(INSTALL_BIN) ./files/stunnel.key $(1)/etc/stunnel/
	$(INSTALL_BIN) ./files/stunnel.pem $(1)/etc/stunnel/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/stunnel.init $(1)/etc/init.d/https
endef

$(eval $(call BuildPackage,stunnel))
