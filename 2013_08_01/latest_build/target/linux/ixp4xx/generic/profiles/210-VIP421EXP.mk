#
# Copyright (C) 2010 Microhard Systems Inc
#
#

define Profile/VIP421EXP
  NAME:=VIP421EXP
  PACKAGES:=kmod-madwifi
endef

define Profile/VIP421EXP/Description
	Microhard IXP4xx based standard board with madwifi driver Profile
endef
$(eval $(call Profile,VIP421EXP))

