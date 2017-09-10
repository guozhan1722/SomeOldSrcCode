#
# Copyright (C) 2010 Microhard Systems Inc
#
#

define Profile/VIP421ANT
  NAME:=VIP421ANT
  PACKAGES:=kmod-madwifi
endef

define Profile/VIP421ANT/Description
	Microhard IXP4xx based standard board with madwifi driver Profile
endef
$(eval $(call Profile,VIP421ANT))

