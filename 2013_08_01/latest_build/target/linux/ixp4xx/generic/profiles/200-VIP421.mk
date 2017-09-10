#
# Copyright (C) 2010 Microhard Systems Inc
#
#

define Profile/VIP421
  NAME:=VIP421
  PACKAGES:=kmod-madwifi
endef

define Profile/VIP421/Description
	Microhard IXP4xx based standard board with madwifi driver Profile
endef
$(eval $(call Profile,VIP421))

