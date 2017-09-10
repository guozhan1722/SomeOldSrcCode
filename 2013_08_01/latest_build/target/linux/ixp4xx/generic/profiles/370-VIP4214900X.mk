#
# Copyright (C) 2010 Microhard Systems Inc
#
#

define Profile/VIP4214900X
  NAME:=VIP4214900X
  PACKAGES:=kmod-madwifi
endef

define Profile/VIP4214900X/Description
	Microhard IXP4xx based standard board with madwifi driver Profile
endef
$(eval $(call Profile,VIP4214900X))

