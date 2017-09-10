#
# Copyright (C) 2010 Microhard Systems Inc
#
#

define Profile/VIP421EXPBR
  NAME:=VIP421EXPBR
  PACKAGES:=kmod-madwifi
endef

define Profile/VIP421EXPBR/Description
	Microhard IXP4xx based standard board with madwifi driver Profile
endef
$(eval $(call Profile,VIP421EXPBR))

