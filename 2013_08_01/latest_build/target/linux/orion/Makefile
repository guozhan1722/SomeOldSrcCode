#
# Copyright (C) 2008-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

ARCH:=arm
BOARD:=orion
BOARDNAME:=Marvell Orion
FEATURES:=squashfs
SUBTARGETS=generic harddisk
CFLAGS=-Os -pipe -march=armv5t -mtune=xscale -funit-at-a-time

LINUX_VERSION:=2.6.32.9

include $(INCLUDE_DIR)/target.mk

KERNELNAME:="uImage"

DEFAULT_PACKAGES += kmod-ath9k wpad-mini

$(eval $(call BuildTarget))
