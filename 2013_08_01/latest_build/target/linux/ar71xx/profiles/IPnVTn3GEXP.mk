#
# Copyright (C) 2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/IPnVTn3GEXP
	NAME:=IPnVTn3GEXP
	PACKAGES:=kmod-usb-core kmod-usb-ohci kmod-usb2
endef

define Profile/IPnVTn3GEXP/Description
	Package set optimized for the Microhard microPCI IPnVTn3GEXP  boards.
endef

$(eval $(call Profile,IPnVTn3GEXP))
