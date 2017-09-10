#
# Copyright (C) 2010 Microhard Systems Inc
#
#

define Profile/VIP421ANTEXP
  NAME:=VIP421ANTEXP
  PACKAGES:=kmod-madwifi
endef

define Profile/VIP421ANTEXP/Description
	Microhard IXP4xx based standard board with madwifi driver Profile
endef
$(eval $(call Profile,VIP421ANTEXP))

