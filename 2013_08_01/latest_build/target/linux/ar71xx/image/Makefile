#
# Copyright (C) 2008-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

VERSION:=$(shell cat $(TOPDIR)/target/linux/ar71xx/base-files/etc/version | grep SOFTWARE | sed 's/SOFTWARE=/\-/g' | sed 's/\./\_/g')
REVISION:=$(shell cat $(TOPDIR)/target/linux/ar71xx/base-files/etc/version | grep BUILD | sed 's/BUILD=/\-r/g')
REVNUMLOW:=$(shell cat $(TOPDIR)/target/linux/ar71xx/base-files/etc/version | grep BUILD | sed 's/BUILD=//g'|sed 's/-//g')
REVNUMHIGH:=$(shell cat $(TOPDIR)/target/linux/ar71xx/base-files/etc/version | grep SOFTWARE | sed 's/SOFTWARE=//g'|sed 's/[v,\.]//g')
#PRONAME:=$(shell cat $(TOPDIR)/target/linux/ar71xx/base-files/etc/version | grep PRODUCTNAME | sed 's/PRODUCTNAME=//g')
fs_64k:=64k
fs_256k:=256k

IMGNAME:=$(BIN_DIR)/$(BOARD)

define imgname
$(BIN_DIR)/$(PROFILE)$(VERSION)$(REVISION)
endef

VMLINUX:=$(IMGNAME)-vmlinux
UIMAGE:=$(IMGNAME)-uImage

define Build/Clean
	$(MAKE) -C u-boot clean
endef

define Build/Compile
	if [ "$(CONFIG_NANOPCI_UBOOT)" = "y" ]; then \
		$(MAKE) -C u-boot compile; \
	fi
endef

define CompressLzma
	$(STAGING_DIR_HOST)/bin/lzma e $(1) -lc1 -lp2 -pb2 $(2)
endef

define PatchKernelLzma
	cp $(KDIR)/vmlinux $(KDIR)/vmlinux-$(1)
	$(STAGING_DIR_HOST)/bin/patch-cmdline $(KDIR)/vmlinux-$(1) '$(strip $(2))'
	$(call CompressLzma,$(KDIR)/vmlinux-$(1),$(KDIR)/vmlinux-$(1).bin.lzma)
endef

define PatchKernelGzip
	cp $(KDIR)/vmlinux $(KDIR)/vmlinux-$(1)
	$(STAGING_DIR_HOST)/bin/patch-cmdline $(KDIR)/vmlinux-$(1) '$(strip $(2))'
	gzip -9 -c $(KDIR)/vmlinux-$(1) > $(KDIR)/vmlinux-$(1).bin.gz
endef

define MkImageLzma
	mkimage -A mips -O linux -T kernel -a 0x80060000 -C lzma \
		-e 0x80060000 -n 'MIPS OpenWrt Linux-$(LINUX_VERSION)' \
		-d $(1) $(2)
endef

define MkImageGzip
	mkimage -A mips -O linux -T kernel -a 0x80060000 -C gzip \
		-e 0x80060000 -n 'MIPS OpenWrt Linux-$(LINUX_VERSION)' \
		-d $(1) $(2)
endef

define Image/BuildKernel
	gzip -9 -c $(KDIR)/vmlinux > $(KDIR)/vmlinux.bin.gz
	$(call CompressLzma,$(KDIR)/vmlinux,$(KDIR)/vmlinux.bin.lzma)
	dd if=$(KDIR)/vmlinux.bin.lzma of=$(VMLINUX).lzma bs=262144 conv=sync
	dd if=$(KDIR)/vmlinux.bin.gz of=$(VMLINUX).gz bs=262144 conv=sync
	$(call MkImageGzip,$(KDIR)/vmlinux.bin.gz,$(UIMAGE)-gzip.bin)
	$(call MkImageLzma,$(KDIR)/vmlinux.bin.lzma,$(UIMAGE)-lzma.bin)
endef

