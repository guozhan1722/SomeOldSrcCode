# 
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

VERSION:=$(shell cat $(TOPDIR)/target/linux/ixp4xx/base-files/etc/version | grep SOFTWARE | sed 's/SOFTWARE=/\-/g' | sed 's/\./\_/g')
REVISION:=$(shell cat $(TOPDIR)/target/linux/ixp4xx/base-files/etc/version | grep BUILD | sed 's/BUILD=/\-r/g')

ifdef CONFIG_PACKAGE_apex
  define Image/Build/Linksys
	BIN_DIR=$(BIN_DIR) $(TOPDIR)/scripts/slugimage.pl \
		-L $(BIN_DIR)/apex/apex-$(2)-armeb.bin \
		-k $(BIN_DIR)/openwrt-$(2)-zImage \
		-r rootfs:$(BIN_DIR)/openwrt-$(BOARD)-$(1).img \
		-p -o $(BIN_DIR)/openwrt-$(2)-$(1).bin
	BIN_DIR=$(BIN_DIR) $(TOPDIR)/scripts/slugimage.pl \
		-F -L $(BIN_DIR)/apex/apex-$(2)-16mb-armeb.bin \
		-k $(BIN_DIR)/openwrt-$(2)-zImage \
		-r rootfs:$(BIN_DIR)/openwrt-$(BOARD)-$(1).img \
		-p -o $(BIN_DIR)/openwrt-$(2)-$(1)-16mb.bin
  endef
endif

define Image/Build/Freecom
	$(INSTALL_DIR) $(TARGET_DIR)/boot
	# TODO: Add special CMDLINE shim for webupgrade image here
	$(CP) $(BIN_DIR)/openwrt-$(2)-zImage $(TARGET_DIR)/zImage
	$(TAR) cfj $(BIN_DIR)/openwrt-$(2)-$(1).img --numeric-owner --owner=0 --group=0 -C $(TARGET_DIR)/ .
	$(STAGING_DIR_HOST)/bin/encode_crc $(BIN_DIR)/openwrt-$(2)-$(1).img $(BIN_DIR)/openwrt-$(2)-$(1)-webupgrade.img
	rm -f $(TARGET_DIR)/zImage
endef

define Image/Prepare
	cp $(LINUX_DIR)/arch/arm/boot/zImage $(KDIR)/zImage
endef

define Image/BuildKernel
	cp $(KDIR)/zImage $(BIN_DIR)/openwrt-$(BOARD)-zImage
	BIN_DIR=$(BIN_DIR) $(TOPDIR)/scripts/arm-magic.sh
	rm -f $(BIN_DIR)/openwrt-$(BOARD)-zImage
endef


define Image/BuildFirmware/squashfs	
	$(STAGING_DIR_HOST)/bin/mkmylofw -B $(PROFILE) -r0 -V1:0 -b0x80000:0x200000:h:$(BIN_DIR)/openwrt-vip-zImage \
		-b0x280000:0xa00000::$(KDIR)/root.$(1) $(BIN_DIR)/$(PROFILE)$(VERSION)$(REVISION).bin

	$(STAGING_DIR_HOST)/bin/mkmylofw -B $(PROFILE) -r0 -V1:0 -b0x80000:0x200000:h:$(BIN_DIR)/openwrt-avila-zImage \
                -b0x280000:0xa00000::$(KDIR)/root.$(1) $(BIN_DIR)/$(PROFILE)$(VERSION)$(REVISION)-avila.bin

	if [ "$(CONFIG_NANOCPU_PACKAGE)" = "y" ]; then \
		(cd $(BIN_DIR)/packages/arm; tar czf $(BIN_DIR)/package.tar.gz *.ipk); \
		$(STAGING_DIR_HOST)/bin/mkmylofw -B $(PROFILE) -r0 -V1:0 -b0x0:0x0::$(BIN_DIR)/package.tar.gz $(BIN_DIR)/$(PROFILE)$(VERSION)$(REVISION).pkg; \
		rm $(BIN_DIR)/package.tar.gz; \
	fi
endef


define Image/Build
	$(call Image/Build/$(1),$(1))
endef

define Image/Build/jffs2-64k
	dd if=$(KDIR)/root.$(1) of=$(BIN_DIR)/openwrt-$(BOARD)-$(1).img bs=65536 conv=sync
endef

define Image/Build/jffs2-128k
	dd if=$(KDIR)/root.$(1) of=$(BIN_DIR)/openwrt-$(BOARD)-$(1).img bs=131072 conv=sync
endef

define Image/Build/squashfs
	$(call prepare_generic_squashfs,$(KDIR)/root.squashfs)
	dd if=$(KDIR)/root.$(1) of=$(BIN_DIR)/openwrt-$(BOARD)-$(1).img bs=131072 conv=sync
	$(call Image/BuildFirmware/$(1),$(1))
endef

$(eval $(call BuildImage))
