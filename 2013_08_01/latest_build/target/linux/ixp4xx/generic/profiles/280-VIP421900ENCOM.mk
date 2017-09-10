#
# Copyright (C) 2010 Microhard Systems Inc
#
#

define Profile/VIP421900ENCOM
  NAME:=VIP421900ENCOM
  PACKAGES:=kmod-madwifi
endef

define Profile/VIP421900ENCOM/Description
	Microhard IXP4xx based standard board with madwifi driver Profile
endef
$(eval $(call Profile,VIP421900ENCOM))

