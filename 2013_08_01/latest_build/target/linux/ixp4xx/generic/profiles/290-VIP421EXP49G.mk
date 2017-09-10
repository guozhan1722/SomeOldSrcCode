#
# Copyright (C) 2010 Microhard Systems Inc
#
#

define Profile/VIP421EXP49G
  NAME:=VIP421EXP49G
  PACKAGES:=kmod-madwifi
endef

define Profile/VIP421EXP49G/Description
	Microhard IXP4xx based standard board with madwifi driver Profile
endef
$(eval $(call Profile,VIP421EXP49G))

