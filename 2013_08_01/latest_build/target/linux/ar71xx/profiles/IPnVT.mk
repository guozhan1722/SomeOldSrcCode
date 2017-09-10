#
# Copyright (C) 2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/IPnVT
	NAME:=IPnVT
	PACKAGES:=kmod-usb-core kmod-usb-ohci kmod-usb2
endef

define Profile/IPnVT/Description
	Package set optimized for the Microhard microPCI IPnVT  boards.
endef

$(eval $(call Profile,IPnVT))
