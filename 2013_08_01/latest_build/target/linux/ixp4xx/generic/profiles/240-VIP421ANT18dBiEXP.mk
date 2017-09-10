#
# Copyright (C) 2010 Microhard Systems Inc
#
#

define Profile/VIP421ANT18dBiEXP
  NAME:=VIP421ANT18dBiEXP
  PACKAGES:=kmod-madwifi
endef

define Profile/VIP421ANT18dBiEXP/Description
	Microhard IXP4xx based standard board with madwifi driver Profile
endef
$(eval $(call Profile,VIP421ANT18dBiEXP))

