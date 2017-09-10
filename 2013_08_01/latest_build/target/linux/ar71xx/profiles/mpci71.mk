#
# Copyright (C) 2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/MPCI71
	NAME:=Microhard microPCI 71xx series
	PACKAGES:=kmod-usb-core kmod-usb-ohci kmod-usb2
endef

define Profile/MPCI71/Description
	Package set optimized for the Microhard microPCI 71xx  boards.
endef

$(eval $(call Profile,MPCI71))
