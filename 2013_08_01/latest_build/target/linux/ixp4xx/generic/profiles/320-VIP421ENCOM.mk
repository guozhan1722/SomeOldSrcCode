#
# Copyright (C) 2010 Microhard Systems Inc
#
#

define Profile/VIP421ENCOM
  NAME:=VIP421ENCOM
  PACKAGES:=kmod-madwifi
endef

define Profile/VIP421ENCOM/Description
	Microhard IXP4xx based standard board with madwifi driver Profile
endef
$(eval $(call Profile,VIP421ENCOM))

