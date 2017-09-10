#
# Copyright (C) 2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/VIP4GEXP
	NAME:=VIP4GEXP
	PACKAGES:=kmod-usb-core kmod-usb-ohci kmod-usb2
endef

define Profile/VIP4GEXP/Description
	Package set optimized for the Microhard VIP4GEXP  boards.
endef

$(eval $(call Profile,VIP4GEXP))
