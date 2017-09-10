#
#

include $(TOPDIR)/rules.mk

PKG_NAME:=httpd-ssl
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/httpd-ssl
	SECTION:=net
	CATEGORY:=Network
	TITLE:=A small web server based on mini_httpd
	DEPENDS:= +libmatrixssl-nothread
endef

define Package/httpd-ssl/description
  A small web server based on mini_httpd
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef


define Build/Configure
endef

define Build/Compile
	-$(MAKE) -C $(PKG_BUILD_DIR) clean
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) \
		OFLAGS="$(TARGET_CFLAGS)" \
		SSL_DEFS="-DUSE_SSL -DHAVE_MATRIXSSL" \
		SSL_INC="$(TARGET_CPPFLAGS)" \
		SSL_LIBS="$(TARGET_LDFLAGS) -lmatrixssl" \
		SSL_OBJS="matrixssl_helper.o" \
		all


endef

define Package/httpd-ssl/install

	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DATA) ./files/httpd.pem $(1)/etc/httpd.pem

	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/httpd.conf $(1)/etc/config/httpd
	

	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/httpd.init $(1)/etc/init.d/httpd
	
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/httpd $(1)/usr/sbin/httpd

endef

define Package/httpd-ssl/prerm
	$${IPKG_INSTROOT}/etc/init.d/httpd stop 2>/dev/null
	$${IPKG_INSTROOT}/etc/init.d/httpd disable 2>/dev/null
	ls >/dev/null 2>&1
endef

$(eval $(call BuildPackage,httpd-ssl))
