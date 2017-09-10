#
# Copyright (C) 2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/IPnDDL
	NAME:=IPnDDL
	PACKAGES:=kmod-usb-core kmod-usb-ohci kmod-usb2
endef

define Profile/IPnDDL/Description
	Package set optimized for the Microhard microPCI IPnDDL  boards.
endef

$(eval $(call Profile,IPnDDL))