define Image/Build/MyLoader
	-$(STAGING_DIR_HOST)/bin/mkmylofw -B $(2) -s $(3) \
		-p0x030000:0xe0000:al:0x80060000:kernel:$(KDIR)/vmlinux.bin.lzma \
		-p0x110000:0:::rootfs:$(KDIR)/root.$(1) \
	$(call imgname,$(1),$(2))-$(4)-factory.img
endef

define Image/Build/Microhard
	$(STAGING_DIR_HOST)/bin/mkmylofw -B $(2) -s $(3) \
		-p0x080000:0x200000:al:0x80060000:kernel:$(KDIR)/vmlinux-pb42.uImage \
		-p0x280000:0xb80000:::rootfs:$(KDIR)/root.$(1) \
	$(call imgname,$(1),$(2)).bin

	if [ "$(CONFIG_NANOPCI_UBOOT)" = "y" ]; then \
		$(STAGING_DIR_HOST)/bin/mkmylofw -B $(2) -s $(3) \
			-p0x0:0x040000:::u-boot:$(KDIR)/u-boot.bin \
			-p0x040000:0x040000:::params:$(KDIR)/params \
			-p0x080000:0x200000:al:0x80060000:kernel:$(KDIR)/vmlinux-pb42.uImage \
			-p0x280000:0xb80000:::rootfs:$(KDIR)/root.$(1) \
		$(call imgname,$(1),$(2))-factory.bin; \
	fi
endef

define Image/Build/MicrohardVIP4G
	$(STAGING_DIR_HOST)/bin/mkmylofw -B $(2) -s $(3) -V$(REVNUMHIGH):$(REVNUMLOW)  \
		-p0x080000:0x200000:al:0x80060000:kernel:$(KDIR)/vmlinux-VIP4G.uImage \
		-p0x280000:0xb80000:::rootfs:$(KDIR)/root.$(1) \
	$(call imgname,$(1),$(2)).bin
endef


define Image/Build/Template/64k/squashfs
	$(call Image/Build/$(1),squashfs,$(2),$(3),$(4),$(5),$(6),$(7))
endef

define Image/Build/Template/256k/squashfs
	$(call Image/Build/$(1),squashfs,$(2),$(3),$(4),$(5),$(6),$(7))
endef

define Image/Build/Profile/WP543
	$(call Image/Build/Template/$(fs_64k)/$(1),MyLoader,wp543,0x800000,8M)
endef

define Image/Build/Profile/MPCI71
	$(call PatchKernelGzip,pb42,board=PB42)
	mkimage -A mips -O linux -T kernel -a 0x80060000 -C gzip -e \
		0x80060000 \
		-n 'MIPS OpenWrt Linux-$(LINUX_VERSION)' \
		-d $(KDIR)/vmlinux-pb42.bin.gz \
		$(KDIR)/vmlinux-pb42.uImage; \
	$(call Image/Build/Template/$(fs_256k)/$(1),Microhard,MPCI71,0x1000000,16M)
endef

define Image/Build/Profile/IPnVT
	$(call PatchKernelGzip,pb42,board=PB42)
	mkimage -A mips -O linux -T kernel -a 0x80060000 -C gzip -e \
		0x80060000 \
		-n 'MIPS OpenWrt Linux-$(LINUX_VERSION)' \
		-d $(KDIR)/vmlinux-pb42.bin.gz \
		$(KDIR)/vmlinux-pb42.uImage; \
	$(call Image/Build/Template/$(fs_256k)/$(1),Microhard,IPnVT,0x1000000,16M)
endef

define Image/Build/Profile/IPnDDL
	$(call PatchKernelGzip,pb42,board=PB42)
	mkimage -A mips -O linux -T kernel -a 0x80060000 -C gzip -e \
		0x80060000 \
		-n 'MIPS OpenWrt Linux-$(LINUX_VERSION)' \
		-d $(KDIR)/vmlinux-pb42.bin.gz \
		$(KDIR)/vmlinux-pb42.uImage; \
	$(call Image/Build/Template/$(fs_256k)/$(1),Microhard,IPnDDL,0x1000000,16M)
