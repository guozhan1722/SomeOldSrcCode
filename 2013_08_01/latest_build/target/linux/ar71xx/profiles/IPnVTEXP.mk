#
# Copyright (C) 2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/IPnVTEXP
	NAME:=IPnVTEXP
	PACKAGES:=kmod-usb-core kmod-usb-ohci kmod-usb2
endef

define Profile/IPnVTEXP/Description
	Package set optimized for the Microhard microPCI IPnVTEXP  boards.
endef

$(eval $(call Profile,IPnVTEXP))
