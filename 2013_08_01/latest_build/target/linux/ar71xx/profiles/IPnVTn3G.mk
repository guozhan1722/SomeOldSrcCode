#
# Copyright (C) 2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/IPnVTn3G
	NAME:=IPnVTn3G
	PACKAGES:=kmod-usb-core kmod-usb-ohci kmod-usb2
endef

define Profile/IPnVTn3G/Description
	Package set optimized for the Microhard microPCI IPnVTn3G  boards.
endef

$(eval $(call Profile,IPnVTn3G))