endef

define Image/Build/Profile/IPnVTn3G
	$(call PatchKernelGzip,pb42,board=PB42)
	mkimage -A mips -O linux -T kernel -a 0x80060000 -C gzip -e \
		0x80060000 \
		-n 'MIPS OpenWrt Linux-$(LINUX_VERSION)' \
		-d $(KDIR)/vmlinux-pb42.bin.gz \
		$(KDIR)/vmlinux-pb42.uImage; \
	$(call Image/Build/Template/$(fs_256k)/$(1),Microhard,IPnVTn3G,0x1000000,16M)
endef

define Image/Build/Profile/IPnVTn3GEXP
	$(call PatchKernelGzip,pb42,board=PB42)
	mkimage -A mips -O linux -T kernel -a 0x80060000 -C gzip -e \
		0x80060000 \
		-n 'MIPS OpenWrt Linux-$(LINUX_VERSION)' \
		-d $(KDIR)/vmlinux-pb42.bin.gz \
		$(KDIR)/vmlinux-pb42.uImage; \
	$(call Image/Build/Template/$(fs_256k)/$(1),Microhard,IPnVTn3GEXP,0x1000000,16M)
endef

define Image/Build/Profile/IPnVTEXP
	$(call PatchKernelGzip,pb42,board=PB42)
	mkimage -A mips -O linux -T kernel -a 0x80060000 -C gzip -e \
		0x80060000 \
		-n 'MIPS OpenWrt Linux-$(LINUX_VERSION)' \
		-d $(KDIR)/vmlinux-pb42.bin.gz \
		$(KDIR)/vmlinux-pb42.uImage; \
	$(call Image/Build/Template/$(fs_256k)/$(1),Microhard,IPnVTEXP,0x1000000,16M)
endef

define Image/Build/Profile/VIP4G
	$(call PatchKernelGzip,VIP4G,board=VIP4G)
	mkimage -A mips -O linux -T kernel -a 0x80060000 -C gzip -e \
		0x80060000 \
		-n 'MIPS OpenWrt Linux-$(LINUX_VERSION)' \
		-d $(KDIR)/vmlinux-VIP4G.bin.gz \
		$(KDIR)/vmlinux-VIP4G.uImage; \
	$(call Image/Build/Template/$(fs_256k)/$(1),MicrohardVIP4G,VIP4G,0x1000000,16M)
endef

define Image/Build/Profile/VIP4GEXP
	$(call PatchKernelGzip,VIP4G,board=VIP4G)
	mkimage -A mips -O linux -T kernel -a 0x80060000 -C gzip -e \
		0x80060000 \
		-n 'MIPS OpenWrt Linux-$(LINUX_VERSION)' \
		-d $(KDIR)/vmlinux-VIP4G.bin.gz \
		$(KDIR)/vmlinux-VIP4G.uImage; \
	$(call Image/Build/Template/$(fs_256k)/$(1),MicrohardVIP4G,VIP4GEXP,0x1000000,16M)
endef

define Image/Build/squashfs
	$(call prepare_generic_squashfs,$(KDIR)/root.squashfs)
	dd if=$(KDIR)/root.squashfs of=$(KDIR)/root.squashfs.tmp0 bs=128k conv=sync
	$(call add_jffs2_mark,$(KDIR)/root.squashfs.tmp0)
	dd if=$(KDIR)/root.squashfs.tmp0 of=$(KDIR)/root.squashfs bs=256k conv=sync
	rm -f $(KDIR)/root.squashfs.tmp0
endef

define Image/Build
	$(call Image/Build/$(1))
	dd if=$(KDIR)/root.$(1) of=$(IMGNAME)-root.$(1) bs=128k conv=sync

	$(call Image/Build/Profile/$(PROFILE),$(1))
endef

$(eval $(call BuildImage))
